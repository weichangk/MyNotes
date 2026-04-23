15-ffasync-optimization.md

# FFAsync 异步框架技术优化方案

---

## 一、缺少任务取消 API，积压任务无法主动撤销

### 问题描述

`FFAsync::postTask` 投递任务后，框架没有返回可取消的句柄。当业务场景（如缩略图加载、AI 预处理）需要在用户切换 Clip 或滚动列表后取消旧任务时，只能依赖上层业务自己实现 token 比对，逻辑分散且容易遗漏。若不取消，已过期的任务仍占用 Worker 线程，执行完成后还可能触发过期 UI 更新。

### 优化方案

为 `postTask / postReplyableResultTask` 引入返回值 `FFTaskHandle`，支持主动取消：

```cpp
class FFTaskHandle {
    std::shared_ptr<std::atomic<bool>> m_cancelled;
public:
    void cancel() { m_cancelled->store(true); }
    bool isCancelled() const { return m_cancelled->load(); }
};

// 框架内部在执行前检查
void FFWorkerThread::runTask(FFTask& task) {
    if (task.handle && task.handle->isCancelled()) {
        task.rejectWithCancellation(); // reject Promise，不执行 lambda
        return;
    }
    task.run();
}

// 调用方：
FFTaskHandle handle = FFAsync::postReplyableResultTask([]() {
    return loadThumbnail(path);
}, feature);

// 切换 Clip 时取消旧请求
handle.cancel();
```

取消操作是无锁的原子写，不影响 Worker 线程调度效率；被取消的任务 Promise reject，`.fail()` 分支可统一处理取消逻辑。

---

## 二、`QueueTaskRunner` 串行队列无法清空已积压的待执行任务

### 问题描述

`FFQueueTaskRunner` 在共享 `TaskQueuePage` 内按序执行任务，若业务场景（如实时参数调节）在短时间内投递大量任务，队列可能积压数十个旧任务，即使结果已无意义，仍会逐一执行，造成 CPU 浪费和延迟输出。当前框架没有提供清空队列的接口。

### 优化方案

在 `FFQueueTaskRunner` 上暴露 `clearPendingTasks()` 接口，配合版本号使已入队任务自失效：

```cpp
class FFQueueTaskRunner {
    std::atomic<uint64_t> m_version{0};

public:
    void clearPendingTasks() {
        ++m_version; // 使所有已投递但未执行的任务失效
    }

    void postTask(std::function<void()> task) {
        uint64_t ver = m_version.load();
        m_taskPage->appendTask([this, task = std::move(task), ver]() {
            if (m_version.load() != ver) return; // 已过期，跳过
            task();
        });
        // 唤醒 Worker...
    }
};

// 实时参数调节场景：
void onParameterChanged(float value) {
    m_runner->clearPendingTasks(); // 丢弃旧参数任务
    m_runner->postTask([value]() { runAlgorithm(value); });
}
```

---

## 三、`AutoReleaseWrapper` 依赖 `QMetaObject::invokeMethod` 可能在调用线程已销毁时崩溃

### 问题描述

`buildAutoRelease(result)` 将堆对象的 `delete` 通过 `QMetaObject::invokeMethod(targetThread, ...)` 调度到调用线程执行。若调用线程（如某个临时 `QThread`）在 Worker 完成前已销毁，`invokeMethod` 目标对象无效，`delete` 永远不会执行，造成**内存泄漏**；若框架内部缓存了裸指针，还可能 UAF。

### 优化方案

用 `QPointer<QObject>` 弱引用检查目标对象是否仍存活，存活则 `invokeMethod`，否则直接在当前线程释放：

```cpp
class AutoReleaseWrapper {
    void* m_ptr;
    QPointer<QObject> m_targetObj; // 弱引用，不延长生命周期
    std::function<void(void*)> m_deleter;

public:
    ~AutoReleaseWrapper() {
        if (m_targetObj) {
            // 目标对象仍存活，调度到其线程释放
            QMetaObject::invokeMethod(m_targetObj, [ptr = m_ptr, del = m_deleter]() {
                del(ptr);
            }, Qt::QueuedConnection);
        } else {
            // 目标已销毁，当前线程直接释放（无 UAF 风险，ptr 是堆上数据）
            m_deleter(m_ptr);
        }
    }
};
```

---

## 四、线程组划分静态固定，低负载时 Background 组线程空占资源

### 问题描述

`FFThreadPool` 将线程按 Foreground / Common / Background 三组静态分配，各组线程数量在初始化时确定。在实际使用中，Foreground 和 Common 组在用户空闲时几乎没有任务，但其线程仍常驻内存；Background 组在批量导入或 AI 处理高峰期又可能不够用。静态分配导致资源无法跨组复用。

### 优化方案

引入**动态借用机制**：当 Background 组有大量待处理任务且其他组空闲时，允许空闲 Worker 临时跨组执行低优先级任务：

```cpp
FFTask* FFWorkerThread::stealTaskFromLowerGroup() {
    // 本组没有任务时，尝试从低优先级组窃取
    if (m_group->taskCount() == 0) {
        auto* backgroundGroup = FFThreadPool::getThreadPool()->getBackgroundGroup();
        return backgroundGroup->tryStealTask(/*maxPriority=*/kTryBest);
    }
    return nullptr;
}

// Worker 主循环：
while (!shouldExit()) {
    FFTask* task = m_group->getNextTask();
    if (!task) task = stealTaskFromLowerGroup(); // Work-stealing
    if (!task) { waitForTask(); continue; }
    runTask(*task);
}
```

Work-stealing 是无锁的 `try_dequeue`，失败即返回 `nullptr`，不阻塞高优先级组。

---

## 五、同一资源（如同一图片路径）的缩略图任务可能被重复投递

### 问题描述

`FFThumbnailCache` 在 `getThumb()` 时若缓存未命中，直接通过 `FFAsync::postTask` 投递解码任务。若用户在缩略图未完成前多次触发同一路径的加载（如反复滚动、多处引用同一媒体），相同路径会被重复投递多次，Worker 线程对同一文件进行多次 IO 和解码，浪费资源。

### 优化方案

在框架或上层缓存层维护"投递中"集合（`in-flight set`），同一 Key 的任务只投递一次，后续请求附加回调，任务完成后统一通知所有等待者：

```cpp
class FFThumbnailCache {
    QMutex m_mutex;
    QHash<QString, QList<ThumbnailCallback>> m_inFlight; // path → pending callbacks

public:
    void requestThumb(const QString& path, ThumbnailCallback cb) {
        QMutexLocker lock(&m_mutex);
        if (m_inFlight.contains(path)) {
            m_inFlight[path].append(cb); // 附加到已有任务
            return;
        }
        m_inFlight[path] = {cb};
        lock.unlock();

        FFAsync::postTask([this, path]() {
            QPixmap pixmap = decodeThumb(path);
            QMutexLocker lock(&m_mutex);
            auto callbacks = m_inFlight.take(path);
            lock.unlock();
            for (auto& c : callbacks) c(pixmap); // 统一通知
        }, feature);
    }
};
```

---

## 六、`startExit()` 等待阻塞任务时没有超时，可能导致程序挂起

### 问题描述

`FFTaskScheduler::startExit()` 通过 `QWaitCondition::wait()` 无限等待 `kMustExecTaskOnExit` 任务计数归零。若某个 `MustExec` 任务内部存在死循环、网络阻塞或资源竞争，`startExit()` 将永久阻塞，导致程序退出时界面卡死，用户感知为"无响应"。

### 优化方案

为等待添加超时，超时后强制继续退出并记录告警日志：

```cpp
void FFTaskScheduler::startExit() {
    m_stateFlag.fetch_or(kExitBit);

    constexpr int kExitTimeoutMs = 5000; // 最多等 5 秒
    QMutexLocker lock(&m_mutex);
    bool finished = m_waitCondition.wait(&m_mutex, kExitTimeoutMs);

    if (!finished) {
        int remaining = blockingTaskCount();
        qWarning() << "[FFAsync] Exit timeout! " << remaining
                   << " MustExec tasks did not finish. Force exiting.";
        // 强制继续退出，Worker 线程后续访问已销毁对象由各 Task 的弱引用保护
    }
}
```
