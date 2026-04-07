# FVBL 总体结构图

> 基于《FVBL框架设计文档》整理，结合当前代码库（v7.17.x）实际结构更新

---

## 一、音视频业务产品整体分层

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                                     UI 层                                            │
│                                                                                     │
│         Filmora 系列  │  Filmii  │  喵影(MiaoYing)  │  VCU  │  其他音视频产品         │
│                                                                                     │
│  职责：处理用户交互及界面展示，调用下层完成产品核心功能，监听事件总线更新 UI 界面          │
└──────────────────────────────────┬──────────────────────────────────────────────────┘
                                   │  调用 VBL 公开接口 / Wrapper 层
                                   │
┌──────────────────────────────────▼──────────────────────────────────────────────────┐
│                            FVBL 业务中间层（本仓库）                                   │
│                                                                                     │
│  ┌─────────────────────────────────────────────────────────────────────────────┐   │
│  │                        业务逻辑层  BusinessLayer                              │   │
│  │                                                                             │   │
│  │  CommonSetting │ ProjectEditor │ SourceManager │ TimelineUX │ TimelineEditor │   │
│  │  VisualEditor  │ MaterialAnalysis │ TemplateMatch │ PreRender │ AIManager    │   │
│  │  AICopilot     │ ProjectConversionService │ PreferenceManager               │   │
│  └──────────────────────────────────┬──────────────────────────────────────────┘   │
│                                     │                                               │
│  ┌──────────────────────────────────▼──────────────────────────────────────────┐   │
│  │                        数据模型层  DataModel                                  │   │
│  │                                                                             │   │
│  │    IDmProject  │  IDmTimeline  │  IDmTrack  │  IDmClip（各类型）              │   │
│  │    IDmMediaItem（各类型）  │  IDmEffect  │  IDmTransition                    │   │
│  └───────┬───────────────────────────────────────────────────────┬────────────┘   │
│          │  PUBLIC 依赖                                            │  PRIVATE 依赖   │
│  ┌───────▼──────────────────────┐          ┌─────────────────────▼──────────────┐  │
│  │   ListenerCenter             │          │       基础服务层  BaseService         │  │
│  │   （IMsEventBus 事件总线）    │          │                                     │  │
│  │                              │          │  BsNet │ BsCloudResource │ BsPreset  │  │
│  │  registerEvent / subscribe   │          │  BsCloudDisk │ BsCloudConfig         │  │
│  │  postEvent（多线程模式）       │          │  BsGpuCheck │ BsPluginManager        │  │
│  │  postStickyEvent             │          │  BackgroundTaskManager │ BsAI        │  │
│  │                              │          │  BSWsid │ BsCommonSetting            │  │
│  └──────────────────────────────┘          │  PlayManager │ EncodeManager         │  │
│                                            │  UndoTemplateStack                  │  │
│                                            │  VblLogger │ VblUtils │ BsDiskFolder │  │
│                                            └──────────────────────┬──────────────┘  │
│                                                                   │                 │
│  ┌────────────────────────────────────────────────────────────────▼──────────────┐  │
│  │                           适配层  Adapter                                      │  │
│  │                                                                               │  │
│  │    IAdapterFactory  │  VblPreviewAdapter  │  VblTimelineRenderAdapter         │  │
│  │    VblProductionAdapter  │  VblMediaInfoAdapter  │  VblAudioWaveAdapter       │  │
│  │    VblThumbAdapter  │  VblFontLibraryAdapter  │  ITimelineVideoAnalyser       │  │
│  │                                                                               │  │
│  │    ┌──────────────────────────────────────────────────────────────────────┐  │  │
│  │    │  VBL_IMPLEMENT_LEVEL=wes（默认）  ←→  VBL_IMPLEMENT_LEVEL=nle        │  │  │
│  │    └──────────────────────────────────────────────────────────────────────┘  │  │
│  └──────────────────────────────────────┬─────────────────────────────────────────┘  │
│                                          │                                           │
│  ─────────────── 横切关注点（全层可用）───│─────────────────────────────────────────  │
│  Common：VblRefCnt │ VblSafePtr │ VblTypeDef │ IDmBaseObj │ IVblTimer               │
└──────────────────────────────────────────┼─────────────────────────────────────────-┘
                                           │
┌──────────────────────────────────────────▼──────────────────────────────────────────┐
│                       底层引擎（WES SDK / NLE SDK / 系统资源）                        │
│                                                                                     │
│                  3rdparty/NLE/   ·   3rdparty/WES-Mobile/                           │
└─────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 二、跨语言 Wrapper 层

```
┌──────────────────────────────────────────────────────────────────────────┐
│                     Wrapper 层（横向包装 FVBL 核心）                        │
│                                                                          │
│  ┌────────────────┐  ┌────────────────┐  ┌───────────┐  ┌────────────┐  │
│  │   vbl-c        │  │ vbl-clrWrapper │  │ vbl-swift │  │  vbl-web   │  │
│  │                │  │                │  │           │  │            │  │
│  │ Objective-C/C  │  │  .NET CLR      │  │  Swift    │  │ WASM/JS    │  │
│  │ iOS / macOS UI │  │  C# Win 产品   │  │ macOS/iOS │  │ Web 产品   │  │
│  └────────┬───────┘  └───────┬────────┘  └─────┬─────┘  └─────┬──────┘  │
└───────────┼──────────────────┼─────────────────┼──────────────┼──────────┘
            └──────────────────┴─────────────────┴──────────────┘
                                       │
                              调用 VBL 核心 C++ 接口
```

---

## 三、模块全景图（含子模块）

### 3.1 业务逻辑层（BusinessLayer）

```
BusinessLayer
├── CommonSetting          ─ FVBL 初始化配置（路径、功能开关），IFbCommonSetting
├── ProjectEditor          ─ 工程生命周期（新建/打开/保存/自动保存/Undo）, IVbProjectEditor
├── SourceManager          ─ 媒体资源库管理（IFbResourceManager / IFbSourceManager）
├── TimelineUX             ─ Filmora 系列时间线交互总入口，IFbTimelineUX
├── TimelineEditor         ─ 时间线 clip/track 数据操作，ITimelineEditor / IFbTrackEditor
│   ├── ClipEditor         ─ 各类 Clip 属性编辑（IFbClipEditor / IFbVideoClipEditor …）
│   ├── EffectEditor       ─ 滤镜编辑，IFbEffectEditor
│   └── TransitionEditor   ─ 转场编辑，IFbTransitionEditor
├── VisualEditor           ─ 时间线/Clip 可视化编辑操作
├── PreRender              ─ 时间线预渲染（后台渲染缓冲），IFbTimelinePreRender
├── MaterialAnalysis       ─ 素材分析（算法权重/阈值/精彩片段筛选），IFbMaterialAnalysis
├── TemplateMatch          ─ 模版自动匹配，IFbTemplateMatch（桌面端专属）
├── AIManager              ─ AI 任务统一调度，IVbAIManager
├── AICopilot              ─ AI Copilot / ActionEditor
├── ProjectConversionService ─ Filmora ↔ Filmii ↔ 喵影 工程格式互转
└── PreferenceManager      ─ 用户偏好持久化
```

### 3.2 数据模型层（DataModel）

```
DataModel
├── IDmProject             ─ 工程文档属性管理（时间线 + 资源区入口）
├── 媒体区资源
│   ├── IDmMediaItem       ─ 媒体项基类（type / mediaId / alias / selected）
│   ├── IDmMediaFolder     ─ 媒体文件夹容器（目录树结构）
│   ├── IDmResourceFolder  ─ 内置资源分类容器
│   ├── IDmSourceMedia     ─ 用户导入媒体（Video / Audio / VideoAudio）
│   ├── IDmResourceMedia   ─ 内置资源（Title / Effect / Transition）
│   ├── IDmElementMedia    ─ 内置画中画资源
│   ├── IDmMusicMedia      ─ 内置音乐资源
│   ├── IDmTemplateMedia   ─ 模板资源（含完整时间线）
│   └── IDmVideoScenceMedia ─ 场景检测片段资源
├── 时间线
│   ├── IDmTimeline        ─ 时间线（轨道管理 / 帧提取 / 序列化）
│   ├── IDmTrack           ─ 轨道（基本属性）
│   └── IDmClip            ─ Clip 基类
│       ├── IDmVideoClip       视频 Clip
│       ├── IDmAudioClip       音频 Clip
│       ├── IDmVideoAudioClip  音视频 Clip
│       ├── IDmTextClip        文字 Clip
│       ├── IDmLayerClip       特效 Clip
│       └── IDmTimelineClip    复合（嵌套时间线）Clip
├── IDmEffect              ─ 滤镜算法参数
├── IDmTransition          ─ 转场算法参数
├── IDmVariableSpeed       ─ 变速参数
├── IDmIntelligentSegment  ─ AI 智能分段
├── IDmSmartMosaic         ─ AI 智能马赛克
└── IDmObjectRemove        ─ AI 对象消除
```

### 3.3 监听中心（ListenerCenter）

```
ListenerCenter
├── IMsEventBus            ─ 事件总线核心接口
│   ├── registerEvent / unregisterEvent
│   ├── subscribe / unsubscribe / setSubscriberActive
│   ├── postMainThreadEvent    主线程投递
│   ├── postAsyncEvent         新开异步线程
│   ├── postBackgroundEvent    固定后台线程
│   ├── postCurrentEvent       当前线程同步
│   ├── postEvent(ThreadMode)  指定线程投递
│   └── postStickyEvent        粘性事件（新订阅者立即收到）
└── MsEventBusLib          ─ 全局单例 + 局部实例创建
```

### 3.4 基础服务层（BaseService）

```
BaseService
├── 网络与云服务
│   ├── BsNet              ─ HTTP 客户端（代理 / 断点续传）
│   ├── BsCloudResource    ─ 云端资源（特效包 / 模板）下载与缓存
│   ├── BsCloudDisk        ─ 三方云盘（Dropbox / Google Drive / OneDrive）
│   └── BsCloudConfig      ─ 云端远程配置（lantern 实现）
├── 媒体处理服务
│   ├── PlayManager        ─ 播放器（预览、帧提取）
│   ├── EncodeManager      ─ 编码导出管理
│   └── BackgroundTaskManager ─ 后台任务调度
│       ├── IBsAudioWaveExtractTask  音频波形提取
│       ├── IBsFaceDetectTask        人脸检测
│       ├── IBsSceneDetectTask       场景检测
│       ├── IBsThumbnailExtractTask  缩略图生成
│       └── IBsVideoFrameExtractTask Clip 视频帧获取
├── 算法与 AI
│   ├── BsAI               ─ AI 算法服务封装
│   └── BsBeatManager      ─ 音频节拍检测
├── 资源与配置管理
│   ├── BsPreset           ─ 本地预设文件管理
│   ├── BsPluginManager    ─ OpenFX 等插件管理
│   ├── BsGpuCheck         ─ GPU 能力探测
│   ├── BsCommonSetting    ─ 公共配置（路径 / 开关）
│   └── BSWsid             ─ 万兴账户授权（IBsWsidAuthorize）
├── 编辑辅助
│   └── UndoTemplateStack  ─ Undo/Redo 管理器（IBsUndoTemplateStack）
└── 基础设施
    ├── VblLogger          ─ 日志系统（IBsLogger / IBsFileChannel / IBsConsoleChannel）
    ├── VblUtils           ─ 工具集（线程池 / FileStream / AES / StringList）
    ├── BsDataStructure    ─ 公共数据结构（IVblList / IVblString）
    └── BsDiskFolder       ─ 磁盘目录工具
```

### 3.5 适配层（Adapter）

```
Adapter  (命名空间 VAL::)
├── 初始化
│   ├── initAdapter / uninitAdapter      全局初始化
│   └── getAdapterFactory               获取 IAdapterFactory
├── 媒体处理
│   ├── VblMediaInfoAdapter             媒体信息获取
│   ├── VblMediaAnalysisAdapter         媒体内容分析
│   ├── VblImageDecoderAdapter          图像解码
│   └── VblVideoFrameExtractAdapter     视频帧提取
├── 时间线渲染
│   ├── VblTimelineRenderAdapter        时间线渲染
│   ├── VblPreviewAdapter               预览/播放
│   └── VblAnimationAdapter             动画/关键帧
├── 音频
│   ├── VblAudioWaveAdapter             音频波形展示
│   └── VblAudioWaveExtractAdapter      音频波形提取
├── 导出
│   └── VblProductionAdapter            编码导出
├── 资源
│   ├── VblThumbAdapter                 缩略图生成
│   ├── VblFontLibraryAdapter           字体库
│   └── VblEffectHelperAdapter          特效资源辅助
├── 系统/平台
│   ├── VblGpuCheckAdapter              GPU 检测
│   ├── VblEncodeParamAdapter           编码参数
│   ├── VblFrameRateAdapter             帧率适配
│   ├── VblCaptureAdapter               屏幕/摄像头采集
│   ├── VblLossnessConverterAdapter     无损转换
│   └── VblDomainProbe                  域名探测（仅桌面端）
├── 插件
│   ├── VblMmpFunctionAdatper           解码库（MMP）
│   └── IVbVstEffectAdapter             VST 音效插件
└── AI
    └── ITimelineVideoAnalyser          时间线视频智能分析
```

---

## 四、模块间关系

### 4.1 模块调用关系总览

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                                UI 层                                          │
│   ① 调用业务接口        ② 监听事件回调（IMsEventBus）                           │
└────────┬────────────────────────────────────────┬────────────────────────────┘
         │ 调用                                    │ 订阅
         ▼                                        ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         BusinessLayer（业务逻辑层）                            │
│                                                                             │
│  ProjectEditor ──────┬──── SourceManager                                   │
│       │              │          │                                           │
│       │         跨编辑器操作     │ 管理媒体资源                                │
│       ▼              │          ▼                                           │
│  TimelineUX ─────────┘    IFbResourceManager                               │
│       │                                                                     │
│       │ 封装业务交互                                                          │
│       ▼                                                                     │
│  TimelineEditor ─── ClipEditor / EffectEditor / TransitionEditor            │
│       │                                                                     │
│       ├── 每次操作：IBsUndoTemplateStack::push()   ← UndoTemplateStack       │
│       └── 每次变更：IMsEventBus::postEvent()       → ListenerCenter → UI     │
│                                                                             │
│  MaterialAnalysis ─── IBsMediaAnalysisTask ─── BackgroundTaskManager       │
│       │                                                                     │
│       └──► TemplateMatch（接收分析结果，自动匹配模板）                          │
└──────────────────────────────────┬──────────────────────────────────────────┘
                                   │ 读写数据
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                          DataModel（数据模型层）                               │
│                                                                             │
│   IDmProject                                                                │
│     ├── IDmMediaFolder  ← SourceManager 写入资源                             │
│     └── IDmTimeline     ← TimelineEditor 写入 Clip/Track 数据                │
│           ├── IDmTrack                                                      │
│           └── IDmClip（各类型）                                               │
│                 ├── IDmEffect                                               │
│                 └── IDmTransition                                           │
│                                                                             │
│   PUBLIC 依赖：ListenerCenter / BsCommonSetting / BsCloudResource / BsCloudConfig
│   PRIVATE 依赖：Adapter / BsNet / BsGpuCheck / BsPreset / BsPluginManager / BsCloudDisk
└──────────────────────────────────┬──────────────────────────────────────────┘
                                   │ 调用底层适配
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                            Adapter（适配层）                                  │
│                                                                             │
│   DataModel 通过 IAdapterFactory 获取各适配器实例：                            │
│     VblPreviewAdapter / VblTimelineRenderAdapter / VblProductionAdapter     │
│     VblMediaInfoAdapter / VblAudioWaveAdapter / VblThumbAdapter  …          │
│                                                                             │
│   ┌──────────────────────────┐   ┌──────────────────────────┐              │
│   │  wes/ 实现（默认）        │   │  nle/ 实现（可切换）       │              │
│   └──────────────────────────┘   └──────────────────────────┘              │
└──────────────────────────────────┬──────────────────────────────────────────┘
                                   │
                                   ▼
                        WES SDK / NLE SDK（底层引擎）
```

### 4.2 模块间关系说明表

| 调用方 | 被调用方 | 关系类型 | 说明 |
|---|---|---|---|
| UI 层 | BusinessLayer | 直接调用 | UI 调用业务接口完成功能，不直接访问 DataModel |
| UI 层 | ListenerCenter | 订阅监听 | UI 通过 subscribe 注册回调，被动接收数据变更通知 |
| ProjectEditor | SourceManager | 协作 | 跨编辑器操作（如导入媒体到时间线）由 ProjectEditor 协调 |
| ProjectEditor | TimelineUX | 管理 | ProjectEditor 持有并管理各编辑器实例 |
| TimelineUX | TimelineEditor | 封装 | TimelineUX 封装具体的 TimelineEditor 操作，面向产品系列定制 |
| TimelineEditor | DataModel | 写数据 | 通过 IDmTimeline::beginEdit/addClip/endEdit 写入 |
| TimelineEditor | UndoTemplateStack | 记录 | 每个可逆操作都调用 push(cmd) 记录到 Undo 栈 |
| TimelineEditor | ListenerCenter | 发布事件 | 操作完成后 postEvent 通知 UI 刷新 |
| MaterialAnalysis | BackgroundTaskManager | 委托执行 | 分析算法作为后台任务提交，异步执行 |
| MaterialAnalysis | TemplateMatch | 数据传递 | MaterialAnalysis 结果作为 TemplateMatch 的输入 |
| DataModel | Adapter | PRIVATE 调用 | DataModel 内部通过 IAdapterFactory 调用底层能力 |
| DataModel | ListenerCenter | PUBLIC 依赖 | DataModel 事件变更通过 EventBus 广播，对外透出 |
| BackgroundTaskManager | Adapter | 调用适配器 | 任务执行时调用对应适配器（如 VblAudioWaveExtractAdapter） |
| BackgroundTaskManager | ListenerCenter | 发布完成事件 | 任务完成后 postMainThreadEvent 通知 UI |
| 所有模块 | VblLogger | 隐式依赖 | 通过 group_private_link_libs 全局注入 |

### 4.3 ListenerCenter 与各模块的交互关系

```
                   ┌──────────────────────────────────┐
                   │         IMsEventBus               │
                   │       （全局事件总线）               │
                   └──────────────────────────────────┘
                    ▲  发布                  │  通知回调
        ┌───────────┴──────────┐            │
        │                      │            ▼
┌───────────────┐    ┌─────────────────┐  ┌───────────────┐
│ TimelineEditor│    │ BackgroundTask  │  │    UI 层       │
│ 数据变更时     │    │ Manager         │  │ 订阅感兴趣事件  │
│ postEvent()   │    │ 任务完成时       │  │ 更新 UI 界面   │
└───────────────┘    │ postEvent()     │  └───────────────┘
                     └─────────────────┘
        ┌───────────────────────────────────────────────┐
        │            常见事件分类                         │
        │  timeline.*   时间线数据变更（clip/track 增删改）│
        │  media.*      媒体区数据变更                    │
        │  project.*    工程级操作（打开/保存/关闭）        │
        │  task.*       后台任务进度/完成                  │
        │  error.*      错误与异常通知                     │
        │  service.*    服务状态变更（网络/GPU/账户）       │
        └───────────────────────────────────────────────┘
```

---

## 五、模块时序图

### 5.1 系统初始化时序

```
UI 层              ProjectEditor      DataModel          Adapter
  │                    │                  │                 │
  │  init()            │                  │                 │
  ├──────────────────► │                  │                 │
  │                    │  initAdapter()   │                 │
  │                    ├──────────────────┼────────────────►│
  │                    │                  │  initAdapter()  │
  │                    │                  │◄────────────────┤
  │                    │                  │  (底层引擎初始化) │
  │                    │  initDataModel() │                 │
  │                    ├─────────────────►│                 │
  │                    │                  │  initDecodeManager()
  │                    │                  ├────────────────►│
  │                    │                  │  initOpenCL()   │
  │                    │                  ├────────────────►│
  │                    │  setCommonSetting│                 │
  │                    ├─────────────────►│                 │
  │                    │  addEffectResPath│                 │
  │                    ├─────────────────►│                 │
  │  init() done       │                  │                 │
  │◄──────────────────┤                  │                 │
```

### 5.2 打开/创建工程时序

```
UI 层          ProjectEditor     SourceManager    DataModel     ListenerCenter
  │                │                  │               │               │
  │ loadProject(path)                 │               │               │
  ├──────────────►│                  │               │               │
  │               │ createDmFactory() │               │               │
  │               ├──────────────────────────────────►│               │
  │               │                  │  IDmClipFactory*              │
  │               │◄─────────────────────────────────┤               │
  │               │ factory->createProject()          │               │
  │               ├──────────────────────────────────►│               │
  │               │                  │  IDmProject*   │               │
  │               │◄─────────────────────────────────┤               │
  │               │ project->deserialize(path)        │               │
  │               ├──────────────────────────────────►│               │
  │               │                  │  (解析 project.xml)           │
  │               │                  │  (加载 IDmTimeline/IDmClip)   │
  │               │                  │               │               │
  │               │ initSourceManager│               │               │
  │               ├─────────────────►│               │               │
  │               │                  │ loadMediaFolder│               │
  │               │                  ├──────────────►│               │
  │               │                  │               │               │
  │               │                  │               │ postEvent("project.loaded")
  │               │                  │               ├──────────────►│
  │               │                  │               │               │ notify UI
  │ onEvent("project.loaded")        │               │               ├──────►│
  │◄─────────────────────────────────────────────────────────────────────────┤
  │ 刷新资源区/时间线 UI              │               │               │
```

### 5.3 时间线编辑时序（以添加 Clip 为例）

```
UI 层         TimelineUX       TimelineEditor    DataModel     UndoStack    EventBus
  │               │                 │               │              │            │
  │ addClip(media)│                 │               │              │            │
  ├──────────────►│                 │               │              │            │
  │               │ createClip(media)               │              │            │
  │               ├────────────────────────────────►│              │            │
  │               │                 │  IDmClip*     │              │            │
  │               │◄───────────────────────────────┤              │            │
  │               │ addClipToTimeline(clip, track)  │              │            │
  │               ├────────────────►│               │              │            │
  │               │                 │ beginEdit()   │              │            │
  │               │                 ├──────────────►│              │            │
  │               │                 │ timeline->addClip(track,clip)│            │
  │               │                 ├──────────────►│              │            │
  │               │                 │ endEdit()     │              │            │
  │               │                 ├──────────────►│              │            │
  │               │                 │               │              │            │
  │               │                 │ push(AddClipCmd)             │            │
  │               │                 ├─────────────────────────────►│            │
  │               │                 │               │              │            │
  │               │                 │ postEvent("timeline.clip.added")          │
  │               │                 ├──────────────────────────────────────────►│
  │               │                 │               │              │            │ notify
  │ onEvent() 刷新 UI               │               │              │            ├──────►UI
  │◄──────────────────────────────────────────────────────────────────────────────────┤
```

### 5.4 Undo/Redo 时序

```
UI 层         TimelineUX       UndoTemplateStack   TimelineEditor   DataModel   EventBus
  │               │                   │                  │               │           │
  │ undo()        │                   │                  │               │           │
  ├──────────────►│                   │                  │               │           │
  │               │ undoStack->undo() │                  │               │           │
  │               ├──────────────────►│                  │               │           │
  │               │                   │ cmd->undo()      │               │           │
  │               │                   ├─────────────────►│               │           │
  │               │                   │                  │ 回滚 DataModel │           │
  │               │                   │                  ├──────────────►│           │
  │               │                   │                  │               │           │
  │               │                   │                  │ postEvent("timeline.changed")
  │               │                   │                  ├───────────────────────────►│
  │ 刷新 UI       │                   │                  │               │           │ notify
  │◄──────────────────────────────────────────────────────────────────────────────────┤
```

### 5.5 Adapter 初始化与适配对象创建时序

```
DataModel        Adapter(initAdapter)    IAdapterFactory    具体适配器实例
    │                    │                     │                   │
    │  initAdapter(path) │                     │                   │
    ├───────────────────►│                     │                   │
    │                    │  (底层引擎 Setup)    │                   │
    │                    │  LoadFontFromPath()  │                   │
    │                    │  initDecodeMgr()     │                   │
    │                    │                     │                   │
    │  getAdapterFactory()                     │                   │
    ├─────────────────────────────────────────►│                   │
    │                    │  IAdapterFactory*   │                   │
    │◄─────────────────────────────────────────┤                   │
    │                    │                     │                   │
    │  factory->createPreviewAdapter()         │                   │
    ├─────────────────────────────────────────►│                   │
    │                    │                     ├──────────────────►│
    │                    │                     │  new WesPreviewAdapter()
    │                    │                     │◄──────────────────┤
    │  IVblPreviewAdapter*                     │                   │
    │◄─────────────────────────────────────────┤                   │
    │                    │                     │                   │
    │  (使用完毕) releaseAdapterFactory()       │                   │
    ├─────────────────────────────────────────►│                   │
    │  uninitAdapter()   │                     │                   │
    ├───────────────────►│                     │                   │
```

### 5.6 后台任务（BackgroundTaskManager）时序

```
UI层/业务层    BackgroundTaskMgr    具体Task实例     Adapter适配器    ListenerCenter
    │                │                   │                │                │
    │ addTask(task)  │                   │                │                │
    ├───────────────►│                   │                │                │
    │                │ 按优先级调度       │                │                │
    │                ├──────────────────►│                │                │
    │                │                   │ start()        │                │
    │                │                   ├───────────────►│                │
    │                │                   │  (底层算法执行) │                │
    │                │                   │                │                │
    │                │                   │  onProgress(n%)│                │
    │                │                   │◄───────────────┤                │
    │                │                   │ postEvent("task.progress", n)   │
    │                │                   ├────────────────────────────────►│
    │ 更新进度条      │                   │                │                │ notify UI
    │◄───────────────────────────────────────────────────────────────────────────┤
    │                │                   │                │                │
    │                │                   │  onComplete()  │                │
    │                │                   │◄───────────────┤                │
    │                │                   │ postEvent("task.done", result)  │
    │                │                   ├────────────────────────────────►│
    │ 更新结果显示    │                   │                │                │ notify UI
    │◄───────────────────────────────────────────────────────────────────────────┤
```

### 5.7 素材分析 + 模板匹配时序

```
UI 层      MaterialAnalysis   BackgroundTaskMgr   TemplateMatch    DataModel    EventBus
  │               │                  │                  │               │            │
  │ startAnalysis(medias, config)    │                  │               │            │
  ├──────────────►│                  │                  │               │            │
  │               │ addAnalysisTasks()                  │               │            │
  │               ├─────────────────►│                  │               │            │
  │               │                  │ (依次执行分析算法) │               │            │
  │               │                  │ IBsSceneDetectTask, IBsFaceDetectTask ...     │
  │               │  onTaskComplete(result)             │               │            │
  │               │◄─────────────────┤                  │               │            │
  │               │                  │                  │               │            │
  │ onProgress()  │                  │                  │               │            │
  │◄──────────────┤                  │                  │               │            │
  │               │                  │                  │               │            │
  │ getResult()   │                  │                  │               │            │
  ├──────────────►│                  │                  │               │            │
  │  AnalysisResult*                 │                  │               │            │
  │◄──────────────┤                  │                  │               │            │
  │               │                  │                  │               │            │
  │ matchTemplate(result, template)  │                  │               │            │
  ├────────────────────────────────────────────────────►│               │            │
  │               │                  │                  │ createProject()            │
  │               │                  │                  ├──────────────►│            │
  │               │                  │                  │ addClips(...)  │            │
  │               │                  │                  ├──────────────►│            │
  │               │                  │                  │               │ postEvent  │
  │               │                  │                  │               ├───────────►│
  │ onEvent("project.created") 刷新 UI                  │               │            │
  │◄───────────────────────────────────────────────────────────────────────────────┤
```

### 5.8 日志系统时序

```
任意模块         VblLogger         IBsFileChannel      IBsConsoleChannel
    │               │                    │                    │
    │ LOG_INFO("msg")│                   │                    │
    ├──────────────►│                   │                    │
    │               │ formatLog(msg)     │                    │
    │               │ (添加[level][tid][date][file][line])    │
    │               │                   │                    │
    │               │ (level >= severity?) 过滤              │
    │               │                   │                    │
    │               │ asyncWrite(log)    │                    │
    │               ├──────────────────►│                    │
    │               │                   │ 异步写入文件        │
    │               │                   │ (不阻塞主线程)      │
    │               │                   │                    │
    │               │ directWrite(log)   │                    │
    │               ├────────────────────────────────────────►│
    │               │                   │                    │ 输出到控制台
```

---

## 六、初始化顺序与集成流程

按优先级由高到低的模块完成及集成顺序（来自设计文档第8章）：

```
1. FVBL 框架初始化         initAdapter() → initDataModel()
2. 程序配置模块             CommonSetting（IFbCommonSetting）
3. 工程操作模块             ProjectEditor（IVbProjectEditor）
4. 资源管理操作模块          SourceManager
5. 时间线交互模块            TimelineUX / TimelineEditor
6. 播放器模块               PlayManager
7. 导出模块                 EncodeManager
8. 监听中心模块              ListenerCenter（IMsEventBus）
9. 撤销还原管理模块           UndoTemplateStack
10. 日志系统模块             VblLogger
11. 时间线各属性编辑模块      ClipEditor / EffectEditor / TransitionEditor
12. 可视化编辑模块           VisualEditor
13. 后台任务管理器模块        BackgroundTaskManager
14. 素材分析模块             MaterialAnalysis
15. 模版匹配应用模块         TemplateMatch
```

---

## 七、典型调用时序（业务视角简版）

### 7.1 DataModel 初始化时序

```
UI / BusinessLayer
    │
    ├─► initAdapter(mediaLibraryPath)           // 初始化适配层
    ├─► initDataModel()                         // 初始化数据模型层
    ├─► getDmFactoryInstance()                  // 获取 IDmClipFactory
    │       │
    │       ├─► createTimeline()                // 创建时间线
    │       ├─► createVideoClip(mediaItem)      // 创建剪辑
    │       └─► createProject()                 // 创建工程
    │
    └─► uninitDataModel() / uninitAdapter()     // 退出时释放
```

### 7.2 业务层编辑时序（以添加 Clip 为例）

```
UI 层
    │
    ├─► IVbProjectEditor::getCurEditingProject()    // 获取当前工程
    ├─► IFbTimelineUX::addClipToTimeline(clip)      // 调用 TimelineUX 业务接口
    │       │
    │       ├─► IDmTimeline::beginEdit()             // 开启编辑事务
    │       ├─► IDmTimeline::addClip(track, clip)    // DataModel 写入数据
    │       ├─► IDmTimeline::endEdit()               // 结束事务
    │       ├─► IBsUndoTemplateStack::push(cmd)      // 记录 Undo 栈
    │       └─► IMsEventBus::postMainThreadEvent(    // 通知 UI
    │               "timeline.clip.added", data)
    │
UI 层监听回调
    └─► onEvent("timeline.clip.added") → 刷新时间线 UI 显示
```

### 7.3 后台任务时序（以音频波形提取为例）

```
UI 层
    │
    ├─► IBsBackgroundTaskManager::addTask(waveTask)  // 提交后台任务
    │
BackgroundTaskManager（后台线程）
    ├─► IBsAudioWaveExtractTask::start()
    ├─► VblAudioWaveExtractAdapter（调用适配层提取）
    └─► IMsEventBus::postMainThreadEvent(            // 完成通知
            "wave.extract.done", result)
    │
UI 层监听回调
    └─► onEvent("wave.extract.done") → 刷新波形显示
```

---

## 七、多平台功能矩阵

| 功能/模块 | Windows | macOS | iOS | Android | Web(WASM) |
|---|:---:|:---:|:---:|:---:|:---:|
| 完整功能集 | ✅ | ✅ | 子集 | 子集 | 精简 |
| TemplateMatch | ✅ | ✅ | ❌ | ❌ | ❌ |
| BsDomainProbe | ✅ | ✅ | ❌ | ❌ | ❌ |
| BsExternMediaTransfer | ✅ | ✅ | ❌ | ❌ | ❌ |
| TBB 多线程 | ✅ | ✅ | ❌ | ❌ | ❌ |
| OpenCL GPU | ✅ | ✅ | ❌ | ❌ | ❌ |
| CLR Wrapper | ✅ | ❌ | ❌ | ❌ | ❌ |
| Swift Wrapper | ❌ | ✅ | ✅ | ❌ | ❌ |
| ObjC Wrapper | ❌ | ✅ | ✅ | ❌ | ❌ |
| EventBus 线程数 | 7 | 7 | 2 | 2 | 2 |
| 库输出格式 | DLL/LIB | dylib | Framework | .so | .js/.wasm |

---

## 八、工程文档结构（工程文件格式设计）

```
WSVEFolder/                         ← 工程根目录
│
├── project_info.json               ← 工程属性（时长/分辨率/版本等）
│
├── Medias/                         ← 工程使用的资源目录
│   ├── Config.json                 ← 资源目录总览（名称/ID/md5）
│   ├── {资源UUID}/
│   │   ├── thumb.fsthumb           ← 缩略图
│   │   ├── timeline.xml            ← 资源序列化数据
│   │   └── [原始媒体文件]           ← 仅打包工程包含
│   └── ...
│
└── Project/                        ← 工程信息目录
    ├── project.xml                 ← 时间线序列化数据（IDmProject 维护）
    ├── thumb.fsthumb               ← 工程预览缩略图
    └── tracks_info.json            ← 轨道信息（数量/高度/可见/锁定）
```

---

## 九、系统错误处理流程

```
底层 / 适配层异常
        │
        └─► IMsEventBus::postMainThreadEvent("error.*", errorInfo)
                │
        UI 层监听中心回调
                │
                ├── 初始化失败        → 提示用户，引导重试
                ├── 文件无法解码      → 提示文件格式不支持
                ├── 导出失败          → 提示具体错误原因
                ├── 工程保存失败      → 提示磁盘空间/权限问题
                ├── 工程打开失败      → 提示版本不兼容或文件损坏
                ├── 资源文件丢失      → 引导用户重定位资源
                ├── 在线资源未下载    → 引导用户下载资源
                └── 资源下载/安装失败 → 提示具体错误原因
```

---

*参考文档：`docs/FVBL框架设计文档.docx`（V1.0，2020.4）| 代码版本：v7.17.x*
