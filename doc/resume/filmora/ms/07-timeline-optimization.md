07-timeline-optimization.md

# 时间线模块技术优化方案

---

## 一、Substitute 替身机制的潜在问题与优化

### 问题描述

1. **commit 时数据提交无法原子化**：当 commit 涉及多个 Clip 同时移动（框选拖拽）时，`IFFTimeline::moveClips()` 内部若对每条 Clip 分别执行移动，中间状态可能触发 VBL 回调，导致 UI 在批量 commit 期间出现短暂错位闪烁。

2. **替身生命周期管理不稳定**：快速拖拽后立即 ESC 或窗口失焦时，如果 commit/cancel 事件时序混乱（如鼠标事件丢失），替身可能无法正常销毁，导致"幽灵图元"残留在 Scene 中。

3. **缺少并发替身互斥**：当多点触控或特殊输入设备（触控板手势）触发多路交互时，可能同时创建多个替身，彼此干扰 commit 逻辑。

### 优化方案

**方案 A：批量 moveClips 的事件暂停**

在 commit 阶段主动使用 `FF_DECLARE_PAUSE_EVENT_COMMANDER` 包裹批量提交，确保所有 Clip 移动完成后再统一触发 UI 更新：

```cpp
void FGraphicsItemSubstituteManager::commit() {
    FF_DECLARE_PAUSE_EVENT_COMMANDER(m_timeline); // 暂停 VBL 事件广播
    for (auto& sub : m_substitutes) {
        m_timeline->moveClips({sub->clip()}, sub->targetTrackIdx(),
                              sub->targetFrame());
    }
    // commander 析构时恢复广播，统一触发一次 UI 更新
}
```

**方案 B：替身保护性销毁**

在 `FGraphicsItemSubstituteManager` 析构函数和窗口失焦（`QEvent::WindowDeactivate`）处增加兜底清理，强制 cancel 所有未完成的替身：

```cpp
FGraphicsItemSubstituteManager::~FGraphicsItemSubstituteManager() {
    for (auto& sub : m_substitutes) sub->cancel();
    m_substitutes.clear();
}
```

**方案 C：单例替身互斥锁**

`startClipAction()` 入口处检查是否有活跃替身，有则先强制取消：

```cpp
void startClipAction(FGraphicsClipItem* item, ActionType type) {
    if (hasActiveSubstitute()) cancelAll(); // 互斥保护
    // ... 创建新替身
}
```

---

## 二、音频波形与视频缩略图性能问题与优化

### 问题描述

1. **缩放时批量重建**：时间轴缩放（`pixelsPerFrame` 变化）导致所有可见 ClipItem 的参数 Key 同时失效，触发大量并发的波形/缩略图重建任务，造成线程池瞬时拥塞、UI 卡顿。

2. **Worker 线程无优先级区分**：位于视口中心的 Clip 与边缘 Clip 使用相同优先级，用户正在关注的区域反而可能后于边缘区域完成。

3. **反复缩放时存在大量无效计算**：用户快速连续缩放时，前几次缩放触发的重建任务在还未完成时就已无效（参数再次变化），但旧任务仍在后台消耗 CPU。

### 优化方案

**方案 A：缩放防抖 + 批量取消**

缩放事件触发后延迟 200ms 再提交重建请求（防抖），延迟期间新的缩放事件重置计时器；同时引入任务取消 token，旧任务在执行前检查 token 是否有效：

```cpp
void onScaleChanged() {
    m_rebuildTimer->start(200); // 200ms 防抖
}

void doRebuildThumbnails() {
    m_taskToken++; // 使所有旧任务失效
    for (auto* clip : visibleClips()) {
        auto token = m_taskToken.load();
        FFAsync::postTask([this, clip, token]() {
            if (m_taskToken.load() != token) return; // 任务已过期
            // 生成缩略图...
        });
    }
}
```

**方案 B：按视口距离排优先级**

提交波形/缩略图生成任务时，计算 ClipItem 与视口中心的距离，距离越近优先级越高：

```cpp
int priority = calcDistanceToViewportCenter(clipItem);
FFAsync::postTaskWithPriority(task, priority); // 近处优先
```

**方案 C：LOD（Level of Detail）多级缩略**

在极小缩放级别（`pixelsPerFrame < 2`）时，用纯色块代替实际解码缩略图；只有 `pixelsPerFrame` 超过阈值时才触发真实解码，避免在用户无法分辨细节时浪费解码资源。

---

## 三、撤销栈内存泄漏与栈溢出问题

### 问题描述

`IFFUndoRedoService` 默认无上限地累积 undo 记录。长时间编辑后（尤其是大量关键帧微调操作），undo 栈可能积累数千条记录，每条记录持有 `IFFClip` 的快照引用，累计内存占用可能超过数百 MB。

### 优化方案

**方案 A：undo 栈深度上限**

设置 undo 记录最大条数（如 100 条），超出后自动淘汰最旧记录：

```cpp
undoRedoSrv->setMaxUndoCount(100);
```

**方案 B：关键帧微调操作合并**

关键帧拖拽时每帧都生成一条 undo 记录，可通过 `beginMacro / endMacro` 在鼠标按下到释放的完整拖拽过程内合并为一条：

```cpp
void FKeyFramePresenter::onKeyFrameDragStart() {
    m_undoMacroCommander = new FUndoMacroScope(timeline->undoRedoSrv());
}
void FKeyFramePresenter::onKeyFrameDragEnd() {
    delete m_undoMacroCommander; // 析构触发 endMacro，合并为一条
}
```

**方案 C：快照延迟序列化**

undo 记录写入时不立即深拷贝 clip 数据，而是记录 diff（操作类型 + 参数），undo 时通过逆操作恢复，而非存储完整快照，显著降低每条记录的内存占用。

---

## 四、主轨磁性排布在复杂场景下的鲁棒性问题

### 问题描述

`rearrangeMainTrack()` 在插入片段时对相邻 Clip 执行自动位移。当主轨上片段数量较多（50+），且同时涉及多个 Clip 联动移动时，排布算法的时间复杂度为 O(n)，且每次移动触发一次 VBL 回调，连锁触发大量 UI 更新，在低性能设备上出现明显卡顿。

### 优化方案

**在 `rearrangeMainTrack` 执行前统一暂停 VBL 事件，排布完成后统一触发一次 UI 更新：**

```cpp
void rearrangeMainTrack(IFFTimeline* timeline, ...) {
    FF_DECLARE_PAUSE_EVENT_COMMANDER(timeline); // 暂停广播
    // 遍历计算所有 Clip 的新位置
    for (auto* clip : mainTrackClips()) {
        timeline->moveClip(clip, newPosition(clip)); // 不触发 UI
    }
    // 析构时恢复广播，统一触发一次 UI 刷新
}
```

同时引入**脏标记批量提交**：排布期间收集所有变更的 Clip，排布结束后一次性 `beginInsertRows`/`notifyChanged`，而非每次移动单独通知。

---

## 五、多轨道渲染帧率抖动问题

### 问题描述

播放时 `IFFTimelineRender::requestFrame(time)` 需遍历当前时刻所有 active Clip、各自解码并合成，轨道数量增多（5 路以上）时合成耗时超过帧间隔（约 33ms），导致播放帧率下降甚至卡顿。

### 优化方案

**方案 A：预解码缓冲（Look-ahead Buffer）**

播放时提前解码后续 N 帧（如 3 帧）存入环形缓冲，播放线程只从缓冲中取帧而不等待解码：

```cpp
// 解码线程：提前解码 future frames
void decodeLookahead(qlonglong currentFrame) {
    for (int i = 1; i <= LOOKAHEAD_SIZE; i++) {
        if (!m_frameBuffer.contains(currentFrame + i)) {
            m_frameBuffer.insert(currentFrame + i, decodeFrame(currentFrame + i));
        }
    }
}

// 播放线程：直接取缓冲帧
QImage getFrame(qlonglong frame) {
    return m_frameBuffer.take(frame); // O(1)
}
```

**方案 B：轨道并行解码**

将各轨道的解码任务分发到线程池并行执行，只有合成阶段在主渲染线程顺序进行，将串行 O(n·decode) 优化为并行 O(decode_max)：

```cpp
QList<QFuture<QImage>> futures;
for (auto* track : activeTracks) {
    futures << QtConcurrent::run([track, frame]() {
        return track->decodeFrame(frame); // 并行解码
    });
}
// 等待所有轨道解码完成后合成
QList<QImage> frames = collectFutures(futures);
return composite(frames);
```
