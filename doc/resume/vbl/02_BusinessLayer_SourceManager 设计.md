# SourceManager 模块详细设计文档

> 所属层：BusinessLayer
> 接口头：`Interface/BusinessLayer/SourceManager/IVbSourceManager.h`
> 工厂函数：`createVbSourceManager(type, resourceTypes, bCloud)`
> 命名空间：`VBL::`

---

## 1. 模块职责

SourceManager 负责管理 VBL 工程的**媒体资源区**，包括：

- **媒体文件夹管理**：新建、删除、重命名、移动文件夹（树形结构）
- **媒体项管理**：导入、删除、重命名、移动媒体条目
- **资源分类支持**：区分用户媒体（视频/音频）、内置云资源（特效/模板/音乐）
- **Undo 集成**：所有变更操作自动推入 UndoStack
- **事件通知**：资源区变更通过 EventBus 广播 `media.*` 事件

---

## 2. 核心接口说明

```cpp
class IVbSourceManager : virtual public IVbResourceManager {
public:
    // 初始化
    virtual Result init(IDmMediaFolder* projectFolder, IBsUndoTemplateStack*, VBLBool bPostEnable) = 0;
    virtual Result reset() = 0;
    virtual Result setUndoManager(IBsUndoTemplateStack*) = 0;

    // 文件夹操作
    virtual Result addMediaFolder(VBLPCharList* folderPaths, IDmMediaFolder* folder) = 0;
    virtual Result removeMediaFolder(IDmMediaFolder* folder) = 0;
    virtual Result renameMediaFolder(VBLConstPChar alias, IDmMediaFolder* folder) = 0;
    virtual Result moveMediaFolder(IDmMediaFolder* dest, IDmMediaFolder* src, ...) = 0;
    virtual Result copyMoveMediaFolder(IDmMediaFolder* dest, IDmMediaFolder* src, ...) = 0;

    // 媒体项操作
    virtual Result addMediaItems(IDmMediaItemList* items, IDmMediaFolder* folder) = 0;
    virtual Result removeMediaItems(IDmMediaItemList* items, IDmMediaFolder* folder) = 0;
    virtual Result renameMediaItem(IDmMediaItem* item, VBLConstPChar alias) = 0;
    virtual Result moveMediaItems(IDmMediaFolder* dest, IDmMediaItemList* items, IDmMediaFolder* src) = 0;
    virtual Result replaceMediaItem(IDmMediaItem* old, IDmMediaItem* newItem, IDmMediaFolder* folder) = 0;
    virtual Result updateMediaItemFrame(IDmMediaItem* item, Rational frame) = 0;
};

// 工厂函数（按资源分类创建不同实例）
IVbSourceManager* createVbSourceManager(BsCategoryType type, BsResourceTypeList resTypes, VBLBool bCloud);
IVbSourceManager* createVbSourceManagerWithSlug(VBLConstPChar categorySlug, BsResourceTypeList resTypes);
```

---

## 3. 资源分类

| BsCategoryType | 说明 | 典型用途 |
|---|---|---|
| `catMedia` | 用户导入媒体 | 视频/音频/图片 |
| `catAudio` | 音乐资源 | 内置/云端音乐库 |
| `bsResMedia` | 内置媒体资源 | 特效/贴纸 |
| `bsResIntroTemplate` | 开场模板 | 模板资源区 |
| `bsResAudio` | 内置音效 | 音效库 |

---

## 4. 依赖关系

```
SourceManager
  ├── PRIVATE 持有 → IBsUndoTemplateStack   （操作可撤销）
  ├── 读写     → IDmMediaFolder            （DataModel 媒体树根节点）
  ├── 读写     → IDmMediaItem              （具体媒体条目）
  └── 发布     → IMsEventBus               （media.* 事件）
```

---

## 5. 时序图

### 5.1 导入媒体文件

```
UI 层         SourceManager      DataModel         UndoStack      EventBus
  │                │                  │                 │              │
  │ addMediaFolder(paths, folder)     │                 │              │
  ├──────────────► │                  │                 │              │
  │                │ 解析文件路径      │                 │              │
  │                │ 创建 IDmSourceMedia 对象           │              │
  │                │ folder->addMediaItem(item)         │              │
  │                ├─────────────────►│                 │              │
  │                │                  │ 更新 IDmMediaFolder 树         │
  │                │ push(AddMediaCmd)│                 │              │
  │                ├──────────────────────────────────── ►│            │
  │                │  postEvent("media.items.added")    │              │
  │                ├──────────────────────────────────────────────────►│
  │ onEvent() 刷新媒体区 UI           │                 │              │ → UI
  │◄──────────────────────────────────────────────────────────────────────┤
```

### 5.2 移动媒体项到文件夹

```
UI 层         SourceManager      DataModel         UndoStack      EventBus
  │                │                  │                 │              │
  │ moveMediaItems(dest, items, src)  │                 │              │
  ├──────────────► │                  │                 │              │
  │                │ src->removeMediaItem(item)         │              │
  │                ├─────────────────►│                 │              │
  │                │ dest->addMediaItem(item)           │              │
  │                ├─────────────────►│                 │              │
  │                │ push(MoveMediaCmd)│                │              │
  │                ├──────────────────────────────────── ►│            │
  │                │ postEvent("media.items.moved")      │              │
  │                ├──────────────────────────────────────────────────►│
  │ onEvent() 刷新 UI │              │                 │              │ → UI
  │◄──────────────────────────────────────────────────────────────────────┤
```

### 5.3 替换媒体（媒体区替换）

```
UI 层         SourceManager      DataModel         UndoStack      EventBus
  │                │                  │                 │              │
  │ replaceMediaItem(old, new, folder)│                 │              │
  ├──────────────► │                  │                 │              │
  │                │ folder->replaceItem(old, new)      │              │
  │                ├─────────────────►│                 │              │
  │                │ 同步更新时间线中   │                 │              │
  │                │ 引用 old 的 clips │                 │              │
  │                │ clip->setMediaRef(new)             │              │
  │                ├─────────────────►│                 │              │
  │                │ push(ReplaceCmd) │                 │              │
  │                ├──────────────────────────────────── ►│            │
  │                │ postEvent("media.item.replaced")   │              │
  │                ├──────────────────────────────────────────────────►│
  │ onEvent() 刷新 UI │              │                 │              │ → UI
  │◄──────────────────────────────────────────────────────────────────────┤
```

---

## 6. 设计要点

| 要点 | 说明 |
|---|---|
| 多实例 | ProjectEditor 同时持有媒体 SourceManager 和音乐 SourceManager 两个独立实例 |
| 云资源 | `bCloud=true` 时实例对接 BsCloudResource 做在线资源懒加载 |
| Slug 创建 | `createVbSourceManagerWithSlug` 用于自定义资源类型标识符，面向扩展资源类别 |
| 事件粒度 | 区分 `media.items.added/removed/moved/renamed/replaced` 等细粒度事件 |
