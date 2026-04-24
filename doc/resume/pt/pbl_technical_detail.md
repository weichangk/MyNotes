pbl_technical_detail.md
# PBL 模块 — 技术实现细节文档

> PBL（Presentation Business Layer）是 Presentory / DemoCreator 演示模式的核心中间层，负责将上层 Qt UI 与下层 WES/tlb 渲染引擎完全解耦，承担项目管理、页面/Clip 生命周期、预览录制控制、设备管理、Undo/Redo、剪贴板等全套业务逻辑。

---

## 一、整体架构

```
┌────────────────────────────────────────┐
│         UI 层（PPresentation）          │
│  PMainWindow / PMainSlides / PTProperty │
│  PPlayerEditor / AITextMgr / ...        │
└──────────────┬─────────────────────────┘
               │ 纯接口（I* 头文件）+ ISignal* 信号总线
┌──────────────▼─────────────────────────┐
│              PBL 中间层                 │
│  PresentationProject / PresentationEditor
│  PagesEditor / ClipFactory              │
│  Manager（Camera/BG/Watermark/...）     │
│  UndoManager / ClipBoard                │
│  Devices（PDevice/PDeviceMgr）          │
│  Preview（PPreview / PRecorder）        │
│  PTimeline / PTrack / PClipsManager     │
└──────────────┬─────────────────────────┘
               │ WESManager（tlb 封装层）
┌──────────────▼─────────────────────────┐
│         WES/tlb 渲染引擎（C++ native）   │
│  ITlbClipFactory / ITlbEditing          │
│  ITlbPreview / ITlbTimeline             │
└────────────────────────────────────────┘
```

**核心设计原则**：UI 层只依赖 `Include/` 下的纯虚接口（`I*.h`），不直接 include 任何实现文件；模块间事件通过 `ISignal*` 单例信号总线解耦，做到接口依赖倒置。

---

## 二、项目生命周期管理（PresentationProject）

### 主业务流程

```
PresentationProject::init()
  ├── WESManager::instance().createTlb()       // 初始化底层 WES 引擎（后台线程）
  ├── new PTimeline()                           // 创建时间轴
  ├── new PresentationEditor()                  // 创建编辑器（持有 12 个子 Manager）
  ├── new ProjectManager()                      // 负责项目 IO
  └── new PagesEditor()                         // 负责页面集合操作
```

**新建 / 加载项目流程**：
1. `IProjectManager::createNewProject(type, fps)` → ProjectManager 初始化空 PTimeline，设置分辨率、帧率
2. `IProjectManager::loadProject(path)` → ProjectManager 反序列化工程文件 → 重建所有 PageClip / Clip 结构 → 通过 `ITimelineCallback::onClip` 回调逐 clip 重建 PBL 对象 → 完成后 emit `ISignalProject::sigProjectLoadFinished`
3. `IProjectManager::saveProject(path)` → ProjectManager 序列化所有 clip 信息 → 完成后 emit `sigProjectSaveFinished`

**自动保存**：`enableAutoSave(true) + changeAutoSaveInterval(minutes)` 设置 QTimer 周期触发保存，`ProjectInfo::isChanged()` 标志避免无修改时的冗余 IO。

---

## 三、页面管理（PagesEditor）

### 核心职责

`PagesEditor` 实现 `IPagesEditor` 接口，是幻灯片页面操作的唯一入口：

| 操作 | 内部行为 |
|------|---------|
| `appendPage()` | 在 PTimeline 追加新 PageClip track，调用 WES beginEditTimeline/endEditTimeline，emit `ISignalPage::sigPageAdded` |
| `insertPage(index)` | 在指定位置插入 track，重排所有页面 TLBegin/TLEnd |
| `removePage(index)` | 移除 track，释放 PageClip 引用，重排 TL 位置 |
| `movePage(from, to)` | track 重排，不创建新对象 |
| `changeTransition(path, applyToAll)` | 为指定 PageClip 设置 TransClip，applyToAll=true 则遍历全部页面 |
| `switchToPage(index)` | WESManager 切换 editing position，emit `sigProjectSwitched` |

### 异步缩略图抓取机制

缩略图的生成是 PagesEditor 中最复杂的异步场景：

```
updateThumbnail(QList<int> indices)
  ├── 少量页面（≤10）：直接用 main preview timeline GetCurrentFrame 逐帧抓取
  └── 大量页面：
        ├── WESManager::CloneTimeline()  // Clone 一份独立的 tlb timeline
        ├── QThread::create(lambda) → 后台线程
        │     └── 逐页调用 GetFrame(clonedTimeline, position) 抓帧
        │     └── 抓完 emit ISignalPage::sigPageThumbnail(index, QImage)
        ├── m_threads.insert(threadId)  // 登记线程 id
        └── cancelUpdateThumbnail(wait) // 可取消 + 等待所有线程结束
```

**关键设计**：Clone timeline 隔离了预览 timeline 与缩略图抓取 timeline，避免主线程渲染被打断；`cancelUpdateThumbnail` 支持 `wait=true` 阻塞等待，用于页面删除前的安全清理。

---

## 四、Clip 体系设计

### 类型层次

```
IBaseClip（接口）
  └── BaseClip（公共实现，持有 tlb::ITlbBaseClip）
        ├── VideoClip
        ├── AudioClip
        ├── ImageClip
        ├── CaptureClip（屏幕/摄像头捕获）
        ├── PPTClip
        ├── TextClip / TitleClip / SubtitleClip
        ├── EffectClip
        ├── TransClip（转场）
        ├── ColorboardClip（纯色板）
        ├── AnimationClip
        ├── GroupClip / ContainerClip
        ├── TimelineClip（子时间轴）
        └── PageClip（页面，特殊：包含多 clip 轨道）
```

### BaseClip 封装 tlb Clip 的方式

```cpp
// BaseClip 持有底层对象（自定义释放器，确保通过 tlb Factory 释放）
std::shared_ptr<tlb::ITlbBaseClip> m_pClip {
    tlbRawPtr,
    [](tlb::ITlbBaseClip* p) { WESManager::instance().getTlbClipFactory()->ReleaseClip(p); }
};
```

所有属性操作（duration、TrimIn/TrimOut、TLBegin/TLEnd、transform、mask、opacity、flip）均透传到 `m_pClip` 对应的 tlb 接口，PBL 层只维护额外的 PBL 侧状态（thumbnail 缓存、userData、groupId 等）。

**时间坐标系双向映射**：
- `sourceTime(renderTm)` → 把时间轴渲染时间点映射到素材源时间（用于 seek）
- `renderTime(srcTime)` → 把素材时间点映射回时间轴渲染位置

### IClipFactory（工厂模式）

`ClipFactory::createVideoClip(path)` 的完整流程：
1. `PMediaData::getSourceInfo(path)` → 获取媒体信息（宽/高/时长/流数量），结果缓存避免重复解析
2. `WESManager::getTlbClipFactory()->CreateVideoClip(streamId, in, out, tlBegin, tlEnd)` → 创建 tlb 原生 clip
3. `new VideoClip(tlbClip)` → 包装为 PBL VideoClip
4. 若媒体有音频流 → 同步创建并关联 AudioClip
5. 设置 `saveFilePath / type / uuid` 等 PBL 属性
6. 返回 `IBaseClipPtr`（SafePtr 包装）

**PageClip 特殊处理**：`PageClip::addClip(clip)` 在添加 CaptureClip 时会触发 `setClipsVisible` 更新层叠可见性；`PageClip` 持有可选的 `ITimelineClipPtr`（音频子时间轴）用于背景音乐。

---

## 五、Undo/Redo 系统（Command 模式）

### 架构

```
IUndoManager（接口）
  └── UndoManager（实现，持有 QUndoStack）
        ├── pushCmd(QUndoCommand*)                         // 直接压栈
        ├── pushCmd(IBaseClipPtrs, redoFn, undoFn)         // Lambda 命令
        ├── pushCmd(QList<SafePtr<IPRef>>, redoFn, undoFn) // 泛型引用命令
        └── beginComposite(name) / endComposite()          // 宏命令（beginMacro/endMacro）
```

### BaseCommand 执行流程

```
BaseCommand::redo()
  ├── m_redoFn()                                  // 调用业务 lambda
  ├── PRESENTATION_PROJECT->getProjectInfo()->setChanged(true) // 标记项目已修改
  └── emit sigPropertyChanged()                   // 通知 UI 刷新

BaseCommand::undo()
  ├── m_undoFn()
  ├── setChanged(true)
  └── emit sigPropertyChanged()
```

**Lambda + SafePtr 组合**：命令内的 lambda 通过捕获 `SafePtr<IBaseClip>` 持有对象引用，保证 undo 时对象不会被销毁，避免悬空指针。

**ISignalUndoManager 总线**：`UndoManager` 的 `canUndo/canRedo` 变化会 emit `ISignalUndoManager::sigUndoAvailable / sigRedoAvailable`，UI 工具栏按钮订阅后自动刷新 enabled 状态。

---

## 六、WESManager（引擎桥接层）

### 初始化异步化设计

```
WESManager::init()
  └── std::thread([]{
        tlb::CreateTlb()
        InitGpuService()        // GPU 检测
        InitFxlab()             // 特效库
        InitDecMgr()            // 解码管理器
        InitOpencl()            // OpenCL 加速
        DCSyncInitHelper::post(InitMediaInfo)  // 异步任务队列
      })
```

将重量级初始化放到后台线程，主线程在 init 返回后即可响应用户操作；`DCSyncInitHelper` 进一步将部分初始化任务异步化、支持依赖序列化。

### 编辑 / 预览模式管理

| API | 作用 |
|-----|------|
| `enterEditingMode()` | 进入 VisualEditing，允许对 timeline clip 实时交互 |
| `exitEditingMode()` | 退出 VisualEditing，恢复正常预览状态 |
| `beginEditTimeline() / endEditTimeline()` | 包裹批量 clip 修改，减少 tlb 的中间渲染触发次数 |
| `setEditingPosition(pos)` | 跳转当前页面编辑时间点 |
| `CloneTimeline()` | 克隆 timeline 供后台缩略图抓帧，不影响主 timeline |
| `GetFrame(timeline, pos)` | 从指定 timeline 抓取指定位置的渲染帧（QImage） |

---

## 七、设备管理（Devices）

`IDevice / ISignalDevice` 抽象了摄像头、麦克风、扬声器、屏幕、窗口等所有输入流：

- `PDevice` 实现 `IDevice`，封装 stream id（tlb 设备标识）、state（Uninit/Inited/Streaming/Error）、init/start/stop/release 生命周期
- `PDeviceMgrData` 维护设备列表、管理设备枚举与热插拔监听
- `ISignalDevice::sigDeviceAdded / sigDeviceRemoved / sigDeviceStateChanged` 驱动 UI 实时响应设备变化（如摄像头拔插后立即更新设备选择列表）
- macOS 额外通过 `MacDeviceMonitor`（ObjC 桥接）补充系统级设备通知

---

## 八、预览与录制（Preview / Recorder）

### 预览流程

```
PPreview::start()
  └── WESManager::getTlbPreview()->Start()
        └── tlb 渲染线程开始渲染帧
              → PreviewMsgCb::onEvent(eventType)
                    ├── PAGE_CHANGED   → ISignalPage::sigCurrentPageChanged
                    ├── PLAY_END       → ISignalPreview::sigPlayEnd
                    └── ANIMATION_END  → ISignalPage::sigAnimationEnd
```

UI 订阅 `ISignalPreview` 信号驱动播放控制条（进度、时间）更新，与渲染线程完全解耦。

### 录制状态机

```
RecordState: Stopped → Recording → Paused → Recording → Stopped
```

- `PRecorder::startRecord()` → `tlbRecorder->Start()` → emit `ISignalRecorder::sigStart`
- `pause() / resume()` → tlb pause/resume → emit `sigPause / sigResume`
- `stopRecord()` → tlb stop → 编码输出文件 → emit `sigStop(outputPath)`
- 录制时间通过 `ISignalRecorder::sigRecordTime(ms)` 定时回调，UI 计时器与引擎保持同步

---

## 九、剪贴板（ClipBoard）

`ClipBoard` 实现 `IClipBoard` 接口，支持两套存储路径：

| 类型 | 存储 |
|------|------|
| `IBaseClipPtrs` | 内部 m_clips 列表（SafePtr 持有，跨 paste 存活）|
| `IRefPtrs` | 内部 m_refs 列表（泛型 PBL 对象）|
| 文本 / 图片 / 路径 / URL | Qt QClipboard 系统剪贴板 |

`m_bWindowBuildInClipboard`（内部/系统剪贴板区分）：当 clip 被复制时走内部路径；文本/图片走系统剪贴板，通过 `QClipboard::dataChanged` 信号监听并在内容变化时 clear 内部 clips，发出 `ISignalClipBoard::sigClipBoardChanged` 通知 UI 刷新可粘贴状态。

---

## 十、生命周期管理（SafePtr + IRef）

PBL 全系采用自定义引用计数智能指针：

```
IRef（纯虚）
  └── BaseClip（实现 AddRef/Release，QAtomicInteger<int> m_ref）

IPRef（提供默认实现）
  └── 其它非 Clip 的 PBL 对象（Manager、Recorder 等）

SafePtr<T>（类似 shared_ptr，但引用计数在对象内）
  ├── 构造：默认调 T::AddRef()
  ├── 析构：调 T::Release()，引用计数到 0 则 delete this
  ├── DynamicCast / StaticCast：支持跨接口安全转换
  └── Attach(raw, false)：附加已有对象而不增加引用计数（接管所有权）
```

Clip 的 tlb 指针单独用 `std::shared_ptr` + 自定义 deleter（`tlbFactory->ReleaseClip`）管理，确保即便 PBL Clip 引用归零后 tlb 资源也经由正确的工厂释放，不发生裸 delete。

---

## 十一、信号总线设计（ISignal* 单例）

每个子系统暴露一个 `QObject` 单例作为信号总线：

| 单例类 | 主要信号 |
|--------|---------|
| `ISignalProject` | sigProjectSwitched / sigProjectChanged / sigProjectLoadFinished / sigAIGCApplyFinished |
| `ISignalPage` | sigPageAdded / sigPageRemoved / sigPageMoved / sigPageThumbnail / sigCurrentPageChanged |
| `ISignalDevice` | sigDeviceAdded / sigDeviceRemoved / sigDeviceStateChanged |
| `ISignalPreview` | sigPlayEnd / sigPlayStart / sigPositionChanged |
| `ISignalRecorder` | sigStart / sigStop / sigPause / sigRecordTime |
| `ISignalUndoManager` | sigUndoAvailable / sigRedoAvailable / sigPropertyChanged |
| `ISignalClipBoard` | sigClipBoardChanged / sigCanPaste |

UI 层只 connect 这些信号，无需包含任何实现头文件，实现彻底的依赖倒置。

---

## 十二、关键设计模式汇总

| 设计模式 | 在 PBL 中的应用 |
|---------|--------------|
| **工厂方法** | `IClipFactory::createXxxClip()` 封装所有 Clip 创建，UI 无需了解 tlb |
| **命令模式** | `BaseCommand` + lambda 封装 undo/redo，配合 `QUndoStack` |
| **观察者/事件总线** | `ISignal*` 单例 Qt 信号，模块间零直接依赖 |
| **门面模式** | `IPresentationEditor` 作为 12 个子 Manager 的统一门面 |
| **适配器** | `WESManager` 适配 tlb API；`PResourceAdpater / FontLibraryAdapter` 适配资源系统 |
| **RAII / 智能指针** | `SafePtr` + `IRef` 引用计数；tlb clip 用 `std::shared_ptr` + 自定义 deleter |
| **模板方法** | `BaseClip` 提供通用 tlb 代理实现，子类 override 特有接口 |
| **单例** | `WESManager::instance()` / `ISignal*::instance()` / `getPresentationProject()` |
