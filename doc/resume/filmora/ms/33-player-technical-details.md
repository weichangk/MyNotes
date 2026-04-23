33-player-technical-details.md
# 播放器模块技术实现细节

> 项目：Wondershare Filmora（C++17 + Qt 5.15）
> 模块：FMediaPlayerView（`Src/FMediaPlayerView`）

---

## 一、模块概述

播放器模块是 Filmora 的核心预览引擎，负责时间线编辑预览、素材库预览、多机位预览、缩略图提取等功能。底层通过 VBL 渲染引擎支持 D3D11/D3D9/OpenGL/GDI 多渲染后端，上层通过三级 Presenter 继承体系（`FFMediaPlayerPresenter → FFMediaPlayerEditorPresenter → FFMainPlayerPresenter`）封装播放控制、可视化编辑、音频波形、示波器等能力，以 PIMPL + Observer 模式完全屏蔽 VBL 底层细节。

---

## 二、整体架构

```
┌──────────────────────────────────────────────────────────────────┐
│                         UI 层（Widget）                           │
│  FFMainPlayerWidget                                               │
│    ├── FFMainPlayerToolBar（播放/暂停/截帧/倍速/全屏等按钮）        │
│    ├── FFMediaPlayerEditorWidget                                  │
│    │     └── FFMediaPlayerWidget（QStackedWidget 渲染容器）        │
│    │           ├── FFVBLMediaPlayerWidget（VBL 原生 HWND 渲染）    │
│    │           ├── StreamMediaPlayerWidget（流媒体 Qt 渲染）        │
│    │           └── FFPreviewImageWidget（静帧预览 QPainter）       │
│    └── FFPlayerOverlayManager（覆盖层管理树）                      │
│          ├── FFCropPanZoomWidget（裁剪/平移/缩放编辑覆盖层）        │
│          ├── FFPlayerDrawingBoard（涂鸦画板覆盖层）                │
│          ├── FFPlayerPlayheadWidget（播放头覆盖层）                │
│          └── FFZoomAdsorbWidget（缩放吸附辅助线）                  │
└──────────────────────────────┬───────────────────────────────────┘
                               │ 信号/槽
┌──────────────────────────────▼───────────────────────────────────┐
│                       Presenter 层（三级继承）                     │
│  FFMainPlayerPresenter（总控）                                     │
│    └── FFMediaPlayerEditorPresenter（可视化编辑：关键帧/Mask/Trim） │
│          └── FFMediaPlayerPresenter（播放基类：状态机/进度/音量）   │
│                ├── VBL 播放器路由（IFFMediaPlayer）                │
│                ├── 流媒体播放器路由（IFFStreamMediaPlayer）         │
│                └── 在线音频播放器路由                              │
└──────────────────────────────┬───────────────────────────────────┘
                               │ VBL 纯虚接口
┌──────────────────────────────▼───────────────────────────────────┐
│                       VBL 接口层（纯虚抽象）                       │
│  IFFMediaPlayer（主播放器）                                        │
│    ├── IFFMediaPlayerPlayhead（播放头：帧范围/seek/lock）           │
│    ├── IFFMediaPlayerZoomer（缩放：比例/帧移动/渲染区矩形）         │
│    ├── IFFMediaPlayerSetting（设置：渲染后端/GPU/画质/安全区）      │
│    ├── IFFDrawingBoard（画板：绘图/橡皮擦/颜色/线宽）               │
│    └── IFFCropPanZoom（裁剪/平移/缩放）                            │
│  IFFMediaEditor（可视化编辑器：鼠标/键盘事件处理）                  │
│  IFFBsThumbManager（缩略图/帧提取管理器）                          │
└──────────────────────────────┬───────────────────────────────────┘
                               │ C linkage 工厂函数
┌──────────────────────────────▼───────────────────────────────────┐
│              VBL/WES 底层渲染引擎（闭源 DLL）                      │
│  createMediaPlayer() / releaseMediaPlayer()                       │
│  D3D11（推荐）/ D3D9 / OpenGL / GDI 渲染实现                      │
│  GPU 硬解码（DXVA2 / NVDEC / QSV）                                │
└──────────────────────────────────────────────────────────────────┘
```

---

## 三、核心数据结构

### 3.1 FFPlayState（播放状态枚举）

```cpp
enum FFPlayState {
    psStopped,      // 停止（初始状态）
    psPlaying,      // 播放中
    psPaused,       // 暂停
    psPreparePlay,  // 预备播放（过渡态，不与其他状态配对）
};
```

### 3.2 FFVideoRenderType（渲染后端）

```cpp
enum FFVideoRenderType {
    vrtD3D11,   // Direct3D 11（Windows 推荐，GPU 加速）
    vrtD3D9,    // Direct3D 9（旧版 Windows 兼容）
    vrtOpenGL,  // OpenGL（macOS 及跨平台路径）
    vrtGDI,     // GDI 软件渲染（兼容/降级）
    vrtDDraw,   // DirectDraw（仅旧版兼容）
};
```

### 3.3 FFPlayQualityType（播放画质）

```cpp
enum FFPlayQualityType {
    pqtFull = 0, // 全画质（1/1 分辨率）
    pqt1_2,      // 1/2 分辨率（降采样，减轻 GPU 压力）
    pqt1_4,      // 1/4 分辨率（低端机器降级）
};
```

### 3.4 FFPlayerChangedType（播放器事件类型）

```cpp
enum FFPlayerChangedType {
    pctPlayState,              // 状态变化（Playing/Paused/Stopped）
    pctProgress,               // 帧进度变化
    pctSeekFinished,           // seek 完成
    pctSnapshot,               // 截帧完成（携带输出路径）
    pctAudioDb,                // 音频分贝（驱动示波器 UI）
    pctFrameElapsedTime,       // 帧渲染耗时（性能监控）
    pctWaitQuickPreviewFinish, // 快速预览等待（before/after）
    pctMultiCamera,            // 多机位帧数据
    pctError,                  // 错误（网络/无效媒体/未知）
    pctPlayEnd,                // 播放自然结束
    // … 共 20+ 类型
};
```

### 3.5 IFFMediaPlayer::Param（播放参数）

```cpp
struct Param {
    IFFAbstractMediaItem* media{nullptr};       // 播放素材（不得为空）
    FFMediaSourceType mediaSourceType{mstUnknow}; // 来源（媒体库/时间线/动画）
    qlonglong         position{0};              // 起始帧位置
    IFFTimeline*      applyTimeline{nullptr};   // 绑定时间线（素材预览时为空）
    bool              skipCheckDuplicateMedia{false};
    bool              needSeek{true};
    bool              seekImmediately{false};
    bool              updateProgress{true};
};
```

### 3.6 FFFastSpeedType（倍速类型）

```cpp
enum class FFFastSpeedType {
    forwardX1 … forwardX32,   // 1x~32x 快进
    forwardX04,                // 0.4x 慢进
    backwardX1 … backwardX32, // 1x~32x 快退
    backwardX04,               // 0.4x 慢退
};
```

---

## 四、主要业务流程

### 4.1 播放控制调用链

```
用户点击"播放"按钮
     → FFMainPlayerToolBar::onPlayClicked()
     → FFMainPlayerPresenter::dispatchMessage(mmtPlay)
     → FFMediaPlayerEditorPresenter::play()
     → FFMediaPlayerPresenter::play()
     → IFFMediaPlayer::play()          [VBL 引擎接口，异步执行]

[VBL 工作线程回调]
IFFMediaPlayerEventObserver::onPlayerChangedEvent(pctPlayState, psPlaying)
     → FFMediaPlayerPresenterPrivate::dealPlayStateMsg()
     → emit sigStateChanged(psPlaying)
     → FFPlayStateMessage 广播（时间线模块同步播放状态）
```

### 4.2 Seek 调用链

```
用户拖动时间线播放头
     → IFFTimelinePlayhead::setCurrentPosition(px)
     → 时间线消息 tmtSeek
     → FFMediaPlayerPresenter::seek(frame)
     → IFFAbstractMediaPlayer::seek(frame, progress=true, force=false)

[VBL 引擎 seek 完成后回调]
onPlayerChangedEvent(pctSeekFinished)
     → emit sigSeekFinished()
     → exitPreviewImageMode()     // 自动退出静帧预览模式
```

### 4.3 播放头与时间线帧同步

```
VBL 引擎每帧渲染完成
     → onPlayerChangedEvent(pctProgress, currentFrame)
     → emit sigProgressChanged(currentFrame)
     → playhead()->setCurrentFrame(currentFrame)
          → IFFMediaPlayerPlayhead::setCurrentFrame()
          → onPlayheadPropertyChangedEvent(pptPosition)
          → emit sigFrameChanged(currentFrame)
          → 时间线 Presenter 接收，更新时间线播放头 UI 位置
```

### 4.4 Quick Preview 同步等待机制

```
时间线添加特效/修改属性 → VBL 内部 Quick Preview 启动
     → onPlayerChangedEvent(pctWaitQuickPreviewFinish, before=true)
     → FFMediaPlayerPresenter 启动 QEventLoop::exec(ExcludeUserInputEvents)
          // 阻塞 UI 输入但保持事件循环（防止窗口冻结）

[VBL Quick Preview 完成]
     → onPlayerChangedEvent(pctWaitQuickPreviewFinish, before=false)
     → emit sigWaitQuickPreviewFinished()
     → QEventLoop::quit()    // 解除阻塞，UI 恢复响应
```

### 4.5 截帧调用链

```
用户点击截帧按钮
     → FFMainPlayerPresenter::snapshot()
     → IFFMediaPlayer::saveCurrentFrame(width, height, filePath)  [异步]

[完成后 VBL 回调]
onPlayerChangedEvent(pctSnapshot, path)
     → emit sigSaveCurrentFrameEnd(path, success)
     → FFSnapshotFormatDialog（首次弹出格式选择：JPG/PNG/BMP）
     → 文件写入完成，系统通知用户
```

### 4.6 缩略图异步提取

```
时间线 Clip 请求缩略图
     → FFTimelineThumbnailPresenter::generate(cachePath)
     → IFFBsThumbManager::startFetchTimelineThumbnailTask(cachePath)
          [VBL 底层异步帧提取]

[每帧完成]
IFFThumbCallback::onThumbnailReady(index, thumbPath, QPixmap)
     → m_oThumbnails[index] = thumbnail
     → emit sigThumbnailChanged()
     → FFTimelineThumbnailWidget 重绘（逐帧填充缩略图条）
```

### 4.7 多播放器实例管理

```
主界面（1个）：FFMainPlayerPresenter
高级字幕编辑：独立 FFMediaPlayerEditorPresenter（pstAdvanceSubtitle）
分屏高级编辑：独立 FFMediaPlayerEditorPresenter（pstAdvanceSplitScreen）
文字特效编辑：独立 FFMediaPlayerEditorPresenter（pstAdvanceText）
多机位模式：  FPlayerMultiCameraPresenter（管理多路帧回调）

// 所有实例通过 FFMediaPlayerFactory 工厂创建
// 产品差异化：setMediaPlayerFactory(new CustomFactory()) 替换工厂
// 音量统一：sycnMainPlayerVolume() 保证所有实例音量一致
```

---

## 五、核心技术点

### 5.1 原生窗口（Native Window）嵌入渲染

VBL 渲染引擎通过直接接管 Win32 HWND 进行 D3D 渲染，避免 Qt 合成器介入：

```cpp
// FFVBLMediaPlayerWidget 初始化
widget->setAttribute(Qt::WA_NativeWindow);               // 强制创建真实 HWND
widget->setAttribute(Qt::WA_DontCreateNativeAncestors);  // 隔离父窗口原生化
winId(); // 触发 HWND 分配

// VBL 引擎接管渲染
vblPlayer->setParentWindow(
    (void*)widget->winId(),       // HWND
    {widget->width(), widget->height()}
);
// 此后 D3D11 SwapChain 绑定该 HWND，VBL 直接 Present，Qt 不参与合成
```

### 5.2 QStackedWidget 多渲染后端无缝切换

`FFMediaPlayerWidget` 内部是 `QStackedWidget`，三个子视图对应三种播放器类型：

| 索引 | Widget | 播放器类型 |
|---|---|---|
| 0 | `FFVBLMediaPlayerWidget` | VBL（本地视频/时间线预览） |
| 1 | `StreamMediaPlayerWidget` | 流媒体/GIF |
| 2 | `FFPreviewImageWidget` | 静帧预览（`QImage` + `QPainter`）|

切换时只需 `stackedWidget->setCurrentIndex(idx)`，无需重建 Widget，渲染上下文保持。

### 5.3 三种媒体来源类型的路由差异

```cpp
enum FFMediaSourceType {
    mstMediaLibrary, // 素材预览：不绑定时间线，无 Quick Preview，独立音量
    mstTimeline,     // 时间线预览：绑定 IFFTimeline，启用 Quick Preview，驱动播放头
    mstAnimation,    // 动画预览：不发 sigStateChanged（防止干扰主播放器状态）
};
```

`FFMediaPlayerPresenterPrivate::dealPlayStateMsg()` 对 `mstAnimation` 的状态变化静默处理，避免其他模块响应动画预览的播放状态。

### 5.4 播放画质动态切换策略

Presenter 维护两套画质配置（时间线预览 / 素材预览各独立）：

```cpp
QMap<bool/*isTimeline*/, FFPlayQualityType> m_mapCurrentQuality;

// 暂停时切换高画质（精细预览）
setting()->setPauseHighQuality(true);   // 暂停时全画质
setting()->setPlaybackHighQuality(false); // 播放时降采样，保帧率

// 用户手动调整画质（工具栏下拉）
setPlayQuality(pqt1_2);  // 对 GPU 性能不足的机器降为 1/2 分辨率
```

### 5.5 PIMPL + 多观察者接口

所有 Presenter 的 Private 类同时实现多个 VBL 观察者接口，通过 `FFObserverHelper` 统一管理 attach/detach：

```cpp
class FFMediaPlayerPresenterPrivate
    : public IFFMediaPlayerEventObserver   // 播放器事件
    , public IFFPlayheadEventObserver      // 播放头事件
    , public IFFDrawingBoardEventObserver  // 画板事件
{
    FFObserverHelper m_observerHelper; // 析构时自动 detach，防野指针
};
// Public 类头文件无任何 VBL 类型暴露，编译隔离彻底
```

### 5.6 Quick Preview 的 QEventLoop 阻塞策略

VBL 内部 Quick Preview 要求上层等待其完成才能继续操作：

```cpp
// 使用 ExcludeUserInputEvents 阻塞鼠标/键盘输入
// 但保留定时器和绘制事件，窗口不冻结
QEventLoop loop;
connect(this, &FFMediaPlayerPresenter::sigWaitQuickPreviewFinished,
        &loop, &QEventLoop::quit);
loop.exec(QEventLoop::ExcludeUserInputEvents);
// 退出后继续执行，用户操作已被过滤
```

### 5.7 多机位帧数据传递（pctMultiCamera）

多机位模式下，VBL 通过 `pctMultiCamera` 事件同时推送多路摄像机的帧数据：

```cpp
// 事件携带 QVariantList，每项包含：
// - QPixmap（帧图像）
// - void*（IFFMultiCameraClip*，需转 FFVBLObjectRefPtr）
// - layoutType（布局模式：左右/上下/叠加）
// - cameraIndex（摄像机索引）
// FPlayerMultiCameraPresenter 接收并按布局渲染到各子视图
```

### 5.8 VBL 工厂函数（C linkage，跨 DLL 安全）

```cpp
// VBL DLL 导出 C 接口，避免跨 DLL 的 C++ 对象所有权问题
extern "C" {
    IFFMediaPlayer* createMediaPlayer();
    void            releaseMediaPlayer(IFFMediaPlayer*);
}
// FFMediaPlayerFactory 封装工厂调用，支持产品差异化替换
```

---

## 六、渲染后端与 GPU 加速

### 6.1 渲染后端选择

`IFFMediaPlayerSetting::setVideoRenderType()` 须在 `setParentWindow()` 前调用：

| 后端 | 场景 | GPU 利用 |
|---|---|---|
| `vrtD3D11` | Windows 默认推荐 | GPU 渲染 + 硬解码（DXVA2/NVDEC） |
| `vrtD3D9` | 旧版 Windows | GPU 渲染 |
| `vrtOpenGL` | macOS / 跨平台 | GPU 渲染 |
| `vrtGDI` | 兼容/降级模式 | 纯 CPU 软件渲染 |

### 6.2 GPU 解码与渲染

```cpp
setting->setUseGpu(true);        // 启用 GPU 渲染（D3D/OpenGL）
setting->setUseGpuDecode(true);  // 启用 GPU 硬解码（DXVA2 / NVDEC / QSV）
// 硬解码帧直接存留显存，避免 CPU→GPU 拷贝，4K 解码性能提升显著
```

### 6.3 全屏多显示器支持

```cpp
// FFPlayerFullScreenPreviewHelper
void enterFullScreen() {
    QScreen* screen = getTargetScreen(); // 检测目标显示器
#ifdef Q_OS_WIN
    widget->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
#else // macOS
    widget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    // macOS 用 Qt::Tool 避免与 Spaces/Mission Control 冲突
#endif
    widget->setGeometry(screen->geometry()); // 覆盖目标屏幕完整区域
    vblPlayer->setFullScreen(true);          // 通知 VBL 调整 SwapChain
}
// 退出全屏时恢复 m_originalWindowFlags + m_originalGeometry
```

---

## 七、设计模式应用

### 7.1 三级继承 + 职责分离

| Presenter | 职责 |
|---|---|
| `FFMediaPlayerPresenter` | 播放状态机、进度回调、音量、seek、截帧、多播放器类型路由 |
| `FFMediaPlayerEditorPresenter` | 可视化编辑（关键帧/Mask/Trim/裁剪）、时间线绑定、Quick Preview |
| `FFMainPlayerPresenter` | 音频波形、示波器、范围标记、多机位、工具栏命令分发 |

### 7.2 PIMPL + Observer

Private 类实现所有 VBL 观察者接口，Public 类头文件零 VBL 类型，编译隔离彻底，接口变更无需重编所有依赖方。

### 7.3 策略模式（多渲染后端）

`FFVideoRenderType` 枚举驱动 VBL 在运行时选择 D3D11/D3D9/OpenGL/GDI，上层代码无任何条件分支，渲染策略完全封装在 VBL 层。

### 7.4 工厂方法

`FFMediaPlayerFactory` 可被子类替换，产品差异化版本覆写工厂方法后，所有创建操作返回定制子类，主流程代码零修改。

### 7.5 组合模式（覆盖层树）

`FFPlayerOverlayManager` 以树形结构管理所有覆盖层 Widget（裁剪/画板/播放头/辅助线），统一处理显示/隐藏/事件传递，与主渲染层完全独立。

---

## 八、关键类职责速查

| 类名 | 职责 |
|---|---|
| `FFMainPlayerPresenter` | 主播放器总控：音频波形、示波器、多机位、工具栏命令分发 |
| `FFMediaPlayerEditorPresenter` | 可视化编辑：关键帧/Mask/Trim/裁剪，时间线绑定，Quick Preview 等待 |
| `FFMediaPlayerPresenter` | 播放基类：状态机、进度回调、seek、截帧、多播放器类型路由 |
| `FFMediaPlayerWidget` | 渲染容器（QStackedWidget），管理 VBL/流媒体/静帧三个子视图 |
| `FFVBLMediaPlayerWidget` | VBL 原生渲染 Widget，`WA_NativeWindow` 提供 HWND 给 D3D 渲染 |
| `FFMainPlayerToolBar` | 工具栏：播放/暂停/截帧/倍速/画质/全屏按钮 |
| `FFPlayerOverlayManager` | 覆盖层管理树：统一管理所有浮层 Widget 的生命周期和事件 |
| `FFPlayerFullScreenPreviewHelper` | 全屏辅助：多显示器检测、平台差异化 flags、SwapChain 通知 |
| `FPlayerMultiCameraPresenter` | 多机位控制：布局管理、多路帧数据渲染 |
| `FFTimelineThumbnailPresenter` | 缩略图提取：调度 IFFBsThumbManager 异步提取，管理帧缓存 |
| `FFUpdateImageThread` | 帧图像异步提取线程，`std::function<QImage()>` 回调驱动 |
| `IFFMediaPlayer` | VBL 主播放器完整接口（窗口绑定/时间线/截帧/全屏/裁剪） |
| `IFFMediaEditor` | VBL 可视化编辑器接口（鼠标/键盘事件处理、选择/编辑模式） |
| `IFFBsThumbManager` | VBL 缩略图/帧提取管理器接口 |
| `FFMediaPlayerFactory` | 播放器工厂，支持产品差异化替换 |
