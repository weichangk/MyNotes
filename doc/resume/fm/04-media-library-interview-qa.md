04-media-library-interview-qa.md

# 资源面板模块面试问答

---

## 一、架构设计类

### Q1：资源面板整体采用什么架构？为什么选择 MVP 而不是 MVC？

**回答：**

资源面板采用 **MVP（Model-View-Presenter）** 分层架构。选择 MVP 而非 MVC 的原因主要有两点：

1. **Qt 的 MVC 局限性**：Qt 的 `QAbstractItemModel` 本质上是 MV（Model-View）框架，Controller 角色模糊。在复杂桌面应用中，如果直接让 View 持有大量业务逻辑，会导致 View 层臃肿难以维护。

2. **可测试性**：MVP 中 View 层通过接口与 Presenter 交互，Presenter 不依赖具体 View 实现，业务逻辑可以脱离 UI 独立测试。MVC 中 View 直接依赖 Model，测试时必须构造完整 UI 环境。

具体落地方式：
- **View 层**（`FMediaItemView`）：仅负责 UI 渲染与事件捕获，通过构造注入 `FFMediaItemManagerPresenter*`，调用 Presenter 的业务方法
- **Presenter 层**（`FFMediaItemManagerPresenter`）：持有业务逻辑，通过 Observer 接口或回调通知 View 更新
- **Model 层**（`FFMediaItemModel` + `FMediaItemData`）：负责数据维护、过滤、排序，通过 `QAbstractItemModel` 接口暴露给 View

---

### Q2：`FFMediaItemModel` 与 `FMediaItemData` 的职责如何划分？

**回答：**

两者是"适配层 + 数据层"的分工：

- **`FMediaItemData`**：是真正的数据持有者和业务处理层，维护 `QList<IFFAbstractMedia*>` 可见列表，实现过滤（`doFilter`）、分组（`doGroupBy`）、排序（`doSort`）逻辑，不继承任何 Qt 类
- **`FFMediaItemModel`**：是 `QAbstractItemModel` 的适配器，将 `FMediaItemData` 的数据翻译成 Qt Model/View 框架所需的接口（`rowCount / data / index / fetchMore`），同时实现多个 VBL Observer 接口监听底层数据变化

这样划分的好处是：过滤/分组等核心业务逻辑与 Qt 框架完全解耦，`FMediaItemData` 可以独立测试；`FFMediaItemModel` 则专注于框架适配，保持薄且简单。

---

### Q3：模块间如何通信？为什么同时用了信号槽、Observer 接口和自定义事件系统三种方式？

**回答：**

三种方式各自覆盖不同的场景需求：

| 通信方式 | 适用场景 | 原因 |
|---|---|---|
| **Qt 信号槽** | View 层内部、同层组件间 | Qt 原生，类型安全，开发效率高 |
| **Observer 接口（`IFF*EventObserver`）** | VBL 底层 → Model 层 | VBL 是 C++ 纯接口层，不依赖 Qt，无法使用信号槽；Observer 接口保证了跨框架通信 |
| **自定义事件（`ListenCustomEvent/SendCustomEvent`）** | 跨模块广播 | 解耦广播场景，发送方不需要知道接收方，类似发布-订阅，比信号槽更适合一对多跨模块广播 |

这是典型的"根据场景选择合适通信手段"的设计，而不是一刀切用某种方式。

---

## 二、缩略图与异步类

### Q4：资源面板的缩略图加载方案是怎么设计的？为什么用双层缓存？

**回答：**

缩略图采用**双层缓存 + 异步懒加载**：

**两层缓存：**
- **路径缓存**（`QCache<QString, QString>`）：`mediaId+size → 文件路径`，避免重复调用 `mediaItem->thumbnailFile()` 做 I/O 查找
- **像素缓存**（`QCache<QString, QPixmap>`）：`mediaId+size → QPixmap`，存储已解码缩放后的图像，避免重复解码

**为什么两层而不是一层？**
因为"取到文件路径"和"将图片解码成 QPixmap"是两个独立的开销：路径查找可能涉及数据库或文件系统访问；解码缩放涉及 CPU 计算。分层缓存可以分别优化两个瓶颈，且路径缓存命中时可以跳过 QPixmap 缓存的序列化开销。

**异步加载流程：**
1. Delegate `paint()` 调用缓存，命中则同步返回 QPixmap
2. 未命中则通过 `FFAsync::postReplyableResultTask` 在线程池异步读取
3. `QtPromise::then` 回调主线程，写入缓存并触发 `update(index)` 局部刷新
4. 弱指针检查保护对象生命周期安全

---

### Q5：高 DPI 下缩略图模糊问题是如何解决的？

**回答：**

问题根因是：Delegate 请求的缩略图尺寸是逻辑像素（如 80×60），但在 2x DPI 屏幕上实际需要 160×120 的物理像素图，如果只读取并缩放到 80×60 后显示，Qt 会再次放大导致模糊。

解决方案：**读取时以目标尺寸的 2 倍加载原图，再按逻辑尺寸缩放**：

```cpp
QSize targetSize = param.size;
QSize readSize = targetSize * 2.0;  // 读取 2x 尺寸
QImageReader reader(thumbnailFile);
reader.setScaledSize(readSize);     // 读取时直接缩放到 2x
QImage image = reader.read();
// 最终存入像素缓存的 QPixmap 是 2x 物理尺寸
// Delegate 以 devicePixelRatio=2 绘制，显示清晰
```

本质是让像素缓存存储的是**物理像素尺寸**的图，绘制时配合 `QPixmap::setDevicePixelRatio(2.0)` 确保渲染清晰。

---

### Q6：鼠标悬停视频预览是怎么实现的？如何保证不卡主线程？

**回答：**

实现分为三个部分：

1. **Worker 线程解码**：`FFVideoPreviewHelper` 将解码逻辑封装为 `QObject` 并移到独立 `QThread`，通过 `QMetaObject::invokeMethod` 异步触发帧缓存任务（`cacheAllFrame`），主线程完全不参与解码

2. **底层解码能力复用**：使用 `FFTimelineBuilder` 构建临时 Timeline（复用编辑器底层渲染引擎 VBL），调用 `timeline->getFrame()` 逐帧解码，不重复实现解码逻辑

3. **Timer 驱动播放**：主线程用 40ms `QTimer` 驱动播放，每次 tick 从帧缓冲（`m_cacheFrame`）取当前帧，发送 `sigUpdate(index)` 触发 Delegate `update(index)` 局部重绘；`QMutexLocker` 保护帧缓冲的读写线程安全

**不卡主线程的关键**：解码（CPU 密集）在 Worker 线程；主线程只做"取帧 + 触发局部重绘"，开销极小。

---

### Q7：异步任务中如何处理对象生命周期问题，防止野指针？

**回答：**

这是异步编程中最容易出 bug 的地方。方案是使用**弱指针（`QWeakPointer` / `std::weak_ptr`）**：

```cpp
auto mediaWeakPtr = QWeakPointer<IFFAbstractMedia>(mediaItem);
auto thisWeakPtr = QWeakPointer<FFThumbnailCache>(this);

FFAsync::postReplyableResultTask([mediaWeakPtr]() -> QPixmap {
    auto media = mediaWeakPtr.lock();
    if (!media) return {};           // 对象已销毁，安全退出
    // 执行解码...
    return pixmap;
}).then([thisWeakPtr](QPixmap pm) {
    auto self = thisWeakPtr.lock();
    if (!self) return;               // Cache 对象已销毁，安全退出
    self->m_cache.insert(key, pm);
});
```

**关键点**：
- 后台任务捕获弱指针而非强指针，不延长对象生命周期
- 回调执行前先 `lock()` 升为强指针，检查有效性
- 即使 UI 层已经销毁了对应的 View 或 Cache，后台任务依然安全退出，不会 crash

---

## 三、Qt 技术类

### Q8：为什么使用自定义 Delegate 而不是 QLabel / QWidget 方式？性能差异在哪里？

**回答：**

**`QWidget` 方式（每个 item 一个 widget）的问题：**
- 每个 item 创建一个 `QWidget` 子控件，1000 个素材 = 1000 个 widget 对象
- 每个 widget 都有独立的事件循环、坐标系统、绘制缓冲，内存开销大
- Qt 不擅长管理大量子 widget 的布局与重绘，滚动时性能极差

**自定义 Delegate 方式的优势：**
- 只有一个 `QListView` 控件，Delegate 在 `paint()` 时按需绘制当前可见区域的 item
- 内存开销固定（只与可见区域大小成正比，不与总素材数量成正比）
- `paint()` 调用由 Qt 框架统一调度，支持 `update(index)` 局部刷新，避免全量重绘
- 可以精确控制绘制逻辑（圆角、阴影、hover 动效、进度条叠加等），灵活性远超标准 widget

**性能数字对比**（以 1000 个素材为例）：
- Widget 方式：启动时创建 1000 个 QWidget，内存约 200MB+，滚动帧率 < 20fps
- Delegate 方式：只有 1 个 QListView + 1 个 Delegate，内存约 10MB，滚动帧率稳定 60fps

---

### Q9：`QAbstractItemModel` 的 `fetchMore / canFetchMore` 是怎么实现分页加载的？

**回答：**

Qt 的分页加载机制：
- `canFetchMore(QModelIndex)`：返回 `true` 表示还有更多数据可加载
- `fetchMore(QModelIndex)`：被 `QListView` 在快接近列表底部时自动调用，触发加载下一批

在 `FFMediaItemModel` 中的实现逻辑：

```cpp
bool FFMediaItemModel::canFetchMore(const QModelIndex& parent) const {
    // m_loadedCount 是已加载到 Model 的数量
    // m_totalCount 是 FMediaItemData 中全量数据数量
    return m_loadedCount < m_data->totalCount();
}

void FFMediaItemModel::fetchMore(const QModelIndex& parent) {
    int remainder = m_data->totalCount() - m_loadedCount;
    int itemsToFetch = qMin(PAGE_SIZE, remainder); // 每次加载 PAGE_SIZE 条

    beginInsertRows(QModelIndex(), m_loadedCount,
                    m_loadedCount + itemsToFetch - 1);
    m_loadedCount += itemsToFetch;
    endInsertRows();
}
```

**效果**：打开有 5000 个素材的文件夹时，只渲染前 50 个，用户向下滚动时自动按需加载，首屏渲染速度快，内存增长平缓。

---

### Q10：`beginInsertRows / endInsertRows` 与 `resetModel` 有什么区别？什么时候用哪个？

**回答：**

| 方式 | 行为 | 适用场景 |
|---|---|---|
| `beginInsertRows / endInsertRows` | 精确通知 View 哪几行新增，View 只更新对应区域 | 增量插入（如导入新素材） |
| `beginRemoveRows / endRemoveRows` | 精确通知 View 哪几行删除 | 删除部分素材 |
| `beginResetModel / endResetModel` | 通知 View 全量数据已变，View 全部重建 | 切换文件夹、全量过滤结果变化 |

**原则**：尽量使用精确的增删通知，只在数据结构发生根本性变化时才用 `resetModel`。

`resetModel` 的代价：View 会清空所有已渲染 item 的缓存（包括已计算好的 item 高度、已缓存的文字布局），重新全量渲染，1000+ 条时用户可以感知到明显闪烁和延迟。

---

## 四、业务与产品类

### Q11：资源面板支持哪些类型的素材？如何做到统一管理不同类型？

**回答：**

支持的素材类型：本地视频、音频、图片、GIF、文件夹、云端素材（云盘）、AI 生成素材等。

统一管理的核心是 VBL 层的 **抽象媒体接口** `IFFAbstractMedia`：

```cpp
class IFFAbstractMedia {
public:
    virtual QString mediaId() const = 0;
    virtual QString localFile() const = 0;
    virtual QString thumbnailFile() const = 0;
    virtual FFMediaType mediaType() const = 0;  // Video/Audio/Image/...
    virtual FFMediaInfo* mediaInfo() const = 0;
};
```

所有媒体类型（`FFVideoMedia`、`FFAudioMedia`、`FFImageMedia`、`FFCloudMedia` 等）均实现此接口。上层的 `FMediaItemData`、`FFMediaItemModel`、Delegate 只依赖 `IFFAbstractMedia*`，无需关心具体类型。

类型相关的差异（如视频显示时长、图片显示尺寸、云端显示下载状态）通过 Delegate 按 `mediaType()` 分支渲染，或通过策略模式注入不同的 `IFFMediaItemFilterProxy`。

---

### Q12：当用户导入大量素材（如 1000 个文件）时，如何保证 UI 不卡顿？

**回答：**

核心策略是**避免同步全量处理**，分三个层次优化：

1. **VBL 层异步导入**：`IFFMediaItemManager::importMedias()` 在后台线程执行文件解析、元数据提取和缩略图生成，不阻塞主线程

2. **Model 层批量通知**：后台逐个导入完成后触发 `onAfterAddItemEvent`，但 `FFMediaItemModel` 不立即刷新，而是将变更收集到 `m_pendingEvents` 队列

3. **View 层更新节流**：`FMediaListView` 的 `m_pUpdateTimer`（100ms）将大量连续的更新事件合并为一次批量刷新，配合 `update(index)` 局部重绘（不触发全量重建）

同时，`fetchMore` 分页机制保证了即使一次性收到 1000 个 item，View 也只渲染当前可见的若干个，剩余的按需加载。

---

### Q13：资源面板的搜索功能是如何实现实时过滤的？

**回答：**

搜索过滤基于**策略模式 + 主线程同步过滤**（当前方案）：

1. 用户在 `FVCGSearchChainWidget` 输入关键字
2. `FMediaItemView::setSearchKeyword(keyword)` → Presenter → `FFMediaItemModel::setViewFilterProxy(proxy)`
3. `FMediaItemData::addFilter(new KeywordFilter(keyword))` 注入过滤策略
4. `FMediaItemData::doFilter()` 遍历所有媒体，执行文件名模糊匹配
5. 更新可见列表 → Model `resetModel()` → View 刷新

**过滤接口**（策略模式）：

```cpp
class IFFMediaItemFilter {
public:
    virtual bool accept(IFFAbstractMedia* media) const = 0;
};
```

新增过滤维度（如按标签、按 AI 类型）只需实现接口并注入，无需修改 `FMediaItemData` 核心逻辑。

**已知优化点**：当前 `doFilter` 在主线程同步执行，大量素材时有输入延迟风险（见优化文档第六节）。

---

### Q14：如果让你给资源面板做一个最重要的技术改进，你会选择什么？

**回答：**

我会优先解决**主线程观察者回调的线程安全问题**，原因如下：

当前 VBL 层的 `onAfterAddItemEvent` 等回调可能在非主线程触发（后台导入完成时），而 `FFMediaItemModel` 直接在回调中调用 `insertRows` 修改 Qt Model，这在 Qt 文档中明确标注为未定义行为——Qt 所有 Model/View 操作必须在主线程。

虽然目前线上没有明显崩溃，但这是一个**潜在的、偶发的、难以复现的崩溃隐患**，在高并发导入（如拖入大量文件）时风险更高。

改进方案：在所有 VBL Observer 回调入口处通过 `QMetaObject::invokeMethod(..., Qt::QueuedConnection)` 强制切换到主线程，或统一引入事件队列，彻底消除这一隐患。

这个改进代价小、收益高，且对外不可见，但能显著提升模块稳定性。

---

## 五、C++ 相关

### Q15：模块中如何管理 `IFFAbstractMedia*` 这类裸指针的生命周期？

**回答：**

`IFFAbstractMedia*` 的生命周期由 VBL 层的 `IFFMediaItemManager` 统一管理：
- 媒体对象在导入时由 Manager 创建，在删除/工程关闭时由 Manager 销毁
- 上层（`FMediaItemData` 等）持有的是**非拥有裸指针**，不负责释放

防止悬空指针的机制：
1. **Observer 回调**：VBL 在销毁媒体对象前触发 `onBeforeRemoveItem` 回调，上层在此回调中从列表移除对应指针
2. **异步任务中使用弱指针**：`FFThumbnailCache` 等异步回调中将 `IFFAbstractMedia*` 包装为 `QWeakPointer` 传入 lambda，后台任务通过 `lock()` 检查有效性后再访问
3. **工程关闭时清理**：`FFMediaItemModel` 连接工程关闭信号（`sigProjectClosed`），在回调中清空所有指针列表

这是典型的"外部所有权 + 通知清理"模式，在 Qt + C++ 混合环境中比 `shared_ptr` 更轻量，但要求 VBL 层的生命周期通知必须可靠。
