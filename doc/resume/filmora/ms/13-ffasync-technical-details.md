13-ffasync-technical-details.md

# FFAsync 异步框架技术实现细节

> 项目：Wondershare Filmora（C++17 + Qt 5.15）
> 模块：FFAsync 异步调度框架（`3rdparty/FilmoraFrameworkPlatform/Src/FFAsync`）

---

## 一、模块概述

FFAsync 是 Filmora 自研的统一异步任务调度框架，解决了 Qt 原生 `QtConcurrent / QFuture` 在以下场景的不足：无任务优先级、回调线程不保证、无串行队列抽象、缺乏优雅退出控制。框架以 **Facade 模式**对外暴露纯静态 `FFAsync` 类，内部由分层的线程池、线程组、任务队列页、调度器共同驱动，并深度集成 `QtPromise` 提供链式异步编程体验。

项目中缩略图加载（`FFThumbnailCache`）、音频波形生成（`FAudioThumbnailService`）、AI 任务处理、首页资源拉取等核心异步场景均基于本框架实现。

---

## 二、整体架构

```
┌──────────────────────────────────────────────────────────┐
│                    对外门面（Facade）                       │
│  FFAsync（纯静态 API）                                      │
│  postTask / postReplyableResultTask                       │
│  createParallelTaskRunner / createQueueTaskRunner         │
│  createThreadTaskRunner                                   │
└───────────────────────────┬──────────────────────────────┘
                            │ 委托
┌───────────────────────────▼──────────────────────────────┐
│                  FFThreadPool（全局单例）                   │
│  管理三个线程组（Foreground / Common / Background）          │
│  postTaskOnTaskQueuePage()                                │
│  getThreadGroupFromTaskFeature()                          │
└──────┬──────────────────────┬──────────────────────────┬─┘
       │                      │                          │
┌──────▼──────┐  ┌────────────▼──────────┐  ┌───────────▼───┐
│  Foreground │  │       Common          │  │  Background   │
│  ThreadGroup│  │     ThreadGroup       │  │  ThreadGroup  │
│  (高优先级)  │  │     (普通任务)         │  │  (后台任务)    │
└──────┬──────┘  └────────────┬──────────┘  └───────────┬───┘
       │                      │                          │
       └──────────────────────┼──────────────────────────┘
                              │
┌─────────────────────────────▼────────────────────────────┐
│              FFWorkerThread（工作线程）                     │
│  loop: getTaskPage → FFTaskScheduler::runTaskAndGetNext   │
│        → task.run() → 取下一个任务或休眠等待               │
└─────────────────────────────┬────────────────────────────┘
                              │
┌─────────────────────────────▼────────────────────────────┐
│              FFTaskScheduler（全局调度器）                  │
│  stateFlag（原子）：退出位 + 阻塞任务计数                    │
│  willPostTask() / runTaskAndGetNextTask()                 │
│  优雅退出：startExit → 等待阻塞任务归零                      │
└──────────────────────────────────────────────────────────┘
```

### 任务运行器三种模式

| 运行器 | 类 | 特性 |
|---|---|---|
| 并行运行器（默认） | `FFParallelTaskRunner` | 每个任务独立 TaskQueuePage，多任务并发执行 |
| 串行队列运行器 | `FFQueueTaskRunner` | 多任务共享同一 TaskQueuePage，FIFO/LIFO 串行执行 |
| 独立线程串行运行器 | `FFThreadTaskRunner` | 继承 QueueTaskRunner，任务在专属独立线程上串行运行 |

---

## 三、核心数据结构

### 3.1 FFTasksFeature（任务特征）

```cpp
struct FFTasksFeature {
    TaskPriority        taskPriority;       // kLowest/kTryBest/kNormal/kTimeCritical/kHighest
    TaskThreadPriority  threadPriority;     // kBackground/kNormal
    TaskBehaviorOnExit  behaviorOnExit;     // kIgnoreTaskOnExit/kIgnoreUnRunTaskOnExit/kMustExecTaskOnExit
    TaskDequeueMethod   dequeueMethod;      // kFIFO/kLIFO
};
```

所有调度决策都基于此结构：线程组选择、优先队列排序、退出时行为、LIFO/FIFO 出队顺序。

### 3.2 TaskQueuePage（任务队列页）

TaskQueuePage 是调度的最小粒度单元，而非单个任务：

- **并行模式**：每个任务单独一个 Page，Page 之间互不依赖，可并发执行
- **队列模式**：多个任务共享同一个 Page，同一 Page 内的任务按序执行，不同 Page 可并发

Page 的所有操作（`appendTask / takeTask / clear`）通过 `ScopedAtomic` 原子操作器串行化，内部使用 `std::atomic<bool> m_isRuningTask` 做快速路径检查，减少互斥锁争用。

### 3.3 TaskQueuePageSortKey（优先级排序键）

```cpp
struct TaskQueuePageSortKey {
    TaskPriority    taskPriority;       // 主排序键：优先级越高越先执行
    TaskDequeueMethod dequeueMethod;    // 次排序键：LIFO 时最新任务优先
    int64_t         createTimestamp;    // 末排序键：精确控制同优先级顺序
    int             workerThreadNumber; // 用于亲和性调度
};
```

`FFTaskPagePriorityQueue` 基于此 Key 维护有序优先队列，保证高优先级任务总是被优先选取执行。

---

## 四、主要业务流程

### 4.1 postReplyableResultTask 完整生命周期

```
调用线程（通常为 UI 线程）
       │
FFAsync::postReplyableResultTask<T>(lambda, feature)
       │  checkPostTaskThreadIsValid()：确认调用线程有 QEventLoop（then 回调需要事件循环）
       │  捕获 currentThread = QPointer(QThread::currentThread())
       │  在堆上分配结果对象：auto* result = new unique_ptr<T>(nullptr)
       │  创建 QPromise<T>（QtPromise）
       │
在 resolver 内部组装 newTask（TaskClosure）：
       │  newTask = [lambda, result, resolve, reject, currentThread]() {
       │      *result = make_unique<T>(lambda());      // 执行实际工作
       │      TaskInvoker::invoke(                     // 处理返回值并 resolve
       │          buildAutoRelease(result).get(),      // RAII 保证在调用线程释放堆对象
       │          resolve, reject, currentThread);
       │  }
       │
FFThreadPool::postTaskOnTaskQueuePage(newTask, feature)
       │  startAtomicOperator() → ScopedAtomic 获取原子操作权
       │  FFTaskScheduler::willPostTask() → 检查退出状态与 Feature 决定是否允许投递
       │  atomicOperator.appendTask(newTask) → 入队列页
       │  threadGroup->startPostTaskQueuePage(...) → 唤醒或启动 Worker
       │
Worker 线程执行
       │  FFTaskScheduler::runTaskAndGetNextTask()
       │  task.run() → newTask() 执行 → lambda 在后台线程运行，结果写入 *result
       │
TaskInvoker::invoke（关键：确保 resolve 在调用线程执行）
       │  若结果类型为普通 T：直接 resolve(std::move(**result))
       │  若结果类型为 QPromise<T>（嵌套）：
       │      内层 Promise 已完成 → 直接 resolve/reject
       │      内层 Promise 未完成 → qtpromise_defer(currentThread, ...)
       │                           在调用线程的事件循环中触发 resolve
       │
调用线程（QEventLoop）
       │  Promise resolve → .then(lambda) 在调用线程执行（自动，无需手动切换）
       └──→ UI 更新、缓存写入等主线程操作安全执行
```

**`buildAutoRelease(result)`** 是防止跨线程释放堆对象的关键设计：将 `result` 指针包装为 `AutoReleaseWrapper`，其析构函数通过 `QMetaObject::invokeMethod` 将 `delete` 调度到调用线程执行，避免 Worker 线程直接释放 UI 线程分配的对象。

### 4.2 串行队列任务（QueueTaskRunner）

```
auto runner = FFAsync::createQueueTaskRunner(feature);

// 多个任务投递到同一 runner（保证串行执行，无需外部加锁）
runner->postReplyableResultTask(taskA);  // 先执行
runner->postReplyableResultTask(taskB);  // 等 A 完成后执行
runner->postReplyableResultTask(taskC);  // 等 B 完成后执行

// 典型应用：AI 算法缓存任务队列（FAlgorithmCacheQueueWorker）
// 保证同一 Clip 的多次参数变更按序处理，结果不乱序
```

### 4.3 优雅退出流程

```
程序退出触发 FFTaskScheduler::startExit()
       │  设置 stateFlag.bit0 = 1（ExitFlag）
       │
后续新投递任务：
       │  willPostTask() 检查 ExitFlag：
       │  ├── TaskBehaviorOnExit::kIgnoreTaskOnExit → 直接拒绝（Promise reject）
       │  ├── kIgnoreUnRunTaskOnExit → 未运行的拒绝，已在运行的继续
       │  └── kMustExecTaskOnExit → 允许投递，计入阻塞计数（stateFlag bits 1..31 += 1）
       │
等待退出：startExit() 阻塞等待 stateFlag.bits[1..31] == 0
       │  每个 MustExec 任务完成后：reduceBlockingTaskNum() → fetch_sub
       │  归零时：wakeOne() 通知退出等待线程
       └──→ 退出完成，线程池安全销毁
```

---

## 五、核心技术点

### 5.1 Promise 回调线程保证（关键设计）

`postReplyableResultTask` 在模板实现中捕获调用线程引用：

```cpp
// FFAsync.inl 核心逻辑（简化）
template<typename F>
auto postReplyableResultTask(F&& task, FFTasksFeature feature) {
    using T = invoke_result_t<F>;
    auto* result = new unique_ptr<T>(nullptr);
    auto currentThread = QPointer(QThread::currentThread()); // 捕获调用线程

    return QPromise<T>([=](auto resolve, auto reject) {
        postTask([=]() {
            *result = make_unique<T>(task());  // Worker 线程执行
            TaskInvoker<T>::invoke(
                buildAutoRelease(result).get(),
                resolve, reject,
                currentThread);  // 确保 resolve 在调用线程执行
        }, feature);
    });
}
```

`TaskInvoker` 内部若检测到当前执行线程 ≠ `currentThread`，则使用 `qtpromise_defer(currentThread, ...)` 将 `resolve` 调度到调用线程的事件循环，**对调用者完全透明**——`.then()` 始终在调用 `postReplyableResultTask` 的线程上执行。

### 5.2 三组线程分组 + 优先级并发限流

```
线程组映射（FFThreadPool::getThreadGroupFromTaskFeature）：

  kTimeCritical / kHighest  →  Foreground ThreadGroup（快速响应 UI 操作）
  kNormal                   →  Common ThreadGroup   （普通任务）
  kTryBest / kLowest        →  Background ThreadGroup（非关键后台任务）
  threadPriority=Background →  强制分配到 Background Group
```

**尽力而为（kTryBest）并发限流**：`maxTryBestThreadCount = 2`，即使 Background 组有空闲线程，同时运行的 `kTryBest` 任务不超过 2 个，防止低优先级任务（如懒加载、埋点）占满线程资源，保证 UI 相关任务的响应性。

### 5.3 LIFO 出队支持"最新优先"场景

对于快速变化的 UI 请求（如滚动时缩略图加载），使用 `LIFO` 出队模式：

```cpp
FFTasksFeature feature;
feature.setDequeueMethod(TaskDequeueMethod::kLIFO);  // 最新提交的任务优先执行
feature.setTaskPriority(TaskPriority::kNormal);

FFAsync::postTask(loadThumbnailTask, feature);
// 用户快速滚动 → 旧的缩略图请求被推后 → 只有最新可见区域的任务被优先执行
```

LIFO 与 `SortKey` 的 `createTimestamp` 共同决定："越新的任务时间戳越大 → 在 LIFO 排序中排在队列前面 → 优先取出执行"。

### 5.4 线程池大小的自适应策略

```cpp
// FFAsyncInitializer.cpp
int maxThreads = qMax(3, QThread::idealThreadCount() - 1);
FFThreadPool::getThreadPool()->startThreadPool({maxThreads}, nullptr);
```

- `QThread::idealThreadCount()` 返回逻辑 CPU 核数，减 1 保留主线程
- 最少保证 3 个线程，确保即使在双核设备上也有足够的并发能力
- Worker 线程支持空闲超时回收（`setWorkerCanRecycle`），避免低负载时线程常驻占用内存

### 5.5 stateFlag 的原子位域设计

`FFTaskScheduler` 将退出控制与阻塞任务计数合并到一个 `std::atomic<quint32>`：

```
stateFlag 位域布局：
  bit  0     = ExitFlag（0=正常, 1=已启动退出）
  bits 1..31 = BlockingTaskCount（kMustExecTaskOnExit 任务的未完成数量）
```

单个原子变量同时携带两种信息，退出判断和计数操作都是无锁的 `fetch_add / fetch_sub`，避免了为退出状态和计数分别加锁的开销。

---

## 六、设计模式应用

### 6.1 Facade 模式

`FFAsync` 类是整个框架的统一入口，对外只暴露 5 个静态方法，屏蔽了线程池、线程组、任务调度器等所有内部复杂性。调用者无需了解任何内部实现即可使用。

### 6.2 Promise 模式

集成 `QtPromise`（qpromise 库），将"异步结果"抽象为 `QPromise<T>` 对象，支持 `.then() / .fail() / .finally()` 链式组合，彻底替代了传统的信号槽或 `QFutureWatcher` 模式。

### 6.3 策略模式（TaskBehaviorOnExit）

退出时任务的处理策略通过枚举注入，框架根据策略决定行为，扩展新的退出策略不需要修改调度核心逻辑。

### 6.4 RAII（资源与事务管理）

- `ScopedAtomic`：作用域内持有原子操作权，析构时自动释放
- `AutoReleaseWrapper`：作用域结束时自动将 `delete` 调度到目标线程执行
- `FF_DECLARE_UNDO_COMMANDER / FF_DECLARE_PAUSE_EVENT_COMMANDER`（上层调用方使用）

### 6.5 单例（Singleton）

`FFThreadPool::getThreadPool()` 返回全局唯一线程池实例，全程序生命周期共享同一调度中心，保证任务优先级与资源配额的全局一致性。

---

## 七、与 QtConcurrent 的核心差异

| 能力 | `QtConcurrent / QFuture` | `FFAsync` |
|---|---|---|
| 任务优先级 | ❌ 无 | ✅ 4 级 TaskPriority + 2 级线程优先级 |
| 回调线程保证 | ❌ 需手动 `QFutureWatcher` + 信号槽 | ✅ `.then()` 自动回到调用线程 |
| 串行任务队列 | ❌ 需外部加锁自实现 | ✅ `createQueueTaskRunner()` 内置 |
| 独立线程串行 | ❌ 需手动 `QThread` + 信号槽 | ✅ `createThreadTaskRunner()` 一行创建 |
| 优雅退出 | ❌ 无细粒度退出控制 | ✅ `TaskBehaviorOnExit` 三种策略 |
| 链式 Promise | ❌ 无 | ✅ `QPromise<T>` + then/fail/finally |
| LIFO 出队 | ❌ 无 | ✅ `TaskDequeueMethod::kLIFO` |

---

## 八、关键类职责速查

| 类名 | 职责 |
|---|---|
| `FFAsync` | 对外门面，5 个静态 API，委托给 FFThreadPool |
| `FFThreadPool` | 全局单例线程池，管理线程组，提供任务投递与运行器创建 |
| `FFThreadGroup` | 线程组，维护优先队列，管理 Worker 线程的启停 |
| `FFWorkerThread` | 工作线程，循环取任务执行，空闲时休眠等待 |
| `FFTaskScheduler` | 全局调度器，控制投递权限、执行权限、优雅退出 |
| `FFTaskQueuePageImpl` | 任务队列页，`ScopedAtomic` 保护的线程安全任务容器 |
| `FFTaskPagePriorityQueue` | 优先队列，按 `SortKey` 排序，保证高优先级任务优先调度 |
| `FFQueueTaskRunner` | 串行队列运行器，共享一个 TaskQueuePage 实现有序执行 |
| `FFThreadTaskRunner` | 独立线程串行运行器，在专属线程上串行处理任务 |
| `FFTasksFeature` | 任务特征描述符，携带优先级/线程级/退出行为/出队方式 |
