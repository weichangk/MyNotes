pbl_optimization.md
# PBL 模块 — 技术优化分析与方案

---

## 一、SafePtr 引用计数 vs 标准 shared_ptr

### 问题

PBL 自研 `SafePtr + IRef` 引用计数体系与 C++ 标准 `std::shared_ptr` 并存：
- `SafePtr` 侵入式（需对象继承 `IRef`），无法包装第三方对象
- `SafePtr` 没有 `weak_ptr` 等价物，循环引用无法自动检测（如 `PageClip` 持有子 `CaptureClip`，`CaptureClip` 反向引用 `PageClip` 时产生循环引用，需手动打破）
- Clip 内部混用 `SafePtr` 与 `std::shared_ptr`（tlb clip 用后者），增加理解成本和维护负担

### 优化方案

1. **短期**：引入 `WeakSafePtr<T>`，允许弱引用持有，专门用于反向引用场景（`PageClip` ↔ 子 Clip 的父子关系），打破潜在循环引用
2. **中期**：统一为 `std::shared_ptr<IBaseClip>` + `std::weak_ptr`，移除侵入式 IRef；IRef 仅保留作为兼容层 shim（Add/Release 委托到 shared_ptr 的 use_count 管理）
3. **长期**：推广 `std::enable_shared_from_this` 替代手工引用计数，利用标准库已有工具（`weak_ptr / make_shared` 等）减少自维护代码量

---

## 二、缩略图抓取线程管理粗放

### 问题

当前方案通过 `m_threads`（`QSet<Qt::HANDLE>`）记录线程 id 管理后台线程：
- 线程无法从外部被取消，仅能设置 `m_cancelThumbnail` 标志由线程自检；若 tlb `GetFrame` 阻塞时间过长（GPU 繁忙），取消响应滞后
- 多次触发 `updateThumbnail` 时可能同时有多个后台线程操作同一 Clone Timeline，存在 Clone Timeline 被重复释放的风险
- 线程异常退出时，Clone Timeline 不一定被正确清理（无 RAII 保护）

### 优化方案

1. **引入任务取消令牌**：用 `std::atomic<bool> cancelToken`（每次 updateThumbnail 创建新实例，通过 `shared_ptr` 共享给线程），线程在每帧抓取前检查令牌，响应更及时
2. **串行化缩略图任务队列**：改为单一后台 worker 线程 + `taskQueue`，多次触发 updateThumbnail 只追加任务，worker 按序处理；避免多线程并发操作同一 Clone Timeline 的竞态
3. **RAII 包装 Clone Timeline**：将 `CloneTimeline` 返回的 handle 包装成 RAII guard，析构时自动调用 `RemoveTimeline`，保证线程异常退出也不泄漏资源

```cpp
struct CloneTimelineGuard {
    tlb::ITimeline* handle;
    ~CloneTimelineGuard() { WESManager::instance().RemoveTimeline(handle); }
};
```

---

## 三、WESManager 初始化完成前的空窗期

### 问题

WESManager 在后台线程初始化，主线程不等待即返回。但部分 UI 操作（如立即打开项目文件）可能在 `getTlbEditing()` / `getTlbPreview()` 返回 null 时崩溃，代码中多处用 `if(!getTlbEditing()) return;` 防御但不彻底。

### 优化方案

1. **初始化状态机 + 操作队列**：定义 `WESState: Uninit / Initializing / Ready`；`Initializing` 阶段收到的 UI 操作放入延迟队列（`pendingOperations`），`Ready` 后按序执行
2. **Promise/Future 通知**：初始化完成后通过 `std::promise<void>` 设置值，需要等待初始化完成的操作可 `future.wait_for(timeout)` 而非轮询 null 判断
3. **UI 层启动遮罩**：主窗口在 `sigWESReady` 前展示加载动画（skeleton screen），用户无法触发任何编辑操作，从根本上消除空窗期的非法调用

---

## 四、ISignal* 信号总线的信号连接管理

### 问题

所有模块订阅全局 `ISignal*` 信号，但断开连接依赖 Qt 对象析构的自动断连（`QObject` 生命周期管理）：
- 若订阅方不继承 `QObject`（如部分工具类），忘记 disconnect 会造成野指针回调
- 信号连接数量随功能增长无限膨胀，难以追踪哪些模块订阅了哪些信号，调试困难
- 没有统一的信号接收日志/追踪机制，问题排查时难以复现信号传递链路

### 优化方案

1. **强制 QObject 订阅方**：规范代码要求所有 connect 的 receiver 必须是 `QObject` 子类，利用 Qt 的自动断连机制；禁止在 lambda 中捕获非 QObject 指针作为 receiver
2. **信号连接注册表**：引入 `SignalConnectionTracker`，在 Debug 构建中记录每个信号的所有连接（sender、receiver、signal name），提供 `dumpConnections()` 调试接口
3. **领域信号化**：将 `ISignalPage` 拆分为 `ISignalPageCRUD`（增删移）、`ISignalPageThumbnail`（缩略图）、`ISignalPageSelection`（选中），细粒度订阅减少无效信号触发

---

## 五、PMediaData 媒体信息缓存无过期策略

### 问题

`PMediaData` 缓存媒体文件路径到 `tlb::SourceInfo` 的映射，缓存永不过期：
- 同路径媒体文件被外部替换（如用户重新导出覆盖），缓存不失效，导致 Clip 属性（时长、分辨率）展示错误
- 项目关闭后缓存仍驻留内存，大量媒体项目切换后内存占用持续增长

### 优化方案

1. **文件修改时间戳校验**：缓存命中时比较 `QFileInfo::lastModified()`，若文件已更新则 invalidate 并重新解析
2. **LRU 缓存上限**：限制缓存最大条目数（如 500 条），超出时按 LRU 策略淘汰，防止无限增长
3. **项目切换时清理**：订阅 `ISignalProject::sigProjectAboutToSwitch`，在项目切换前清空与当前项目关联的缓存条目

---

## 六、UndoManager 的 Undo/Redo 粒度问题

### 问题

Lambda 命令捕获时序与实际执行时序不同，存在语义陷阱：
- 若 redoFn lambda 捕获的是当时的 Clip 状态值（非 SafePtr），undo 后对象状态已变，redo 重新执行时可能得到错误结果
- 多个快速操作（如拖动属性滑块）会在 QUndoStack 中堆积大量单帧命令，undo 一次只回退一帧变化，用户需连续多次 Ctrl+Z

### 优化方案

1. **值快照命令**：redo/undo lambda 应捕获操作前后的值快照（before/after），而非依赖对象当前状态；推广 `FunctionMapCommand<T>{ beforeValue, afterValue }` 模式
2. **合并操作命令**：对连续的同类属性修改（相同 clipId + 相同属性名）设置时间窗口（如 300ms），在 `QUndoCommand::mergeWith` 中合并为单条命令，减少 undo 步骤数
3. **beginComposite 规范化**：强制要求拖动类操作在 mousePress 时 `beginComposite`、mouseRelease 时 `endComposite`，保证一次拖动在 undo 栈中只是一条宏命令

---

## 七、ClipFactory 媒体类型扩展耦合

### 问题

`ClipFactory::createVideoClip` 内部有大量 `if (format == "xxx") { ... }` 的格式判断分支，每新增一种特殊媒体格式（如 HEIF、AV1）都要修改 `ClipFactory.cpp`，违反开闭原则。

### 优化方案

1. **注册式 Clip 构建器**：定义 `IClipBuilder` 接口（`bool canHandle(sourceInfo)` + `IBaseClipPtr build(sourceInfo, tlbClip)`），`ClipFactory` 维护构建器注册表；各格式/类型对应独立 Builder 类，新增格式只需注册新 Builder
2. **责任链应用**：Builder 注册表按优先级形成责任链，`ClipFactory` 依次询问每个 Builder 能否处理，首个返回 true 的 Builder 执行创建；核心流程不修改

---

## 八、跨平台设备监控的抽象不足

### 问题

设备热插拔监控在 Windows 与 macOS 使用完全不同的机制（Win32 消息 vs ObjC `AVCaptureDevice` 通知），但目前没有统一的抽象接口层，平台差异代码散落在不同文件中，增加新平台（如 Linux）时改动范围大。

### 优化方案

1. **统一 IDeviceMonitor 接口**：定义 `IDeviceMonitor { virtual void start() = 0; virtual void stop() = 0; }` 并通过工厂函数按平台创建实现（Windows/macOS/Linux），`PDeviceMgrData` 只持有 `IDeviceMonitor` 接口指针
2. **编译期平台隔离**：`DeviceMonitorFactory.cpp` 按 `#ifdef Q_OS_WIN / Q_OS_MAC` 返回对应实现，平台代码不泄漏到公共层
