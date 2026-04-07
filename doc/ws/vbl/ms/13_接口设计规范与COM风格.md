# 13 · 接口设计规范与 COM 风格

> **面试价值**：⭐⭐⭐⭐  **优先级**：P1
> **相关文件**：
> - Interface/ 目录（所有接口定义）
> - Include/Common/IDmBaseObj.h — 基础接口
> - Include/Common/IDmProtocol.h — Protocol 接口

---

## 1. 接口命名约定

VBL 严格遵循以 I 开头的接口命名约定：

    IDm*   -- DataModel 层接口（IDmClip, IDmTimeline, IDmProject）
    IBs*   -- BaseService 层接口（IBsUndoManager, IBsProxy）
    IVb*   -- VBL 顶层接口（IVbLModule, IVbProjectEditor）
    IMs*   -- 模块系统接口（IMsEventBus）
    
    约定规则：
    1. 接口以 I 开头 + 模块前缀 + 功能名
    2. 接口只含纯虚函数，不含数据成员（极少数例外）
    3. 接口声明在 Interface/ 目录，实现在 modules/ 目录
    4. 接口有对应的工厂函数（CreateXxx()）

---

## 2. 基础接口 IDmBaseObj

所有 VBL 对象都实现 IDmBaseObj，它定义了最基本的生命周期和查询协议：

    // Include/Common/IDmBaseObj.h
    class IDmBaseObj {
    public:
        virtual ~IDmBaseObj() = default;
        
        // COM 风格引用计数
        virtual int AddRef() = 0;
        virtual int Release() = 0;
        virtual int RefValue() = 0;
        
        // 协议扩展接口
        virtual IDmProtocol* GetProtocol() = 0;
        virtual void SetProtocol(IDmProtocol* value) = 0;
        
        // 查询接口（类似 COM 的 QueryInterface）
        virtual void* Query(VBLConstPChar name) = 0;
    };

Query() 方法的典型实现（MsEventBus 中）：

    // MsEventBus.h
    void* Query(VBLConstPChar name) override {
        if (strcmp(name, "IMsEventBus") == 0) {
            return (IMsEventBus*)this;
        }
        return nullptr;
    }

这是 COM QueryInterface 的简化版本：通过字符串名称查询对象是否支持某个接口，
支持则返回正确类型的指针，不支持则返回 nullptr。
调用方不需要知道对象的具体类型，只需知道接口名。

---

## 3. 接口 + 工厂的标准模式

VBL 的接口总是配合工厂函数使用，调用方只依赖接口：

    // 接口定义（Interface/DataModel/IDmTimeline.h）
    class IDmTimeline : public IDmBaseObj {
    public:
        virtual Result AddClip(SafePtr<IDmClip> clip, int trackIndex,
                               int64_t position) = 0;
        virtual Result RemoveClip(const std::string& clipId) = 0;
        virtual SafePtr<IDmClip> GetClip(const std::string& clipId) const = 0;
        virtual int GetTrackCount() const = 0;
        // ...
    };
    
    // 工厂函数（用于创建实现对象）
    class IDmTimelineFactory {
    public:
        virtual SafePtr<IDmTimeline> CreateTimeline() = 0;
    };

    // 实现（modules/DataModel/DmTimeline.h）— 调用方不引用此头文件
    class DmTimeline : public IDmTimeline, public RefCnt<DmTimeline> {
        // 实现 IDmTimeline 的所有纯虚函数
    };

    // 调用方代码（只包含接口头文件）
    IDmTimelineFactory* factory = GetTimelineFactory();
    SafePtr<IDmTimeline> timeline = factory->CreateTimeline();
    timeline->AddClip(clip, 0, 0);  // 完全不知道 DmTimeline 的存在

---

## 4. COM 风格生命周期管理

VBL 的对象生命周期模仿 Windows COM（Component Object Model）的设计：

    规则 1：对象以引用计数为 1 被创建
    规则 2：每次持有对象（拷贝指针）必须 AddRef()
    规则 3：每次释放对象（不再使用）必须 Release()
    规则 4：引用计数归零时对象自动销毁（delete this）
    规则 5：接口指针而非实现类指针传递（依赖倒置）

与标准 COM 的区别：
- VBL 不实现完整的 IUnknown（没有 GUID 和 QueryInterface 的 HRESULT 返回）
- Query() 方法是简化的 QueryInterface，用字符串替代 IID_xxx GUID
- 引用计数用 C++ atomic 而非 WIN32 InterlockedIncrement

---

## 5. 接口与实现的目录分离

    项目结构（体现关注点分离）：
    
    Interface/
        DataModel/
            IDmClip.h         # 接口声明（只有纯虚函数）
            IDmTimeline.h
            IDmProject.h
        BaseService/
            IBsUndoManager.h
            IBsProxy.h
        ListenerCenter/
            IMsEventBus.h
    
    modules/
        DataModel/
            DmClip.h/.cpp     # 接口的具体实现
            DmTimeline.h/.cpp
        BaseService/
            BsUndoManager/
            BsProxy/
        ListenerCenter/
            MsEventBus.h/.cpp

    Include/Common/
        IDmBaseObj.h          # 所有接口的基类
        VblSafePtr.h          # 智能指针（接口的持有方式）
        VblResultDef.h        # 接口返回值类型

这种分离的好处：
- 上层模块只包含 Interface/ 头文件，不引入实现细节
- 更换实现（如从 WES 切换到 NLE）不需要修改接口头文件
- 接口头文件可以作为 SDK 文档使用

---

## 6. 接口隔离原则（ISP）在 VBL 中的体现

VBL 的接口粒度很小，遵循单一职责：

    // 细粒度接口（各自独立）
    IDmBaseObj     -- 生命周期基础（AddRef/Release/Query）
    IDmDescription -- 对象描述（GetName/SetName/GetId...）
    IDmClip        -- Clip 操作接口
    IVblSerializeUserData  -- 序列化接口
    IVblRenderItem -- 渲染接口
    
    // 一个 DmClip 可能同时实现多个接口（多重继承）
    class DmVideoClip : public IDmClip,
                        public IDmDescription,
                        public IVblSerializeUserData,
                        public RefCnt<DmVideoClip> {
        // 实现所有接口的纯虚函数
    };

---

## 7. 面试要点

1. **I 前缀命名约定强制接口与实现分离**：IDm/IBs/IVb 前缀让代码阅读者立刻知道这是接口，
   寻找实现需要去 modules/ 目录，而不是在头文件中看到具体实现代码，降低认知负担。

2. **Query() 是轻量版 QueryInterface**：COM 的 QueryInterface 通过 GUID 查询接口，
   VBL 的 Query() 用字符串，更简单但功能等价。调用方用字符串名称运行时检查对象是否
   支持某个接口，是一种运行时多态机制（不依赖静态类型系统）。

3. **纯虚接口 + SafePtr 是 VBL 对象的标准持有方式**：函数参数和返回值用
   SafePtr<IDmXxx>（接口指针的强引用），而非具体实现类型，符合依赖倒置原则（DIP）。
   编译时不需要看到实现类的完整定义，只需要接口头文件。

4. **多重继承用于组合接口能力**：DmVideoClip 同时实现 IDmClip + IDmDescription +
   IVblSerializeUserData，比"为每个接口单独创建适配器"更直接。
   C++ 多重继承在接口场景（无数据成员的纯虚类）不会有菱形继承问题。

5. **COM 风格解决了 DLL 边界的所有权问题**：跨 DLL 传递对象时，谁负责 delete？
   COM 的答案是：谁创建谁管理，引用计数归零时自动 delete this。
   这避免了"A 模块创建，B 模块 delete"导致 CRT 不匹配的崩溃问题。

---

## 8. 可能被追问的问题

**Q1：为什么要接口与实现分离？用一个类直接写不行吗？**
依赖倒置原则（DIP）：高层模块不应该依赖低层模块的实现细节，只依赖抽象接口。
好处：可以替换实现（如 WES->NLE）不影响调用方；便于 mock 测试（注入 MockTimeline）；
减少编译依赖（不包含实现头文件就不触发重编译）。

**Q2：VBL 的 Query() 和 dynamic_cast 相比有什么优势？**
dynamic_cast 依赖 RTTI（运行时类型信息），跨 DLL 时 RTTI 可能不一致（不同编译器/版本）。
Query() 用字符串比较，不依赖 RTTI，跨 DLL 安全；缺点是性能稍低（字符串比较 vs 指针比较）。

**Q3：纯虚接口的多重继承会有什么问题？如何避免？**
纯虚接口（无数据成员）的多重继承不会有菱形继承的数据成员二义性问题。
但如果接口链很长（A->B->C->D 每层都有 AddRef），需要注意虚函数重写的二义性。
VBL 通过在实现类中显式指定 `using RefCnt<T>::AddRef` 来消除二义性。

**Q4：IDmProtocol 是什么？GetProtocol/SetProtocol 的用途？**
IDmProtocol 是一个钩子接口，允许外部代码注册到对象的销毁事件中。
当 RefCnt 引用计数归零触发 delete this 前，会调用 protocol_->Destroy()。
用途：对象池可以用此机制得知对象即将销毁，从而从"活跃对象"列表中移除引用；
或者父对象注册 Protocol 监听子对象的销毁事件。
