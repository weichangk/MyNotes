interview-cpp.md
# C++ 技术面试知识点

> 基于 Wondershare Filmora Windows 客户端（C++17 + Qt 5.15）真实代码整理
> 项目路径：`E:\dcproject\filmora11-win`

---

## 目录

1. [面向对象：继承与多态](#1-面向对象继承与多态)
2. [智能指针](#2-智能指针)
3. [模板编程](#3-模板编程)
4. [Lambda 与函数对象](#4-lambda-与函数对象)
5. [移动语义与右值引用](#5-移动语义与右值引用)
6. [RAII 资源管理](#6-raii-资源管理)
7. [设计模式](#7-设计模式)
8. [并发与线程](#8-并发与线程)
9. [STL 容器与算法](#9-stl-容器与算法)
10. [C++17 新特性](#10-c17-新特性)

---

## 1. 面向对象：继承与多态

### 核心概念

- **纯虚函数（Pure Virtual Function）**：`= 0` 声明，派生类必须实现
- **接口类（Interface Class）**：只含纯虚函数的抽象基类，用 `I` 前缀命名
- **虚继承（Virtual Inheritance）**：解决菱形继承中的二义性问题

### 项目代码示例

**接口类定义（I 前缀约定）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFVBLModel/IFFAbstractMedia.h
class FFVBLMODEL_EXPORT IFFAbstractMedia : virtual public IFFVBLObject
{
public:
    // 内嵌纯虚回调接口（观察者）
    class OnDestoryDelegate {
    public:
        virtual void onDestroy(IFFAbstractMedia* media) = 0;
    };

    virtual FFVBLMODEL::FFMediaType type() = 0;   // 纯虚：子类必须实现
    virtual QString name() const = 0;
    virtual bool setName(const QString& name) = 0;
};
```

**多继承接口**

```cpp
// 文件：Include/TemplateMode/FVideoTemplate/FVideoTemplatePresenter.h
// 同时继承 FFPresenter、IFFUndoRedoServiceEventObserver、IFFWSIDEventObserver
class FVideoTemplatePresenter : public FFPresenter,
                                public IFFUndoRedoServiceEventObserver,
                                public IFFWSIDEventObserver
{
    Q_OBJECT
    // ...
    void onUndoRedoChangedEvent(FFVBLMODEL::FFDataChangedMode mode) override;
    void onWSIDResponse(FFWSIDResponseCode code) override;
};
```

**观察者连接器中的接口继承**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFObserverHelper.h
class IFFObserverConnector
{
public:
    virtual ~IFFObserverConnector() = default;  // 虚析构，必须！
    virtual void* subject() = 0;
    virtual void* observer() = 0;
    virtual void detach() = 0;
};

// 模板化实现类，多继承两个接口
template<typename Subject, typename Observer>
class FFObserverConnector : public IFFObserverConnector,
                            public IFFDestroyEventObserver
{
    // ...
};
```

### 常见面试题

**Q：纯虚析构函数有什么用？为什么接口类析构一定要是虚的？**

A：接口类若析构不为虚，通过基类指针 `delete` 派生类对象时，只会调用基类析构，导致内存泄漏。项目中所有接口类析构都声明为 `virtual ~IXxx() = default;`。

**Q：虚继承解决什么问题，原理是什么？**

A：解决菱形继承中基类被重复继承的问题。原理：编译器生成 vbptr（虚基类指针），指向虚基类表（vbtable），确保基类只有一份实例。项目中 `IFFAbstractMedia : virtual public IFFVBLObject` 即此用法。

---

## 2. 智能指针

### 核心概念

| 类型 | 所有权 | 使用场景 |
|------|--------|----------|
| `unique_ptr` | 独占 | 资源明确归某一方所有 |
| `shared_ptr` | 共享（引用计数） | 多处共享同一资源 |
| `weak_ptr` | 非拥有（弱引用） | 打破循环引用；观察生命周期 |

### 项目代码示例

**`unique_ptr` 管理线程组（FFThreadPool）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Src/FFAsync/FFThreadPool.h
class FFThreadPool : public QObject, public FFTaskRunnerDelegate
{
private:
    // unique_ptr：线程池对这些资源有独占所有权
    std::unique_ptr<FFThreadGroup> m_backgroundThreadGroup;
    std::unique_ptr<FFThreadGroup> m_commonThreadGroup;
    std::unique_ptr<FFThreadGroup> m_foregroundThreadGroup;
    const std::unique_ptr<FFTaskScheduler> m_taskScheduler;
};
```

**`shared_ptr` + `weak_ptr` 组合（防止悬空指针）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFSubject.h
template<typename Observer>
class FFSubject
{
    struct ObserverWrapper {
        FFSafeSubject<Observer> observer;
    };
    // shared_ptr 持有观察者包装；weak_ptr 遍历时不延长生命周期
    using ObserverWrapperPtr     = std::shared_ptr<ObserverWrapper>;
    using ObserverWrapperWeakPtr = std::weak_ptr<ObserverWrapper>;

    // m_existMark：用于在 notify 过程中检测 Subject 自身是否已被销毁
    mutable std::shared_ptr<char> m_existMark = std::make_shared<char>();

    inline bool notifyAny(const std::function<bool(Observer*)>& notifer) const noexcept
    {
        std::weak_ptr<char> bExist = m_existMark;   // 弱引用捕获
        // ...遍历过程中通过 bExist.lock() 判断 Subject 是否还存活
        if (!bExist.lock()) break;  // Subject 已销毁，终止通知
    }
};
```

**跨线程安全释放 QObject（`shared_ptr` 自定义删除器）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h
// 关键：QFutureWatcher 是 QObject，不能在非主线程 delete
// 用 deleteLater 作自定义删除器，确保在正确线程释放
watcherPtr = std::shared_ptr<QFutureWatcher<T>>(
    new QFutureWatcher<T>,
    std::bind(&QObject::deleteLater, std::placeholders::_1)  // 自定义删除器！
);
```

**`shared_ptr` 作函数参数（共享上下文）**

```cpp
// 文件：Include/TemplateMode/FTemplateView/FTemplateMediaItemIconView.h
FTemplateMediaItemIconView(
    std::shared_ptr<FMediaContext> context,  // 多个 View 共享同一 context
    FFUIView* parent = nullptr
);
```

### 常见面试题

**Q：`shared_ptr` 循环引用如何解决？**

A：用 `weak_ptr` 打破环。项目中 `FFSubject` 用 `ObserverWrapperWeakPtrList` 弱引用存储观察者，避免 Subject 持有观察者导致循环引用，遍历时先 `.lock()` 升级为 `shared_ptr` 再使用。

**Q：`shared_ptr` 的线程安全性如何？**

A：引用计数的增减是原子操作（线程安全），但被管理对象的访问不是线程安全的——需要额外加锁。项目中 `FFSubject` 的 `attach/detach/notify` 都配合 `std::recursive_mutex` 使用。

**Q：为什么使用自定义删除器？**

A：见上述 `FFFuture` 代码——`QFutureWatcher` 是 `QObject`，必须在创建它的线程中销毁，使用 `deleteLater` 作删除器，让 Qt 事件循环在正确线程上执行销毁，避免跨线程 `delete` 崩溃。

---

## 3. 模板编程

### 核心概念

- **类模板**：泛化数据结构或算法
- **函数模板**：泛化函数行为
- **变参模板（Variadic Templates）**：接受任意数量类型参数
- **SFINAE**：通过 `std::enable_if` 在编译期选择重载
- **类型萃取**：`std::invoke_result`、`std::is_void_v`、`std::is_polymorphic`

### 项目代码示例

**类模板 + 变参模板 + `std::invoke`**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFSubject.h
template<typename Observer>
class FFSubject
{
public:
    // 变参模板：可传入成员函数指针和任意参数
    // 用法：subject.notify(&IObserver::onEvent, arg1, arg2);
    template <typename Func, typename... Args>
    inline void notify(Func func, Args&&... args) const
    {
        auto functor = [&](Observer* observer)
        {
            std::invoke(func, observer, std::forward<Args>(args)...);
            return true;
        };
        notifyAny(functor);
    }
};
```

**SFINAE + `std::enable_if`（按返回类型选择不同重载）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h

// 当 ParentResultType 不为 void 时调用此版本（传参数）
template<typename ParentResultType1>
typename std::enable_if<!std::is_void_v<ParentResultType1>>::type
callFunctionWithReturnResult()
{
    promise.reportResult(function(parentFuture.result()));
}

// 当 ParentResultType 为 void 时调用此版本（无参数）
template<typename ParentResultType2>
typename std::enable_if<std::is_void_v<ParentResultType2>>::type
callFunctionWithReturnResult()
{
    promise.reportResult(function());
}
```

**`using` 类型别名模板（推导 then 的返回类型）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h
// C++17 版本
#if _HAS_CXX17
template <typename F, typename _R = T>
using return_type_t = typename std::conditional_t<
    std::is_void_v<_R>,
    std::invoke_result<std::decay_t<F>>,
    std::invoke_result<std::decay_t<F>, _R>>::type;
#endif

// 使用：自动推导 then 之后的类型
template <typename Function>
FFFuture<return_type_t<Function>> then(Function &&func)
{
    return then(nullptr, std::forward<Function>(func));
}
```

### 常见面试题

**Q：`std::forward` 和 `std::move` 的区别？**

A：
- `std::move`：无条件转为右值引用，用于明确表达"移走"语义
- `std::forward`：完美转发，保留原始值类别（左值传左值，右值传右值）；在模板函数中配合 `&&` 参数使用，如 `then(Function &&func)` 中 `std::forward<Function>(func)`

**Q：`std::invoke` 的作用？**

A：统一调用普通函数、成员函数指针、函数对象、lambda；`std::invoke(func, observer, args...)` 等价于 `(observer->*func)(args...)`，使模板代码与调用方式无关。

---

## 4. Lambda 与函数对象

### 核心概念

- 捕获方式：值捕获 `[=]`、引用捕获 `[&]`、混合 `[x, &y]`、移动捕获 `[x = std::move(x)]`（C++14）
- `mutable` lambda：使值捕获的拷贝可修改
- `std::function`：类型擦除的通用函数包装器

### 项目代码示例

**lambda 作为异步 continuation（跨线程安全捕获）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h

// context 不为空时：在 context 所在线程执行回调（Qt 主线程）
auto obj_continuation = [
    func    = std::forward<Function>(func),   // 移动语义捕获
    promise,
    context = QPointer<QObject>(context)      // 弱引用，防止 context 销毁后访问
](QFuture<T> parent) mutable {

    QMetaObject::invokeMethod(context,
        [func = std::move(func), promise = std::move(promise), parent]() mutable {
            SyncContinuation<...> job(std::forward<Function>(func), parent, std::move(promise));
            job.execute();
        }
    );

    if (context.isNull()) {  // context 已销毁，取消 future
        promise.reportStarted();
        promise.reportCanceled();
        promise.reportFinished();
    }
};
```

**lambda 作信号槽回调**

```cpp
// 文件：Src/TemplateMode/FTemplateView/FTemplateMediaItemIconView.cpp

// 引用捕获 this，触发控件重绘
connect(itemDelegate, &FMediaItemIconDelegate::sigRepaint, this, [&]() {
    update();
});

// 捕获 this 并调用成员函数
connect(m_model, &FTemplateMediaItemModel::sigFetchMoreSuccess, this, [&]() {
    restartCheckResourceThumbnailTimer();
});
```

**`std::function` 作回调存储**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h
// 存储 continuation 回调，类型无关
std::function<void(QFuture<T>)> m_continuation;

// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFSubject.h
// 接受任意可调用对象
inline bool notifyAny(const std::function<bool(Observer*)>& notifer) const noexcept;
```

### 常见面试题

**Q：lambda 捕获 `this` 和 `[=]` 捕获 this 的区别？**

A：`[this]` 捕获的是指针（与 `[=]` 默认捕获 this 指针相同），若对象在 lambda 执行前析构则为悬空指针。项目中使用 `QPointer<QObject>(context)` 解决此问题——`QPointer` 在对象销毁后自动置 `null`，lambda 通过 `context.isNull()` 检测安全。

**Q：什么场景用 `mutable` lambda？**

A：当用值捕获（`[=]` 或 `[x = std::move(x)]`），但需要在 lambda 内修改该副本时加 `mutable`。项目中 continuation lambda 用 `mutable`，因为内部会 `std::move(func)` 和 `std::move(promise)`。

---

## 5. 移动语义与右值引用

### 核心概念

- **右值引用 `T&&`**：绑定到临时对象，可"窃取"其资源
- **移动构造 / 移动赋值**：转移资源所有权，避免深拷贝
- **完美转发**：`std::forward<T>(arg)` 保留值类别

### 项目代码示例

**移动语义转移 `QFutureInterface`（避免拷贝）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h
template<typename Function, typename ResultType, typename ParentResultType>
class SyncContinuation
{
public:
    // 接受右值引用，用 move 转移大对象所有权
    SyncContinuation(Function &&func,
                     const QFuture<ParentResultType> &f,
                     QFutureInterface<ResultType> &&p)   // p 用右值引用
        : promise(std::move(p))                          // 移动构造
        , parentFuture(f)
        , function(std::forward<Function>(func))         // 完美转发
    {}
};
```

**Lambda 内 `std::move` 转移捕获变量**

```cpp
// context 为 nullptr 的 continuation（无需线程切换）
auto continuation = [
    func    = std::forward<Function>(func),  // 完美转发到捕获
    promise                                   // 按值捕获
](QFuture<T> parent) mutable {
    SyncContinuation<Function, result_type, T> continuationJob(
        std::forward<Function>(func),
        parent,
        std::move(promise)    // 转移 promise 所有权
    );
    continuationJob.execute();
};
```

### 常见面试题

**Q：`std::move` 之后原对象的状态如何？**

A：处于"有效但未指定"状态（valid but unspecified），可以赋新值，但不能直接使用。项目中 `std::move(promise)` 之后 promise 变为"空"的 `QFutureInterface`，不再使用它。

**Q：什么情况下不需要写移动构造函数？**

A：若类成员全为支持移动的类型（`unique_ptr`、`vector` 等），编译器自动生成移动构造。项目中 `FFThreadPool` 的成员全是 `unique_ptr`，无需手写移动构造。

---

## 6. RAII 资源管理

### 核心概念

RAII（Resource Acquisition Is Initialization）：构造时获取资源，析构时释放资源，利用栈上对象生命周期自动管理资源。

### 项目代码示例

**析构自动 detach 观察者（FFObserverHelper）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFObserverHelper.h
class FFObserverHelper
{
public:
    ~FFObserverHelper()
    {
        detach();  // 析构时自动解除所有观察者注册，防止野指针通知
    }
    // ...
};
```

**析构自动断连（FFObserverConnector）**

```cpp
template<typename Subject, typename Observer>
class FFObserverConnector : public IFFObserverConnector
{
public:
    ~FFObserverConnector() override
    {
        detach();  // 析构时调 Subject::detach，RAII 确保不遗漏
    }
};
```

**互斥锁 RAII（`lock_guard`）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFSubject.h
void attach(Observer* observer)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);  // 构造加锁，析构解锁
    // 即使中途异常，lock_guard 析构也会释放锁
    m_observers.append(...);
}
```

### 常见面试题

**Q：`lock_guard` 和 `unique_lock` 的区别？**

A：`lock_guard`：轻量、不可移动、不可手动解锁，适合简单临界区。`unique_lock`：可移动（能放入容器）、可手动 `lock/unlock`、支持条件变量，灵活但稍重。项目中用 `lock_guard` 保护 `attach/detach` 等简单操作。

---

## 7. 设计模式

### 7.1 观察者模式（Observer Pattern）

项目中实现了一套完整的、生产级的观察者框架，位于 `FFCore` 层。

**核心设计**：`FFSubject<Observer>`（被观察者模板）+ `FFObserverHelper`（自动 attach/detach 助手）+ `FFObserverConnector`（单个连接 RAII 对象）

```
FFSubject<IObserver>          FFObserverHelper
    │  attach(observer)           │  attach(subject, observer)
    │  detach(observer)     ─────►│      ├─ 创建 FFObserverConnector
    │  notify(&IObserver::func)   │      └─ 析构时自动 detach
    └─ ObserverWrapperList        └─ IFFObserverConnector 列表
```

```cpp
// 典型使用：Presenter 注册监听 Service
class MyPresenter : public IMyServiceObserver
{
    FFObserverHelper m_observerHelper;
    void init(MyService* service) {
        m_observerHelper.attach(service, this);  // 自动管理生命周期
    }
    // MyPresenter 析构时 m_observerHelper 析构，自动 detach
};
```

### 7.2 PIMPL 模式（Pointer to Implementation）

隐藏实现细节，减少编译依赖，实现 ABI 稳定。

```cpp
// 文件：Include/TemplateMode/FVideoTemplate/FVideoTemplatePresenter.h

// 头文件只暴露接口，Private 类定义在 .cpp 中
class FVideoTemplatePresenter : public FFPresenter
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(FVideoTemplatePresenter)   // Qt 提供的 PIMPL 宏

private:
    QScopedPointer<FVideoTemplatePresenterPrivate> d;  // 不透明指针
    Q_DISABLE_COPY(FVideoTemplatePresenter)            // 禁止拷贝
};
```

### 7.3 工厂模式（Factory Pattern）

```cpp
// 线程任务运行器工厂方法（FFThreadPool.h）
std::shared_ptr<FFTaskRunner>
    createParallelTaskRunner(const FFTasksFeature& task_feature);

std::shared_ptr<FFQueueTaskRunner>
    createQueueTaskRunner(const FFTasksFeature& task_feature);

std::shared_ptr<FFThreadTaskRunner>
    createThreadTaskRunner(const FFTasksFeature& task_feature);
```

### 7.4 单例模式（Singleton Pattern）

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Src/FFAsync/FFThreadPool.h
class FFThreadPool : public QObject
{
public:
    static FFThreadPool* getThreadPool();  // 全局访问点
    static void create();
};

// FUserGuideManager 等多处均有 getInstance() 模式
static FUserGuideManager* getInstance();
```

### 7.5 策略模式（Strategy Pattern）

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Src/FFAsync/FFThread.h
// 线程入口由外部 Delegate 决定（策略）
class FFThread
{
public:
    class Delegate {
    public:
        virtual ~Delegate();
        virtual void entryThreadMain();  // 策略接口：线程执行内容由外部决定
    };
    FFThread(Delegate* delegate, const QString& threadName);
};
```

---

## 8. 并发与线程

### 核心概念

- `std::thread`：标准线程，平台无关
- `std::mutex` / `std::recursive_mutex`：互斥量
- `std::lock_guard` / `std::unique_lock`：RAII 锁
- `std::future` / `std::promise`：异步结果传递

### 项目架构

```
FFThreadPool（线程池单例）
    ├── m_backgroundThreadGroup   ← 后台低优先级任务
    ├── m_commonThreadGroup       ← 普通任务
    └── m_foregroundThreadGroup   ← 前台高优先级任务
          │
          └── postTask(closure, feature)
              createParallelTaskRunner()
              createQueueTaskRunner()
```

**平台抽象：条件编译 QThread vs std::thread**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Src/FFAsync/FFThread.h
class FFThread
#ifdef USE_QT_THREAD
    : public QThread          // Qt 平台使用 QThread
#endif
{
private:
#ifndef USE_QT_THREAD
    std::unique_ptr<std::thread> m_thread;  // 标准 C++ 线程
#endif
};
```

**recursive_mutex 防止死锁**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFSubject.h
// recursive_mutex：同一线程可多次加锁，防止 attach 内再调 attach 死锁
mutable std::recursive_mutex m_lock;

void attach(Observer* observer) {
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    // ...
}
void detach(Observer* observer) {
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    // ...
}
```

**FFFuture：Qt 风格的异步 then 链**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h
// 用法示例（假设）：
make_fffuture(QtConcurrent::run(heavyTask))
    .then([](ResultType result) {   // 任务线程回调
        return processResult(result);
    })
    .then(mainWidget, [](ProcessedType r) {  // 切换到 mainWidget 所在线程（主线程）
        ui->label->setText(r);
    });
```

### 常见面试题

**Q：`recursive_mutex` 什么情况下用？**

A：同一线程可能递归地请求同一锁时。普通 mutex 在同一线程二次 lock 时死锁，`recursive_mutex` 记录加锁次数，允许重入，解锁次数需与加锁次数匹配。缺点：性能稍差，掩盖设计问题，应尽量避免。

**Q：如何跨线程安全地操作 QObject？**

A：QObject 不能在非所属线程中直接调用方法。项目中的方案：
1. `QMetaObject::invokeMethod(context, lambda)` ——将 lambda 发送到 context 所在线程的事件队列执行（FFFuture::then 的实现）
2. 信号槽跨线程连接时使用 `Qt::QueuedConnection`

---

## 9. STL 容器与算法

### 项目中的典型用法

**`std::optional` 替代"魔法返回值"**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFSubject.h
// 返回 optional 而不是 -1 或 nullptr，语义更清晰
std::optional<int> indexOf(Observer* observer) const
{
    for (int i = 0; i < m_observers.size(); ++i) {
        if (observer == m_observers[i]->observer.orgData())
            return i;           // 找到：返回有值的 optional
    }
    return std::nullopt;        // 未找到：返回空 optional
}

// 调用侧：
if (const auto idx = indexOf(observer))    // 隐式 bool 转换
{
    m_observers.removeAt(idx.value());     // 取值用 .value()
}
```

**`using` 定义类型别名（提高可读性）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFVBLModel/IFFAbstractMedia.h
using DownloadInfo = std::pair<quint64, quint64>;  // <已下载字节, 总字节>
```

**Qt 容器与 STL 容器混用**

```cpp
// FFSubject 中混用 QList 和 std::shared_ptr / std::weak_ptr
QList<std::shared_ptr<ObserverWrapper>>  m_observers;      // QList 存 shared_ptr
QList<std::weak_ptr<ObserverWrapper>>    copyObservers;    // 遍历时用弱引用拷贝
```

---

## 10. C++17 新特性

### 10.1 `if constexpr`（编译期条件分支）

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFObserverHelper.h
// 编译期根据类型特征选择不同的 dynamic_cast 路径
void initSubject(Subject* subject)
{
    // 非多态类型（无虚函数表）在编译期跳过此分支
    if constexpr(std::is_polymorphic<Subject>::value)
    {
        if (FFObject* obj = dynamic_cast<FFObject*>(subject))
            m_pSafeFFObjSubject = obj;
        else if (QObject* subObj = dynamic_cast<QObject*>(subject))
            m_pSafeQObjSubject = subObj;
    }
}
```

对比旧写法（运行期 if）：非多态类型会在运行期执行 dynamic_cast（可能导致 UB），`if constexpr` 则在编译期剪枝，更安全更高效。

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFObserverHelper.h
// 编译期检测 Subject 是否有 addObserver/removeObserver 方法（SFINAE 宏展开）
if constexpr(has_member_addObserver<Subject, IFFDestroyEventObserver*>::value
          && has_member_removeObserver<Subject, IFFDestroyEventObserver*>::value)
{
    m_pSubject->addObserver(this);  // 只有 Subject 有这两个方法时才编译此代码
}
```

### 10.2 `std::optional`

见第 9 节 `FFSubject::indexOf` 示例。

### 10.3 结构化绑定（Structured Bindings）

```cpp
// 文件：Src/FFObserverHelper.h（attach 方法）
// C++17：if 初始化语句 + 结构化绑定
if (const auto idx = indexOf(subject, observer); idx != -1)
{
    return true;  // 已注册
}
```

### 10.4 `std::invoke_result` / `std::decay_t`

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h
// 推导函数调用返回类型，支持 void 和非 void 两种情况
template <typename F, typename _R = T>
using return_type_t = typename std::conditional_t<
    std::is_void_v<_R>,
    std::invoke_result<std::decay_t<F>>,        // F() 无参调用
    std::invoke_result<std::decay_t<F>, _R>     // F(_R) 有参调用
>::type;
```

### 常见面试题

**Q：`if constexpr` 和普通 `if` 的区别？**

A：`if constexpr` 的条件在编译期求值，未选中的分支**不参与编译**（不产生代码，也不进行类型检查）。普通 `if` 两个分支都会编译。这使得模板代码可以写出在特定类型下才有效的代码，而不需要模板特化。

**Q：C++17 之前如何实现 `if constexpr` 的效果？**

A：通过模板特化或标签分发（tag dispatch）实现编译期分支。C++17 的 `if constexpr` 大幅简化了此类代码。

---

## 附录：项目中涉及的核心文件索引

| 文件 | C++ 知识点 |
|------|------------|
| `3rdparty/.../FFCore/FFSubject.h` | 类模板、变参模板、std::optional、recursive_mutex、shared_ptr/weak_ptr |
| `3rdparty/.../FFCore/FFObserverHelper.h` | if constexpr、多继承、RAII、QPointer |
| `3rdparty/.../FFCore/FFFuture.h` | SFINAE、移动语义、lambda 捕获、自定义删除器 |
| `3rdparty/.../FFAsync/FFThread.h` | 策略模式、条件编译、std::thread vs QThread |
| `3rdparty/.../FFAsync/FFThreadPool.h` | 单例、工厂、unique_ptr |
| `3rdparty/.../FFWidgets/FFEventFilter.h` | 纯虚接口、事件过滤器 |
| `Include/.../FVideoTemplatePresenter.h` | PIMPL、多继承、Q_DECLARE_PRIVATE |
