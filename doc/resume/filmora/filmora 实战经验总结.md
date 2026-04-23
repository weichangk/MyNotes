# 实战经验总结

> 基于 Filmora Windows 客户端（C++17 + Qt 5.15）项目实战经验整理
> 覆盖：C++17 技术点 / Qt 框架深度 / 设计模式实践 / 多进程/IPC 架构

---

## 一、C++17 技术点

### 1.1 智能指针的选择与实践

**项目中同时使用了三种智能指针，各有其适用场景：**

```cpp
// FilmoraApp.h 中的对比：

// std::unique_ptr：独占所有权，无法共享，轻量
std::unique_ptr<FFWccHelper> m_wccHelper;

// std::shared_ptr：共享所有权，引用计数，可多处持有
std::shared_ptr<FFScheduleProject> m_taskWatcher;
std::shared_ptr<FFScheduleProject> m_pPrepareRunTask;

// QScopedPointer：Qt 风格的独占指针，在 Qt 对象树中更自然
QScopedPointer<FFAsync::FFAsyncInitializer> m_pInitializer;
QScopedPointer<FilmoraAppEventHandler> m_pEventHandler;

// std::weak_ptr：弱引用，防止循环引用（FFAppLicense 中）
std::weak_ptr<IFFAppStoreLicense> m_pAppstoreLicense;
```

**ms要点：**
- `unique_ptr` 零开销（无引用计数），优先使用
- `shared_ptr` 有线程安全的引用计数，但有开销；适用于生命周期不确定的共享对象
- `weak_ptr` 打破循环引用：A 持有 B 的 `shared_ptr`，B 持有 A 的 `weak_ptr`
- `QScopedPointer` 与 `unique_ptr` 功能类似，但不支持移动语义，在 Qt 老代码中常见
- **常见陷阱**：`shared_ptr` 的 `this` 指针问题 → 需要继承 `std::enable_shared_from_this`

---

### 1.2 std::atomic_bool — 无锁标志位

```cpp
// FilmoraApp.h
std::atomic_bool m_bNeedResetFolder = false;
```

**使用场景：** 后台线程（如 IPC 回调线程）写入标志，主线程读取。

**ms要点：**
- `std::atomic<bool>` 保证读写的原子性，防止数据竞争
- 相比 `QMutex` 加锁，原子操作无锁竞争，性能更高
- 适用于"单写单读"或"多读少写"的简单标志
- `std::memory_order`：默认 `memory_order_seq_cst`（最强保证），性能要求高时可放宽

---

### 1.3 Lambda 表达式与捕获

项目中大量使用 Lambda，尤其在异步任务中：

```cpp
// FFAsync 异步任务（FFThumbnailCache.cpp 模式）
FFAsync::FFAsync::postReplyableResultTask(
    [mediaItem]() -> QImage {              // 值捕获：后台线程执行，mediaItem 生命周期安全
        return decodeFirstFrame(mediaItem);
    },
    [this, mediaItem](QImage thumbnail) {  // this 捕获：注意对象生命周期！
        updateThumbnailDisplay(thumbnail);
    }
);

// QtConcurrent 中的 Lambda（AntiPiracyHelper.cpp）
QtConcurrent::run([this]() {
    performAntiPiracyCheck();
});
```

**ms要点：**
- `[=]` 值捕获所有局部变量，`[&]` 引用捕获（注意悬空引用风险）
- 在异步回调中捕获 `this` 很危险：对象可能在 Lambda 执行前已被销毁
- 安全做法：捕获 `std::weak_ptr<ThisType>` 而非裸 `this`，回调时 `lock()` 检查
- `QPointer<T>` 是 Qt 中 `weak_ptr` 的等价物，专用于 `QObject` 派生类

---

### 1.4 结构化绑定（C++17）

```cpp
// C++17 结构化绑定在项目中的典型用法
// （虽然项目大量使用 QMap，但 C++17 范围 for 更简洁）

// 遍历 QMap
for (auto& [key, value] : downloadings) {
    processDownload(key, value);
}

// 遍历 std::map
for (const auto& [slugId, progress] : m_progressMap) {
    updateUI(slugId, progress);
}
```

---

### 1.5 移动语义在消息传递中的应用

```cpp
// 避免大对象拷贝（如包含 QImage 的消息）
struct FFExportResult {
    QImage previewFrame;  // 大对象
    QString outputPath;
};

// 移动构造传递，避免深拷贝
void onExportCompleted(FFExportResult&& result) {
    m_result = std::move(result);  // 移动，不拷贝
}
```

---

### 1.6 if constexpr 与跨平台条件编译

```cpp
// 项目中的跨平台处理（FilmoraApp.h）
#ifdef Q_OS_WIN
    std::unique_ptr<FFWccHelper> m_wccHelper;
#endif
#ifdef Q_OS_MAC
    QString m_strProjectPath;
    bool m_bMetalEnable = true;
#endif
```

**现代 C++ 方式：**
```cpp
// C++17 if constexpr（在模板中更优雅）
template<typename PlatformHelper>
void initPlatformSpecific() {
    if constexpr (std::is_same_v<PlatformHelper, WinHelper>) {
        // Windows 专属逻辑，编译期确定
    } else {
        // macOS 专属逻辑
    }
}
```

---

### 1.7 模板懒加载单例（FFLazySingleton）

```cpp
// 项目自实现的线程安全懒加载单例模板
template<typename T>
class FFLazySingleton {
public:
    static T* getInstance() {
        static T instance;  // C++11 保证静态局部变量初始化线程安全
        return &instance;
    }
protected:
    FFLazySingleton() = default;
    // 防止拷贝
    FFLazySingleton(const FFLazySingleton&) = delete;
    FFLazySingleton& operator=(const FFLazySingleton&) = delete;
};

// 使用：FAICopilotService、FFAppLicense
FAICopilotService::getInstance()->postMessage(msg);
```

**ms要点：**
- C++11 之后静态局部变量的初始化是线程安全的（编译器保证）
- `= delete` 禁用拷贝构造和拷贝赋值，彻底防止拷贝
- `friend class FFLazySingleton<T>` 让基类可以调用 `protected` 构造函数

---

## 二、Qt 框架深度

### 2.1 信号槽机制深度解析

**底层原理（moc 元对象系统）：**
```
Q_OBJECT 宏 + moc 工具
    │
    ├── 生成 moc_ClassName.cpp
    ├── 包含 metaObject()、qt_metacall() 实现
    └── 信号函数体由 moc 生成（调用 QMetaObject::activate）

connect(sender, SIGNAL(xxx()), receiver, SLOT(yyy()))
// ↑ 运行时字符串解析（老式写法）

connect(sender, &Sender::xxx, receiver, &Receiver::yyy)
// ↑ 编译期类型检查（现代写法，推荐）
```

**跨线程信号槽：**
```cpp
// Qt::QueuedConnection — 跨线程安全
// 信号在发送线程发出，槽在接收对象所属线程的事件循环中执行
connect(workerThread, &Worker::resultReady,
        this, &MainWindow::updateUI,
        Qt::QueuedConnection);  // 线程间跨越，Qt 自动队列化
```

**ms要点：**
- `Qt::DirectConnection`：同线程，直接调用（默认）
- `Qt::QueuedConnection`：跨线程，事件队列投递
- `Qt::BlockingQueuedConnection`：跨线程，调用方阻塞等待槽执行完成（慎用，易死锁）
- 为什么 `Q_OBJECT` 必须在第一个 private 位置？→ moc 解析要求

---

### 2.2 线程池调优实战

```cpp
// Src/FCore/FApplication.cpp
int count = QThreadPool::globalInstance()->maxThreadCount();
if (count < 16) {
#ifdef Q_OS_WIN
    QThreadPool::globalInstance()->setMaxThreadCount(16);
#else
    QThreadPool::globalInstance()->setMaxThreadCount(24);
#endif
}
```

**问题场景：**
- 4 核机器默认线程池容量 = 4
- FFAsync、QPixmap、QtConcurrent 等同时竞争线程
- 常驻后台任务（如缩略图生成队列）占满线程池
- `QPixmap::load()` 申请线程失败 → 主线程卡死

**解决思路：**
1. 线程池容量扩展（本项目采用）
2. 为关键任务使用 `FThreadPool`（独立池，不与全局竞争）
3. 设置 `QRunnable::setAutoDelete(false)` + 手动管理生命周期

---

### 2.3 QPointer 安全持有 UI 对象

```cpp
// FilmoraApp.h（macOS 专属）
QPointer<IFCBSUpdater> m_pCbsMetalUpdate;

// 使用方式
if (m_pCbsMetalUpdate) {        // QPointer 自动检查指针是否有效
    m_pCbsMetalUpdate->start();
}
// QObject 被删除后，QPointer 自动置 nullptr（不会悬空）
```

**与裸指针的区别：**
- 裸指针：对象被删除后变为悬空指针，访问导致 UB
- `QPointer`：对象删除后自动变为 `nullptr`，安全

**适用场景：** 异步回调中持有 UI 对象的弱引用（如后台任务完成后更新 UI，但 UI 可能已关闭）

---

### 2.4 Qt 事件过滤器（event/notify 重写）

```cpp
// FilmoraApp.h
bool notify(QObject *obj, QEvent *event) override;
bool event(QEvent *) override;

// FilmoraApp.cpp 中 FilmoraAppEventHandler 实现 IFFMessageFilter
bool onFilterEvent(FFMessage* msg) override {
    auto pActiveScene = FF_UI_SCENE_MGR->activeScene();
    if (msg->isApplicationMessage()) return false; // 全局命令不过滤
    if (auto pActiveModuleWdg = QApplication::activeModalWidget()) {
        if (widgetToScene(pActiveModuleWdg) == pActiveScene)
            return false; // 当前场景可处理
        return true;      // 丢弃其他场景的命令
    }
    return false;
}
```

**ms要点：**
- `QApplication::notify()` 是所有 Qt 事件的统一入口，重写可做全局异常处理
- `QObject::installEventFilter()` 可在不修改目标类的情况下拦截其事件
- 本项目用自定义消息过滤器实现"多场景隔离"（模板编辑模式不响应普通编辑命令）

---

### 2.5 高 DPI 适配

```cpp
// main.cpp
FilmoraApp::initHighDPI();  // 必须在 QApplication 构造之前调用

// FApplication.cpp
QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
// 同时在 main 前设置：
// QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
```

**ms要点：**
- Qt 5.x 高 DPI 支持需要两个属性：`AA_EnableHighDpiScaling`（布局缩放）+ `AA_UseHighDpiPixmaps`（图片缩放）
- Qt 6.x 默认开启高 DPI，无需手动设置
- 自定义绘制（`QPainter`）需要使用 `devicePixelRatio()` 获取缩放比，手动处理

---

### 2.6 多线程方案对比

项目中同时使用了三种多线程方案：

| 方案 | 使用场景 | 优缺点 |
|------|---------|-------|
| `QThread` | 长期运行的后台线程（`FSystemMonitorThread`）| 灵活，控制精确；需手动管理生命周期 |
| `QtConcurrent::run` | 简单的一次性后台任务（反盗版校验）| 简单易用；使用全局线程池，无法精细控制 |
| `FFAsync::postTask` | 异步任务队列（缩略图、网络请求）| 支持回调、可返回结果；依赖 FFAsync 库 |

**最佳实践：**
- 短期任务用 `QtConcurrent` 或 `FFAsync`
- 常驻线程用 `QThread` + `worker object` 模式（将 Worker 对象 `moveToThread`）
- 避免直接继承 `QThread` 并重写 `run()`（违反 Qt 设计哲学）

---

## 三、设计模式实践

### 3.1 模板方法模式（Template Method）

```cpp
// FApplication 定义框架（run() 方法）
int FApplication::run() {
    // 模板方法，定义算法骨架
    init();          // [虚函数] 子类重写
    prepareRun();    // [虚函数] 子类重写
    doRun();         // [虚函数] 子类重写
    runFinished();   // [虚函数] 子类重写
    unInit();        // [虚函数] 子类重写
    return nRst;
}

// FilmoraApp（具体子类）重写各步骤
void FilmoraApp::init() override {
    // 具体初始化逻辑（20+ 个 initXXX 调用）
}
```

**价值：**
- `FApplication` 只定义"流程"，不关心具体"内容"
- 不同应用（主程序 `FilmoraApp`、子进程 `FEventTrackingApp`、`FExportApp`）共享相同启动流程
- 开闭原则：流程对扩展开放，对修改关闭

---

### 3.2 观察者模式（Observer）

```cpp
// ffcore::ObserverContainer<T> 的使用
ffcore::ObserverContainer<IFStartedAppObserver> m_startedAppObserverList;

// 注册
void FilmoraApp::addStartedAppObserver(IFStartedAppObserver* observer) {
    m_startedAppObserverList.addObserver(observer);
}

// 通知
void FilmoraApp::notifyRunFinishBegin() {
    m_startedAppObserverList.notifyObservers([](IFStartedAppObserver* obs) {
        obs->onAppStarted();
    });
}

// 观察者接口
class IFStartedAppObserver {
public:
    virtual void onAppStarted() = 0;
};
```

**与 Qt 信号槽的区别：**
| 维度 | ObserverContainer | Qt 信号槽 |
|------|-----------------|---------|
| 类型安全 | 接口约束，编译期检查 | 老式写法运行期检查 |
| 多态 | 通过虚函数实现 | 通过 moc 元对象实现 |
| 线程安全 | 需手动保证 | Qt 自动处理（QueuedConnection） |
| 适用场景 | 跨模块事件通知 | 同模块 UI 交互 |

---

### 3.3 单例模式三种变体

```cpp
// 变体 1：FFLazySingleton（线程安全的懒加载，推荐）
class FAICopilotService : public FFLazySingleton<FAICopilotService> {};
FAICopilotService::getInstance()->postMessage(msg);

// 变体 2：GetInstance 静态方法（传统单例）
AntiPiracyMgr::GetInstance()->Check();

// 变体 3：ffcore::FFDisableDestructor（防析构单例）
// 用于程序退出时仍需访问的对象（防止析构顺序问题）
static ffcore::FFDisableDestructor<FActiveLineSession> s_instance;
return s_instance.get();
```

**ms陷阱：**
- "双重检查锁定（DCLK）"在 C++11 之前有 UB 风险（指令重排）
- C++11 后静态局部变量初始化线程安全，推荐使用（变体 1 的原理）
- `FFDisableDestructor` 解决了"全局/静态析构顺序不确定"导致的 use-after-free

---

### 3.4 工厂方法模式（Factory Method）

```cpp
// IFFMediaLibrary 的工厂方法
IFFMediaLibrary* pLib = IFFMediaLibrary::createMediaLibrary();

// 意义：
// 1. 主进程和导出子进程（FExportExe）都需要媒体库，但可能使用不同的实现
// 2. 工厂方法隐藏实现细节，调用方只依赖接口
// 3. 便于测试时注入 Mock 实现
```

---

### 3.5 MVP（Model-View-Presenter）模式

项目全面采用 MVP 分离关注点：

```
Model                      View                    Presenter
─────                      ────                    ─────────
IFFMediaLibrary      FMediaLibraryView     FFMediaLibraryPresenter
IFFTimeline          FTimelineView         FTimelinePresenter
IFFCloudDisk         FFCloudDiskView       (内嵌于 FFCloudDisk)

┌──────────┐         ┌──────────┐         ┌──────────────┐
│  Model   │◄────────│Presenter │─────────►│    View      │
│（业务数据）│  接口调用│（逻辑协调）│ 更新显示  │（Qt UI 控件）│
└──────────┘         └──────────┘◄─────────└──────────────┘
                                  用户事件
```

**Presenter 的职责：**
- 响应 View 的用户事件（点击、拖拽）
- 调用 Model（接口）的业务方法
- 将 Model 返回的数据转换为 View 可显示的格式
- 协调多个 View 和 Model 之间的交互

---

### 3.6 RAII 资源管理

```cpp
// QMutexLocker 实现 RAII 加锁
void FThreadPool::GetThread() {
    QMutexLocker locker(&m_mutex);  // 构造时加锁，析构时自动解锁
    // 即使中途 return 或抛出异常，锁也会被正确释放
    ...
}

// QScopedPointer 实现 RAII 内存管理
QScopedPointer<FFAsync::FFAsyncInitializer> m_pInitializer;
// FilmoraApp 析构时，m_pInitializer 自动 delete

// FTryLocker（项目自定义）
class FTryLocker {
    FTryLocker(QMutex* mutex) : m_bLocked(mutex->tryLock()) {}
    ~FTryLocker() { if (m_bLocked) m_pMutex->unlock(); }
};
```

---

## 四、多进程/IPC 架构

### 4.1 ms高频：为什么选择多进程而非多线程？

**核心答案：稳定性 > 性能**

| 场景 | 多进程方案 | 多线程方案 |
|------|---------|---------|
| 视频编码崩溃 | 只有 FExportExe 崩溃，用户可继续编辑项目 | 整个 Filmora 崩溃，丢失未保存工作 |
| 内存泄漏 | 子进程退出后内存完全释放 | 泄漏累积直到主进程重启 |
| 权限需要 | 单独子进程提权不影响主进程安全性 | 整个进程需要提权 |
| 调试 | 单独调试某个子进程（附加调试器） | 所有逻辑混在一起，复杂 |

**多进程的代价：**
- IPC 序列化/反序列化开销（但相比编码 CPU 占用可忽略）
- 子进程启动延迟（启动时预先拉起，用户感知不到）
- 跨进程调试复杂（需要工具支持）

---

### 4.2 IPC 通信的序列化策略

**本项目使用的策略（基于 FIPC*Event 系列）：**
```
自定义二进制协议（猜测，基于 Qt 的 QDataStream）：

优点：
- 紧凑，传输效率高
- 强类型，编译期约束

缺点：
- 版本兼容性差（字段变化需同步更新两端）
- 调试困难（不可读）
```

**行业常见 IPC 序列化方案对比：**

| 方案 | 优点 | 缺点 | 适用场景 |
|------|------|------|---------|
| 自定义二进制（本项目） | 高效，强类型 | 版本管理难 | 闭源、内部通信 |
| JSON | 可读，跨语言 | 解析开销大 | 配置、调试 |
| Protocol Buffers | 高效+强类型+向后兼容 | 需要额外工具 | 大型分布式系统 |
| 共享内存 | 最高性能 | 同步复杂 | 高频大数据（视频帧） |

---

### 4.3 进程间同步的几种模式

**1. 事件驱动（本项目主要方式）：**
```
子进程就绪时发送 "ServerStarted" IPC 消息
主进程收到后才开始发送业务命令
→ 解耦，异步，无阻塞
```

**2. 共享内存 + 信号量：**
```
用于高频数据（如播放帧）：
子进程写入共享内存 → 触发信号量 → 主进程读取并渲染
→ 零拷贝，高性能
```

**3. 轮询（应避免）：**
```
主进程定时检查子进程状态
→ CPU 浪费，有延迟，不推荐
```

---

### 4.4 进程崩溃处理

```cpp
// FDmpSender 模块
class FMiniDmpSender {
    // Windows 结构化异常处理（SEH）+ BugSplat
    // 子进程崩溃时自动收集 minidump 并上报
};

// 主进程检测子进程异常退出
// QProcess::finished(int exitCode, QProcess::ExitStatus exitStatus)
// exitStatus == QProcess::CrashExit → 显示错误提示，可重试
```

---

### 4.5 典型ms问题与回答

**Q：Filmora 为什么把导出功能做成独立进程？**

A：
1. **隔离崩溃**：编解码器（尤其是第三方编解码库）存在稳定性问题，独立进程崩溃不影响用户正在编辑的项目
2. **内存效率**：导出时需要大量内存（解码缓冲、编码缓冲），任务结束后子进程退出，内存完全释放
3. **CPU 调度**：操作系统可以对导出进程和主进程做独立的 CPU 调度，避免编码 100% 占用影响 UI 响应
4. **独立初始化**：子进程可以独立初始化解码器和媒体库，不与主进程的状态耦合

---

**Q：如何解决跨进程共享对象状态的问题？**

A：
1. **不共享状态**：每个子进程都有自己的完整初始化（如 `FExportExe` 有独立的 `IFFMediaLibrary` 实例）
2. **通过 IPC 传递数据**：需要的数据通过消息序列化传递，而不是共享内存（除了高频帧数据）
3. **主进程作为数据权威**：所有状态变更都在主进程进行，子进程通过 IPC 请求数据，不维护本地状态副本

---

**Q：如何防止 QObject 在多线程中被访问导致的问题？**

A：
1. **`QPointer<T>`**：持有 `QObject` 的弱引用，对象被删除后自动为 `nullptr`
2. **`Qt::QueuedConnection`**：跨线程信号槽，确保槽在对象所属线程执行
3. **`QMetaObject::invokeMethod(obj, slot, Qt::QueuedConnection, ...)`**：主动将调用投递到对象所属线程
4. **`QObject::moveToThread()`**：将 Worker 对象移动到工作线程，其槽函数在工作线程执行

---

## 五、大型项目工程化经验

### 5.1 模块化 CMake 构建

```cmake
# 每个模块独立 CMakeLists.txt
# 通过 CMake target 表达依赖关系
add_library(FFMediaLibrary SHARED ...)
target_link_libraries(FFMediaLibrary
    PRIVATE FF::FFAsync         # 异步框架
    PRIVATE FF::FFCore          # 核心框架
    PRIVATE Qt5::Core           # Qt 基础
)
```

**收益：**
- 增量编译：只重编变化的模块
- 依赖显式化：`PRIVATE`/`PUBLIC`/`INTERFACE` 控制传递依赖
- 跨平台：同一 CMakeLists，生成 VS 或 Xcode 工程

### 5.2 接口与实现分离

```cpp
// 所有跨模块依赖通过 IFF* 接口（30+ 个）
// 接口头文件放在公共 include 目录
// 实现类（FFMediaLibrary 等）在各自模块内部

// 好处：
// 1. 模块可以独立测试（注入 Mock 接口）
// 2. 底层实现可以替换（VBL/WES 换库）
// 3. 减少编译依赖（只包含接口头文件，不包含实现）
```

### 5.3 代码质量保证

- **clang-format**：统一代码格式，消除风格争议（`.clang-format` 配置文件提交到仓库）
- **单元测试**：`Tests/FAsync_test`、`Tests/FUI_test` 等
- **VLD（Visual Leak Detector）**：Debug 模式下检测内存泄漏（`RunVLD` 宏控制）
- **崩溃上报**：BugSplat + `FDmpSender`，生产环境自动收集 minidump

### 5.4 调试工具链

- **QSpy**：Qt 对象监控工具（`USE_DEBUG_TOOL=ON` 编译），实时查看对象树和信号连接
- **`qDebug/qInfo/qWarning`**：Release 版本自动禁用（`CMAKE_CXX_FLAGS_RELEASE` 添加禁用宏）
- **`LOG_INFO/LOG_ERROR`**：自定义日志宏，写入日志文件，支持异步 I/O

---

*文档生成时间：2026-03-30*
