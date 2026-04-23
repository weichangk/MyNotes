# 05 · 自定义智能指针体系 — SafePtr / RefCnt / WeakPtr

> **面试价值**：⭐⭐⭐⭐⭐  **优先级**：P0
> **相关文件**：Include/Common/VblSafePtr.h, VblRefCnt.h, VblWeakPtr.h, VblControlBlock.h

---

## 1. 概念：Intrusive vs Non-intrusive 引用计数

VBL 实现了一套自定义 intrusive（侵入式）引用计数智能指针体系，与标准库对比：

| 标准库 | VBL 实现 | 说明 |
|--------|----------|------|
| std::shared_ptr | VBL::SafePtr | 强引用 |
| std::weak_ptr | VBL::WeakPtr | 弱引用 |
| enable_shared_from_this | VBL::RefCnt | 对象基类 |

| 维度 | std::shared_ptr（non-intrusive）| VBL SafePtr（intrusive）|
|------|------|------|
| 引用计数位置 | 独立 ControlBlock 堆分配 | 嵌入对象内部（RefCnt 基类）|
| 内存分配次数 | new+shared_ptr: 2次 | 始终 1次 |
| ABI 兼容性 | 跨编译器不稳定 | 可跨 DLL 传递裸指针 |
| 侵入性 | 非侵入，任意类均可 | 需继承 RefCnt<T> |

---

## 2. RefCnt<T> — 引用计数基类（VblRefCnt.h）

    // 节选自 Include/Common/VblRefCnt.h
    template <typename T>
    class RefCnt {
    public:
        virtual inline int AddRef() {
            if (!m_pCtrlBlock) return 0;
            return m_pCtrlBlock->AddRef();
        }
        virtual inline int Release() {
            if (!m_pCtrlBlock) return 0;
            int refCount = m_pCtrlBlock->Release();
            if ((refCount == 0) && (!is_destroying)) {
                is_destroying = true;
                if (IDmProtocol *p = protocol_) { p->Destroy(); }  // COM 风格钩子
                delete this;   // 自我销毁
            }
            return refCount;
        }
    protected:
        RefCnt() : m_pCtrlBlock(new VblControlBlock()), is_destroying(false) {}
        virtual ~RefCnt() { delete m_pCtrlBlock; }
    private:
        VblControlBlock* m_pCtrlBlock;  // 强+弱引用计数
        bool is_destroying;             // 防析构重入
        IDmProtocol* protocol_ = nullptr;
    };

关键设计：
- is_destroying 标志位防止析构期间重入 Release()
- IDmProtocol* protocol_ 是 COM 风格扩展点，对象销毁时回调 Destroy()
- ControlBlock 由 RefCnt 自己分配，与对象同生命周期

---

## 3. SafePtr<T> — 强引用智能指针（VblSafePtr.h）

    // 节选自 Include/Common/VblSafePtr.h
    template <typename T>
    class SafePtr {
    public:
    #define NotAddRef false
        // 构造：默认 AddRef，NotAddRef 可跳过（工厂函数返回场景）
        explicit SafePtr(T* obj, bool isAddRef = true) : ref_(obj) {
            if (isAddRef) AddRef();
        }
        SafePtr() : ref_(nullptr) {}
        SafePtr(const SafePtr& other) : ref_(other.ref_) { AddRef(); }  // 拷贝：AddRef
        ~SafePtr() { Release(); }                                        // 析构：Release
        SafePtr(SafePtr&& other) : ref_(other.ref_) { other.ref_ = nullptr; }
        // 赋值：copy-and-swap 保证异常安全和自赋值安全
        void operator=(const SafePtr& other) {
            if (this == &other) return;
            SafePtr tmp(other);  // 先 AddRef 新对象
            Swap(tmp);           // Swap 后 tmp 析构时 Release 旧对象
        }
    private:
        T* ref_;
    };

**NotAddRef 宏**：工厂函数内 new 对象后，返回 SafePtr 时用 SafePtr(raw, NotAddRef)
跳过 AddRef，防止引用计数翻倍（从 1 变 2）导致内存泄漏。

---

## 4. WeakPtr<T> — 弱引用智能指针（VblWeakPtr.h）

    // 节选自 Include/Common/VblWeakPtr.h
    template <typename T>
    class WeakPtr {
    public:
        explicit WeakPtr(const SafePtr<T>& other) : ref_(other.get()) {
            initControlBlock(ref_);
            WeakAdd();   // 增加弱引用计数
        }
        ~WeakPtr() { WeakRelease(); }
        // 尝试升级为强引用（对象已销毁则返回空 SafePtr）
        SafePtr<T> Lock() const {
            if (m_ctrBlock && m_ctrBlock->IsAlive()) {
                return SafePtr<T>(ref_);   // 成功：AddRef + 返回强引用
            }
            return SafePtr<T>();            // 失败：对象已销毁
        }
        bool IsExpired() const { return !m_ctrBlock || !m_ctrBlock->IsAlive(); }
    private:
        T* ref_;
        VblControlBlock* m_ctrBlock = nullptr;
    };

---

## 5. 强弱引用计数分工

    m_strongCount = 0  -->  销毁对象本体（delete this）
    m_weakCount   = 0  -->  回收 ControlBlock 自身

生命周期演示：

    SafePtr<A> sp(new A);     // strong=1, weak=0
    WeakPtr<A> wp(sp);        // strong=1, weak=1
    sp = SafePtr<A>();        // strong=0 --> delete A; ControlBlock 仍存（weak=1）
    wp.Lock();                // IsAlive()=false --> 返回空 SafePtr（安全）
    // wp 析构 --> weak=0 --> ControlBlock 被回收

---

## 6. 工厂函数完整用法示例

    // 文件: modules/DataModel/DmClipFactory.cpp
    SafePtr<IDmVideoClip> DmClipFactory::CreateVideoClip(const DmMediaInfo& info) {
        VideoClip* raw = new VideoClip(info);
        return SafePtr<IDmVideoClip>(raw, NotAddRef);  // 不再 AddRef，保持计数=1
    }

    // 调用方
    SafePtr<IDmVideoClip> clip = factory->CreateVideoClip(info);  // strong=1
    {
        SafePtr<IDmVideoClip> clip2 = clip;  // strong=2（拷贝 AddRef）
    }  // clip2 析构 --> strong=1
    // clip 析构 --> strong=0 --> delete VideoClip

---

## 7. 面试要点

1. **Intrusive 的核心优势是 ABI 稳定性**：Filmora 是多 DLL 架构，std::shared_ptr
   跨编译器 ABI 不稳定，裸指针+虚函数 AddRef/Release 是 COM 的成熟方案，可安全跨 DLL。

2. **SafePtr 赋值用 copy-and-swap 保证异常安全**：先 AddRef 新对象，再 Swap，
   tmp 析构时自动 Release 旧对象，保证自赋值安全和异常安全。

3. **ControlBlock 分离强弱引用计数**：强引用归零销毁对象本体，弱引用归零才回收
   ControlBlock，确保 WeakPtr 持有 ControlBlock 时可安全调用 IsAlive()。

4. **NotAddRef 是工厂函数的惯用法**：new 对象后工厂用 NotAddRef 返回 SafePtr，
   防止引用计数翻倍（1->2）。这是 COM/intrusive 体系中极常见的模式。

5. **is_destroying 标志防析构重入**：Release() 触发 delete this 期间，析构函数
   可能再次触发 Release()，is_destroying 让第二次进入直接返回。

---

## 8. 可能被追问的问题

**Q1：为什么 VBL 不直接用 std::shared_ptr？**
① ABI 稳定性：跨 DLL 传递 shared_ptr 可能崩溃（不同 CRT 版本），
裸指针+AddRef/Release 是 COM 的成熟方案；
② 性能：一次内存分配 vs 两次；
③ 可与 Windows COM 对象互操作（IUnknown 风格）。

**Q2：WeakPtr::Lock() 如何保证线程安全？**
ControlBlock 的 m_strongCount 是 std::atomic<int>。Lock() 检查 IsAlive() 后
立即原子地 AddRef，保证检查和加计数之间不会归零，避免 TOCTOU 竞态。

**Q3：SafePtr 的性能开销有多大？**
每次拷贝/析构涉及原子操作，约比普通内存访问慢 5~20x。VBL 在热路径
（渲染帧）用裸指针传递，只在所有权边界（存储/返回值）用 SafePtr。

**Q4：RefCnt<T> 和 RefCnt<void> 有什么区别？**
RefCnt<T> 中 delete this 依赖 T 的析构路径。RefCnt<void> 不参与具体类型的 delete，
由虚析构机制统一处理。MsEventBus 多重继承自 IMsEventBus，用 RefCnt<void> 避免冲突。

**Q5：如何防止对象被意外的多次 Release 而 double-free？**
is_destroying 标志位是第一道防线。此外，SafePtr 在 Release() 后会将 ref_ 置为 nullptr，
后续对 nullptr 的 Release 调用会被 AddRef/Release 的 null 检查直接跳过。
