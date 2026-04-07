# TimelineUX / TimelineEditor 模块详细设计文档

> 所属层：BusinessLayer
> 接口头（TimelineUX）：`Interface/BusinessLayer/TimelineUX/IVbTimelineUX.h`
> 接口头（TimelineEditor）：`Interface/BusinessLayer/TimelineEditor/IVbTimelineEditor.h`
> 工厂函数：通过 `ProjectEditor::getTimelineUX()` 获取
> 命名空间：`VBL::`

---

## 1. 模块职责

### TimelineUX
TimelineUX 是面向产品 UI 的**时间线交互总入口**。它在 TimelineEditor 之上封装了一套针对 Filmora 产品系列的业务逻辑，包括：
- 高级编辑操作（变速、冻帧、J/L Cut、磁吸模式）
- 粘贴板管理（复制/剪切/粘贴）
- 模板应用
- 复合片段（嵌套时间线）、多机位剪辑
- AI 视频风格化、AI 模板生成

### TimelineEditor
TimelineEditor 是**时间线数据层的直接操作者**，负责：
- 轨道的增删改
- Clip 的增删改移位
- 转场的添加
- 所有操作都配套 Undo/Redo 入栈

两者关系：`TimelineUX` 组合使用 `TimelineEditor`（TimelineUX 对外暴露，TimelineEditor 可直接使用）。

---

## 2. 核心接口说明

### 2.1 TimelineUX（IVbTimelineUX）

```cpp
class IVbTimelineUX : virtual public IVbBaseUX {
    // 编辑事务
    virtual Result beginEditTimeline() = 0;
    virtual Result endEditTimeline() = 0;

    // 时间线配置
    virtual Result changeTimelineFrameRate(Rational frameRate) = 0;
    virtual Result changeTimelineConfig(const TimelineConfig& config) = 0;

    // 轨道操作
    virtual Result addTracks(TrackType type, VBLInt start_idx, VBLInt count) = 0;
    virtual Result moveTracks(IDmTrackList* pTracks, VBLInt to_idx) = 0;
    virtual Result removeTracks(VBLIntList* idxs) = 0;

    // Clip 基础操作
    virtual Result addClips(VBLInt track_idx, VBLLonglong pos, IDmClipList*, TimelineOperationType) = 0;
    virtual Result deleteClips(IDmClipList*) = 0;
    virtual Result moveClipsOffset(IDmClipList*, VBLIntList* offsetTracks, VBLLonglong offsetTime, ...) = 0;
    virtual Result splitClips(IDmClipList*, VBLLonglong splitPos, IDmClipList* newClips) = 0;

    // Trim 操作
    virtual Result leftTrimClips(IDmClipList*, VBLLonglong offsetTime) = 0;
    virtual Result rightTrimClips(IDmClipList*, VBLLonglong offsetTime) = 0;
    virtual Result jCutClip(IDmClip*, VBLLonglong offsetTime, VBLBool isVideo) = 0;
    virtual Result lCutClip(IDmClip*, VBLLonglong offsetTime, VBLBool isVideo) = 0;

    // 变速
    virtual Result changeSpeedRatio(IDmClip*, VBLInt segmentIdx, VBLREAL speed) = 0;
    virtual Result addFreezeFrame(IDmClip*, VBLLonglong posTime, VBLLonglong duration) = 0;
    virtual Result removeFreezeFrame(IDmClip*, VBLInt segmentIndex) = 0;
    virtual Result addPointForCurvedSpeed(IDmClip*, VBLLonglong posTime, VBLREAL speed) = 0;

    // 转场
    virtual Result addTransitionToClip(IDmTransition*, IDmClip*, TransitionType) = 0;
    virtual Result applyDurationToTransitions(IDmTransitionList*, VBLLonglong duration) = 0;

    // 粘贴板
    virtual Result copyOrCutClips(IDmClipList*, IDmTransition*, VBLBool isCut) = 0;
    virtual Result pasteClips(IDmClipList* trans, VBLLonglong pos, VBLInt trackIdx, ...) = 0;
    virtual Result canPasteClips(CanPasteType&, VBLIntList* errorCode) = 0;

    // 模板与复合操作
    virtual Result addTemplate(IDmTimeline*, IVblResSerializeInfo*, ...) = 0;
    virtual Result mergeClipsToNewClip(IDmClipList*, VBLInt trackIdx, ...) = 0;
    virtual Result expandTimelineClip(IDmClip*, VBLInt vTrackIdx, VBLInt aTrackIdx, ...) = 0;

    // AI 功能
    virtual Result setAiVideoStylizerEnable(IDmClip*, VBLBool enable) = 0;
    virtual Result applyAiVideoStylizerResult(IDmClip*, IDmDeepClonableObj* data) = 0;
    virtual Result setAiTemplateGenerateEnable(IDmClip*, VBLBool enable, ...) = 0;

    // 磁吸模式
    virtual Result switchMainTrackMagneticMode(VBLBool bOpen, VBLBool bDependMainTrack) = 0;

    // 字幕转换
    virtual Result subtitleClip2TitleClip(IDmClipList*, IDmClipList* titleClips, ...) = 0;
};
```

### 2.2 TimelineEditor（IVbTimelineEditor）

```cpp
class IVbTimelineEditor : virtual public IVbBaseTimelineEditor {
    virtual Result changeTimelineFrameRate(Rational frameRate) = 0;
    virtual Result changeTimelineConfig(const TimelineConfig& config) = 0;
    virtual Result addTracks(TrackType type, VBLInt start_idx, VBLInt count) = 0;
    virtual Result moveTracks(IDmTrackList*, VBLInt to_idx) = 0;
    virtual Result removeTrack(VBLInt idx) = 0;
    virtual Result addClips(VBLInt track_idx, IDmClipList*, TrackType) = 0;
    virtual Result deleteClips(IDmClipList*) = 0;
    virtual Result deleteClipsOfOneTrack(VBLInt trackIdx) = 0;
    virtual Result moveClipsOffsetTime(IDmClipList*, VBLLonglong offsetTime) = 0;
    virtual Result moveClipsOffsetTrack(IDmClipList*, VBLInt offsetTrack) = 0;
    virtual Result relinkMedia(VBLConstPChar oldMediaId, VBLConstPChar newMediaId, ...) = 0;
    virtual Result splitClips(IDmClipList*, VBLLonglong splitPos, IDmClipList* newClips) = 0;
    virtual Result leftTrimClips(IDmClipList*, VBLLonglong offsetTime) = 0;
    virtual Result rightTrimClips(IDmClipList*, VBLLonglong offsetTime) = 0;
    virtual Result jCutClip(IDmClip*, VBLLonglong offset, VBLBool isVideo) = 0;
    virtual Result lCutClip(IDmClip*, VBLLonglong offset, VBLBool isVideo) = 0;
    virtual Result setActive(VBLBool active) = 0;
    virtual Result changeTimelineProperty(VBLConstPChar key, const Property& value) = 0;
    virtual Result clearRemix(IDmClip*) = 0;
    virtual Result setBrandClip(IDmClip*) = 0;
};
```

---

## 3. 各子 ClipEditor 接口列表

| 接口 | 职责 |
|---|---|
| `IVbClipEditor` | Clip 通用属性（enable/locked/位置/透明度/混合模式） |
| `IVbVideoClipEditor` | 视频 Clip（缩放/旋转/裁剪/翻转/降噪/HDR） |
| `IVbAudioClipEditor` | 音频 Clip（音量/淡入淡出/降噪/变声） |
| `IVbTextClipEditor` | 文字 Clip（字体/颜色/动画/气泡） |
| `IVbEffectEditor` | 滤镜/特效（参数调整/添加/删除） |
| `IVbTransitionEditor` | 转场（时长/参数） |
| `IVbVariableSpeedEditor` | 变速（速率/曲线编辑） |
| `IVbLayerClipEditor` | 特效 Clip（画中画/贴纸） |
| `IVbAnimationEditor` | 关键帧动画 |
| `IVbMarkerEditor` | 时间线标记点编辑 |
| `IVbSubtitleClipEditor` | 字幕 Clip |
| `IVbTimelineClipEditor` | 嵌套时间线 Clip |
| `IVbMultiCameraClipEditor` | 多机位剪辑 Clip |
| `IVbCustomMaskEditor` | 自定义遮罩 |
| `IVbSmartShortInfoEditor` | AI 智能短片信息 |
| `IVbMotionTrackingFollowerEditor` | 运动追踪跟随 |
| `IVbObjectRemoveEditor` | AI 对象消除编辑 |

---

## 4. 依赖关系

```
TimelineUX
  ├── 组合 → TimelineEditor        （直接操作 DataModel 数据）
  ├── 组合 → 各 ClipEditor         （子属性编辑器群）
  ├── 持有 → IBsUndoTemplateStack  （每个可逆操作推栈）
  ├── 读写 → IDmTimeline           （时间线数据根对象）
  └── 发布 → IMsEventBus           （timeline.* 事件）
```

---

## 5. 时序图

### 5.1 添加 Clip 到时间线

```
UI 层       TimelineUX      TimelineEditor    DataModel    UndoStack    EventBus
  │              │                 │               │             │           │
  │ addClips(track, pos, clips)    │               │             │           │
  ├─────────────►│                 │               │             │           │
  │              │ 校验轨道类型/边界条件             │             │           │
  │              │ addClips(track, clips)           │             │           │
  │              ├────────────────►│               │             │           │
  │              │                 │ beginEdit()   │             │           │
  │              │                 ├──────────────►│             │           │
  │              │                 │ timeline->addClipAt(track, pos, clip)   │
  │              │                 ├──────────────►│             │           │
  │              │                 │               │ 更新轨道数据 │           │
  │              │                 │ endEdit()     │             │           │
  │              │                 ├──────────────►│             │           │
  │              │ push(AddClipCmd)│               │             │           │
  │              ├─────────────────────────────────────────────── ►│         │
  │              │ postEvent("timeline.clip.added")│             │           │
  │              ├──────────────────────────────────────────────────────────►│
  │ onEvent() 刷新 UI              │               │             │           │ → UI
  │◄──────────────────────────────────────────────────────────────────────────────┤
```

### 5.2 分割 Clip

```
UI 层       TimelineUX      TimelineEditor    DataModel    UndoStack    EventBus
  │              │                 │               │             │           │
  │ splitClips(clips, position)    │               │             │           │
  ├─────────────►│                 │               │             │           │
  │              │ splitClips(clips, pos, newClips)│             │           │
  │              ├────────────────►│               │             │           │
  │              │                 │ clip->split(pos) → newClip  │           │
  │              │                 ├──────────────►│             │           │
  │              │                 │               │ 原 clip 缩短  │          │
  │              │                 │               │ 新 clip 插入  │          │
  │              │ push(SplitCmd)  │               │             │           │
  │              ├────────────────────────────────────────────── ►│          │
  │              │ postEvent("timeline.clip.split")│             │           │
  │              ├──────────────────────────────────────────────────────────►│
  │ onEvent() 更新两段 UI          │               │             │           │ → UI
  │◄──────────────────────────────────────────────────────────────────────────────┤
```

### 5.3 添加转场

```
UI 层       TimelineUX      TimelineEditor    DataModel    UndoStack    EventBus
  │              │                 │               │             │           │
  │ addTransitionToClip(trans, clip, type)         │             │           │
  ├─────────────►│                 │               │             │           │
  │              │ 检查 clip 与相邻 clip 是否满足转场条件          │           │
  │              │ 若满足：addTransitionToClip()   │             │           │
  │              ├────────────────►│               │             │           │
  │              │                 │ clip->setTransition(trans)  │           │
  │              │                 ├──────────────►│             │           │
  │              │                 │               │ 调整 clip 长度适应转场   │
  │              │ push(AddTransCmd)               │             │           │
  │              ├────────────────────────────────────────────── ►│          │
  │              │ postEvent("timeline.transition.added")        │           │
  │              ├──────────────────────────────────────────────────────────►│
  │ onEvent() 渲染转场区            │              │             │           │ → UI
  │◄──────────────────────────────────────────────────────────────────────────────┤
```

### 5.4 变速（曲线速度）

```
UI 层       TimelineUX     DataModel    UndoStack   EventBus
  │              │               │            │          │
  │ addPointForCurvedSpeed(clip, pos, speed)  │          │
  ├─────────────►│               │            │          │
  │              │ clip->variableSpeed()->addCurvePoint(pos, speed)
  │              ├──────────────►│            │          │
  │              │ push(CurveSpeedCmd)        │          │
  │              ├─────────────────────────── ►│         │
  │              │ postEvent("timeline.clip.speed.changed")     │
  │              ├──────────────────────────────────────────────►│
  │ onEvent() 刷新变速面板 UI     │            │          │ → UI  │
  │◄─────────────────────────────────────────────────────────────────┤
```

### 5.5 J/L Cut

```
UI 层       TimelineUX      TimelineEditor    DataModel    UndoStack
  │              │                 │               │             │
  │ jCutClip(clip, offsetTime, isVideo=true)       │             │
  ├─────────────►│                 │               │             │
  │              │ jCutClip(clip, offset, true)    │             │
  │              ├────────────────►│               │             │
  │              │                 │ videoClip->rightTrim(offset)│
  │              │                 ├──────────────►│             │
  │              │                 │ audioClip->rightTrim(offset + jcut)
  │              │                 ├──────────────►│             │
  │              │                 │               │ 视频先结束   │
  │              │                 │               │ 音频延迟结束  │
  │              │ push(JCutCmd)   │               │             │
  │              ├────────────────────────────────────────────── ►│
  │ onEvent() 刷新时间线           │               │             │
```

---

## 6. 设计要点

| 要点 | 说明 |
|---|---|
| 事务模型 | `beginEditTimeline/endEditTimeline` 包裹批量操作，避免中间态广播 |
| register/apply 模式 | `registerXxx(isPost=false)` 仅操作数据不广播；`isPost=true` 操作+广播，支持预览拖拽 |
| TimelineOperationType | 区分操作来源（用户拖放/键盘/粘贴/模板/AI），影响 Undo 描述和事件数据 |
| 多轨添加 | `addClipsToMultTracks` 支持音视频同步多轨添加，保证原子性 |
| 磁吸模式 | 主轨磁吸时，副轨 clip 跟随主轨 clip 移动 |
