pbl_project_content.md
# PBL 模块 — 项目内容及成果

---

## 项目背景

PBL（Presentation Business Layer）是 Presentory / DemoCreator 演示模式的核心中间层，横跨项目生命周期管理、页面与 Clip 操作、预览/录制控制、设备管理、Undo/Redo、剪贴板等全套业务领域，是连接上层 Qt UI 与下层 WES/tlb 渲染引擎的唯一桥梁。

---

## 项目内容

### 一、PBL 接口体系设计与实现

设计并实现贯穿整个 Presentory 产品的核心中间层接口体系，将上层 Qt UI 与底层 WES/tlb 渲染引擎完全解耦。

- **纯虚接口分层**：`Include/` 目录下全部以纯虚接口暴露（`IPresentationEditor / IPagesEditor / IBaseClip / IClipFactory / IUndoManager / IClipBoard / IDevice / IPPreview / IPRecorder` 等），UI 层只依赖接口头文件，不包含任何实现；实现细节封装在 `Src/` 子模块，通过 `getPresentationProject()` / `getClipFactoryInstance()` 等工厂函数返回接口指针
- **门面统一访问**：`IPresentationEditor` 作为门面，统一暴露 12 个子管理器（摄像头、背景、水印、麦克风、屏幕/窗口/白板捕获、动画、虚拟场景、帧管理等），上层按需获取所需管理器，接口边界清晰
- **SafePtr + IRef 生命周期体系**：设计 `SafePtr<T>` 引用计数智能指针（`IRef / IPRef` 为基类），所有跨模块传递的对象均通过 `SafePtr` 包装；Clip 持有的 tlb 原生对象额外用 `std::shared_ptr` + 自定义 deleter 管理，确保资源通过正确的 tlb 工厂释放，杜绝裸 delete 与内存泄漏

---

### 二、Clip 工厂与多类型 Clip 实现

设计并实现 `IClipFactory / ClipFactory` 及完整的 Clip 类型体系（14+ 种类型），统一封装 tlb 原生 Clip 创建与 PBL 层属性管理。

- **工厂模式封装创建链路**：`ClipFactory::createVideoClip(path)` 内部串联 `PMediaData` 媒体信息缓存（避免重复解析）→ `WESManager` 获取 tlb ClipFactory 创建原生 clip → 包装为 PBL VideoClip + 自动关联 AudioClip → SafePtr 返回；新增 Clip 类型只需继承 `BaseClip` 并扩展工厂，主流程零改动
- **时间坐标系双向映射**：`IBaseClip` 统一定义 `sourceTime(renderTm) / renderTime(srcTime)`，支持时间轴渲染位置与素材源时间的双向互查，用于精确 seek 与裁剪点计算
- **PageClip 多轨道容器**：`PageClip` 作为特殊 Clip 容器管理一个页面内的所有子 Clip 轨道，支持 `addClip / removeClip / moveClipLayer / group / ungroup`，并持有可选的音频子时间轴（`ITimelineClipPtr`）用于背景音乐

---

### 三、PagesEditor 页面管理与异步缩略图

实现 `IPagesEditor` 全套页面 CRUD 操作，并针对大量页面的缩略图生成设计了 Clone Timeline 后台线程方案。

- **完整页面操作**：`appendPage / insertPage / removePage / movePage / changeTransition / switchToPage` 等均封装对 PTimeline 的 track 操作与 WES 的 `beginEditTimeline/endEditTimeline` 包裹，批量修改时减少 tlb 中间渲染触发次数
- **异步缩略图抓取**：大量页面（>10 页）时 `WESManager::CloneTimeline()` 克隆独立 tlb timeline，通过 `QThread::create` 在后台线程逐页调用 `GetFrame` 抓取渲染帧，完成后 emit `ISignalPage::sigPageThumbnail(index, QImage)` 回调 UI 更新；`cancelUpdateThumbnail(wait=true)` 支持安全取消与线程等待，防止页面删除时的竞态问题
- **切换动效管理**：`changeTransition(path, applyToAll)` 支持单页与批量两种应用模式，通过 `IPageClip::setTransition(transClip, duration)` 写入 tlb 转场数据

---

### 四、Undo/Redo 系统

基于 Command 模式设计 `IUndoManager / UndoManager`，支持 Lambda 命令封装、泛型对象引用保活、宏命令组合。

- **Lambda + SafePtr 命令封装**：`pushCmd(IBaseClipPtrs, redoFn, undoFn)` 将 lambda 与 SafePtr 列表打包成 `BaseCommand`（继承 `QUndoCommand`）；命令内 SafePtr 持有对象引用，保证 undo 时对象不被析构，彻底消除悬空指针风险
- **宏命令支持**：`beginComposite(name) / endComposite()` 对应 `QUndoStack::beginMacro / endMacro`，一组业务操作（如「批量删除页面」）作为一个原子 undo 步骤，保证操作粒度的业务合理性
- **变更通知链路**：`BaseCommand::redo/undo` 执行后自动调用 `ProjectInfo::setChanged(true)` 并 emit `sigPropertyChanged`，UndoManager 通过 `ISignalUndoManager` 广播 `canUndo / canRedo` 状态，UI 工具栏按钮无需轮询即可实时响应

---

### 五、WESManager 引擎桥接与异步初始化

封装 WES/tlb 渲染引擎全部 API，实现引擎初始化异步化与编辑/预览模式管理。

- **异步初始化**：`WESManager::init()` 将重量级操作（InitGpuService / InitFxlab / InitDecMgr / InitOpencl）放入独立 `std::thread`，`DCSyncInitHelper` 进一步把部分子任务异步化；主线程在 init 返回后立即可响应用户输入，初始化完成后信号通知 UI 解锁功能
- **编辑/预览模式切换**：`enterEditingMode / exitEditingMode` 管理 tlb VisualEditing 状态；`beginEditTimeline / endEditTimeline` 批量包裹 clip 修改，避免每次单 clip 操作触发一次完整重渲染
- **Clone Timeline 帧抓取**：`CloneTimeline()` 为缩略图抓取提供独立 timeline 副本，`GetFrame(clonedTimeline, position)` 精准抓帧，抓取结束后 `RemoveTimeline(clonedTimeline)` 释放资源，全程对主预览 timeline 无干扰

---

### 六、设备管理与热插拔支持

`IDevice / PDevice / PDeviceMgrData` 统一抽象摄像头、屏幕、麦克风、扬声器、窗口等所有输入设备流。

- **设备生命周期管理**：`PDevice` 封装 stream id、state（Uninit→Inited→Streaming→Error）、init/start/stop/release；`PDeviceMgrData` 管理设备枚举列表，统一热插拔事件处理
- **跨平台设备监控**：Windows 通过 DirectShow/DXGI 设备变更通知；macOS 通过 `MacDeviceMonitor`（ObjC 桥接 `AVCaptureDevice` 通知）补充系统级热插拔；均通过 `ISignalDevice::sigDeviceAdded / sigDeviceRemoved` 统一上报 UI

---

### 七、ISignal* 信号总线（观察者解耦）

设计并实现全套 `ISignal*` 单例信号总线，实现模块间彻底的依赖倒置。

- **全局事件广播**：`ISignalProject / ISignalPage / ISignalDevice / ISignalPreview / ISignalRecorder / ISignalUndoManager / ISignalClipBoard` 七大信号总线，覆盖项目 IO、页面变更、设备热插拔、预览播放、录制状态、撤销、剪贴板全部核心事件
- **零直接依赖**：UI 各模块（`PMainSlides / PTProperty / PPlayerEditor / AITextMgr`）只 connect 信号，不 include 任何 PBL 实现头文件；PBL 内部子模块之间同样通过 `ISignal*` 通信（如 PagesEditor 订阅 `ISignalPage::sigPageDurationChanged`），做到模块内/外一致的解耦

---

## 项目成果

- **引擎完全解耦**：UI 层通过纯虚接口与 `ISignal*` 总线访问 PBL，实现接口与实现的彻底分离；更换底层渲染引擎（如从 tlb 换为新引擎）时 UI 层零修改，仅需替换 WESManager 与 Clip 实现
- **多线程安全的缩略图方案**：Clone Timeline + 后台线程 + 取消机制，在大量页面场景下消除主线程阻塞，页面面板滚动与缩略图更新并行进行，用户体验流畅
- **可扩展 Clip 体系**：14+ 种 Clip 类型统一由 `ClipFactory` 创建，新增类型无需改动现有创建逻辑；`IBaseClip::userData(key, QByteArray)` 扩展点支持上层任意附加元数据
- **工程质量保障**：SafePtr + IRef 引用计数体系配合 tlb clip 自定义 deleter，在复杂的多模块、多线程环境下实现了零内存泄漏与零悬空指针的内存管理目标
- **产品落地**：PBL 中间层支撑了 Presentory 独立产品上线及后续集成进 DemoCreator 演示模式的完整迭代周期，稳定支撑 Windows / macOS 双平台、10 语言的生产环境运行
