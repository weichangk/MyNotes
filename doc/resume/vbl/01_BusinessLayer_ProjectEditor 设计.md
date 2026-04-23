# ProjectEditor 模块详细设计文档

> 所属层：BusinessLayer
> 接口头：`Interface/BusinessLayer/ProjectEditor/IVbProjectEditor.h`
> 工厂函数：`createProjectEditor()`
> 命名空间：`VBL::`

---

## 1. 模块职责

ProjectEditor 是 VBL 业务层的**工程生命周期总管理器**，负责：

- **工程创建**：新建空工程或按比例配置创建
- **工程加载**：从磁盘加载 `.xml` 工程文件，重建 DataModel 对象树
- **工程保存**：覆盖保存、另存为、打包存档（含媒体文件）
- **自动保存**：定时自动保存，支持崩溃恢复
- **Undo/Redo 管理**：持有 `IBsUndoTemplateStack`，协调各子编辑器的操作入栈
- **资源管理协调**：持有 `IVbSourceManager`（媒体区）和音乐 SourceManager
- **媒体重定位**：媒体文件路径变更时的重链接
- **AI 工程操作**：支持 LongToShort 场景的工程切换与布局变更

---

## 2. 核心接口说明

```cpp
class IVbProjectEditor : virtual public IDmBaseObj {
public:
    // 生命周期
    virtual Result init() = 0;
    virtual Result unInit() = 0;

    // Undo 管理
    virtual Result setUndoManager(IBsUndoTemplateStack* undoMgr) = 0;
    virtual IBsUndoTemplateStack* getUndoTemplateStack() = 0;

    // 工程新建
    virtual Result createNewProject() = 0;
    virtual Result createNewProject(RatioType ratioType, const TimelineConfig& backupConfig) = 0;

    // 工程加载
    virtual Result loadProject(VBLConstPChar path, VBLPCharList* relocateList, ...) = 0;
    virtual Result loadProjectEx(VBLConstPChar path) = 0;  // 异步加载
    virtual Result stopLoadProject() = 0;

    // 工程保存
    virtual Result saveProject(...) = 0;
    virtual Result saveAsProject(VBLConstPChar path, ...) = 0;
    virtual Result archiveProject(VBLConstPChar path, ...) = 0;    // 打包含媒体
    virtual Result stopSaveProject() = 0;

    // 自动保存
    virtual Result autoSaveProject() = 0;
    virtual Result setAutoSaveEnabled(VBLBool) = 0;
    virtual Result setAutoSaveParams(VBLInt intervalSecond, VBLInt maxCount) = 0;
    virtual VBLConstPChar getAutoSaveProject(VBLConstPChar oriPath = nullptr) = 0;
    virtual Result restoreAutoSaveProject(VBLConstPChar path, ...) = 0;

    // 工程对象访问
    virtual IDmProject* getCurEditingProjct() = 0;
    virtual Result setProject(IDmProject* project) = 0;
    virtual VBLBool isProjectChanged() = 0;

    // 配置
    virtual Result getProjectConfig(TimelineConfig& config) = 0;
    virtual Result setProjectConfig(const TimelineConfig& config) = 0;

    // 资源管理
    virtual IVbSourceManager* getSourceManager() = 0;
    virtual Result setSourceManager(IVbSourceManager*) = 0;
    virtual IVbSourceManager* getMusicManager() = 0;

    // 媒体操作
    virtual Result relocateMediaFile(...) = 0;
    virtual Result importMediaIntoTrack(...) = 0;
    virtual Result mergeClips(...) = 0;

    // 子编辑器访问
    virtual IVbTimelineUX* getTimelineUX() = 0;
    virtual IVbVisualEditor* getVisualEditor() = 0;
};
```

---

## 3. 依赖关系

```
ProjectEditor
  ├── PRIVATE 持有 → IBsUndoTemplateStack  （Undo/Redo 栈）
  ├── PRIVATE 持有 → IVbSourceManager      （媒体资源区）
  ├── PRIVATE 持有 → IVbSourceManager      （音乐资源区）
  ├── PRIVATE 持有 → IVbTimelineUX         （时间线交互层）
  ├── PRIVATE 持有 → IVbVisualEditor       （可视化编辑器）
  ├── 读写     → IDmProject              （通过 DataModel 工厂创建）
  └── 订阅     → IMsEventBus             （project.* 事件广播）
```

---

## 4. 时序图

### 4.1 新建工程

```
UI 层                     ProjectEditor          DataModel              EventBus
  │                            │                     │                     │
  │  createNewProject()        │                     │                     │
  ├──────────────────────────► │                     │                     │
  │                            │  getDmFactoryInstance()                   │
  │                            ├────────────────────►│                     │
  │                            │  IDmClipFactory*    │                     │
  │                            │◄────────────────────┤                     │
  │                            │  factory->createProject()                 │
  │                            ├────────────────────►│                     │
  │                            │  IDmProject*        │                     │
  │                            │◄────────────────────┤                     │
  │                            │  factory->createTimeline()                │
  │                            ├────────────────────►│                     │
  │                            │  IDmTimeline*       │                     │
  │                            │◄────────────────────┤                     │
  │                            │  project->setTimeline(tl)                 │
  │                            ├────────────────────►│                     │
  │                            │  postEvent("project.created")             │
  │                            ├─────────────────────────────────────────► │
  │  onEvent("project.created")│                     │                     │ notify
  │◄───────────────────────────────────────────────────────────────────────┤
  │  刷新工程 UI               │                     │                     │
```

### 4.2 加载工程

```
UI 层           ProjectEditor    SourceManager     DataModel    ListenerCenter
  │                  │                │                │              │
  │ loadProject(path)│                │                │              │
  ├────────────────► │                │                │              │
  │                  │ getDmFactory() │                │              │
  │                  ├───────────────────────────────► │              │
  │                  │ createProject()│                │              │
  │                  ├───────────────────────────────► │              │
  │                  │ project->deserialize(path)      │              │
  │                  ├───────────────────────────────► │              │
  │                  │               │  解析 XML 重建   │              │
  │                  │               │  IDmTimeline    │              │
  │                  │               │  IDmClip 对象树  │              │
  │                  │               │                 │              │
  │                  │ setSourceManager(sm)            │              │
  │                  ├───────────────►│                │              │
  │                  │               │ init(folder)    │              │
  │                  │               ├───────────────► │              │
  │                  │               │ loadMediaItems()│              │
  │                  │               ├───────────────► │              │
  │                  │  postEvent("project.loaded")    │              │
  │                  ├─────────────────────────────────────────────── ►│
  │ onEvent() 刷新 UI│                │                │              │ → UI
  │◄─────────────────────────────────────────────────────────────────────────┤
```

### 4.3 保存工程

```
UI 层           ProjectEditor     DataModel      文件系统     ListenerCenter
  │                  │                │              │               │
  │ saveProject()    │                │              │               │
  ├────────────────► │                │              │               │
  │                  │ project->serialize()          │               │
  │                  ├───────────────►│              │               │
  │                  │               │ 生成 project.xml              │
  │                  │               ├────────────── ►│              │
  │                  │               │ 保存媒体引用    │              │
  │                  │               ├───────────────►│              │
  │                  │ progressCallback(50%)         │               │
  │◄────────────────────────────── ─ ─ ─ ─ ─ ─      │               │
  │                  │               │ 完成           │               │
  │                  │  postEvent("project.saved")   │               │
  │                  ├─────────────────────────────────────────────── ►│
  │ onEvent() 更新标题栏              │              │               │ → UI
  │◄──────────────────────────────────────────────────────────────────────┤
```

### 4.4 自动保存

```
ISyncAutoSaveTimer    ProjectEditor     DataModel     文件系统
       │                   │                │             │
  定时触发               │                │             │
       ├──────────────────►│               │             │
       │   autoSaveProject()               │             │
       │                   │ isProjectChanged()          │
       │                   ├───────────────►│             │
       │                   │ = true        │             │
       │                   │ project->serializeToAutoSaveDir()
       │                   ├───────────────────────────── ►│
       │                   │               │ 写 autosave.xml │
       │                   │ saveAutoSaveInfo(path)      │
       │                   ├───────────────────────────── ►│
       │                   │               │             │
   崩溃恢复场景：           │               │             │
       │ getAutoSaveProject()              │             │
       ├──────────────────►│               │             │
       │  path             │               │             │
       │◄──────────────────┤               │             │
       │ restoreAutoSaveProject(path)      │             │
       ├──────────────────►│               │             │
       │                   │ loadProject(path)           │
       │                   ├───────────────►│             │
```

### 4.5 媒体重定位

```
UI 层           ProjectEditor     DataModel      ListenerCenter
  │                  │                │               │
  │ relocateMediaFile(old, new)       │               │
  ├────────────────► │                │               │
  │                  │ 遍历时间线所有 clip               │
  │                  │ 找到引用 oldMedia 的 clip         │
  │                  │ clip->setMediaPath(newPath)     │
  │                  ├───────────────►│               │
  │                  │ updateAllTimelineMedia(item)    │
  │                  ├───────────────►│               │
  │                  │               │ 更新渲染引用    │
  │                  │  postEvent("media.relocated")  │
  │                  ├───────────────────────────────► │
  │ onEvent() 刷新 UI│               │               │ → UI
  │◄──────────────────────────────────────────────────────┤
```

---

## 5. 设计要点

| 要点 | 说明 |
|---|---|
| 工程文件格式 | `project.xml`（IDmProject 序列化）+ `Config.json`（媒体区）|
| 自动保存目录 | 与原工程同目录下的 `.autosave/` 子目录 |
| 打包存档 | `archiveProject` 将媒体文件复制至工程包，支持跨机分享 |
| LongToShort | 支持按 AI 脚本切换工程布局（`changeVideoLayout`）|
| Undo 边界 | 创建/加载/保存工程不进入 Undo 栈，仅编辑操作入栈 |
| 线程安全 | `autoSaveProject` 要求在主线程调用，序列化为同步操作 |
