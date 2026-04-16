14-ffasync-project-content

# 项目内容

- 设计并实现 **FFAsync 自研异步调度框架**（Facade 模式对外暴露 5 个静态 API），内部由三级线程组（Foreground / Common / Background）+ `FFTaskPagePriorityQueue` 优先队列驱动，支持 4 级 `TaskPriority` 与 2 级线程优先级，替代 `QtConcurrent` 在优先级、串行队列、优雅退出方面的不足，全项目缩略图、音频波形、AI 推理等核心异步场景均基于此框架实现。

- 实现 **`postReplyableResultTask` Promise 回调线程自动保证**：调用时捕获 `QPointer(QThread::currentThread())`，Worker 线程完成任务后由 `TaskInvoker` 检测线程差异，若不在调用线程则通过 `qtpromise_defer` 将 `resolve` 投递回调用线程事件循环，`.then()` 始终在调用方线程执行，对上层完全透明，彻底消除手动 `QFutureWatcher` + 信号槽的样板代码。

- 设计 **`TaskQueuePage` 调度粒度抽象**与三种任务运行器：`FFParallelTaskRunner`（每任务独立 Page，并发执行）、`FFQueueTaskRunner`（多任务共享 Page，FIFO/LIFO 串行执行）、`FFThreadTaskRunner`（专属独立线程串行运行），AI 算法缓存队列（`FAlgorithmCacheQueueWorker`）使用 QueueTaskRunner 保证同一 Clip 多次参数变更按序处理、结果不乱序。

- 实现 **`stateFlag` 原子位域优雅退出机制**：将退出标志（bit 0）与阻塞任务计数（bits 1–31）合并到单个 `std::atomic<quint32>`，通过 `TaskBehaviorOnExit` 三种策略（忽略/忽略未运行/必须执行）精细控制退出时任务行为，主线程 `startExit()` 无锁等待阻塞计数归零后安全销毁线程池，消除退出时线程安全问题。

- 实现 **`buildAutoRelease` 跨线程堆对象安全释放**：将 Worker 线程需要释放的调用线程堆对象包装为 `AutoReleaseWrapper`，析构时通过 `QMetaObject::invokeMethod` 将 `delete` 调度到原始分配线程执行，配合 `ScopedAtomic` 原子操作器串行化 TaskQueuePage 读写，保证框架内所有内存操作无 UAF 风险；并通过 `kTryBest` 并发上限（`maxTryBestThreadCount = 2`）防止低优先级后台任务（懒加载、埋点）占满线程资源。
