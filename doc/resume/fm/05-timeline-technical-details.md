05-timeline-technical-details.md

# 时间线模块技术实现细节

> 项目：Wondershare Filmora（C++17 + Qt 5.15）
> 模块：时间线（FTimelineView / FFTimeline）

---

## 一、模块概述

时间线（Timeline）是 Filmora 编辑器的核心编辑区域，承载多轨视频、音频、特效、贴纸、字幕等全类型轨道的可视化编排与交互。用户在此完成素材的添加、裁剪、移动、分割、变速、关键帧编辑等所有剪辑操作，并实时预览合成结果。

模块分为三个主要层次：

| 层次 | 路径 | 职责 |
|---|---|---|
| `FTimelineView` | `Src/FTimelineView` | UI 展示层（GraphicsScene/Item、Ruler、Playhead、波形、缩略图） |
| `FTimelinePresenter` | `Include/FTimelineView` + `Src/FTimelineView` | 业务逻辑层（操作协调、撤销、播放控制、子 Presenter 管理） |
| `FFVBLModel`（子模块） | `3rdparty/FilmoraFrameworkPlatform/Src/FFVBLModel` | 底层数据与渲染层（`IFFTimeline/IFFTrack/IFFClip`、渲染引擎） |

---

## 二、整体架构

时间线采用 **MVP + GraphicsScene 双层 UI** 架构：

```
┌─────────────────────────────────────────────────────────────┐
│                       View 层（UI）                           │
│  FTimelineWidget（顶层容器）                                   │
│  FGraphicsTimelineScene（QGraphicsScene，承载全部可视元素）      │
│  FGraphicsTrackItem / FGraphicsClipItem（轨道/片段图元）         │
│  FTimelineRuler / FTimelinePlayheadWidget（标尺、播放头）         │
│  各类 Service：SnapService、RectSelectService、ThumbnailService  │
└──────────────────────────┬──────────────────────────────────┘
                           │ FFMessage 消息派发
┌──────────────────────────▼──────────────────────────────────┐
│                    Presenter 层（业务协调）                     │
│  FTimelinePresenter（顶层控制器）                               │
│  FKeyFramePresenter / FTransitionPresenter                    │
│  FMultiCameraClipPresenter / FTimelineSharedClipPresenter     │
└──────────────────────────┬──────────────────────────────────┘
                           │ IFFTimeline 接口调用
┌──────────────────────────▼──────────────────────────────────┐
│                   VBL 底层数据与渲染层                          │
│  IFFTimeline / IFFTrack / IFFClip（数据模型接口）               │
│  IFFTimelineRender / FFTimelineRender（多轨合成渲染引擎）        │
│  IFFUndoRedoService（撤销/重做服务）                            │
└─────────────────────────────────────────────────────────────┘
```

UI 层基于 **Qt Graphics View Framework**，而非标准 Model/View；`FGraphicsTimelineScene` 维护轨道与片段的图元树，通过 Observer 接口订阅 VBL 数据变更并同步更新图元。

---

## 三、主业务流程

### 3.1 媒体添加流程

```
资源面板拖入 / 双击
       │
FTimelinePresenter::addMedias(FFTimelineAddMediaParam)
       │  prepareAddMedias()：检查授权、按需触发下载、分辨率选择对话框
       │
IFFTimeline::addMediaClips()         ← VBL 层执行，创建 IFFClip 对象
       │  触发 IFFTimelineEventObserver 回调
       │
FTimelineSceneEventWatcher（Scene 注册的观察者）
       │  构造 FGraphicsClipItem，插入 Scene 对应 FGraphicsTrackItem
       │
Presenter 发出 sigAddedClips 信号
       └──→ 属性面板、播放器等订阅模块响应
```

### 3.2 Substitute（替身）拖拽移动流程

这是时间线模块最核心的性能设计，解决了拖拽时"频繁修改数据层导致 UI 抖动"的问题：

```
鼠标按下 FGraphicsClipItem
       │
FGraphicsItemSubstituteManager::startClipAction()
       │  创建"替身" FGraphicsClipItemSubstitute（轻量 QGraphicsItem）
       │  原 ClipItem 设为半透明（标记为"被替代"状态）
       │
鼠标拖拽（持续）
       │  scenePosToFrame() 计算目标帧位置
       │  SnapService::calcSnapOffset() 计算最近吸附点偏移
       │  仅更新替身图元位置（不触碰 IFFClip 数据）
       │
鼠标释放 → Handler::commit()
       │  读取替身的目标位置/轨道
       │  调用 IFFTimeline::moveClips(...)（一次性数据提交）
       │  IFFUndoRedoService 记录单条可撤销操作
       │  销毁替身，ClipItem 恢复显示
       │
ESC 取消 → Handler::cancel()
       └──  销毁替身，数据层零修改
```

**核心价值**：整个拖拽过程中数据层只被修改一次（commit 时），避免逐帧触发 VBL 回调与 UI 重建，是时间线流畅交互的基础。

### 3.3 裁剪（Trim）与分割（Split）

```
拖拽 Clip 的 Trim Handle（入点/出点）
       │
替身 TrimAction 启动（同 Move，只移动替身的裁剪边界）
       │
commit → IFFTimeline::trimClips() / trimStartToFrame / trimEndToFrame
       │
一条 undo 记录入栈
```

```
Ctrl+B / 菜单"分割"
       │
FTimelinePresenter::splitClips(qlonglong frame)
       │
IFFTimeline::splitClips(...)   ← 在指定帧位置将 Clip 一分为二
       │
Scene 收到 clipAdded/clipModified 回调，更新图元
```

### 3.4 撤销 / 重做

```
用户操作（move/trim/split/add/delete...）
       │
每个操作包装为 IFFUndoableOp，push 到 IFFUndoRedoService
       │
FF_DECLARE_UNDO_COMMANDER / beginMacro + endMacro
→ 将多个子操作合并为一条 undo entry（如"粘贴+移动"算一步）
       │
Ctrl+Z → IFFUndoRedoService::undo()
       │  VBL 层回滚数据 → 触发 Observer 回调 → Scene 同步 UI
       │
Ctrl+Y → redo()，同理
```

FF_DECLARE_PAUSE_EVENT_COMMANDER 在合并操作期间暂停 VBL 事件广播，避免 UI 在中间状态触发多次无效刷新。

### 3.5 播放与播放头

```
用户点击播放 / 键盘空格
       │
FTimelinePresenter → timeline->playhead()->play()
       │
IFFTimelineRender::requestFrame(time)
       │  遍历当前时刻所有 active IFFClip
       │  解码各轨帧数据（视频解码 / 音频 PCM）
       │  应用特效、关键帧插值（贝塞尔/线性/阶梯）
       │  多轨合成输出一帧
       │
推送到 FMediaPlayerView 显示
       │
播放头位置更新 → FTimelinePlayheadWidget 重绘
       └──→ Scene scroll 保证播放头可见
```

### 3.6 音频波形异步生成

```
ClipItem 创建后触发 FAudioThumbnailService::createThumbnail(param)
       │
QMetaObject::invokeMethod(worker, "generateThumbnail")
       │  Worker 在独立 QThread 中运行
       │  解码音频 PCM → 计算 RMS 包络 → 绘制为 QImage
       │  支持 Audio Ducking：calcDuckingFactor 在多轨时调整波形显示增益
       │
Worker 完成：emit sigFinished(param, image)
       │
主线程 slotGenerateThumbnailFinished()
       │  插入缓存、通知 ClipItem 局部重绘（update()）
```

---

## 四、核心技术点

### 4.1 Qt Graphics View Framework 的深度应用

时间线 UI 全部基于 `QGraphicsScene / QGraphicsView / QGraphicsItem` 实现：

- `FGraphicsTimelineScene` 继承 `QGraphicsScene`，维护 `m_oTimelineTrackItemMap`（轨道 ID → TrackItem 的映射）
- 每个轨道是一个 `FGraphicsTrackItem`（包含 TrackHead 控件区域），每个片段是一个 `FGraphicsClipItem`（子 Item）
- 坐标系：场景 X 轴为时间轴（帧位置），Y 轴为轨道纵坐标；`frameToScenePos / scenePosToFrame / durationToPixel` 负责双向转换
- 时间轴缩放通过调整 `pixelsPerFrame`（范围约 0.5px/帧 ～ 80px/帧）动态改变坐标映射，所有 ClipItem 宽度随之重算

### 4.2 帧级精度坐标系

所有时间相关计算均以**帧**为最小单位（`qlonglong frame`），而非时间（秒/毫秒），保证在任意帧率下的精确对齐：

- 坐标转换函数内置帧误差修正（处理浮点像素 → 帧整数的取整精度问题）
- Snap 计算与 Trim 的边界调整均在帧坐标系内完成，再映射回像素坐标绘制

### 4.3 吸附对齐系统（Snap Service）

`FGraphicsTimelineSceneSnapService` 在拖拽/裁剪时实时计算吸附：

- 可吸附点集合：所有 Clip 的入点/出点、Playhead 当前位置、Marker 标记点
- 拖拽时扫描候选吸附点，取距离最近且小于阈值（像素距离）的点计算偏移量
- 主轨（Main Track）具有**磁性排布**行为：`rearrangeMainTrack()` 在向主轨插入时自动让相邻 Clip 位移，通过 `MTrackSnapHandler` 实现纵向轨道联动

### 4.4 视频缩略图 + 音频波形异步缓存

| 类型 | 实现 | 线程模型 | 缓存策略 |
|---|---|---|---|
| 视频缩略图 | `FFVideoThumbnailService`（VBL 层） | 线程池异步解码 | 参数化 Key（clipId + 采样区间 + 可视宽度），状态机（tsBuilt/tsBuilding/tsReBuild） |
| 音频波形 | `FAudioThumbnailService` + `FAudioThumbnailWorker` | `moveToThread` 独立 QThread | 同参数判等，参数不匹配则重建 |

缓存重建条件：clip 入/出点变化、可视宽度变化（缩放级别改变）、素材变更；参数不变时直接复用缓存图像，无需重新解码。

### 4.5 虚拟化渲染与绘制优化

- **可视区域裁剪**：Scene 只调度（`scheduleRepaint`）当前视口内可见的 ClipItem 重绘，超出视口的 Item 不参与绘制计算
- **冻结绘制模式**（`openFreezeDrawMode / closeFreezeDrawMode`）：在批量后台更新或动画过渡期间暂停 Scene 刷新，批量完成后统一刷新一次
- **ClipItem 绘制缓存**：ClipItem 缓存已绘制的缩略图/波形图像，避免每帧重解码；只在数据变更时失效并重建
- **FPerformancePileInsetor / FGraphicsItemBoundingRectCache**：加速大量 Item 的包围盒计算与碰撞检测

### 4.6 撤销/重做的 Command 封装

项目使用宏辅助的 Command 封装：

```cpp
// 合并多个子操作为一条 undo entry
FF_DECLARE_UNDO_COMMANDER(timeline->undoRedoSrv());
// ... 执行多个 IFFTimeline 操作 ...
// commander 析构时自动 endMacro，整体作为一条 undo

// 在合并期间暂停 VBL 事件广播，避免 UI 中间状态刷新
FF_DECLARE_PAUSE_EVENT_COMMANDER(timeline);
```

支持 `beginMacro / endMacro` 实现任意粒度的操作合并，保证"拖拽移动"、"粘贴+对齐"等复合操作在 undo 栈中仅占一条记录。

---

## 五、设计模式应用

### 5.1 MVP（Model-View-Presenter）

- **View 层**：`FGraphicsTimelineScene`、`FGraphicsClipItem` 等，负责 UI 绘制与交互事件捕获，通过 `FFMessage` 消息将事件上报给 Presenter
- **Presenter 层**：`FTimelinePresenter` 持有业务决策逻辑，调用 `IFFTimeline` 接口修改数据
- **Model 层**：`IFFTimeline / IFFTrack / IFFClip` 纯接口，VBL 底层实现，不感知 UI

### 5.2 命令模式（Command Pattern）

所有可撤销操作（add/move/trim/split/delete）均封装为 `IFFUndoableOp` 压入 `IFFUndoRedoService`，支持 `undo() / redo()`，且支持 `beginMacro / endMacro` 合并多个操作为一条历史记录。

### 5.3 观察者模式（Observer）

- `IFFTimelineEventObserver`：监听 clip 增删改、轨道变化
- `IFFVideoThumbnailEventObserver`：监听视频缩略图生成完成
- `IFAudioThumbnailEventObserver`：监听波形生成完成
- Scene 统一实现上述 Observer 接口，作为数据层到 UI 层的"消息汇聚点"

### 5.4 Builder + Factory（替身构造）

- `FGraphicsItemSubstituteBuilder`：构建拖拽/裁剪/分割等操作对应的替身对象
- `FGraphicsSubstituteActionFactory`：根据操作类型（Move/Trim/Split）创建对应的 Action Handler
- 扩展新的交互行为只需增加新的 Builder/Action 实现，不影响核心 Substitute 管理器

### 5.5 策略模式 / Service 插件化

Scene 持有多个可插拔 Service，各自独立实现不同能力：

| Service | 职责 |
|---|---|
| `FGraphicsTimelineSceneSnapService` | 吸附对齐计算 |
| `FGraphicsTimelineSceneRectSelectService` | 框选多个 Clip |
| `FAudioThumbnailService` | 音频波形生成 |
| `FFClipThumbnailServiceBuilder` | 视频缩略图管理 |
| `FGraphicsTimelineTrackFillerService` | 空轨道填充渲染 |

---

## 六、模块间通信机制

| 通信方式 | 使用场景 |
|---|---|
| `FFMessage` 消息派发（`sendMessage / dispatchMessage`） | UI 层 → Presenter 层的操作上报 |
| Qt 信号/槽 | Presenter → 属性面板、播放器、资源面板等外部模块 |
| `IFFTimelineEventObserver` | VBL 数据层 → Scene/Presenter 的数据变更通知 |
| `IFFTimeline` 接口调用（同步） | Presenter → VBL 层的业务操作 |
| 全局事件总线（`IFFEventBus` / `FBroadcastMessageType`） | 跨模块广播（如工程关闭、导出启动） |
| `QMetaObject::invokeMethod`（Qt::QueuedConnection） | Worker 线程结果安全回传主线程 |

---

## 七、关键类职责速查

| 类名 | 层次 | 核心职责 |
|---|---|---|
| `FTimelinePresenter` | Presenter | 顶层控制，addMedias/moveClips/splitClips/undo/redo/seek |
| `FGraphicsTimelineScene` | View | QGraphicsScene 管理，轨道-图元映射，Service 管理，事件观察 |
| `FGraphicsClipItem` | View | 片段图元，绘制缩略图/波形/关键帧面板，Trim Handle 交互 |
| `FGraphicsItemSubstitute` | View | 替身对象，拖拽中代替真实 ClipItem，commit/cancel 语义 |
| `FGraphicsTimelineSceneSnapService` | Service | 吸附点计算，磁性对齐 |
| `FAudioThumbnailService` | Service | 波形异步生成，Worker 线程管理，参数化缓存 |
| `FKeyFramePresenter` | Presenter | 关键帧编辑（添加/删除/曲线调整） |
| `IFFTimeline` | VBL | 时间线数据接口，track/clip CRUD，playhead，undoRedo |
| `IFFTimelineRender` | VBL | 多轨合成渲染引擎，requestFrame 接口 |
| `IFFUndoRedoService` | VBL | 撤销/重做栈，beginMacro/endMacro，undo/redo |
