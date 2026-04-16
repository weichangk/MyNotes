08-timeline-interview-qa.md

# 时间线模块面试问答

---

## 一、架构设计类

### Q1：时间线模块整体架构是怎样的？为什么选用 Qt Graphics View Framework 而不是 QListView/QTableView 方案？

**回答：**

时间线采用 MVP 分层：View 层（`FGraphicsTimelineScene` + 各类图元/Service）、Presenter 层（`FTimelinePresenter` 及子 Presenter）、数据层（`IFFTimeline / IFFTrack / IFFClip` 接口 + VBL 渲染引擎）。

选择 Graphics View Framework 而非 Model/View 框架的原因：

1. **自由坐标系**：时间轴本质上是二维画布（X 轴为时间，Y 轴为轨道高度），需要支持任意缩放、子元素（关键帧菱形、Trim Handle、波形叠加）的精确定位。QListView 只能做线性列表，无法满足
2. **复杂交互**：每个 Clip 有独立的 hover、press、drag、resize 区域；Trim Handle 是 ClipItem 的子区域，需要独立命中测试。Graphics View 的 `itemAt()` + `boundingRect()` + `shape()` 机制天然支持这些
3. **性能**：Graphics View 支持视口裁剪（只渲染可见区域）、脏区域局部刷新（`update(rect)`），大量 Clip 时性能远好于 Widget 方案

---

### Q2：FGraphicsTimelineScene 和 FTimelinePresenter 分别负责什么？如何分工？

**回答：**

- **FGraphicsTimelineScene**（View 层）：维护轨道 ID → TrackItem 的映射表，管理各类 Service（Snap/RectSelect/Thumbnail），实现 Observer 接口监听 VBL 数据变更并同步创建/销毁/更新图元；负责场景级的事件分发（鼠标事件路由到各 Service 或图元）
- **FTimelinePresenter**（Presenter 层）：持有业务逻辑，响应 Scene 上报的 `FFMessage` 消息，调用 `IFFTimeline` 接口执行真实数据修改（add/move/trim/split），管理子 Presenter 生命周期（关键帧/多机位/转场），协调授权检查和媒体下载

**分工原则**：Scene 只处理"如何显示"和"用户做了什么手势"，不知道业务含义；Presenter 只处理"这个手势应该做什么操作"，不直接操作图元。

---

### Q3：IFFTimeline 接口与 UI 层如何保持数据同步？

**回答：**

采用**观察者模式**，`FGraphicsTimelineScene` 实现 `IFFTimelineEventObserver` 接口并注册到 `IFFTimeline`：

```cpp
timeline->attach(scene->eventWatcher()); // 注册观察者
```

VBL 层每次数据变更（clip 添加/删除/移动/属性变化）后回调对应 Observer 方法，Scene 在回调中：
- `onClipAdded`：创建对应的 `FGraphicsClipItem` 并插入 TrackItem
- `onClipRemoved`：销毁对应图元
- `onClipMoved`：更新图元的 `setPos()`
- `onKeyFrameChanged`：通知关键帧面板重绘

这样 UI 始终是数据的"被动反映"，不存在 UI 与数据不一致的问题。

---

## 二、Substitute 替身机制类

### Q4：Substitute 替身机制是什么？为什么需要它？

**回答：**

Substitute（替身）是拖拽/裁剪交互时引入的"影子图元"机制。

**问题背景**：如果拖拽时直接修改 `IFFClip` 数据，每次鼠标移动（约 16ms）都触发 `IFFTimeline::moveClips()`，VBL 层会回调 `onClipMoved`，Scene 重新计算所有相关图元位置，造成：
1. 帧率下降（VBL 层每次操作有加锁/持久化开销）
2. undo 栈每帧都写入一条记录（无法撤销到拖拽前状态）
3. 无法实现 ESC 取消（数据已被修改）

**Substitute 方案**：
- 鼠标按下时创建轻量替身图元（只有位置/宽度，无业务数据），原 ClipItem 半透明
- 拖拽时只移动替身（纯 UI 操作，不触碰 VBL），60fps 流畅
- 鼠标释放时一次性 commit → `IFFTimeline::moveClips()`（只写入一条 undo）
- ESC 时销毁替身，数据层零修改

---

### Q5：Substitute commit 时如何保证数据一致性？多个 Clip 同时拖拽怎么处理？

**回答：**

commit 时使用 `FF_DECLARE_PAUSE_EVENT_COMMANDER` 暂停 VBL 事件广播，在暂停期间将所有 Clip 的目标位置批量提交到 `IFFTimeline::moveClips()`（接口支持传入 Clip 列表），VBL 层对整个列表原子性处理；`FF_DECLARE_UNDO_COMMANDER` 则将这次批量移动包装成一条 undo 记录。

```
beginPauseEvent        ← 暂停 VBL 广播
beginMacro             ← 开始合并 undo
for each substitute:
    IFFTimeline::moveClips(...)
endMacro               ← 合并为一条 undo
endPauseEvent          ← 恢复广播，统一触发一次 UI 更新
```

结果：无论拖拽了多少个 Clip，commit 后 UI 只刷新一次，undo 栈只增加一条记录。

---

## 三、Qt 技术类

### Q6：时间线的帧坐标系与像素坐标系是如何设计的？

**回答：**

时间线内部时间表示统一使用**帧（qlonglong frame）**，所有数据存储和逻辑计算（吸附、trim 边界、undo 记录）均在帧坐标系内完成。

像素坐标系只用于 UI 渲染，通过 `pixelsPerFrame`（当前缩放级别，单位 px/frame）双向转换：

```cpp
qreal frameToSceneX(qlonglong frame) {
    return frame * m_pixelsPerFrame;
}

qlonglong scenePosToFrame(qreal x) {
    return qRound(x / m_pixelsPerFrame); // 取整到最近帧
}
```

**为什么不用时间（秒）？**  
帧是编辑精度的最小单位。使用秒/毫秒在不同帧率（24fps / 30fps / 60fps）素材混编时会产生浮点误差，而帧是整数，精确对齐且无精度损失。

---

### Q7：音频波形 QImage 的生成是在哪个线程完成的？如何安全传递回主线程？

**回答：**

波形生成在独立 `QThread` 中完成：

```cpp
// FAudioThumbnailService 构造时：
m_pWorker->moveToThread(&m_oThread);
m_oThread.start();

// 触发生成（主线程调用，跨线程投递）：
QMetaObject::invokeMethod(m_pWorker, "generateThumbnail",
                          Qt::QueuedConnection,
                          Q_ARG(ThumbnailParam, param));

// Worker 线程完成后：
emit sigFinished(param, image); // signal 跨线程

// 主线程槽函数（自动在主线程执行）：
void slotGenerateThumbnailFinished(ThumbnailParam param, QImage image) {
    m_cache.insert(param.key(), image);
    notifyClipItemUpdate(param.clipId()); // 局部重绘
}
```

**线程安全保证**：
- `QMetaObject::invokeMethod(..., Qt::QueuedConnection)` 保证调用在目标线程队列中执行
- Signal/Slot 跨线程连接自动使用 `Qt::QueuedConnection`，QImage 以值拷贝传递，无共享内存

---

### Q8：大量 Clip 场景下（如 1000+ 个片段）如何保证时间线的渲染性能？

**回答：**

三层优化：

1. **可视区域裁剪**：`FGraphicsTimelineScene` 通过 `QGraphicsView::visibleRegion()` 和场景坐标映射，只对当前视口内的 ClipItem 调用 `scheduleRepaint()`，超出视口的 Item 不参与绘制计算。Graphics View 框架本身也有 BSP 树加速的 `itemsInRect()` 查询。

2. **冻结绘制模式**：批量操作（如粘贴 100 个 Clip、批量撤销）期间调用 `openFreezeDrawMode()`，暂停 Scene 的所有刷新请求；所有操作完成后调用 `closeFreezeDrawMode()` 触发一次全量重绘。

3. **ClipItem 绘制缓存**：ClipItem 缓存已生成的缩略图/波形 QPixmap，`paint()` 直接绘制缓存图像，只有数据变更时（trim 边界改变/缩放级别改变）才重新触发异步生成。

---

## 四、业务与产品类

### Q9：用户快速连续撤销时，时间线如何保证性能不下降？

**回答：**

两个关键设计：

1. **操作合并**：移动/裁剪等操作已通过 `FF_DECLARE_UNDO_COMMANDER` 合并为单条记录，用户感知的"一次操作"确实只对应 undo 栈的一条，不会产生大量细碎记录

2. **暂停事件广播**：`undo()` 内部 VBL 层回滚数据时，可使用 pause event 机制将回滚产生的多个中间 `onClipMoved` 回调合并，只在 undo 完成后统一通知 UI 刷新一次

如果 undo 栈过深（长时间编辑），还需通过 `setMaxUndoCount` 设置上限，防止内存无限增长（已在优化文档中说明）。

---

### Q10：多轨道编辑时，视频轨和音频轨的同步是如何保证的？

**回答：**

同步分两个层面：

**数据层同步**：`IFFTimeline` 的 Track 数据结构中，视频 Clip 和其关联的音频 Clip 作为"绑定组"（linked group），移动/删除视频 Clip 时，VBL 层自动同步移动/删除绑定的音频 Clip，`Presenter::moveClips()` 只需传入视频 Clip，不需要显式处理音频。

**多机位音频同步**：`FTimelineAudioSynService` 在后台分析多路音频波形（FFAsync 异步任务），通过互相关算法计算各路音频的偏移量，然后统一调用 `timeline->moveClips()` 对齐。整个分析过程在后台线程完成，UI 显示进度对话框，完成后一次性提交结果。

---

### Q11：时间线中的关键帧编辑是如何实现的？

**回答：**

关键帧编辑由 `FKeyFramePresenter` 负责，UI 上每个关键帧表现为 ClipItem 下方的菱形图标（`FGraphicsKeyFramePanelItem`）。

**流程**：
- 用户点击关键帧菱形 → `FKeyFramePresenter::addKeyFrame(frame, value)` → `IFFClip::keyFrameService()->addKeyFrame(...)` 写入 VBL
- 插值方式（线性/贝塞尔/阶梯）存储在 VBL 层关键帧对象中，渲染时 `IFFTimelineRender` 按插值方式计算当前帧的属性值
- 贝塞尔控制点的 UI 编辑由 `FKeyFramePresenter` 通过 `IFFKeyFrameHelper` 接口操作，修改后触发预览重渲染

关键帧操作同样走 undo/redo（`IFFUndoableOp`），且拖动关键帧时用 Substitute 类似的"拖动中不提交、释放时 commit"模式，避免每帧更新触发重渲染。

---

## 五、C++ 与工程类

### Q12：`IFFClip*` 裸指针在异步场景（如波形生成后回调）中如何防止悬空？

**回答：**

IFFClip 的生命周期由 VBL 层的 `IFFTimeline` 管理，销毁时（删除或替换操作）VBL 会先触发 `onClipAboutToBeRemoved` Observer 回调。

**FAudioThumbnailService 的防护**：
- 缓存以 `clipId`（字符串 ID）为 Key，而非裸指针
- 异步任务 lambda 只捕获 `clipId` 和参数，不持有 `IFFClip*`
- 任务完成后通过 `clipId` 在缓存中查找对应 ClipItem；若此时 ClipItem 已被销毁（`onClipAboutToBeRemoved` 中从缓存移除），则回调中的更新为空操作

```cpp
// Worker 完成后回调（主线程）
void onThumbnailFinished(QString clipId, QImage image) {
    auto* clipItem = m_scene->findClipItem(clipId);
    if (!clipItem) return; // 已销毁，安全跳过
    clipItem->setWaveformImage(image);
    clipItem->update();
}
```

**关键点**：避免在异步任务中持有 `IFFClip*`，改用 ID + Scene 查找的两段式访问，将生命周期管理与异步逻辑解耦。
