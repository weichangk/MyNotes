# 06 · 事件总线 — MsEventBus

> **面试价值**：⭐⭐⭐⭐⭐  **优先级**：P0
> **相关文件**：
> - modules/ListenerCenter/MsEventBus.h/.cpp
> - Interface/ListenerCenter/IMsEventBus.h
> - Include/Common/VblEventbusWrapper.h

---

## 1. 概念介绍

事件总线（Event Bus）是一种"发布-订阅"模式的基础设施，解决组件间解耦通信问题：
- **发布者**：发送事件，不知道谁会处理
- **订阅者**：订阅事件，不知道谁发送
- **事件总线**：中间人，负责路由和分发

相比直接的 Observer 模式，事件总线的优势是：订阅者和发布者完全不需要知道彼此的存在。

---

## 2. MsEventBus 架构概览

    // Interface/ListenerCenter/IMsEventBus.h（接口）
    class IMsEventBus : public IDmBaseObj {
    public:
        // 1. 注册/取消注册事件
        virtual Result registerEvent(VBLConstPChar evName, VBLBool active = true) = 0;
        virtual Result unregisterEvent(VBLConstPChar evName) = 0;
        
        // 2. 订阅/取消订阅（返回 token，用于取消订阅）
        virtual VBLInt subscribe(VBLConstPChar evName, Callback cb,
                                 void* target = nullptr, VBLBool active = true) = 0;
        virtual Result unsubscribe(VBLInt token) = 0;
        
        // 3. 发布事件（不同线程模式）
        virtual Result postMainThreadEvent(VBLConstPChar evName,
                                           const EventValue& data) = 0;
        virtual Result postCurrentEvent(VBLConstPChar evName,
                                        const EventValue& data) = 0;
        virtual Result postBackgroundEvent(VBLConstPChar evName,
                                           const EventValue& data) = 0;
        virtual Result postAsyncEvent(VBLConstPChar evName,
                                      const EventValue& data) = 0;
        virtual Result postEvent(ThreadMode tm, VBLConstPChar evName,
                                 const EventValue& data) = 0;
        
        // 4. Sticky event（粘性事件）
        virtual Result postStickyEvent(ThreadMode tm, VBLConstPChar evName,
                                       EventValue data) = 0;
        virtual Result fetchStickEvent(VBLConstPChar evName, VBLInt count,
                                       EventValue*& evValues, VBLInt& evCount,
                                       VBLBool back = true, VBLBool take = false) = 0;
    };

---

## 3. 线程分发模式（ThreadMode）

这是 MsEventBus 最核心的设计之一：

    enum class ThreadMode {
        MAINTHREAD,      // 在主线程（UI 线程）回调订阅者
        ASYNC,           // 在新的异步线程回调
        BACKGROUND,      // 在后台线程池中回调
        CURRENT,         // 在 post 的调用线程中同步回调
        WORKER_INDEX,    // 在指定序号的 worker 线程回调（有序保证）
    };

各模式的适用场景：

| ThreadMode | 适用场景 | 线程安全要求 |
|------------|----------|-------------|
| MAINTHREAD | UI 更新、触发重绘 | 只在主线程执行，无需额外同步 |
| ASYNC | 一次性耗时操作，无顺序要求 | 回调本身需线程安全 |
| BACKGROUND | 数据处理，复用线程池 | 回调本身需线程安全 |
| CURRENT | 需要同步返回结果 | 调用方和回调在同一线程 |
| WORKER_INDEX | 有序操作（如日志写入）| 同一 index 的回调有序 |

---

## 4. PIMPL 封装实现

文件：modules/ListenerCenter/MsEventBus.h

    // MsEventBus.h（接口侧，不暴露实现细节）
    class MsEventBus : public IMsEventBus, public RefCnt<void> {
    public:
        explicit MsEventBus(VBLConstPChar title = nullptr);
        ~MsEventBus() override;
        
        // IMsEventBus 接口实现
        Result registerEvent(VBLConstPChar evName, VBLBool active = true) override;
        VBLInt subscribe(VBLConstPChar evName, Callback cb, ...) override;
        Result postMainThreadEvent(...) override;
        Result postStickyEvent(...) override;
        // ...
        
    private:
        struct Impl;                      // 前向声明
        std::unique_ptr<Impl> m_impl;     // PIMPL：隐藏 mutex/thread/map 等实现细节
    };

    // MsEventBus.cpp（实现细节，外部头文件不感知这些）
    struct MsEventBus::Impl {
        std::unordered_map<std::string, EventInfo>  events;       // 注册的事件
        std::unordered_map<int, SubscriberInfo>     subscribers;  // 订阅者列表
        std::vector<StickyEventEntry>               stickyEvents; // 粘性事件缓存
        
        std::mutex   mutex;
        std::thread  bgThread;       // 后台事件分发线程
        // 分发逻辑内部方法...
    };

---

## 5. Sticky Event（粘性事件）

粘性事件是 MsEventBus 的特色功能，来源于 Android EventBus 的设计：

**普通事件**：先 post，后 subscribe → 订阅者收不到
**粘性事件**：先 post，后 subscribe → 订阅者能收到（事件"粘"在总线上）

应用场景：
- 网络状态变化（先上报，后续新订阅者也能知道当前状态）
- 媒体文件加载完成（导入完成后，晚订阅的组件也能获取到结果）
- 初始化状态通知（VBL 初始化完成事件）

    // 发布粘性事件（会持久保存在 stickyEvents 中）
    eventBus->postStickyEvent(
        ThreadMode::MAINTHREAD,
        "MediaLoadComplete",
        EventValue{...}
    );
    
    // 晚于 post 的订阅者也能收到（订阅时立即触发一次）
    int token = eventBus->subscribe("MediaLoadComplete", [](const EventValue& v) {
        // 即使在 post 之后订阅，这里也能收到事件
        handleMediaLoaded(v);
    });
    
    // 主动获取粘性事件历史
    EventValue* values = nullptr;
    VBLInt count = 0;
    eventBus->fetchStickEvent("MediaLoadComplete", 10, values, count);

---

## 6. 典型使用流程

    // ===== 注册事件（通常在模块初始化时）=====
    // 文件: modules/DataModel/DmTimeline.cpp（示意）
    m_eventBus->registerEvent("TimelineClipAdded");
    m_eventBus->registerEvent("TimelineClipRemoved");
    m_eventBus->registerEvent("TimelinePositionChanged");
    
    // ===== 订阅（UI 层或其他模块）=====
    // 文件: UI 层（示意）
    int token1 = m_eventBus->subscribe(
        "TimelineClipAdded",
        [this](const EventValue& v) {
            // 在主线程中刷新时间线视图
            RefreshTimelineView();
        },
        this,  // target：用于按 target 批量取消订阅
        true   // active：是否立即激活
    );
    
    // ===== 发布事件（DataModel 层有变更时）=====
    // 文件: modules/DataModel/DmTimeline.cpp（示意）
    void DmTimeline::AddClip(SafePtr<IDmClip> clip) {
        // ... 添加 clip 逻辑 ...
        
        // 通知订阅者（在主线程回调）
        EventValue data;
        data["clipId"] = clip->GetId();
        data["trackIndex"] = trackIndex;
        m_eventBus->postMainThreadEvent("TimelineClipAdded", data);
    }
    
    // ===== 取消订阅（UI 层析构时）=====
    m_eventBus->unsubscribe(token1);
    // 或者按 target 批量取消
    // m_eventBus->unsubscribeByTarget(this);

---

## 7. 与 Android EventBus 的对比

MsEventBus 的设计明显受到 Android EventBus 的影响：

| 特性 | MsEventBus | Android EventBus |
|------|------------|------------------|
| 线程模式 | ThreadMode 枚举 | @Subscribe(threadMode=) |
| 粘性事件 | postStickyEvent | postSticky() |
| 取消订阅 | token-based | unregister(this) |
| 事件类型 | 字符串 key | Java 类类型 |
| 类型安全 | EventValue（弱类型）| 强类型（class）|

VBL 用字符串 key 而非 Java 类类型，因为 C++ 跨 ABI 没有统一的 RTTI，字符串更稳定。

---

## 8. 面试要点

1. **MsEventBus 是 VBL 跨层解耦的核心机制**：DataModel 层的数据变更通过 EventBus
   通知 BusinessLayer，BusinessLayer 再通知 UI，三层之间没有直接依赖，只依赖事件名约定。

2. **ThreadMode 是关键设计决策**：发布者指定"在哪个线程通知订阅者"，避免了订阅者自己
   处理线程切换（dispatch_async/runOnUiThread）的复杂性。MAINTHREAD 模式最常用，
   因为大多数 UI 更新需要在主线程执行。

3. **PIMPL 隐藏了 mutex/thread 等实现细节**：使用方头文件不依赖 <mutex> <thread>，
   减少编译依赖传播，改动实现不会触发使用方重编译。

4. **Sticky event 解决了"订阅时机早晚"的问题**：视频编辑软件中很多状态是"单次变更，
   多次查询"的场景（如媒体加载完成、AI 分析结果），粘性事件让晚注册的模块也能获取状态。

5. **token-based 取消订阅比"按 this 指针"更精确**：每个 subscribe() 返回唯一 token，
   可以精确取消某一个订阅，而不影响同一 target 的其他订阅。避免了按对象批量取消
   导致的意外取消问题。

---

## 9. 可能被追问的问题

**Q1：MsEventBus 如何保证多线程安全？**
Impl 内部维护 mutex 保护 events/subscribers 的读写。postMainThreadEvent 会
将事件打包后 dispatch 到主线程队列（通过平台相关机制：Win32 PostMessage / 
macOS dispatch_async / GCD），实际回调在主线程串行执行，不需要订阅者额外加锁。

**Q2：postCurrentEvent 和 postMainThreadEvent 有什么区别？应用场景？**
postCurrentEvent：在调用 post 的线程中同步执行回调，是同步调用，post 返回时回调已完成。
postMainThreadEvent：将事件投递到主线程队列，异步执行，post 返回后回调可能还未执行。
应用场景：需要等待回调结果时用 CURRENT；需要 UI 更新（只能主线程）时用 MAINTHREAD。

**Q3：订阅者在回调中再次 post 事件会导致死锁吗？**
MAINTHREAD 模式下，如果在主线程回调中再 post MAINTHREAD 事件，会再次进入队列等待，
不会死锁（队列是 FIFO 异步的）。CURRENT 模式下的递归 post 同一事件可能导致无限
递归，VBL 需要在设计上避免这种情况（设置最大递归深度或检查递归标志）。

**Q4：事件名用字符串而非枚举有什么代价？**
①性能：字符串比较比枚举/整数慢（可以用 hash 优化）；
②编译期无类型检查（拼写错误运行时才发现）；
③优点：ABI 稳定，跨模块/DLL 可以用相同字符串；新模块不需要修改中心枚举。
VBL 通常在模块头文件中定义事件名常量（const char* kEventXxx）来减少拼写错误风险。

**Q5：如果订阅者在回调中自我析构（调用 unsubscribe），会崩溃吗？**
如果实现不当可能崩溃（迭代订阅者列表时修改列表）。VBL 的标准实现是：
通知前先拷贝订阅者快照（snapshot），再逐个回调；unsubscribe 将对应 token 标记为
失效，下次分发时清理。这样在回调内 unsubscribe 是安全的。
