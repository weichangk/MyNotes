# Adapter 模块详细设计文档

> 所属层：Adapter
> 接口头：`Interface/Adapter/`（33 个头文件）
> 初始化入口：`Interface/Adapter/VblAdapterLib.h`
> 命名空间：`VAL::`（区别于其他层的 `VBL::`）
> 底层实现：`modules/Adapter/wes/`（默认）或 `modules/Adapter/nle/`（可切换）

---

## 1. 模块职责

Adapter 层是 VBL 与底层 NLE/WES 视频引擎之间的**隔离适配层**：

- **屏蔽引擎差异**：统一接口，底层可在 WES SDK 和 NLE SDK 之间无缝切换（`VBL_IMPLEMENT_LEVEL`）
- **对象适配**：为每个 DataModel 对象（Timeline/Clip/Effect）创建对应的底层适配对象
- **能力代理**：渲染、解码、编码、特效执行、字体、音频等所有底层能力通过此层代理
- **统一初始化**：`initAdapter` / `uninitAdapter` 管理底层引擎生命周期

---

## 2. 全局初始化接口（VblAdapterLib.h）

```cpp
namespace VAL {
extern "C" {
    // 初始化底层引擎（传入媒体库路径）
    VALLIB_API void initAdapter(VBLConstPChar mediaLibraryPath);
    VALLIB_API void uninitAdapter();

    // 获取适配器工厂（主入口）
    VALLIB_API IAdapterFactory* getAdapterFactory(VBLConstPChar szPath = nullptr);

    // 解码管理器
    VALLIB_API VBL::Result initDecodeMgr();
    VALLIB_API VBL::Result initOpenClForGPU();
    VALLIB_API VBL::GpuSeviceFxLabSetupStep getServiceSetupStep();

    // 解码库接口
    VALLIB_API IMmpFunctionAdapter* getDecoderLibInterface();

    // OpenFX 插件
    VALLIB_API VBL::Result toAddOpenFXPluginPath(VBLConstPChar openfxPluginPath);
    VALLIB_API VBLBool isOpenFXPluginValid(VBLConstPChar path);

    // 性能与渲染配置
    VALLIB_API VBL::Result _setPerformanceMode(VBLInt mode);  // 0=Turbo, 1=MemorySafe
    VALLIB_API VBL::Result _setRenderPlatform(VBL::RenderPlatform platform);
    VALLIB_API void _setAudioWaveCacheEnabled(VBLBool enabled);
    VALLIB_API VBL::Result _setTLBProductName(VBLConstPChar productName);

    // AI 能力
    VALLIB_API VBL::Result setAlgorithmCachePath(VBLConstPChar path);
    VALLIB_API VBL::Result videoBlurSearch(VBLConstPChar keywordEmbedding,
        VBL::VBLPCharList* videoPaths, VBL::IVblSmartCutSearchResultList* results);
}
}
```

---

## 3. IAdapterFactory（适配器工厂）

工厂是所有适配器实例的创建入口（`getAdapterFactory()` 获取，用完需 `Release()`）：

```cpp
class IAdapterFactory : public VBL::IDmBaseObj {
    // 时间线适配器
    virtual ITimelineAdapter* createTimelineAdapter() = 0;

    // 媒体信息适配器
    virtual IMediaInfoAdapter* createMediaInfoAdapter(VBLConstPChar filePath, VBLConstPChar md5) = 0;
    virtual IMediaInfoAdapter* createPICMediaInfoAdapter(VBLConstPChar filePath, VBLConstPChar md5) = 0;

    // Clip 适配器系列
    virtual IVideoClipAdapter* createVideoClipAdapter(ClipType, VBLConstPChar path, ...) = 0;
    virtual IVideoClipAdapter* createPICVideoClipAdapter(ClipType, VBLConstPChar path, ...) = 0;
    virtual IVideoClipAdapter* createLottieVideoClipAdapter(ClipType, VBLConstPChar path, ...) = 0;
    virtual IAudioClipAdapter* createAudioClipAdapter(VBLConstPChar path, ...) = 0;
    virtual ITextClipAdapter* createTextClipAdapter(VBLConstPChar text, ...) = 0;
    virtual ISubtitleClipAdapter* createSubtitleClipAdapter(VBLConstPChar path) = 0;
    virtual ILayerClipAdapter* createLayerClipAdapter(VBLConstPChar configPath, ...) = 0;
    virtual IEffectAdapter* createEffectAdapter(VBLConstPChar configPath) = 0;
    virtual IEffectAdapter* createEffectAdapterById(VBLConstPChar effectId) = 0;
    virtual ITransitionAdapter* createTransitionClipAdapter(VBLConstPChar configPath) = 0;
    virtual ICompositeClipAdapter* createTitleTimelineClipAdapterFromPath(VBLConstPChar path) = 0;
    virtual ITimelineVideoClipAdapter* createTimelineVideoClipAdapter(ITimelineAdapter*, ...) = 0;
    virtual IMultiCamVideoClipApdater* createMultiVideoClipAdapter(ITimelineAdapter*) = 0;
    virtual IBufferClipAdapter* createBufferClipAdapter(uint8* data, uint32 len, w, h) = 0;
    virtual IAnimationAdapter* createAnimationAdapter(VBLConstPChar configPath) = 0;

    // 资源路径注册
    virtual VBL::Result addEffectResPath(VBLConstPChar resPath, VBLBool isPackage) = 0;
    virtual VBL::Result addTemplateResPath(VBLConstPChar resPath) = 0;
    virtual VBL::Result setEncryptPaths(VBL::VBLPCharList* paths) = 0;

    // 默认特效
    virtual VBL::Result addConfigEnabledDefaultEffects(IClipAdapter* pClip) = 0;
};
```

---

## 4. 适配器接口分类

### 4.1 时间线渲染（VblTimelineRenderAdapter.h）

```
ITimelineAdapter
  ├── addVideoClipAdapter(track, pos, clip)    向底层时间线添加视频 clip
  ├── addAudioClipAdapter(track, pos, clip)    向底层时间线添加音频 clip
  ├── addTransitionAdapter(clip, trans)        添加转场
  ├── addEffectAdapter(clip, effect)           添加特效
  ├── setTimelineConfig(config)               设置分辨率/帧率
  ├── getFrame(pos, width, height, buffer)    提取指定时间点帧
  ├── beginEdit() / endEdit()                 批量编辑事务
  └── getDuration()                           时间线总时长
```

### 4.2 预览播放（VblPreviewAdapter.h）

```
IPreviewAdapter
  ├── setParentWindow(hwnd, w, h)    绑定渲染窗口
  ├── setBounds(x, y, w, h)         设置渲染区域
  ├── setTimeline(ITimelineAdapter)  绑定时间线
  ├── setMedia(IDmBaseMedia)         绑定单媒体
  ├── play() / pause() / stop()     播放控制
  ├── seek(pos)                      跳转
  ├── setVolume(vol) / setMute()     音量
  ├── getFrame(w, h, buffer)        当前帧截图
  └── setCallback(IPlayCallback)    播放回调（位置/状态/错误）
```

### 4.3 导出生产（VblProductionAdapter.h）

```
IProductionAdapter
  ├── setTimeline(ITimelineAdapter)
  ├── setExportParam(format, codec, bitrate, ...)
  ├── start(outputPath) / stop() / pause() / resume()
  ├── progress() → 0~100
  └── setCallback(IProductionCallback)
```

### 4.4 媒体信息（VblMediaInfoAdapter.h）

```
IMediaInfoAdapter（通过 createMediaInfoAdapter(filePath) 创建）
  ├── width() / height()
  ├── duration()            总时长（us）
  ├── frameRate()           帧率（Rational）
  ├── videoStreamCount()    视频流数量
  ├── audioStreamCount()    音频流数量
  ├── hasAlpha()            是否含透明通道
  ├── isHDR()
  └── mediaType()           Video/Audio/Image/VideoAudio
```

### 4.5 音频波形提取（VblAudioWaveExtractAdapter.h）

```
IAudioWaveExtractAdapter
  ├── setMediaPath(path)
  ├── setWaveConfig(samplesPerPixel, channels)
  ├── start() / stop()
  ├── progress() → 0~100
  ├── getWaveData() → PCM 振幅数组
  └── setCallback(IWaveExtractCallback)
```

### 4.6 缩略图生成（VblThumbAdapter.h）

```
IThumbAdapter
  ├── setMediaPath(path)
  ├── extractFrame(pos, width, height, buffer)
  ├── extractFrameList(posList, width, height, callback)
  └── saveThumbnail(pos, width, height, filePath)
```

### 4.7 字体库（VblFontLibraryAdapter.h）

```
IFontLibraryAdapter（通过 createFontLibraryAdapter() 创建）
  ├── loadFontFromPath(path)
  ├── getFontList() → 字体名列表
  ├── getFontInfo(fontName) → 字体元数据
  └── isFontLoaded(fontName) → bool
```

### 4.8 GPU 检测（VblGpuCheckAdapter.h）

```
IGpuCheckAdapter
  ├── isGpuAvailable() → bool
  ├── getGpuInfo() → GpuInfo（型号/显存/驱动版本）
  ├── supportHardwareEncode(codec) → bool
  ├── supportHardwareDecode(codec) → bool
  └── getGpuMemory() → bytes
```

### 4.9 其他适配器

| 适配器 | 接口文件 | 说明 |
|---|---|---|
| `IVariableSpeedAdapter` | VblVariableSpeedAdapter.h | 变速帧重新映射 |
| `IVideoFrameExtractAdapter` | VblVideoFrameExtractAdapter.h | 视频帧批量提取 |
| `IMediaAnalysisAdapter` | VblMediaAnalysisAdapter.h | 素材智能分析算法 |
| `IRgbCurveAdapter` | VblRgbCurveAdapter.h | RGB 曲线调色 |
| `IAnimationAdapter` | VblAnimationAdapter.h | 关键帧动画渲染 |
| `IImageScopeAdapter` | VblImageScopeAdapter.h | 示波器/直方图分析 |
| `ILossnessConverterAdapter` | VblLossnessConverterAdapter.h | 无损格式转换 |
| `ICaptureAdapter` | VblCaptureAdapter.h | 屏幕/摄像头采集 |
| `IDomainProbe` | VblDomainProbe.h | 网络域名探测（桌面端）|
| `ITimelineVideoAnalyser` | ITimelineVideoAnalyser.h | 时间线视频智能分析 |

---

## 5. 底层实现切换机制

```cmake
# modules/Adapter/CMakeLists.txt
set(VBL_IMPLEMENT_LEVEL "wes" CACHE STRING "Set the implement for vbl (wes, nle)")

if(VBL_IMPLEMENT_LEVEL STREQUAL "wes")
    add_subdirectory(wes)   # 加载 WES SDK 实现
elseif(VBL_IMPLEMENT_LEVEL STREQUAL "nle")
    add_subdirectory(nle)   # 加载 NLE SDK 实现
endif()
```

两套实现提供完全相同的 `IAdapterFactory` 等接口，DataModel 层代码无需修改即可切换底层引擎。

---

## 6. 依赖关系

```
Adapter
  ├── 实现 → WES SDK（3rdparty/WES/）或 NLE SDK（3rdparty/NLE/）
  ├── 被 DataModel PRIVATE 依赖
  ├── 被 BackgroundTaskManager 依赖（任务执行时调用）
  └── 被 PlayManager / EncodeManager 依赖
```

---

## 7. 时序图

### 7.1 Adapter 初始化（系统启动）

```
BusinessLayer/UI    Adapter(initAdapter)   底层引擎(WES/NLE)
        │                  │                      │
        │ initAdapter(mediaLibPath)               │
        ├─────────────────►│                      │
        │                  │ WES::Initialize(mediaLibPath)
        │                  ├─────────────────────►│
        │                  │                      │ 注册解码插件
        │                  │                      │ 初始化渲染管线
        │                  │                      │ 加载字体引擎
        │                  │ initDecodeMgr()       │
        │                  ├─────────────────────►│
        │                  │                      │ 注册媒体解码器
        │                  │ initOpenClForGPU()    │
        │                  ├─────────────────────►│
        │                  │                      │ 初始化 OpenCL 上下文
        │                  │                      │ GPU 检测
        │ 获取工厂           │                      │
        │ getAdapterFactory()                      │
        ├─────────────────►│                      │
        │  IAdapterFactory*│                      │
        │◄─────────────────┤                      │
```

### 7.2 导入媒体文件（媒体信息读取）

```
DataModel(createSourceMedia)  Adapter(MediaInfo)    WES/NLE(Decode)
            │                       │                    │
            │ factory->createMediaInfoAdapter(filePath)  │
            ├──────────────────────►│                    │
            │                       │ WES::MediaInfo::probe(filePath)
            │                       ├───────────────────►│
            │                       │                    │ 解析媒体容器头
            │                       │                    │ 读取视频流参数
            │                       │                    │ 读取音频流参数
            │                       │  MediaMetadata     │
            │                       │◄───────────────────┤
            │  IMediaInfoAdapter*   │                    │
            │◄──────────────────────┤                    │
            │ info->width()         │                    │
            │ info->height()        │                    │
            │ info->duration()      │                    │
            │ 创建 IDmSourceMedia 对象，存储元数据         │
```

### 7.3 时间线渲染帧（播放/截图）

```
PlayManager    IPreviewAdapter    ITimelineAdapter    WES渲染管线
    │                │                  │                  │
    │ seek(pos)      │                  │                  │
    ├───────────────►│                  │                  │
    │                │ timeline->seek(pos)                 │
    │                ├─────────────────►│                  │
    │                │                  │ 遍历该时刻可见 clip │
    │                │                  │ 解码视频帧         │
    │                │                  │ 应用特效/转场      │
    │                │                  │ 混合渲染输出       │
    │                │                  ├─────────────────►│
    │                │                  │  渲染到帧缓冲区   │
    │                │                  │◄─────────────────┤
    │                │ callback->onFrameReady()            │
    │◄───────────────┤                  │                  │
    │ 更新预览窗口   │                  │                  │
```

### 7.4 Clip 添加到底层时间线

```
DataModel(IDmTimeline::addClip)   ITimelineAdapter    IVideoClipAdapter
            │                            │                   │
            │ timelineAdapter->addVideoClipAdapter(track, pos, clipAdapter)
            ├───────────────────────────►│                   │
            │                            │ WES::Timeline::addClip(track, pos, handle)
            │                            ├──────────────────►│
            │                            │                   │ 注册 clip 到底层
            │                            │                   │ 建立解码上下文
            │                            │                   │ 预加载首帧
```

### 7.5 WES ↔ NLE 实现切换

```
CMake 配置阶段:
    VBL_IMPLEMENT_LEVEL = "wes"
    ↓
    modules/Adapter/wes/ 编译
    ↓
    生成 Adapter.dll (包含 WES 实现)

运行时:
    DataModel → IAdapterFactory::createVideoClipAdapter()
              → WesAdapterFactory::createVideoClipAdapter()
              → new WesVideoClipAdapter(WES::ClipHandle)

切换到 NLE:
    VBL_IMPLEMENT_LEVEL = "nle"
    ↓
    modules/Adapter/nle/ 编译
    ↓
    DataModel 代码无需任何修改
              → NleAdapterFactory::createVideoClipAdapter()
              → new NleVideoClipAdapter(NLE::ClipHandle)
```

---

## 8. 设计要点

| 要点 | 说明 |
|---|---|
| 命名空间隔离 | Adapter 使用 `VAL::` 命名空间，避免与 `VBL::` 接口混淆 |
| 工厂生命周期 | `getAdapterFactory()` 每次调用 AddRef，使用完必须 `Release()`（RAII） |
| DataModel PRIVATE 依赖 | Adapter 对上层业务层不可见，只有 DataModel 知道 Adapter 的存在 |
| 一一对应关系 | 每个 IDmClip 对象内部持有一个对应的 IXxxClipAdapter，生命周期绑定 |
| GPU 初始化顺序 | `initDecodeMgr()` → `initOpenClForGPU()` 有严格顺序，OpenCL 依赖解码管理器 |
| 性能模式 | Turbo 模式优先速度（高内存占用），MemorySafe 模式限制内存使用 |
| OpenFX 支持 | 通过 `toAddOpenFXPluginPath` 动态加载第三方特效插件（桌面端）|
