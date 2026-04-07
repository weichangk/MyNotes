# PlayManager / EncodeManager 模块详细设计文档

> 所属层：BaseService
> 接口头（PlayManager）：`Interface/BaseService/PlayManager/IBsPlayManager.h`
> 接口头（EncodeManager）：`Interface/BaseService/EncodeManager/`
> 工厂函数：`createPlayManager()`
> 命名空间：`VBL::`

---

## 1. 模块职责

### PlayManager（播放管理器）
负责 VBL 中的**视频预览播放能力**：
- 时间线预览（绑定 IDmTimeline 播放）
- 单媒体预览（绑定 IDmBaseMedia 播放）
- 帧提取（截图/缩略图）
- 渲染窗口绑定（setParentWindow）
- 播放控制（play/pause/stop/seek/forward/backward）
- 播放参数（分辨率/音量/静音/播放速度/HDR）

### EncodeManager（编码导出管理器）
负责 VBL 中的**视频导出编码能力**：
- 按指定参数将时间线导出为视频文件
- 支持多种格式（MP4/MOV/GIF/MP3 等）
- 导出进度回调
- 支持取消/暂停导出

---

## 2. PlayManager 核心接口

```cpp
class IBsPlayManager : virtual public IDmBaseObj {
    // 窗口与布局
    virtual Result setParentWindow(void* parentWnd, VBLInt width, VBLInt height, VBLInt viewId) = 0;
    virtual Result setBounds(VBLInt x, VBLInt y, VBLInt width, VBLInt height) = 0;
    virtual Result getBounds(VBLInt& x, VBLInt& y, VBLInt& width, VBLInt& height) = 0;

    // 视图变换
    virtual Result setZoomLevel(const ZoomLevelInfo& info, RectF* panRectF = nullptr) = 0;
    virtual Result setSimpleMotion(const RectF& src, const RectF& dst) = 0;

    // 媒体绑定
    virtual Result setMedia(IDmBaseMedia* media) = 0;
    virtual Result setMediaEx(IDmBaseMedia*, IDmTimeline* applyTimeline, VBLLonglong start) = 0;
    virtual IDmBaseMedia* getMedia() = 0;
    virtual Result setDuration(VBLLonglong in_point, VBLLonglong duration) = 0;

    // 媒体信息查询
    virtual VBLLonglong mediaLength() = 0;
    virtual Rational frameRate() = 0;
    virtual SizeN mediaSize() = 0;
    virtual PlayState state() = 0;
    virtual VBLLonglong playTime() = 0;

    // 播放控制
    virtual Result seek(VBLLonglong playTime, bool progress, bool force, bool audio) = 0;
    virtual Result play() = 0;
    virtual Result pause() = 0;
    virtual Result stop() = 0;
    virtual Result forward(bool progress, bool force, bool audio) = 0;
    virtual Result backward(bool progress, bool force, bool audio) = 0;
    virtual Result setPlayerSpeed(VBLREAL speed) = 0;

    // 音频
    virtual Result setVolume(VBLInt volume) = 0;
    virtual Result setMute(VBLBool muted) = 0;

    // 帧提取
    virtual Result getCurrentFrame(VBLInt width, VBLInt height, VBLByte* buffer) = 0;
    virtual Result getSnapshpot(VBLInt width, VBLInt height, VBLByte* buffer, VBLLonglong pos) = 0;
    virtual Result saveCurrentFrame(VBLInt width, VBLInt height, VBLConstPChar filePath) = 0;

    // 播放器属性（playerKey）
    virtual Result setPlayerProperty(VBLConstPChar key, const Property& value) = 0;
    virtual Result playerProperty(VBLConstPChar key, Property& value) = 0;

    // 回调
    virtual Result addCallback(IPlayCallback* callback) = 0;
    virtual Result removeCallback(IPlayCallback* callback) = 0;

    // 丢帧统计
    virtual Result startCountDropFrame() = 0;
    virtual Result endCountDropFrame(VBLInt& frameCount) = 0;
};
```

### PlayManager 播放器属性键（playerKey）

| Key | 类型 | 说明 |
|---|---|---|
| `resolution` | int | 预览分辨率（1=全画质，2=1/2，4=1/4）|
| `playbackHighQuality` | int | 播放质量（0=Low, 1=Medium, 2=High, 3=Max）|
| `pauseHighQuality` | bool | 暂停时高质量渲染 |
| `useGpu` | bool | 启用 GPU 渲染 |
| `useGpuDecode` | bool | 启用 GPU 解码 |
| `videoRenderType` | int | 渲染后端（D3D9/D3D11/OpenGL）|
| `audioRenderType` | int | 音频后端（DirectSound/WASAPI/CoreAudio）|
| `hdrMode` | int | HDR 模式（0=SDR, 1=PQ, 2=HLG, 3=DCI）|
| `backgroundColor` | int | 播放器背景色 |
| `enableFullSpeed` | bool | 全速播放（不受帧率限制）|

---

## 3. 依赖关系

```
PlayManager
  ├── 依赖 → Adapter（VblPreviewAdapter）   （底层渲染/解码）
  ├── 依赖 → DataModel（IDmTimeline）       （时间线数据源）
  └── 回调 → IPlayCallback                 （播放状态/位置回调给 UI）

EncodeManager
  ├── 依赖 → Adapter（VblProductionAdapter）（底层编码）
  ├── 依赖 → DataModel（IDmTimeline）
  └── 发布 → IMsEventBus（encode.progress / encode.done）
```

---

## 4. 时序图

### 4.1 时间线预览播放

```
UI 层         PlayManager      Adapter(Preview)    DataModel    IPlayCallback
  │                │                  │                │              │
  │ createPlayManager()              │                │              │
  ├──────────────► │                  │                │              │
  │ setParentWindow(hwnd, w, h)      │                │              │
  ├──────────────► │                  │                │              │
  │ addCallback(cb)│                  │                │              │
  ├──────────────► │                  │                │              │
  │                │ PreviewAdapter::setParentWindow(hwnd)            │
  │                ├─────────────────►│                │              │
  │ setMedia(timeline)               │                │              │
  ├──────────────► │                  │                │              │
  │                │ PreviewAdapter::bindTimeline(tl)  │              │
  │                ├─────────────────►│                │              │
  │ seek(0)        │                  │                │              │
  ├──────────────► │                  │                │              │
  │                │ PreviewAdapter::seek(0)           │              │
  │                ├─────────────────►│                │              │
  │ play()         │                  │                │              │
  ├──────────────► │                  │                │              │
  │                │ PreviewAdapter::play()            │              │
  │                ├─────────────────►│                │              │
  │                │                  │ 渲染帧 → 显示  │              │
  │                │                  │ onPlayPosition(t)             │
  │                │                  │◄───────────────┤              │
  │                │ callback->onPlaybackPositionChanged(t)           │
  │◄───────────────────────────────────────────────────────────────── ►│
  │ 更新进度条     │                  │                │              │
```

### 4.2 单帧截图

```
UI 层         PlayManager      Adapter(Preview)
  │                │                  │
  │ seek(targetPos, true, true)       │
  ├──────────────► │                  │
  │                │ PreviewAdapter::seek(targetPos)
  │                ├─────────────────►│
  │                │                  │ 解码目标帧
  │                │                  │ 渲染到帧缓冲
  │ getSnapshpot(w, h, buffer, pos)   │
  ├──────────────► │                  │
  │                │ PreviewAdapter::readPixels(buffer)
  │                ├─────────────────►│
  │                │  buffer 数据      │
  │                │◄─────────────────┤
  │ 拿到 RGBA 数据 │                  │
  │◄──────────────┤                  │
  │ 处理缩略图显示  │                  │
```

### 4.3 导出视频

```
UI 层       EncodeManager    Adapter(Production)   DataModel   EventBus
  │               │                 │                  │           │
  │ createEncodeManager()           │                  │           │
  ├─────────────► │                 │                  │           │
  │ setExportParams(params)         │                  │           │
  ├─────────────► │                 │                  │           │
  │ setTimeline(timeline)           │                  │           │
  ├─────────────► │                 │                  │           │
  │ startExport() │                 │                  │           │
  ├─────────────► │                 │                  │           │
  │               │ ProductionAdapter::createEncoder(params)       │
  │               ├───────────────── ►│                │           │
  │               │ encoder->setTimeline(tl)           │           │
  │               ├───────────────── ►│                │           │
  │               │ encoder->start()  │                │           │
  │               ├───────────────── ►│                │           │
  │               │                  │ 逐帧读取 DataModel            │
  │               │                  │ IDmTimeline::getFrame(pos)   │
  │               │                  ├─────────────────►│           │
  │               │                  │  帧数据           │           │
  │               │                  │◄─────────────────┤           │
  │               │                  │ 编码压缩 → 写文件 │           │
  │               │ onProgress(50%)  │                  │           │
  │◄──────────────┤                  │                  │           │
  │ 进度条更新     │                  │                  │           │
  │               │ onComplete()     │                  │           │
  │               │◄─────────────────┤                  │           │
  │               │ postEvent("encode.done", filePath)  │           │
  │               ├───────────────────────────────────────────────── ►│
  │ onEvent() 导出完成弹窗           │                  │           │ → UI
  │◄─────────────────────────────────────────────────────────────────────┤
```

### 4.4 Trim 预览

```
UI 层（Trim 面板）  PlayManager    Adapter    DataModel
    │                   │              │           │
    │ setMedia(sourceMedia)            │           │
    ├──────────────────►│              │           │
    │ setDuration(inPoint, duration)   │           │
    ├──────────────────►│              │           │
    │ play()            │              │           │
    ├──────────────────►│              │           │
    │                   │ 仅播放 [inPoint, inPoint+duration) 区间
    │                   │ 到达 inPoint+duration 自动 stop
    │ callback->onPlayEnd()           │           │
    │◄──────────────────┤              │           │
    │ 恢复 Trim 面板状态 │              │           │
```

---

## 5. 设计要点

| 要点 | 说明 |
|---|---|
| 窗口绑定 | `setParentWindow` 必须在 `setMedia` 之前调用，且需在主线程执行 |
| 多实例 | 支持同时创建多个 PlayManager 实例（如主预览 + Trim 预览并存）|
| 分辨率自适应 | 可动态切换 `resolution` 属性在低画质（流畅）和高画质（精准）间切换 |
| 丢帧统计 | `startCountDropFrame/endCountDropFrame` 用于性能调优 |
| 导出取消 | EncodeManager 支持 `stop()` 取消正在进行的导出任务 |
