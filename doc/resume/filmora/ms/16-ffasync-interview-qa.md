16-ffasync-interview-qa.md

# FFAsync 异步框架面试问答

---

## 一、架构设计类

### Q1：为什么不直接用 `QtConcurrent` 或 `QThread`，而要自研 FFAsync 框架？

**回答：**

`QtConcurrent` 的核心缺陷有三个：

1. **无任务优先级**：`QThreadPool` 只有一个全局队列，缩略图加载和 AI 推理抢同一批线程，高优先级的 UI 响应任务无法保证抢先执行。
2. **回调线程不保证**：`QFuture::then()`（Qt 6 起才有）在 Qt 5.15 不存在；`QFutureWatcher::finished` 信号只能在特定线程连接，业务代码必须手动管理线程切换。
3. **缺乏串行队列抽象**：AI 算法缓存要求同一 Clip 的多次参数变更按序处理，`QtConcurrent` 没有串行 Queue 概念，需要业务层自己加锁实现，代码分散且容易出错。

FFAsync 针对性地解决了这三个问题，同时增加了优雅退出、LIFO 出队、kTryBest 并发限流等能力，成本不高但收益显著。

---

### Q2：FFAsync 的整体分层是怎样的？各层职责是什么？

**回答：**

FFAsync 分四层：

1. **`FFAsync`（Facade 门面）**：对外暴露 5 个纯静态方法，屏蔽所有内部复杂性，调用方无需了解线程池细节。
2. **`FFThreadPool`（全局单例）**：管理三个线程组（Foreground / Common / Background），根据 `FFTasksFeature` 将任务分配到对应线程组，提供运行器创建接口。
3. **`FFThreadGroup + FFWorkerThread`（执行层）**：线程组维护优先队列（`FFTaskPagePriorityQueue`），Worker 循环取 TaskQueuePage 执行，空闲时休眠等待。
4. **`FFTaskScheduler`（调度控制器）**：全局唯一，控制任务投递权限（是否允许投递）、执行权限（退出时跳过还是继续），以及优雅退出的原子状态机。

---

### Q3：`postReplyableResultTask` 是如何保证 `.then()` 回调在调用线程执行的？

**回答：**

关键在三个步骤：

1. **调用时捕获线程**：`postReplyableResultTask` 在调用方线程执行时，将 `QPointer(QThread::currentThread())` 存入闭包。
2. **Worker 执行完成后检测线程差异**：任务在 Worker 线程跑完后，`TaskInvoker::invoke()` 检查 `QThread::currentThread() != currentThread`（捕获的调用线程）。
3. **线程不符则 defer**：若不在调用线程，通过 `qtpromise_defer(currentThread, resolve, value)` 将 `resolve` 投递到调用线程的 `QEventLoop`，在下一次事件循环处理时执行，保证 `.then()` 的 lambda 始终在调用线程运行。

这个过程对上层完全透明，调用方只需 `.then([](T result) { /* 在调用线程 */ })`，不需要任何手动线程切换。

---

## 二、核心机制类

### Q4：`TaskQueuePage` 是什么？为什么调度粒度是 Page 而不是单个任务？

**回答：**

`TaskQueuePage` 是调度的最小粒度单元，代表"一组需要原子执行的任务序列"。

**并行模式**：每个任务独立一个 Page，Page 之间互不依赖，可被不同 Worker 并发取走执行，实现真正的并发。

**串行队列模式**：多个任务共享同一个 Page，Worker 取到 Page 后必须按顺序执行 Page 内所有任务，其他 Worker 无法插入，天然实现串行保证。

Page 级别的调度比任务级别灵活：串行场景不需要额外锁，并行场景不需要额外通信，优先级排序在 Page 粒度进行，开销更低。

---

### Q5：`stateFlag` 原子位域的设计意图是什么？为什么不用两个独立变量？

**回答：**

`stateFlag` 是一个 `std::atomic<quint32>`，bit 0 是退出标志，bits 1–31 是阻塞任务计数。

**设计意图**：在退出检查和计数更新之间消除 TOCTOU（Time-Of-Check-Time-Of-Use）竞争。如果用两个独立原子变量，"检查退出标志"和"递增计数"之间存在窗口期，可能出现：线程 A 检查退出 = false，然后被挂起，主线程设置退出并等待计数为 0，然后 A 递增计数，导致主线程永久等待或提前退出。

单一原子变量配合 `fetch_add` / `fetch_sub` 的原子语义，退出标志和计数的检查与更新是原子的，彻底消除该窗口期，同时比两把锁的方案性能更高。

---

### Q6：三种任务运行器分别适用什么场景？

**回答：**

| 运行器 | 核心特性 | 典型场景 |
|---|---|---|
| `FFParallelTaskRunner`（默认） | 每个任务独立 Page，全并发 | 缩略图批量解码、独立的网络请求 |
| `FFQueueTaskRunner` | 共享 Page，严格串行 FIFO/LIFO | AI 算法缓存（保证参数按序处理）、顺序写入文件 |
| `FFThreadTaskRunner` | 继承 Queue，独享专属线程 | 音频波形生成（需要固定线程的音频解码上下文）、常驻后台服务 |

选择依据：**需要并发** → Parallel；**需要串行且可以共享 Worker** → Queue；**需要串行且任务需要线程局部状态（如解码器上下文）** → Thread。

---

### Q7：LIFO 出队模式在什么场景下有优势？实现原理是什么？

**回答：**

**场景**：用户在资源面板或时间线快速滚动，大量缩略图加载请求被投递。用户最终停在某个位置，只有最新可见区域的缩略图有意义，旧的请求已无需展示。LIFO 保证最新投递的任务优先执行，用户感知到的缩略图加载速度大幅提升，旧请求被自然延迟（而不是浪费资源执行后再丢弃）。

**实现原理**：`TaskQueuePageSortKey` 包含 `dequeueMethod + createTimestamp`，LIFO 模式下 `createTimestamp` 越大（越新）排序值越高，优先队列 `FFTaskPagePriorityQueue` 总是取出时间戳最大的 Page 执行，天然实现"最新优先"。

---

## 三、线程安全与生命周期类

### Q8：Worker 线程执行任务时，如何保证对调用线程分配的堆对象的安全访问和释放？

**回答：**

`postReplyableResultTask` 在调用线程堆上分配结果对象 `auto* result = new unique_ptr<T>(nullptr)`，Worker 线程写入值后需要释放它。直接在 Worker 线程 `delete` 会造成"跨线程释放"问题（调用线程的分配器可能有线程局部状态）。

框架用 `buildAutoRelease(result)` 将 `result` 包装为 `AutoReleaseWrapper`，其析构函数通过 `QMetaObject::invokeMethod(targetThread, [=]() { delete result; }, Qt::QueuedConnection)` 将 `delete` 调度回调用线程的事件循环执行。这确保对象在分配它的线程上释放，符合各平台分配器的预期。

---

### Q9：多个 Worker 线程同时竞争同一个 `TaskQueuePage` 时如何保证线程安全？

**回答：**

`FFTaskQueuePageImpl` 使用 `ScopedAtomic` 原子操作器串行化所有 Page 操作：

- `appendTask / takeTask / clear` 都必须先通过 `ScopedAtomic::acquire()` 获取原子操作权（本质上是基于 `std::atomic<bool>` 的自旋 + 让步）
- 同时只有一个操作者持有操作权，其他调用者自旋等待
- `m_isRuningTask` 原子标志作为快速路径：若 Page 正在被 Worker 执行中，`takeTask` 直接返回 nullptr，避免争抢

相比互斥锁，`ScopedAtomic` 在竞争极低（Page 操作耗时微秒级）时开销更小，且作用域 RAII 保证不会忘记释放。

---

## 四、性能与优化类

### Q10：`kTryBest` 任务的并发限流是如何实现的？为什么需要它？

**回答：**

`kTryBest` 对应"尽力而为"优先级，分配到 Background 线程组。框架设置 `maxTryBestThreadCount = 2`，即使 Background 组有更多空闲线程，同时运行的 `kTryBest` 任务也不超过 2 个。

**必要性**：低优先级任务（懒加载、埋点上报、非关键预加载）数量可能远大于高优先级任务。如果不限流，Background 组 N 个线程全被低优先级任务占满，新来的缩略图加载（`kNormal`）或 UI 响应任务（`kTimeCritical`）找不到空闲线程。限制 `kTryBest` 并发数，保留线程余量给更重要的任务，是"优先级"语义在资源层面的落地。

---

### Q11：线程池大小为什么设计为 `max(3, idealThreadCount - 1)`？

**回答：**

- **`idealThreadCount`** 返回逻辑 CPU 核数，代表系统能真正并行的最大线程数。
- **减 1**：主线程（UI 线程）也是 CPU 密集消费者，预留一个逻辑核给主线程，避免 Worker 线程和主线程互相竞争，减少主线程卡顿。
- **最小 3**：双核设备的 `idealThreadCount = 2`，减 1 后只剩 1 个 Worker，完全串行，严重影响并发能力。保底 3 个线程确保 Foreground / Common / Background 三个优先级各有线程可调度，不退化为单线程模型。
- **Worker 空闲超时回收**：低负载时自动回收闲置 Worker，避免线程常驻占用内存，与最大值上限共同构成弹性伸缩。

---

### Q12：如果调用 `postReplyableResultTask` 的线程没有 `QEventLoop`，会发生什么？

**回答：**

框架在 `postReplyableResultTask` 入口处调用 `checkPostTaskThreadIsValid()`，断言当前线程具有 `QEventLoop`（检查方式：`QThread::currentThread()->eventDispatcher() != nullptr`）。

**原因**：`.then()` 回调需要通过 `qtpromise_defer` 投递到调用线程的事件循环。如果线程没有事件循环（如一个 `std::thread` 或没有 `exec()` 的 `QThread`），`QMetaObject::invokeMethod` 的 `QueuedConnection` 永远不会触发，Promise 永远不会 resolve，调用方的 `.then()` 永远不执行，造成逻辑挂起且没有错误提示。

因此框架在开发期 assert fail，强制调用方要么在有事件循环的线程调用（主线程或 `exec()` 的子线程），要么改用 `postTask`（不需要回调线程保证的场景）。
