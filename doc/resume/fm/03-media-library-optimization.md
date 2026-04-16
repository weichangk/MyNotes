03-media-library-optimization.md

# 资源面板模块技术优化方案

---

## 一、缩略图缓存问题与优化

### 问题描述

1. **内存缺乏动态管控**：`QCache` 以"图片数量"为 cost 单位，并非实际字节数。高分辨率缩略图（如 4K 视频首帧）与低分辨率图片在缓存中占用 cost 相同，实际内存使用波动极大，大量 4K 素材时容易导致内存暴涨。

2. **缓存 Key 复用率低**：目前 Key 由 `mediaId + size` 组成，而 `size` 直接取 Delegate 传入的像素尺寸。不同 DPI 环境下同一素材可能产生多份缓存（如 `80x60`、`120x90`、`160x120`），导致相同内容重复缓存。

3. **缓存未分冷热**：全量素材（含不在视口内的）与可见区域素材使用同一缓存池，LRU 淘汰时容易将用户正在浏览的热数据提前淘汰。

4. **并发请求风暴**：同一 `mediaId` 在 Delegate `paint()` 时可能被多次同步调用（列表滚动时连续帧），导致对同一缩略图的异步请求被重复提交。

### 优化方案

**方案 A：字节级别内存感知缓存**

将 `QCache<QString, QPixmap>` 替换为自定义的 `MemoryAwareCache`，以实际字节数（`pixmap.width() * pixmap.height() * 4`）作为 cost：

```cpp
class MemoryAwareCache {
    struct Entry { QPixmap pixmap; int bytes; };
    QMap<QString, Entry> m_map;
    QQueue<QString> m_lruQueue;
    int m_currentBytes = 0;
    int m_maxBytes;
public:
    void insert(const QString& key, const QPixmap& pm) {
        int bytes = pm.width() * pm.height() * 4;
        evictIfNeeded(bytes);
        m_map[key] = {pm, bytes};
        m_lruQueue.enqueue(key);
        m_currentBytes += bytes;
    }
};
```

**方案 B：请求去重（In-flight Deduplication）**

引入 `QSet<QString> m_inFlightKeys`，异步任务提交前检查是否已有相同 Key 的请求在途，避免重复提交：

```cpp
bool getThumbnail(const QString& key, ...) {
    if (m_inFlightKeys.contains(key)) return false;
    m_inFlightKeys.insert(key);
    FFAsync::postReplyableResultTask([key]() {
        // 加载...
    }).then([this, key](QPixmap pm) {
        m_inFlightKeys.remove(key);
        m_cache.insert(key, pm);
        emit updated(key);
    });
}
```

**方案 C：分层缓存（L1 热 + L2 冷）**

对可视区域内素材使用小而快的 L1 缓存（固定 N 张），滚动出视口后降级到 L2 大缓存，重新进入视口时优先从 L2 命中，减少 I/O。

---

## 二、视频预览帧解码问题与优化

### 问题描述

1. **内存上限过大**：`MAX_CACHE_FRAME=1000` 帧，对于 1080P 视频每帧约 6MB，最坏情况预览单个视频占用约 6GB 内存，对低内存设备极不友好。

2. **Worker 线程无停止机制**：鼠标快速在多个视频素材间移动时，旧的解码任务未被及时取消，多个 Worker 并发解码不同视频，造成线程和内存泄漏风险。

3. **帧率固定（40ms/帧）**：不感知实际视频帧率，低帧率视频（如 24fps）与高帧率视频（60fps）均以 25fps 播放，与原片节奏不符。

### 优化方案

**方案 A：取消机制 + 有限缓冲**

引入 `std::atomic<bool> m_cancelFlag`，Worker 每解码一帧后检查 flag，鼠标移开时立即设置取消标记：

```cpp
void FFVideoPreviewHelper::setMedia(const PlayMediaInfo& info) {
    m_cancelFlag.store(true);  // 取消旧任务
    m_cancelFlag.store(false); // 准备新任务
    QMetaObject::invokeMethod(m_worker, "cacheFrames", Q_ARG(...));
}

// Worker 中:
void cacheFrames(...) {
    for (int i = 0; i < maxFrame; i++) {
        if (m_cancelFlag.load()) return; // 及时退出
        auto frame = timeline->getFrame(i);
        m_frameCache.push(frame);
    }
}
```

**方案 B：动态帧率适配**

从 `FFMediaInfo` 获取视频的实际帧率，动态调整播放 Timer 间隔：

```cpp
double fps = mediaInfo->videoFps();
int interval = fps > 0 ? (int)(1000.0 / fps) : 40;
m_playTimer->setInterval(interval);
```

**方案 C：环形缓冲区（Ring Buffer）**

用固定大小（如 30 帧）的环形缓冲替代无界队列，解码线程写、播放线程读，超出容量时覆盖最旧帧，从根本上约束内存上限。

---

## 三、大量素材场景下的性能问题与优化

### 问题描述

1. **Model Reset 代价大**：切换分类或文件夹时，`FFMediaItemModel` 有时调用 `resetModel()`，触发全量视图重建，1000+ 素材时用户可感知闪烁和卡顿。

2. **主线程过滤/排序**：`FMediaItemData::doFilter() / doSort()` 在主线程同步执行，素材量大时会阻塞 UI 线程导致界面冻结。

3. **Delegate 绘制无缓存**：每次 `paint()` 都重新构造文字布局（`QFontMetrics::elidedText`），大量素材滚动时 CPU 消耗明显。

### 优化方案

**方案 A：差量更新代替 resetModel**

利用 `beginInsertRows / endInsertRows`、`beginRemoveRows / endRemoveRows` 进行精确差量通知，避免全量 reset：

```cpp
// 对比前后两份列表，计算 diff
auto [toAdd, toRemove] = diffMediaLists(oldItems, newItems);
for (auto& item : toRemove) {
    int row = indexOf(item);
    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();
}
// 同理处理 toAdd
```

**方案 B：异步过滤/排序**

将 `doFilter / doSort` 迁移到后台线程执行，完成后通过主线程回调更新 Model：

```cpp
FFAsync::postReplyableResultTask([items, filter]() {
    return filterItems(items, filter); // 后台执行
}).then([this](QList<IFFAbstractMedia*> result) {
    // 主线程 diff 更新 Model
    applyDiffUpdate(result);
});
```

**方案 C：Delegate 文字布局缓存**

在 `FMediaItemData` 或 Delegate 中缓存每项的计算文字（`QStaticText`），只在数据变更时重新计算，`paint()` 中直接使用预计算结果：

```cpp
// 在数据变更时预计算
item.cachedText = QStaticText(metrics.elidedText(name, Qt::ElideRight, width));

// paint() 中直接绘制
painter->drawStaticText(textRect.topLeft(), item.cachedText);
```

---

## 四、观察者回调线程安全问题与优化

### 问题描述

底层 VBL 层（`IFFMediaItemManager`）的 `onAfterAddItemEvent / onRemoveItemEvent` 回调可能在非主线程触发（如后台导入完成），而 `FFMediaItemModel` 在回调中直接调用 `insertRows / removeRows` 修改 Model，这在 Qt 中属于未定义行为（Qt Model 操作必须在主线程）。

### 优化方案

在 Observer 回调入口处强制切换到主线程：

```cpp
void FFMediaItemModel::onAfterAddItemEvent(IFFAbstractMedia* media) {
    // 判断当前是否在主线程
    if (QThread::currentThread() != qApp->thread()) {
        // 转发到主线程
        QMetaObject::invokeMethod(this, [this, media]() {
            doInsertMedia(media);
        }, Qt::QueuedConnection);
        return;
    }
    doInsertMedia(media);
}
```

或统一引入事件队列，所有 VBL 回调先入队，主线程循环消费：

```cpp
// VBL 回调（任意线程）
m_pendingEvents.enqueue({EventType::Add, media});
QMetaObject::invokeMethod(this, &FFMediaItemModel::processPendingEvents,
                          Qt::QueuedConnection);

// 主线程消费
void processPendingEvents() {
    while (!m_pendingEvents.isEmpty()) {
        auto event = m_pendingEvents.dequeue();
        // 安全处理...
    }
}
```

---

## 五、文件夹监听与外部变更感知

### 问题描述

当用户在操作系统层面（Explorer/Finder）对已导入的素材进行移动、重命名、删除时，资源面板无法感知，仍显示"失联"素材的旧缩略图，只有重新打开工程才能更新，用户体验差。

### 优化方案

引入 `QFileSystemWatcher` 对已导入素材的父目录进行监听，当目录内容变化时触发重新扫描并比对差异：

```cpp
QFileSystemWatcher* m_watcher = new QFileSystemWatcher(this);
m_watcher->addPath(folder->localPath());

connect(m_watcher, &QFileSystemWatcher::directoryChanged, this,
        [this](const QString& path) {
            // 异步重新扫描该目录，diff 更新 Model
            rescanFolder(path);
        });
```

同时对"失联"素材以特殊样式（半透明 + 警告图标）标记，引导用户手动重链，而非静默显示旧状态。

---

## 六、搜索性能问题与优化

### 问题描述

当前搜索对每次键盘输入都立即触发 `doFilter()`，在大量素材（5000+）时每次过滤遍历整个列表，出现明显输入延迟。

### 优化方案

**方案 A：输入防抖（Debounce）**

搜索框内容变化后延迟 300ms 再触发过滤，连续输入时重置计时器：

```cpp
connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
    m_searchDebounceTimer->start(300); // 重置 300ms 计时
    m_pendingSearchText = text;
});

connect(m_searchDebounceTimer, &QTimer::timeout, this, [this]() {
    applySearch(m_pendingSearchText);
});
```

**方案 B：倒排索引**

对媒体文件名、标签等关键字段预建倒排索引（`QHash<QString, QSet<MediaId>>`），搜索时查索引替代全量遍历，时间复杂度从 O(n) 降至 O(1) 查找 + O(m) 结果合并：

```cpp
// 导入时建索引
void indexMedia(IFFAbstractMedia* media) {
    for (const QString& word : tokenize(media->fileName())) {
        m_invertedIndex[word].insert(media->mediaId());
    }
}

// 搜索时查索引
QSet<MediaId> search(const QString& keyword) {
    return m_invertedIndex.value(keyword.toLower());
}
```
