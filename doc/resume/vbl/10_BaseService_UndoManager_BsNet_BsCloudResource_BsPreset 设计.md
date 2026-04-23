# UndoManager / BsNet / BsCloudResource / BsPreset 模块详细设计文档

> 所属层：BaseService
> 命名空间：`VBL::`

---

## 一、UndoManager（撤销/重做管理器）

### 接口头
`Interface/BaseService/BsUndoManager/IBsUndoTemplateStack.h`

### 1.1 职责
- 维护编辑操作的 **Undo/Redo 命令栈**
- 支持 **Macro（批量命令合并）**：beginMacro/endMacro 将多个子操作合为一步 Undo
- 设置最大历史步数 `setUndoLimit`
- 查询当前 Undo/Redo 可用状态

### 1.2 核心接口

```cpp
class IBsUndoTemplateStack : virtual public IDmBaseObj {
    enum undoRedoStatus { InOther, InUndo, InRedo };

    virtual Result clear() = 0;
    virtual Result push(IBsUndoTemplateItem* item) = 0;
    virtual IBsUndoTemplateItem* getItem(VBLInt index) = 0;
    virtual VBLInt index() = 0;         // 当前 undo 栈指针位置
    virtual VBLBool canUndo() = 0;
    virtual VBLBool canRedo() = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual Result beginMacro(const char* text) = 0;
    virtual Result endMacro() = 0;
    virtual VBLInt count() = 0;
    virtual Result setUndoLimit(int limit) = 0;
    virtual VBLInt undoLimit() const = 0;
    virtual undoRedoStatus status() = 0;
};

// IBsUndoTemplateItem：每个可撤销操作的基类
class IBsUndoTemplateItem {
    virtual void redo() = 0;   // 重做（正向执行）
    virtual void undo() = 0;   // 撤销（逆向执行）
    virtual const char* text() = 0;  // 操作描述（UI 显示）
};
```

### 1.3 时序图：Undo/Redo 完整流程

```
UI 层       TimelineUX    IBsUndoTemplateStack   TimelineEditor   DataModel   EventBus
  │              │                  │                   │               │          │
  │ [操作：添加 Clip]                │                   │               │          │
  │              │ addClips(...)    │                   │               │          │
  │              ├────────────────── ─ ─ ─             │               │          │
  │              │ [执行完毕]        │                   │               │          │
  │              │ push(AddClipCmd) │                   │               │          │
  │              ├─────────────────►│                   │               │          │
  │              │                  │ stack: [AddClipCmd] ← index        │          │
  │              │                  │                   │               │          │
  │ Cmd+Z        │                  │                   │               │          │
  ├─────────────►│                  │                   │               │          │
  │              │ undoStack->undo()│                   │               │          │
  │              ├─────────────────►│                   │               │          │
  │              │                  │ status = InUndo   │               │          │
  │              │                  │ getItem(index)->undo()             │          │
  │              │                  ├──────────────────►│               │          │
  │              │                  │                   │ 逆向删除 Clip  │          │
  │              │                  │                   ├──────────────►│          │
  │              │                  │ index--           │               │          │
  │              │                  │ status = InOther  │               │          │
  │              │                  │ postEvent("timeline.clip.removed")│          │
  │              │                  ├─────────────────────────────────────────────►│
  │ onEvent() 刷新 UI               │                   │               │          │ → UI
  │◄────────────────────────────────────────────────────────────────────────────────────┤
  │              │                  │                   │               │          │
  │ Cmd+Shift+Z  │                  │                   │               │          │
  ├─────────────►│                  │                   │               │          │
  │              │ undoStack->redo()│                   │               │          │
  │              ├─────────────────►│                   │               │          │
  │              │                  │ getItem(index+1)->redo()          │          │
  │              │                  ├──────────────────►│               │          │
  │              │                  │                   │ 重新添加 Clip  │          │
  │              │                  │                   ├──────────────►│          │
  │              │                  │ index++           │               │          │
```

### 1.4 Macro 合并操作

```
TimelineUX        UndoTemplateStack
    │                    │
    │ beginMacro("paste clips")
    ├───────────────────►│
    │                    │ 开始宏，后续 push 合并为一组
    │ push(AddClipCmd1)  │
    ├───────────────────►│   ← 合并入 macro
    │ push(AddClipCmd2)  │
    ├───────────────────►│   ← 合并入 macro
    │ push(AddTransCmd)  │
    ├───────────────────►│   ← 合并入 macro
    │ endMacro()         │
    ├───────────────────►│
    │                    │ 合并为 "paste clips" 一条记录
    │                    │ 用户 Cmd+Z 一次撤销全部3步
```

---

## 二、BsNet（网络请求模块）

### 接口头
`Interface/BaseService/BsNet/`

### 2.1 职责
- 提供统一的 **HTTP Client**（GET/POST/PUT/DELETE）
- 支持**断点续传下载**
- 支持**代理配置**（系统代理/自定义代理）
- 支持**文件上传**（AI 任务前置上传）
- 供 BsCloudResource / BsCloudDisk / AIManager / BsCloudConfig 使用

### 2.2 核心功能

```cpp
// 下载管理
IBsDownloadManager
  ├── addDownloadTask(url, localPath, callback) → taskId
  ├── pauseTask(taskId)
  ├── resumeTask(taskId)
  ├── cancelTask(taskId)
  └── queryTaskProgress(taskId) → progress%

// HTTP 请求
IBsHttpClient
  ├── get(url, headers, callback)
  ├── post(url, body, headers, callback)
  ├── setProxy(proxyInfo)
  └── setTimeout(ms)
```

### 2.3 时序图：断点续传下载

```
BsCloudResource    BsNet(Download)    文件系统    EventBus
     │                   │                │           │
     │ addDownloadTask(url, localPath)    │           │
     ├──────────────────►│                │           │
     │                   │ HTTP GET Range: bytes=0-   │
     │                   ├───────────────►│           │
     │                   │ 写入本地文件   │           │
     │                   │ 中断/网络断开  │           │
     │                   │                │           │
     │ resumeTask(id)    │                │           │
     ├──────────────────►│                │           │
     │                   │ 读取已下载字节数│           │
     │                   │◄───────────────┤           │
     │                   │ HTTP GET Range: bytes=N-   │
     │                   ├───────────────►│           │
     │                   │ 续传写入       │           │
     │                   │ onComplete()   │           │
     │ callback(localPath)│◄──────────────┤           │
     │◄──────────────────┤                │           │
     │ postEvent("resource.downloaded")  │           │
     ├───────────────────────────────────────────────►│
```

---

## 三、BsCloudResource（云端资源管理）

### 接口头
`Interface/BaseService/BsCloudResource/`

### 3.1 职责
- 管理特效包、模板、转场、贴纸、字体等**内置云端资源**的元数据
- 按需下载资源包（通过 BsNet）
- 缓存已下载的本地资源包路径
- 向 DataModel 提供资源的本地路径，DataModel 可加载使用

### 3.2 核心功能

```cpp
IBsCloudResourceManager
  ├── queryResourceList(category, callback)    // 查询资源列表
  ├── downloadResource(resId, callback)        // 下载指定资源
  ├── getLocalResourcePath(resId) → path       // 获取本地缓存路径
  ├── isDownloaded(resId) → bool
  ├── deleteLocalResource(resId)               // 清理本地缓存
  └── refreshResourceList(category)           // 刷新云端列表
```

### 3.3 时序图：资源下载与使用

```
UI 层      BsCloudResource    BsNet      文件系统    DataModel
  │               │               │           │           │
  │ 用户点击特效包 │               │           │           │
  │ downloadResource(resId)       │           │           │
  ├──────────────►│               │           │           │
  │               │ isDownloaded(resId)?      │           │
  │               │◄──────────────────────────┤           │
  │               │ = false       │           │           │
  │               │ addDownloadTask(url, cachePath)        │
  │               ├──────────────►│           │           │
  │               │               │ 下载资源包zip          │
  │               │               │ 解压到 cachePath       │
  │               │               ├──────────►│           │
  │               │ onDownloadComplete(cachePath)          │
  │               │◄──────────────┤           │           │
  │ 资源下载完成   │               │           │           │
  │◄──────────────┤               │           │           │
  │               │               │           │           │
  │ addEffectToClip(resId, clip)  │           │           │
  ├──────────────►│               │           │           │
  │               │ getLocalResourcePath(resId)→ path      │
  │               │ addEffectResPath(path)    │           │
  │               ├───────────────────────────────────────►│
  │               │               │           │ 加载特效   │
```

---

## 四、BsPreset（预设管理）

### 接口头
`Interface/BaseService/BsPreset/`

### 4.1 职责
- 管理用户自定义**预设参数文件**（滤镜预设/LUT/变速曲线等）
- 预设文件的增删查改
- 预设文件的序列化/反序列化（JSON 格式）
- 导入/导出预设文件（分享给他人）

### 4.2 核心功能

```cpp
IBsPresetManager
  ├── createPreset(type, name, params) → presetId
  ├── deletePreset(presetId)
  ├── renamePreset(presetId, newName)
  ├── getPreset(presetId) → IDmPresetParam*
  ├── listPresets(type) → presetList
  ├── importPreset(filePath) → presetId     // 导入 .preset 文件
  └── exportPreset(presetId, filePath)      // 导出为 .preset 文件
```

### 4.3 时序图：保存并应用预设

```
UI 层（效果面板）  BsPreset    DataModel（IDmEffect）  EventBus
    │                │                  │                  │
    │ 用户调整好滤镜参数               │                  │
    │ createPreset("color", name, params)                  │
    ├───────────────►│                  │                  │
    │                │ 序列化参数为 JSON│                  │
    │                │ 写入 presets/color/xxx.json         │
    │                │ 返回 presetId    │                  │
    │◄───────────────┤                  │                  │
    │                │                  │                  │
    │ 应用预设到 Clip：                 │                  │
    │ applyPreset(presetId, clip)        │                  │
    ├───────────────────────────────────►│                 │
    │                │ getPreset(presetId) → IDmPresetParam* │
    │                │◄──────────────────┤                  │
    │                │                  │ clip->effect()->applyPreset(param)
    │                │                  ├──────────────────►│
    │                │                  │ postEvent("timeline.clip.changed")
    │                │                  ├──────────────────────────────────►│
    │ onEvent() 刷新属性面板            │                  │                │ → UI
    │◄─────────────────────────────────────────────────────────────────────────┤
```

---

## 五、BSWsid（账户与授权）

### 接口头
`Interface/BaseService/BSWsid/`

### 5.1 职责
- 万兴账户登录状态管理
- AI 功能消费额度校验（`IBsWSIDBusinessInfo`）
- 会员类型查询
- 授权令牌刷新

### 5.2 时序图：AI 功能授权校验

```
AIManager         BSWsid         BsNet（云端授权）
    │                │                   │
    │ setManagerConfig("consumeLogNo", no)
    ├──────────────────────────────────── ─ ─
    │                │                   │
    │ addTask(...)   │                   │
    ├─► 内部校验授权  │                   │
    │   queryBusinessInfo(functionType)   │
    ├───────────────►│                   │
    │                │ HTTP 查询额度      │
    │                ├──────────────────►│
    │                │  余额/权限信息     │
    │                │◄──────────────────┤
    │                │ hasSufficientCredit? → true
    │◄───────────────┤                   │
    │ 继续执行任务    │                   │
```
