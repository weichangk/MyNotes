01-media-library-technical-details.md

# 资源面板模块技术实现细节

> 项目：Wondershare Filmora（C++17 + Qt 5.15）
> 模块：资源面板（FMediaLibraryView / FFMediaLibrary）

---

## 一、模块概述

资源面板（Media Library）是 Filmora 编辑器的核心入口面板，承载本地媒体文件、云端素材、AI 生成素材等全类型资源的统一管理与展示。用户通过该面板完成媒体导入、浏览、搜索、过滤、预览，并将素材拖入时间轴进行编辑。

模块在代码上分为两个主要子工程：

| 子工程 | 路径 | 职责 |
|---|---|---|
| `FMediaLibraryView` | `Src/FMediaLibraryView` | UI 展示层（视图、列表、委托、绘制） |
| `FFMediaLibrary` | `Src/FFMediaLibrary` | 业务逻辑层（Presenter、数据模型、缩略图缓存、预览） |
| `FFVBLModel`（子模块） | `3rdparty/FilmoraFrameworkPlatform/Src/FFVBLModel` | 底层媒体实体层（媒体对象、元数据解析、媒体库索引） |

---

## 二、整体架构

资源面板整体采用 **MVP 分层架构**，各层职责清晰、单向依赖：

```
┌─────────────────────────────────────────────────────────┐
│                    View 层（UI）                          │
│  FMediaLibraryView / FMediaCategoryView                  │
│  FMediaFolderView / FMediaItemView                       │
│  FMediaListView / FMediaItemIconView                     │
│  自定义 Delegate / Painter（FMediaItemIconDelegate 等）    │
└────────────────────────┬────────────────────────────────┘
                         │ Presenter 接口调用
┌────────────────────────▼────────────────────────────────┐
│                   Presenter 层（业务协调）                 │
│  FFMediaLibraryPresenter                                 │
│  FFMediaCategoryPresenter                                │
│  FFMediaItemManagerPresenter                             │
└────────────────────────┬────────────────────────────────┘
                         │ QAbstractItemModel 接口
┌────────────────────────▼────────────────────────────────┐
│                   Model 层（数据）                         │
│  FFMediaItemModel（QAbstractItemModel 适配器）             │
│  FMediaItemData（媒体集合维护、过滤、分组、排序）            │
│  FFMediaTreeModel（文件夹树结构）                          │
└────────────────────────┬────────────────────────────────┘
                         │ VBL 接口（IFFMediaItemManager 等）
┌────────────────────────▼────────────────────────────────┐
│                   VBL 底层媒体层                           │
│  IFFAbstractMedia / FFMediaItem                          │
│  FFMediaLibrary / FFMediaFactory                         │
│  FFMediaInfo / FFMediaContentAnalyser（元数据解析）        │
└─────────────────────────────────────────────────────────┘
```

---

## 三、主业务流程

### 3.1 媒体导入流程

```
用户拖拽文件 / 选择文件
       │
FMediaItemView::importMedia(FFMediaImportParam)
       │
FFMediaItemManagerPresenter::importMedia()
       │
IFFMediaItemManager::importMedias()     ← VBL 层，异步执行
       │  ┌──────────────────────────────────┐
       │  │ 文件格式识别（FFMediaInfoFactory） │
       │  │ 元数据解析（分辨率/时长/码率等）     │
       │  │ 生成缩略图文件（thumbnailFile）     │
       │  │ 写入媒体库索引                     │
       │  └──────────────┬───────────────────┘
       │                 │ 触发 Observer 回调
IFFMediaItemEventObserver::onAfterAddItemEvent()
       │
FFMediaItemModel 更新 FMediaItemData
       │  ├── insertRows（局部刷新）
       │  └── 通知 View 刷新
QListView / Delegate 重绘（向缩略图缓存请求 QPixmap）
```

### 3.2 缩略图异步加载流程

缩略图采用**双层缓存 + 异步懒加载**机制：

```
Delegate::paint() 调用 FFThumbnailCache::getThumbnailBySize()
       │
       ├── [命中路径缓存] → 检查像素缓存
       │       ├── [命中像素缓存] → 直接返回 QPixmap，同步绘制
       │       └── [未命中] → 触发异步加载（见下方）
       │
       └── [未命中路径缓存] → 从 mediaItem->thumbnailFile() 取文件路径
                                       │
                      FFAsync::postReplyableResultTask（线程池）
                                       │
                       QImageReader 读取原图 → 按策略缩放/裁剪
                       （注：读取 2x 尺寸，防止高 DPI 下模糊）
                                       │
                      QtPromise::then 回调主线程
                                       │
                      插入路径缓存 + 像素缓存（QCache<QString, QPixmap>）
                                       │
                      updateCallback 触发 view->update(index) 局部刷新
```

**缓存 Key 设计**：`mediaId + size`（场景裁剪素材会附加 sceneIndex/sceneStart/sceneEnd），避免不同尺寸缓存互相污染。

### 3.3 鼠标悬停视频预览流程

```
用户鼠标悬停到视频素材
       │
FMediaListView 检测 hover 事件
       │
FFVideoPreviewHelper::setMedia(PlayMediaInfo)
       │
Worker 线程（QObject 移到独立 QThread）
       │  FFTimelineBuilder 构建临时 timeline
       │  IFFTimeline::getFrame() 逐帧解码
       │  缓存至 m_cacheFrame（最多 MAX_CACHE_FRAME=1000 帧）
       │
QTimer（~40ms，约 25fps）触发
       │
sigUpdate(index) → View 局部刷新，从帧缓存取当前帧绘制
```

### 3.4 分类与文件夹切换流程

```
用户点击分类 Tab / 文件夹节点
       │
FMediaCategoryView / FMediaFolderView 捕获交互
       │
FFMediaCategoryPresenter::switchCategory() / switchFolder()
       │  （首次访问分类时惰性创建对应 Presenter）
       │
FFMediaItemModel::load(IFFFolderMedia* folder)
       │
FMediaItemData::load(folder)  ← 从 VBL 层获取当前文件夹媒体集合
       │  doFilter()   ← 应用当前过滤条件
       │  doGroupBy()  ← 应用分组逻辑（按类型/日期等）
       │  doSort()     ← 排序
       │
Model::resetModel() / insertRows() → View 刷新
```

### 3.5 搜索与过滤流程

```
用户在搜索框输入关键字
       │
FVCGSearchChainWidget → FMediaItemView::setSearchKeyword(keyword)
       │
Presenter 通知 FFMediaItemModel::setViewFilterProxy(proxy)
       │
FMediaItemData::addFilter(IFFMediaItemFilter)
FMediaItemData::doFilter()   ← 执行关键字匹配 / 类型过滤
       │
Model 发出数据变更信号 → View 刷新显示过滤结果
```

---

## 四、核心技术点

### 4.1 QAbstractItemModel + 自定义 Delegate 渲染架构

UI 展示层以 Qt 的 Model/View 架构为基础：

- **FFMediaItemModel** 继承 `QAbstractItemModel`，为 `QListView` 提供标准数据接口（`index / parent / rowCount / data / fetchMore`）
- **FMediaItemData** 作为 model 内部数据源，维护经过过滤/分组/排序后的 `QList<IFFAbstractMedia*>` 可见列表
- **自定义 Delegate**（`FMediaItemIconDelegate`、`FMediaItemTableDelegate`）负责在不同视图模式（图标/列表/详情）下绘制缩略图、文件名、时长、类型标记等，利用 `QPainter` 完全自绘，支持 hover 状态动效
- **分页加载**：通过 `canFetchMore / fetchMore` 实现大量素材时的按需加载，避免一次性渲染全量数据

### 4.2 双层缩略图缓存

```cpp
// 路径缓存：mediaId+size → 文件路径
QCache<QString, QString> m_thumbnailPathCache;

// 像素缓存：mediaId+size → QPixmap（已缩放到目标尺寸）
QCache<QString, QPixmap> m_thumbnailPixmapCache;
```

- 两级缓存分离关注点：路径缓存避免重复 I/O 查找；像素缓存避免重复解码缩放
- 缓存大小通过 `setMaxCost()` 控制内存上限，采用 LRU 策略自动淘汰
- 为解决 Hi-DPI 下缩略图模糊问题，实际以目标尺寸 **2 倍**读取图像后再缩放

### 4.3 FFAsync 自研异步框架

项目集成了自研的 `FFAsync` 异步任务框架（配合 `QtPromise`），解决 Qt 原生 `QFuture/QtConcurrent` 在复杂链式异步场景下的不足：

```cpp
// 示例：缩略图后台加载
FFAsync::postReplyableResultTask(
    [mediaWeakPtr, size]() -> QPixmap {
        if (mediaWeakPtr.isNull()) return {};
        QImageReader reader(mediaWeakPtr->thumbnailFile());
        // 读取并缩放...
        return pixmap;
    }
).then([thisWeakPtr, index](QPixmap pixmap) {
    if (thisWeakPtr.isNull()) return;
    // 回写主线程缓存并触发局部刷新
    thisWeakPtr->m_cache.insert(key, pixmap);
    emit thisWeakPtr->updateCallback(index);
});
```

- **弱指针生命周期保护**：避免异步回调时对象已销毁导致的 UAF（use-after-free）
- **任务优先级**：框架支持 `kTimeCriticalPriority` 等优先级，UI 可视区域缩略图优先加载
- **主线程回调**：`.then()` 自动在主线程执行，无需手动 `QMetaObject::invokeMethod`

### 4.4 视频帧预解码机制

- `FFVideoPreviewHelper` 将解码工作迁移到独立 `QThread`，通过 `QMetaObject::invokeMethod` 跨线程调用
- 使用 `FFTimelineBuilder` 构建临时播放 Timeline（复用编辑器底层渲染能力），逐帧调用 `getFrame()` 解码
- 帧缓冲最多 1000 帧，通过 `QMutexLocker` 保证多线程访问安全
- UI 播放靠 `QTimer`（40ms 间隔）驱动，发送 `sigUpdate(index)` 触发 Delegate 局部重绘

### 4.5 元数据解析（VBL 层）

媒体元数据（分辨率、时长、帧率、编解码格式、音频通道数等）由 VBL 层 `FFMediaInfo / FFMediaContentAnalyser` 负责解析，隔离了上层对底层解码库（ffmpeg 等）的直接依赖：

- `FFMediaInfoFactory` 工厂方法根据文件扩展名/MIME 类型分发到对应解析器
- 解析结果缓存在 `FFMediaItem` 对象中，通过 `IFFAbstractMedia` 接口暴露给上层

### 4.6 更新节流（UI 性能保护）

`FMediaListView` 使用 **合并更新定时器**（100ms 延迟）防止短时大量媒体事件导致界面频繁重绘：

```
收到大量 onAfterAddItemEvent
       │
将 index 加入 m_pNeedUpdateMedia 待更新集合
       │
m_pUpdateTimer（100ms 单次触发）
       │
批量处理 m_pNeedUpdateMedia → update(index) 局部刷新
```

---

## 五、设计模式应用

### 5.1 MVP（Model-View-Presenter）

模块最核心的架构模式，完整落地了 MVP 分层：

- **View 层**：`FMediaItemView`、`FMediaListView` 等，只负责 UI 渲染与用户交互事件捕获，不持有业务逻辑
- **Presenter 层**：`FFMediaLibraryPresenter`、`FFMediaItemManagerPresenter`，协调 View 与 Model，持有业务决策逻辑
- **Model 层**：`FFMediaItemModel`（数据模型）、`FMediaItemData`（数据处理），对 View 完全无感知

View 通过构造函数注入 Presenter 引用，Presenter 通过 Observer 或接口回调通知 View 更新。

### 5.2 观察者模式（Observer）

模块间数据流转大量依赖自定义观察者接口：

- `IFFMediaItemEventObserver`：监听底层媒体增删改事件（`onBeforeAddItem / onAfterAddItem / onRemoveItem / onRenameItem`）
- `IFFMediaFolderEventObserver`：监听文件夹结构变化
- `IFFFolderMedia::ChildMediaItemDestroyObserve`：监听子媒体对象销毁，及时清理引用

`FFMediaItemModel` 同时实现多个 Observer 接口，作为 VBL 层到 UI 层的"数据桥"。

### 5.3 工厂模式（Factory）

- `FFMediaFactory`：根据媒体类型（视频/音频/图片/GIF/文件夹）创建对应的 `IFFAbstractMedia` 实例
- `FFMediaInfoFactory`：根据文件类型分发到对应的元数据解析器
- `FFFolderMediaItemFactory`：负责文件夹类型媒体对象的创建

工厂模式隔离了上层对具体媒体类型实现的依赖，新增媒体类型只需扩展工厂，不影响上层代码。

### 5.4 策略模式（Strategy）

过滤与排序逻辑通过策略接口实现，支持运行时动态替换：

- `IFFAbstractMediaFilterProxy`：抽象过滤策略，`FMediaItemData` 通过 `setViewTypeFilterProxy()` 注入不同策略
- 分组策略（`doGroupBy`）、排序策略（`doSort`）也通过枚举/接口切换，无需修改核心逻辑

### 5.5 代理模式（Proxy / Delegate）

- Qt Delegate 机制（`QAbstractItemDelegate` 子类）作为 View 与渲染逻辑之间的代理，将复杂绘制逻辑与列表控件解耦
- `FFThumbnailCache` 作为缩略图访问的缓存代理，对 Delegate 屏蔽了文件 I/O 和解码细节

### 5.6 缓存模式（Cache）

- `FFThumbnailCache`：双层 LRU 缓存，路径缓存 + 像素缓存
- `FFVideoPreviewHelper`：帧缓冲（`m_cacheFrame`），预先解码并在内存中维护待播放帧

### 5.7 惰性初始化（Lazy Initialization）

- 每个分类对应的 `FFMediaCategoryPresenter` 在首次访问时按需创建，避免启动时一次性初始化大量模块
- 缩略图按需异步加载（Delegate 绘制时才触发加载，不预加载全量）

---

## 六、模块间通信机制

| 通信方式 | 使用场景 |
|---|---|
| Qt 信号/槽 | View 层内部、Model→View 数据变更通知 |
| 自定义事件（`ListenCustomEvent / SendCustomEvent`） | 跨模块广播（如 `NotifyUpdateMediaItemView`） |
| Observer 接口（`IFF*EventObserver`） | VBL 底层 → Model 层的数据变更回调 |
| Presenter 接口调用 | View → Presenter 的业务操作 |
| VBL 接口（`IFFMediaItemManager` 等） | Presenter → 底层媒体管理的操作调用 |
| 全局 Manager / 宏（`FF_PROJECT_MANAGER`） | 跨模块访问工程编辑器等全局服务 |
| 异步回调（FFAsync / QtPromise） | 后台任务结果回主线程通知 |

---

## 七、关键类职责速查

| 类名 | 层次 | 核心职责 |
|---|---|---|
| `FMediaLibraryView` | View | 主容器，分类 Tab、工具栏、子视图管理 |
| `FMediaItemView` | View | 素材颗粒区视图，导入/预览/定位 API |
| `FMediaListView` | View | 具体列表/网格控件，鼠标事件、更新节流 |
| `FFMediaItemModel` | Model | `QAbstractItemModel` 适配 + 多 Observer 实现 |
| `FMediaItemData` | Model | 媒体集合维护、过滤/分组/排序逻辑 |
| `FFMediaLibraryPresenter` | Presenter | 顶层业务协调，分类切换、Presenter 生命周期 |
| `FFMediaItemManagerPresenter` | Presenter | 导入、删除、移动、下载等素材操作 |
| `FFThumbnailCache` | 服务 | 双层缓存 + 异步缩略图加载 |
| `FFVideoPreviewHelper` | 服务 | Worker 线程帧解码 + Timer 播放 |
| `IFFAbstractMedia` | VBL | 底层媒体实体接口（文件路径/缩略图/元数据） |
| `FFMediaInfo` | VBL | 媒体元数据解析（时长/分辨率/编码） |
| `FFMediaFactory` | VBL | 媒体对象工厂，按类型创建实例 |
