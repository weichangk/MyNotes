# 04 · 设计模式 — Observer / Builder / Strategy / PIMPL / Proxy

> **面试价值**：⭐⭐⭐⭐  **优先级**：P1
> **相关文件**：
> - Include/Common/safelist.h / safemap.h
> - modules/BusinessLayer/TimelineUX/ — TimelineDataBuilder
> - modules/ListenerCenter/MsEventBus.h — PIMPL
> - modules/BaseService/BsProxy/BsProxy.h

---

## 1. Observer（观察者模式）— SafelyObserverBase / weak_ptr 安全观察者

### 1.1 问题背景

普通观察者模式的经典问题：被观察对象生命周期比观察者长时，通知已销毁的观察者会导致悬空指针崩溃。

### 1.2 VBL 的解决方案

VBL 通过 weak_ptr 持有观察者，在通知前检查是否存活：

    // Include/Common/safelist.h（核心思路）
    template<typename ObserverType>
    class SafelyObserverBase {
    public:
        void AddObserver(std::shared_ptr<ObserverType> observer) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_observers.push_back(observer);  // 弱持有
        }
    protected:
        // 通知：先拿快照再释放锁，防止迭代器失效
        void NotifyObservers(std::function<void(ObserverType&)> fn) {
            std::vector<std::weak_ptr<ObserverType>> snapshot;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                snapshot = m_observers;
            }
            for (auto& wp : snapshot) {
                if (auto sp = wp.lock()) {   // lock() 失败 = 观察者已销毁
                    fn(*sp);
                }
            }
        }
    private:
        std::vector<std::weak_ptr<ObserverType>> m_observers;
        mutable std::mutex m_mutex;
    };

### 1.3 SafelyObserverBase vs EventBus 对比

| 维度 | SafelyObserverBase | MsEventBus |
|------|--------------------|-|
| 耦合度 | 较高（需实现 Observer 接口）| 低（事件名字符串）|
| 类型安全 | 强类型（模板）| 弱类型（EventValue 泛化）|
| 生命周期 | weak_ptr 自动处理 | 需手动 unsubscribe |
| 适用场景 | 模块内部回调 | 跨模块解耦通信 |

---

## 2. Builder（建造者模式）— TimelineDataBuilder

TimelineUX 层用 Builder 将"时间线数据的构建过程"与"具体构建策略"分离：

    // 抽象建造者：固定构建步骤顺序（模板方法）
    class TimelineDataBuilder {
    public:
        virtual void BuildTracks()      = 0;
        virtual void BuildClips()       = 0;
        virtual void BuildTransitions() = 0;
        virtual void BuildEffects()     = 0;
        virtual TimelineData GetResult() = 0;
        
        // 模板方法：固定构建顺序，子类填充具体逻辑
        TimelineData Build() {
            BuildTracks();
            BuildClips();
            BuildTransitions();
            BuildEffects();
            return GetResult();
        }
    };
    
    // 具体建造者1：完整时间线
    class TimelineBuilder : public TimelineDataBuilder {
        void BuildTracks()      override { /* 构建所有轨道 */ }
        void BuildClips()       override { /* 构建所有 Clip */ }
        void BuildTransitions() override { /* 构建转场 */ }
        void BuildEffects()     override { /* 构建特效 */ }
        TimelineData GetResult() override { return m_data; }
        TimelineData m_data;
    };
    
    // 具体建造者2：仅 Clips 的简化时间线（如 AI 一键成片导出用）
    class ClipsTimelineBuilder : public TimelineDataBuilder {
        void BuildTracks()      override { /* 只建主轨道 */ }
        void BuildClips()       override { /* 构建 Clip */ }
        void BuildTransitions() override { /* 跳过 */ }
        void BuildEffects()     override { /* 跳过 */ }
        TimelineData GetResult() override { return m_data; }
        TimelineData m_data;
    };

调用方只选择 Builder，不关心构建细节：

    std::unique_ptr<TimelineDataBuilder> builder;
    if (needFullTimeline) {
        builder = std::make_unique<TimelineBuilder>(sourceData);
    } else {
        builder = std::make_unique<ClipsTimelineBuilder>(sourceData);
    }
    TimelineData data = builder->Build();  // 统一接口触发构建

---

## 3. PIMPL（编译防火墙）— MsEventBus 的 struct Impl

PIMPL（Pointer to IMPLementation）将实现细节移到 .cpp 文件的私有结构体，.h 只暴露接口。

**优点**：修改实现不触发头文件变更（减少重编译传播）；ABI 稳定；隐藏第三方库依赖。

    // MsEventBus.h（对外头文件——极简，不暴露任何实现细节）
    // 文件路径: modules/ListenerCenter/MsEventBus.h
    class MsEventBus : public IMsEventBus, public RefCnt<void> {
    public:
        explicit MsEventBus(VBLConstPChar title = nullptr);
        ~MsEventBus() override;   // 必须在 .cpp 中定义（Impl 完整类型处）
        Result subscribe(VBLConstPChar evName, Callback cb, ...) override;
        Result postMainThreadEvent(...) override;
    private:
        struct Impl;                      // 前向声明，完整定义在 .cpp 中
        std::unique_ptr<Impl> m_impl;     // PIMPL 核心
    };

    // MsEventBus.cpp（实现文件——所有细节都在这里）
    struct MsEventBus::Impl {
        std::unordered_map<std::string, EventInfo>  events;
        std::unordered_map<int, SubscriberInfo>     subscribers;
        std::vector<StickyEventEntry>               stickyEvents;
        std::mutex   mutex;
        std::thread  bgThread;
    };
    
    MsEventBus::MsEventBus(VBLConstPChar title)
        : m_impl(std::make_unique<Impl>()) {}
    
    // 析构函数必须在 .cpp 中定义，否则 unique_ptr<Impl> 的析构器无法实例化
    MsEventBus::~MsEventBus() = default;

---

## 4. Proxy（代理模式）— BsProxy 转码代理管理

原始视频（4K H.265）编辑性能差，BsProxy 为其创建低码率代理文件，对调用方完全透明：

    // modules/BaseService/BsProxy/BsProxy.h
    class BsProxy {
    public:
        // 请求创建代理（异步后台转码）
        Result RequestProxy(const std::string& originalPath,
                            ProxyConfig config, ProxyCallback callback);
        
        // 透明访问：有代理返回代理路径，没有返回原始路径
        std::string GetProxyPath(const std::string& originalPath) const;
        
        ProxyStatus GetProxyStatus(const std::string& originalPath) const;
        Result DeleteProxy(const std::string& originalPath);
    };

调用流程：

    TimelineEditor: "给我 raw.mp4 的播放路径"
        ↓
    BsProxy: 有代理? → 返回 raw_proxy.mp4（低码率，快速编辑）
             没代理? → 返回 raw.mp4（降级使用原始文件）
        ↓
    RenderEngine: 播放对应文件

---

## 5. 面试要点

1. **Observer + weak_ptr 是处理生命周期不确定性的标准方案**：`weak_ptr::lock()` 失败自动跳过已销毁的观察者；通知时先拿 snapshot 再释放锁，防止回调内反向修改列表导致迭代器失效。

2. **PIMPL 的本质是减少头文件依赖链**：MsEventBus.h 不出现 `<mutex>` `<thread>` `<unordered_map>`，使用方重编译代价极低。Filmora 这种百万行级别项目中 PIMPL 能显著降低增量编译时间。

3. **Builder = 分步骤构建 + 隐藏构建细节**：TimelineDataBuilder 的模板方法 Build() 固定构建顺序，子类只填充各步骤的具体逻辑，避免"参数爆炸"式的构建函数。

4. **Proxy 使切换对调用方透明**：BsProxy::GetProxyPath() 对调用方只是"获取播放路径"，代理是否存在完全透明，实现了编辑性能和原始质量之间的无缝切换。

5. **Strategy vs Builder**：TimelineDataBuilder 的子类既是 Builder（分步骤构建），也是 Strategy（可互换的算法实现）。实际项目中这两种模式经常自然融合。

---

## 6. 可能被追问的问题

**Q1：为什么通知 Observer 时要先拿快照（snapshot）？**
防止通知回调内部修改观察者列表导致迭代器失效。例如 Observer A 的回调调用了 RemoveObserver(B)，直接在 m_observers 上迭代会迭代器悬空。先拷贝 snapshot，对 snapshot 迭代，原 list 的修改不影响当前迭代。

**Q2：PIMPL 有什么代价？**
①一次额外堆分配（new Impl）；②每次方法调用多一次指针解引用（m_impl->method()）；③unique_ptr<Impl> 要求析构函数在 .cpp 中定义，否则编译错误。热路径代码需权衡是否使用。

**Q3：Proxy 和 Decorator 的区别？**
Proxy：控制对象的访问（访问控制/懒加载/缓存），BsProxy 拦截"获取播放路径"请求，决定返回代理还是原始路径。
Decorator：动态为对象增加功能（行为增强）。
关键区别：Proxy 管理目标对象的访问控制；Decorator 专注于功能增强，持有目标对象引用。

**Q4：VBL 中 Observer 和 EventBus 如何选择？**
模块内部、类型确定、需要强类型回调 → SafelyObserverBase；
跨模块、松耦合、事件类型运行时确定 → MsEventBus。
VBL 实际上两者并用：DataModel 层内部用 Observer，跨层（DataModel → UI）用 EventBus。
