# VBL 项目整体设计架构文档

> 文档版本：v7.17.x
> 最后更新：2026-03
> 维护团队：VBL 中间层团队

---

## 目录

1. [项目概述](#一项目概述)
2. [整体架构图](#二整体架构图)
3. [分层架构详解](#三分层架构详解)
4. [核心模块详细说明](#四核心模块详细说明)
   - 4.1 [Adapter（适配层）](#41-adapter适配层)
   - 4.2 [DataModel（数据模型层）](#42-datamodel数据模型层)
   - 4.3 [ListenerCenter（事件总线）](#43-listenercenter事件总线)
   - 4.4 [BaseService（基础服务群）](#44-baseservice基础服务群)
   - 4.5 [BusinessLayer（业务逻辑层）](#45-businesslayer业务逻辑层)
   - 4.6 [公共基础设施（Common）](#46-公共基础设施common)
5. [模块依赖关系](#五模块依赖关系)
6. [头文件分层规范](#六头文件分层规范)
7. [接口设计规范](#七接口设计规范)
8. [对象生命周期管理](#八对象生命周期管理)
9. [Wrapper 层（跨语言对接）](#九wrapper-层跨语言对接)
10. [多平台支持与构建变体](#十多平台支持与构建变体)
11. [CMake 构建体系](#十一cmake-构建体系)
12. [测试体系](#十二测试体系)

---

## 一、项目概述

### 1.1 定位

VBL（Video Business Layer）是万兴科技音视频创意产品矩阵（Filmora、Filmii、喵影等）的**业务中间层**。其核心价值在于：

- **对下**：封装底层音视频引擎（WES / NLE），屏蔽引擎差异，提供统一适配接口
- **对上**：为 UI 层提供稳定、高内聚、低耦合的业务接口，UI 只负责接口调用和界面展示
- **横向**：通过工程配置支持多产品线快速接入，一套中间层服务多个产品

### 1.2 设计目标

| 目标 | 说明 |
|---|---|
| 多产品支持 | 通过工程配置（CMake 选项）快速适配不同产品系列 |
| 快速集成 | UI 层只需简单接口调用，无需理解底层实现细节 |
| 高扩展性 | 模块化设计，新功能以新模块或子模块形式扩展 |
| 高解耦 | 分层清晰，模块间通过接口依赖，不直接引用实现 |
| 可维护性 | 统一构建体系、统一命名规范、统一生命周期管理 |

### 1.3 版本与平台

- **当前版本**：7.17.x（`VBL_VERSION_MAJOR.MINOR.PATCH.TWEAK`）
- **底层实现**：默认 `wes`，可切换为 `nle`（`-DVBL_IMPLEMENT_LEVEL=nle`）
- **支持平台**：

| 平台 | 构建标识 | 说明 |
|---|---|---|
| Windows | `win`（MSVC） | 主要开发平台，支持完整功能集 |
| macOS x86_64 | `mac` | 支持完整功能集 |
| macOS arm64 | `arm64` | Apple Silicon 原生支持 |
| macOS Universal | `universal` | arm64 + x86_64 Fat Binary |
| iOS | `IOS` | 移动端，功能子集 |
| Android | `ANDROID_<ABI>` | 移动端，功能子集（arm64-v8a 等） |
| Web / WASM | — | Emscripten 编译，功能精简版 |

---

## 二、整体架构图

```
┌────────────────────────────────────────────────────────────────────────────────────┐
│                         UI 层（Filmora / Filmii / 喵影 / VCU …）                   │
│                     （不在本仓库，通过 VBL 公共接口与中间层交互）                      │
└────────────────────────────────┬───────────────────────────────────────────────────┘
                                 │  调用 initDataModel() / getProjectEditor() 等
          ┌──────────────────────┼─────────────────────────────────┐
          │          跨语言       │  Wrapper 层                      │
          │  ┌───────────┐ ┌─────┴──────┐ ┌──────────┐ ┌────────┐ │
          │  │  vbl-c    │ │vbl-clrWrap │ │vbl-swift │ │vbl-web │ │
          │  │(ObjC/iOS) │ │(.NET/Win)  │ │ (Swift)  │ │(WASM)  │ │
          │  └─────┬─────┘ └─────┬──────┘ └────┬─────┘ └───┬────┘ │
          └────────┼─────────────┼──────────────┼───────────┼──────┘
                   └─────────────┴──────────────┴───────────┘
                                          │
┌─────────────────────────────────────────▼──────────────────────────────────────────┐
│                               VBL 业务中间层（本仓库）                               │
│                                                                                    │
│  ┌──────────────────────────────────────────────────────────────────────────────┐  │
│  │                          BusinessLayer（业务逻辑层）                           │  │
│  │  ProjectEditor │ TimelineEditor │ AIManager │ AICopilot │ PreferenceManager …  │  │
│  └────────────────────────────────────┬─────────────────────────────────────────┘  │
│                                       │                                             │
│  ┌────────────────────────────────────▼─────────────────────────────────────────┐  │
│  │                            DataModel（数据模型层）                             │  │
│  │       IDmProject │ IDmTimeline │ IDmTrack │ IDmClip │ IDmMediaItem …           │  │
│  └────────┬──────────────────────────────────────────┬───────────────────────────┘  │
│           │   PUBLIC                                  │  PRIVATE                     │
│  ┌────────▼──────────────────────────────────────────▼───────────────────────────┐  │
│  │                          BaseService（基础服务群）                              │  │
│  │  BsNet │ BsCloudResource │ BsCloudDisk │ BsPreset │ BackgroundTaskManager …    │  │
│  │  BsAI  │ BSWsid │ BsGpuCheck │ BsPluginManager │ VblLogger │ VblUtils …        │  │
│  └──────────────────────────────────────┬──────────────────────────────────────┘  │
│                                          │                                          │
│  ┌───────────────────────────────────────▼──────────────────────────────────────┐  │
│  │                            Adapter（适配层）                                   │  │
│  │            wes/ ← VBL_IMPLEMENT_LEVEL → nle/                                   │  │
│  └───────────────────────────────────────┬──────────────────────────────────────┘  │
│                                           │                                         │
│  ──────────────── 横跨所有层 ─────────────│─────────────────────────────────────  │
│  ListenerCenter（IMsEventBus 事件总线）    │   Common（RefCnt / SafePtr / TypeDef）  │
└───────────────────────────────────────────┼─────────────────────────────────────────┘
                                            │
┌───────────────────────────────────────────▼─────────────────────────────────────────┐
│                          底层引擎（WES SDK / NLE SDK / 系统资源）                     │
│                           3rdparty/NLE/  ·  3rdparty/WES-Mobile/                    │
└─────────────────────────────────────────────────────────────────────────────────────┘
```

> **说明**：ListenerCenter 和 Common 是横切关注点，被所有业务层引用；Wrapper 层在 VBL 之外横向包装，面向不同宿主语言/平台。

---

## 三、分层架构详解

### 3.1 底层引擎层

不在本仓库，通过头文件（`3rdparty/NLE/`）和二进制 SDK（WES-Mobile 等）以库的形式引入。VBL 通过 Adapter 层与之交互，完全屏蔽了引擎接口变化对上层的影响。

### 3.2 Adapter 层（适配层）

- **职责**：将底层引擎（WES/NLE）的能力翻译成 VBL 统一接口，是 VBL 与引擎的唯一结合点
- **设计原则**：单向依赖，只有 DataModel/BusinessLayer 依赖 Adapter，Adapter 不依赖上层
- **可插拔**：编译时通过 `VBL_IMPLEMENT_LEVEL` 选择引擎实现，同一套接口无缝切换 WES 或 NLE

### 3.3 BaseService 层（基础服务群）

- **职责**：提供可复用的基础能力（网络、云资源、本地文件、账户授权、插件管理、后台任务等）
- **设计原则**：每个子服务独立编译（可通过 `VBL_<subdir>` 选项单独关闭），互相之间低耦合
- **共享基础**：所有模块共享 `VblLogger`、`VblUtils`、`BsDataStructure`、`BsDiskFolder`（在 `modules/CMakeLists.txt` 中配置为 `group_private_link_libs`）

### 3.4 DataModel 层（数据模型层）

- **职责**：定义 VBL 全局的核心数据结构（项目、时间线、轨道、剪辑、媒体资源等），是整个中间层的数据中心
- **设计原则**：数据与逻辑分离，DataModel 只负责数据的存储和访问，业务逻辑交给 BusinessLayer
- **线程安全**：通过 Intel TBB 实现并发资源管理（桌面端）

### 3.5 ListenerCenter 层（事件总线）

- **职责**：提供跨模块、跨线程的事件发布/订阅机制，是模块间解耦通信的核心基础设施
- **设计原则**：任何模块都不直接调用另一个模块的实现，而是通过发布事件来驱动行为
- **线程模型**：支持主线程、异步线程、当前线程、后台线程投递

### 3.6 BusinessLayer 层（业务逻辑层）

- **职责**：封装 UI 层的复合业务操作（工程打开/保存、时间线编辑、AI 功能、偏好配置等），是 UI 直接调用的高阶 API
- **设计原则**：聚合 DataModel 与 BaseService 能力，以工程（Project）为中心组织业务流程

### 3.7 Wrapper 层（跨语言适配）

- **职责**：将 C++ 核心接口包装为其他语言可调用的形式（Objective-C/C, .NET CLR, Swift, WASM）
- **设计原则**：Wrapper 只做语言绑定，不包含业务逻辑，业务逻辑统一在 C++ 核心层实现

### 3.8 UI 层（消费方）

不在本仓库，通过 VBL 导出的 C++ 接口（或 Wrapper）调用中间层能力。UI 层的职责仅限于：调用接口、处理回调、渲染界面。

---

## 四、核心模块详细说明

### 4.1 Adapter（适配层）

**源码位置**：`modules/Adapter/`
**接口目录**：`Interface/Adapter/`
**命名空间**：`VAL::`（区别于其他模块的 `VBL::`）

#### 职责与设计

Adapter 是 VBL 与底层引擎的唯一结合点，负责：

- 封装底层媒体解码/编码能力（`VblProductionAdapter`、`VblMediaInfoAdapter`）
- 封装时间线渲染能力（`VblTimelineRenderAdapter`、`VblPreviewAdapter`）
- 封装音频处理能力（`VblAudioWaveAdapter`、`VblAudioWaveExtractAdapter`）
- 封装图像处理（`VblImageDecoderAdapter`、`VblRgbCurveAdapter`）
- 封装字体库（`VblFontLibraryAdapter`）
- 封装视频分析（`ITimelineVideoAnalyser`）
- 封装插件（`IVbVstEffectAdapter`）

#### 实现切换机制

```
modules/Adapter/
├── CMakeLists.txt         # 根据 VBL_IMPLEMENT_LEVEL 加载子目录
├── wes/                   # WES 引擎实现（默认）
│   ├── Timeline/          # 时间线相关适配器实现
│   ├── Preview/           # 预览相关
│   ├── Capture/           # 采集相关
│   ├── AudioAlgorithm/    # 音频算法
│   └── Environment/       # 环境/系统相关
└── nle/                   # NLE 引擎实现（可选）
```

#### 关键 C 导出接口（`Interface/Adapter/VblAdapterLib.h`）

| 函数 | 说明 |
|---|---|
| `initAdapter(mediaLibraryPath)` | 初始化 Adapter 层 |
| `uninitAdapter()` | 释放 Adapter 层 |
| `getAdapterFactory(szPath)` | 获取适配器工厂，用于创建各类 Adapter 实例 |
| `getDecoderLibInterface()` | 获取解码库接口 `IMmpFunctionAdapter*` |
| `initDecodeMgr()` | 初始化解码管理器 |
| `_setPerformanceMode(mode)` | 设置渲染性能模式（0: Turbo, 1: 内存安全） |
| `_setRenderPlatform(platform)` | 设置渲染平台（GPU/CPU） |
| `setAlgorithmCachePath(path)` | 设置算法缓存路径 |
| `videoBlurSearch(...)` | 视频模糊搜索（语义检索） |

#### 适配器接口列表

| 接口文件 | 功能 |
|---|---|
| `VblAdapterLib.h` | 全局初始化与工厂函数（C 导出） |
| `VblAdapterFactory.h` | 适配器工厂接口 `IAdapterFactory` |
| `VblPreviewAdapter.h` | 预览/播放适配 |
| `VblTimelineRenderAdapter.h` | 时间线渲染适配 |
| `VblProductionAdapter.h` | 导出/生产适配 |
| `VblAudioWaveAdapter.h` | 音频波形展示适配 |
| `VblAudioWaveExtractAdapter.h` | 音频波形提取适配 |
| `VblMediaInfoAdapter.h` | 媒体信息获取适配 |
| `VblMediaAnalysisAdapter.h` | 媒体内容分析适配 |
| `VblImageDecoderAdapter.h` | 图像解码适配 |
| `VblThumbAdapter.h` | 缩略图生成适配 |
| `VblEncodeParamAdapter.h` | 编码参数适配 |
| `VblFontLibraryAdapter.h` | 字体库适配 |
| `VblAnimationAdapter.h` | 动画（关键帧）适配 |
| `VblVariableSpeedAdapter.h` | 变速适配 |
| `VblCaptureAdapter.h` | 采集（屏幕/摄像头）适配 |
| `ITimelineVideoAnalyser.h` | 时间线视频智能分析（人脸/场景/运动等） |
| `VblGpuCheckAdapter.h` | GPU 能力检测适配 |
| `VblDomainProbe.h` | 域名探测（网络质量检测） |
| `VblVisualEditingAdapter.h` | 可视化编辑适配 |
| `VblBlockCallbackAdapter.h` | 块级回调适配 |

---

### 4.2 DataModel（数据模型层）

**源码位置**：`modules/DataModel/`
**接口目录**：`Interface/DataModel/`
**命名空间**：`VBL::`

#### 职责与设计

DataModel 是 VBL 的数据中心，定义了视频编辑的全量数据结构：

```
IDmProject（项目）
├── IDmTimeline（时间线）
│   ├── IDmTrack（轨道）
│   │   └── IDmClip（剪辑）[多种类型]
│   │       ├── IDmEffect（特效列表）
│   │       ├── IDmTransition（转场）
│   │       ├── IDmVariableSpeed（变速）
│   │       └── IDmIntelligentSegment（AI 分段）
│   ├── Markers（标记点管理）
│   └── SentenceManager（字幕/对话管理）
└── ResourceManager（资源管理）
    ├── CloudDiskMgr（云盘资源）
    ├── IDmMediaItem（媒体项）
    └── IDmSmartShortInfo（智能剪辑信息）
```

#### 初始化入口（`Interface/DataModel/VblDataModelLib.h`）

| 函数 | 说明 |
|---|---|
| `initDataModel()` | 初始化数据模型（需在 initAdapter 之后调用） |
| `uninitDataModel()` | 释放数据模型 |
| `getDmFactoryInstance()` | 获取 `IDmClipFactory*`，用于创建 Clip/Timeline/Project 等对象 |
| `initDecodeManager()` | 初始化解码管理器 |
| `initOpenCL()` | 初始化 OpenCL GPU 加速 |
| `addEffectResPath(resPath)` | 添加特效资源路径 |
| `setPerformanceMode(mode)` | 设置渲染性能模式 |
| `setRenderPlatform(platform)` | 设置渲染平台 |

#### 核心数据结构接口

**IDmTimeline**（`Interface/DataModel/IDmTimeline.h`）

时间线是视频编辑的核心数据结构，承载全部轨道与剪辑数据：

| 方法 | 说明 |
|---|---|
| `beginEdit()` / `endEdit()` | 编辑事务边界（批量修改） |
| `trackCount()` | 获取轨道数量 |
| `addTrack(type, idx)` | 添加轨道 |
| `addClip(track_idx, clip)` | 向轨道添加剪辑 |
| `clip(track_idx, idx)` | 获取指定剪辑 |
| `getFrame(time, w, h, buffer)` | 按时间提取帧数据 |
| `clone()` | 克隆时间线 |
| `addTimelineCallback(cb)` | 注册时间线变化回调 |
| `setTimelineProperty(key, value)` | 设置时间线属性（KV 模型） |
| `clearAllData()` | 清空全部数据 |

**IDmClip**（`Interface/DataModel/IDmClip.h`）

剪辑基类，所有类型剪辑的公共接口：

| 属性/方法 | 说明 |
|---|---|
| `type()` | 剪辑类型（视频/音频/字幕/贴纸/特效等） |
| `duration()` | 持续时长 |
| `alias()` / `setAlias()` | 显示名称 |
| `clipKey::timelineBegin/End` | 在时间线上的起止时间 |
| `clipKey::markIn/Out` | 媒体源的入点/出点 |
| `clipKey::enable` / `locked` | 启用/锁定状态 |
| Effect 子对象 | `IDmEffect`、`IDmTransition` |
| AI 子对象 | `IDmIntelligentSegment`、`IDmObjectRemove`、`IDmSmartMosaic` |

**IDmMediaItem**（`Interface/DataModel/IDmMediaItem.h`）

媒体素材项基类：

| 属性/方法 | 说明 |
|---|---|
| `type()` | 媒体类型（MediaType 枚举） |
| `mediaId()` / `setMediaId()` | 唯一标识 |
| `alias()` / `setAlias()` | 显示名称 |
| `isSelected()` / `setSelected()` | 选中状态 |
| `addCallback(cb)` | 注册媒体项变化回调 |

#### CMake 依赖（`modules/DataModel/CMakeLists.txt`）

```cmake
target_link_libraries(DataModel
    PUBLIC
        VBL::ListenerCenter      # 事件总线（PUBLIC 传递给上层）
        VBL::BsCommonSetting     # 公共配置
        VBL::BsCloudResource     # 云资源
        VBL::BsCloudConfig       # 云配置
    PRIVATE
        VBL::Adapter             # 底层适配（隐藏实现细节）
        VBL::BsNet               # 网络服务
        VBL::BsGpuCheck          # GPU 检测
        VBL::BsPreset            # 预设管理
        VBL::BsPluginManager     # 插件管理
        VBL::BsCloudDisk         # 云盘服务
        TBB_LIBS                 # Intel TBB（非 Web/Mobile）
)
```

---

### 4.3 ListenerCenter（事件总线）

**源码位置**：`modules/ListenerCenter/`
**接口目录**：`Interface/ListenerCenter/`
**命名空间**：`VBL::`

#### 职责与设计

ListenerCenter 实现了一个**多线程事件总线**，是模块间解耦通信的核心基础设施：

- 任何模块通过 `subscribe` 订阅感兴趣的事件
- 任何模块通过 `postEvent` 发布事件
- 事件名称以字符串标识（类型安全由调用方保证）
- 支持多种线程投递策略，避免跨线程调用复杂性

#### 核心接口（`Interface/ListenerCenter/IMsEventBus.h`）

```cpp
class IMsEventBus : public IDmBaseObj {
public:
    enum ThreadMode {
        MAINTHREAD = 0,   // 主线程投递
        ASYNC,            // 新开异步线程
        CURRENT,          // 当前线程同步投递
        BACKGROUND,       // 固定后台线程
        USER,             // 用户自定义线程
        MAX_THREAD = 7    // 桌面端最大线程数（Web/Mobile 为 2）
    };

    // 事件注册管理
    virtual Result registerEvent(VBLConstPChar evName, VBLBool active = true) = 0;
    virtual Result unregisterEvent(VBLConstPChar evName) = 0;
    virtual Result setEventActive(VBLConstPChar* evNameList, VBLUInt size, VBLBool active) = 0;

    // 订阅管理
    virtual VBLInt subscribe(VBLConstPChar evName, Callback cb, void* target, VBLBool active) = 0;
    virtual Result unsubscribe(VBLInt subId) = 0;
    virtual Result setSubscriberActive(VBLInt subId, VBLBool active) = 0;

    // 事件投递
    virtual Result postEvent(ThreadMode tm, VBLConstPChar evName, const EventValue& data) = 0;
    virtual Result postMainThreadEvent(VBLConstPChar evName, const EventValue& data) = 0;
    virtual Result postCurrentEvent(VBLConstPChar evName, const EventValue& data) = 0;
    virtual Result postBackgroundEvent(VBLConstPChar evName, const EventValue& data) = 0;
    virtual Result postAsyncEvent(VBLConstPChar evName, const EventValue& data) = 0;

    // 粘性事件（Sticky Event）
    virtual Result postStickyEvent(ThreadMode tm, VBLConstPChar evName, EventValue data) = 0;
    virtual Result fetchStickEvent(VBLConstPChar evName, ...) = 0;
};
```

#### 粘性事件（Sticky Event）

粘性事件会在发布后保留，新订阅者订阅时能立即收到最近一次事件值，适合"状态广播"场景（如网络状态、GPU 状态等）。

#### 平台差异

| 平台 | MAX_THREAD | 备注 |
|---|---|---|
| Windows / macOS | 7 | 完整线程池 |
| iOS / Android | 2 | 精简版（MOBILE 宏控制） |
| Web / WASM | 2 | Emscripten 线程限制 |

---

### 4.4 BaseService（基础服务群）

**源码位置**：`modules/BaseService/`
**接口目录**：`Interface/BaseService/`
**命名空间**：`VBL::`（各子模块独立）

BaseService 是一组独立的服务子模块，每个子模块都是独立的 CMake 目标（`VBL::<SubModule>`），可按需启用/禁用。

#### 子模块详细说明

| 子模块 | CMake 目标 | 职责 |
|---|---|---|
| **BsNet** | `VBL::BsNet` | HTTP 客户端封装，支持代理、断点续传、重试（`Internal/BaseService/BsNet/`） |
| **BsCloudResource** | `VBL::BsCloudResource` | 云端资源（特效/模板/预设包）的请求、下载、本地缓存管理 |
| **BsCloudDisk** | `VBL::BsCloudDisk` | 第三方云盘服务对接：Dropbox / Google Drive / OneDrive |
| **BsCloudConfig** | `VBL::BsCloudConfig` | 云端配置拉取（基于 lantern 实现的远程配置） |
| **BsPreset** | `VBL::BsPreset` | 本地预设文件管理（特效参数预设、项目模板等） |
| **BsPluginManager** | `VBL::BsPluginManager` | 插件管理，包括 OpenFX 插件的加载、查询、参数管理 |
| **BsGpuCheck** | `VBL::BsGpuCheck` | GPU 能力探测，判断硬件加速可用性 |
| **BackgroundTaskManager** | `VBL::BackgroundTaskManager` | 后台长时任务调度框架，内置任务类型：音频波形提取、人脸检测、场景检测、视频帧提取等 |
| **BsAI** | `VBL::BsAI` | AI 算法服务的统一接口封装 |
| **BSWsid** | `VBL::BSWsid` | 万兴账户（WSID）授权与订阅查询：`IBsWsidAuthorize`、`IBsWSIDBusinessInfo`、`IBsWSIDChatgptInfo` |
| **BsCommonSetting** | `VBL::BsCommonSetting` | 公共配置（路径、功能开关等），被 DataModel 以 PUBLIC 依赖透出 |
| **BsBeatManager** | `VBL::BsBeatManager` | 音频节拍检测与管理 |
| **VblLogger** | `VBL::VblLogger` | 统一日志系统，全模块共享依赖 |
| **VblUtils** | `VBL::VblUtils` | 工具集：线程池、文件流（`FileStream`）、AES 加密、字符串操作、HTTP 工具等 |
| **BsDataStructure** | `VBL::BsDataStructure` | 公共数据结构：`IVblList`、`IVblString` 等（全模块共享依赖） |
| **BsDiskFolder** | `VBL::BsDiskFolder` | 磁盘目录工具（全模块共享依赖） |

#### 共享基础依赖（所有模块隐式获得）

在 `modules/CMakeLists.txt` 中通过 `group_private_link_libs` 统一配置：

```cmake
set(group_private_link_libs
    VBL::BsDataStructure    # 公共数据结构
    VBL::VblLogger          # 日志系统
    VBL::VblUtils           # 工具集
    VBL::BsDiskFolder       # 磁盘工具
)
```

所有通过 `AppendModulesBasic` 创建的模块都自动获得这四个私有依赖。

---

### 4.5 BusinessLayer（业务逻辑层）

**源码位置**：`modules/BusinessLayer/`
**接口目录**：`Interface/BusinessLayer/`
**命名空间**：`VBL::`

BusinessLayer 是 UI 层最直接使用的高阶 API，聚合 DataModel 与 BaseService 能力，以工程（Project）为中心组织业务流程。

#### 子模块详细说明

| 子模块 | 核心接口 | 职责 |
|---|---|---|
| **ProjectEditor** | `IVbProjectEditor` | 工程生命周期管理：新建/加载/保存/自动保存/撤销/恢复，是 UI 层最核心的入口 |
| **TimelineEditor** | `IVbTimelineEditor` | 时间线编辑操作：添加/删除/移动剪辑，分割/合并，批量操作等 |
| **AIManager** | `IVbAIManager` | AI 任务统一调度中心：智能剪辑、AI 字幕、背景分离、AI 降噪等 |
| **AICopilot** | `IVbAICopilot` / `IVbActionEditor` | AI Copilot 功能：脚本/提词器支持、智能动作编辑 |
| **ProjectConversionService** | — | 工程文件格式转换：Filmora ↔ Filmii ↔ 喵影（MiaoYing）工程互转 |
| **PreferenceManager** | — | 用户偏好配置持久化（快捷键、默认参数、工作区设置等） |
| **MaterialAnalysis** | — | 素材分析（分辨率、帧率、编码格式等统计） |
| **TemplateMatch** | `IVbTemplateMatch` | 模板匹配算法（桌面端专属，移动端不编译） |

#### ProjectEditor 关键接口（`Interface/BusinessLayer/ProjectEditor/IVbProjectEditor.h`）

| 方法 | 说明 |
|---|---|
| `init()` / `unInit()` | 初始化/释放 ProjectEditor |
| `createNewProject()` | 创建新工程 |
| `loadProject(path)` | 从文件加载工程 |
| `saveProject(path)` | 保存工程到文件 |
| `getCurEditingProject()` | 获取当前编辑中的 IDmProject |
| `setUndoManager(...)` | 配置撤销管理器 |
| `setSourceManager(...)` | 配置素材管理器 |
| `setAutoSaveEnabled(bool)` | 启用/禁用自动保存 |
| `getTimelineUX()` | 获取 TimelineUX 实例（时间线交互层） |
| `mergeClips(...)` | 合并多个剪辑 |

---

### 4.6 公共基础设施（Common）

**头文件位置**：`Include/Common/`
**命名空间**：`VBL::`

所有模块共用的基础设施，通过 Include 目录直接引用，不作为独立 CMake 目标。

#### VblRefCnt（引用计数基类）

```cpp
// Include/Common/VblRefCnt.h
template <typename T>
class RefCnt {
    virtual int AddRef();
    virtual int Release();   // 引用归零时调用 IDmProtocol::Destroy() 并 delete this
    virtual int RefValue();
    // 绑定销毁协议
    void bindProtocol(IDmProtocol* p);
};
```

#### VblControlBlock（线程安全控制块）

为 `RefCnt` 提供原子操作支持，保证多线程下引用计数的正确性。

#### VblSafePtr / VblWeakPtr

智能指针封装，类似 `shared_ptr` / `weak_ptr`，但适配 VBL 的 `RefCnt` 体系。

#### VblTypeDef（统一类型定义）

| 类型 | 说明 |
|---|---|
| `VBLInt` / `VBLUInt` | 带符号/无符号整型 |
| `VBLLonglong` | 64 位整型（时间戳单位：微秒） |
| `VBLConstPChar` | `const char*` |
| `VBLBool` | 布尔类型 |
| `VBLByte` | 字节类型 |
| `Result` | 操作结果码 |
| `Property` | 属性值（联合体，支持多种基本类型） |

#### IVblTimer（定时器接口）

```cpp
class IVblTimer {
    virtual void setTimeOutCallBack(func, pUserData) = 0;
    virtual void setInterval(msec) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void Release() = 0;
};
```

---

## 五、模块依赖关系

### 5.1 完整依赖图

```
                    ┌─────────────────────┐
                    │   BusinessLayer      │
                    │ (ProjectEditor 等)   │
                    └──────────┬──────────┘
                               │ 依赖
              ┌────────────────▼────────────────┐
              │            DataModel             │
              └────┬──────────────────┬──────────┘
         PUBLIC    │                  │ PRIVATE
    ┌──────────────▼──────┐   ┌──────▼──────────────────────┐
    │  ListenerCenter     │   │  BaseService 子模块群        │
    │  BsCommonSetting    │   │  Adapter                     │
    │  BsCloudResource    │   │  BsNet / BsGpuCheck          │
    │  BsCloudConfig      │   │  BsPreset / BsPluginManager  │
    └─────────────────────┘   │  BsCloudDisk / TBB           │
                               └──────────────────────────────┘

    所有模块隐式私有依赖（group_private_link_libs）：
    VBL::BsDataStructure  VBL::VblLogger  VBL::VblUtils  VBL::BsDiskFolder
```

### 5.2 CMake 依赖速查表

| 依赖方 | 类型 | 被依赖方 |
|---|---|---|
| DataModel | PUBLIC | ListenerCenter、BsCommonSetting、BsCloudResource、BsCloudConfig |
| DataModel | PRIVATE | Adapter、BsNet、BsGpuCheck、BsPreset、BsPluginManager、BsCloudDisk |
| ListenerCenter | PRIVATE | TBB（非 Web/Mobile） |
| vbl-c wrapper | LINK | DataModel、VblLogger、PlayManager、ProjectEditor、BsUndoManager |
| 所有模块 | PRIVATE（隐式） | BsDataStructure、VblLogger、VblUtils、BsDiskFolder |

### 5.3 移动端排除模块

当 `VBL_MOBILE=ON` 或 `VBL_WEB=ON` 时，以下模块不参与编译：

```cmake
set(IGNORE_LIBS
    BsDomainProbe           # 域名探测（移动端无需）
    BsExternMediaTransfer   # 外部媒体传输
    TemplateMatch           # 模板匹配（计算资源密集）
)
```

---

## 六、头文件分层规范

VBL 严格按照三层头文件规范组织代码，`AppendModulesBasic` CMake 函数自动将三个目录的头文件加入编译：

```
Include/<ModuleGroup>/
│   ├── *_Exports.h        # 动态库导出宏定义（如 VBLLIB_API）
│   ├── 工具类头文件        # 可供其他模块使用的公共类
│   └── VblXxx_Exports.h   # 模块导出宏

Interface/<ModuleGroup>/
│   ├── I*.h               # 纯虚接口类（DataModel: IDm*, BusinessLayer: IVb*, BaseService: IBs*）
│   └── Vbl*Lib.h          # C 风格工厂/初始化导出函数

Internal/<ModuleGroup>/
│   ├── *Internal.h        # 内部扩展接口（install 时不包含）
│   └── *Parse.h / *Impl.h # 内部实现辅助头

Include/Common/
    ├── VblTypeDef.h        # 统一类型定义
    ├── VblPredefined.h     # 全局宏（导出宏模板、VBL_DISABLE_COPY 等）
    ├── VblRefCnt.h         # 引用计数基类
    ├── VblSafePtr.h        # 智能指针
    ├── VblControlBlock.h   # 线程安全控制块
    ├── IDmProtocol.h       # 对象销毁协议接口
    ├── IDmBaseObj.h        # 所有 VBL 接口对象的公共基类
    ├── IVblTimer.h         # 定时器接口
    └── VblFrameRateUtility.h # 帧率工具函数
```

| 目录 | 安装时导出 | 使用场景 |
|---|---|---|
| `Include/<Module>/` | 是（部分） | 模块间调用的工具类、导出宏 |
| `Interface/<Module>/` | 是（全部） | 模块对外契约，UI/上层调用的抽象接口 |
| `Internal/<Module>/` | **否** | 仅模块内部使用的实现细节 |
| `Include/Common/` | 是 | 全局基础设施，所有代码均可引用 |

---

## 七、接口设计规范

### 7.1 整体风格

VBL 采用 **COM 风格**的纯虚接口设计：

```cpp
// 1. 定义纯虚接口类（继承自 IDmBaseObj）
class IDmTimeline : virtual public IDmBaseMedia {
public:
    virtual Result addTrack(VBLUInt type, VBLInt idx) = 0;
    // ...
};

// 2. 提供 C 风格工厂函数（extern "C" 导出）
namespace VBL {
    extern "C" {
        VBLLIB_API IDmClipFactory* getDmFactoryInstance();
    }
}
```

这种设计的优势：
- 接口与实现完全分离（ABI 稳定性）
- C 风格工厂函数跨编译器/语言兼容
- 对象生命周期由引用计数统一管理

### 7.2 命名规范

| 范围 | 规则 | 示例 |
|---|---|---|
| 命名空间 | `VBL::` 统一（Adapter 例外使用 `VAL::`） | `VBL::IDmTimeline` |
| DataModel 接口 | `IDm` 前缀 | `IDmClip`、`IDmTimeline`、`IDmMediaItem` |
| BusinessLayer 接口 | `IVb` 前缀 | `IVbProjectEditor`、`IVbAIManager` |
| BaseService 接口 | `IBs` 前缀 | `IBsBackgroundTaskManager`、`IBsCloudDisk` |
| Adapter 接口 | `VAL::I` 前缀 | `VAL::IAdapterFactory`、`VAL::IClipAdapter` |
| 导出宏 | `<MODULE>LIB_API` | `VBLLIB_API`、`VALLIB_API` |
| CMake 变量 | `VBL_` 前缀 | `VBL_ENABLE_TESTS`、`VBL_MOBILE` |
| CMake 目标 | `VBL::<Name>` 别名 | `VBL::DataModel`、`VBL::ListenerCenter` |

### 7.3 属性访问模式

DataModel 广泛使用 **KV 属性模式**代替独立 getter/setter：

```cpp
// 通过字符串 key 访问属性（clipKey 命名空间定义了常量）
clip->setClipProperty(clipKey::enable, true);
clip->setClipProperty(clipKey::timelineBegin, 1000000LL); // 1s，单位微秒

// 时间单位统一为 VBLLonglong（微秒）
```

### 7.4 回调机制

VBL 使用两种回调机制：

1. **接口回调**：实现 `IXxxCallback` 接口（纯虚类），通过 `addCallback()` 注册
2. **函数指针回调**：`typedef void(*Callback)(void* target, VBLConstPChar, EventValue)`，用于 EventBus

### 7.5 Windows 特有约定

```cpp
// 防止 windows.h 的 min/max 宏污染
target_compile_definitions(${name} PUBLIC NOMINMAX)

// 多处理器并行编译
target_compile_options(${name} PRIVATE "/MP")

// DLL 导出宏模板（各模块各自实现）
#ifdef <MODULE>_EXPORTS
#define <MODULE>LIB_API __declspec(dllexport)
#else
#define <MODULE>LIB_API __declspec(dllimport)
#endif
```

---

## 八、对象生命周期管理

### 8.1 引用计数体系

所有 VBL 接口对象均继承自 `IDmBaseObj`，后者继承 `RefCnt<T>`：

```
IDmBaseObj
├── AddRef()      → 引用计数 +1
├── Release()     → 引用计数 -1，归零时自动销毁
└── RefValue()    → 查询当前引用计数
```

### 8.2 销毁流程

```
Release() 调用
    ↓
VblControlBlock::Release()（原子操作）
    ↓ 引用计数 == 0
IDmProtocol::Destroy() 回调（如已绑定）
    ↓
delete this（对象内存释放）
```

### 8.3 智能指针使用建议

```cpp
// 推荐使用 VblSafePtr 管理对象生命周期
VblSafePtr<IDmTimeline> timeline = factory->createTimeline();
// 超出作用域自动 Release()

// 弱引用（不影响生命周期）
VblWeakPtr<IDmClip> weakClip = clip;
if (auto clip = weakClip.lock()) {
    // 安全使用
}
```

### 8.4 工厂创建 vs. 直接构造

VBL 对象统一通过工厂函数创建，**禁止直接 `new`**：

```cpp
// ✅ 正确：通过工厂创建
IDmClipFactory* factory = getDmFactoryInstance();
IDmClip* clip = factory->createVideoClip(mediaItem);

// ❌ 禁止：直接构造
IDmClip* clip = new DmVideoClip(...); // 不要这样做
```

---

## 九、Wrapper 层（跨语言对接）

VBL 核心层为纯 C++，通过 `wrapper/` 目录下的各语言绑定支持多平台 UI 集成。

### 9.1 vbl-c（Objective-C / C 绑定）

**目录**：`wrapper/vbl-c/`
**用途**：iOS / macOS 平台 UI 调用
**CMake 链接**：

```cmake
TARGET_LINK_LIBRARIES(vbl-c
    VBL::DataModel
    VBL::VblLogger
    VBL::PlayManager
    VBL::ProjectEditor
    VBL::BsUndoManager
)
```

典型封装（`.h` + `.mm`）：

```
VBLBaseObj.h/.mm         → IDmBaseObj 的 ObjC 封装
VBLProjectObj.h/.mm      → IDmProject 的 ObjC 封装
VBLMediaInfoObj.h/.mm    → 媒体信息 ObjC 封装
VBLBackgroundTaskManager.h/.mm → 后台任务 ObjC 封装
```

### 9.2 vbl-clrWrapper（.NET CLR 绑定）

**目录**：`wrapper/vbl-clrWrapper/`
**用途**：Windows 平台 C# 产品（VCU 等）
**技术方案**：C++/CLI（`/clr` 编译），将 C++ 虚函数接口翻译为 .NET 托管类
**核心文件**：

```
DmTimelineClrWrapper.h    → IDmTimeline 的 CLR 封装
DmClipClrWrapper.h        → IDmClip 的 CLR 封装
BsPlayerClrWrapper.h      → 播放器 CLR 封装
BsEncoderClrWrapper.h     → 编码器 CLR 封装
vblClrWrapper.cpp         → 统一入口
```

C# 调用示例（`wrapper/C#Demo/`）：
- 通过 `[DllImport]` 或 CLR 托管类调用 VBL 接口
- 见 `docs/FVBL 2.0技术文档/FVBL2.0非C++语言接口对接方案.md`

### 9.3 vbl-swift（Swift 绑定）

**目录**：`wrapper/vbl-swift/`
**用途**：macOS / iOS Swift UI 集成
**技术方案**：通过 `vbl-c` ObjC 层间接调用（Swift → ObjC → C++）

### 9.4 Web SDK（WASM）

**目录**：`wrapper/vbl_web_sdk/`
**用途**：Web 平台音视频编辑
**构建**：独立的 CMake 项目，使用 Emscripten 编译
**构建脚本**：

| 脚本 | 平台 |
|---|---|
| `centos-build.sh` | Linux CI 环境 |
| `win-build.bat` | Windows 本地调试 |

VBL 主仓库的 Web 构建通过 `-DVBL_WEB=ON` 启用，启用后会：
1. 设置 `VBL_Cropping=ON`（裁剪模式）
2. 添加 `MOBILE`/`Cropping` 宏
3. 为所有模块目标添加 Emscripten 特有链接选项（`-sWASM=1`、`-sPTHREAD_POOL_SIZE=32` 等）

---

## 十、多平台支持与构建变体

### 10.1 平台差异矩阵

| 特性 | Windows | macOS | iOS | Android | Web/WASM |
|---|---|---|---|---|---|
| 完整功能集 | ✅ | ✅ | 子集 | 子集 | 精简版 |
| TBB 多线程 | ✅ | ✅ | ❌ | ❌ | ❌ |
| TemplateMatch | ✅ | ✅ | ❌ | ❌ | ❌ |
| BsDomainProbe | ✅ | ✅ | ❌ | ❌ | ❌ |
| OpenCL GPU | ✅ | ✅ | ❌ | ❌ | ❌ |
| ListenerCenter MAX_THREAD | 7 | 7 | 2 | 2 | 2 |
| CLR Wrapper | ✅ | ❌ | ❌ | ❌ | ❌ |
| Swift Wrapper | ❌ | ✅ | ✅ | ❌ | ❌ |
| 库类型 | DLL/LIB | dylib | Framework | .so | .js/.wasm |

### 10.2 关键 CMake 选项

| 选项 | 默认 | 说明 |
|---|---|---|
| `VBL_IMPLEMENT_LEVEL` | `wes` | 底层引擎（`wes` 或 `nle`） |
| `VBL_MOBILE` | OFF | 启用移动端构建（Android/iOS） |
| `VBL_WEB` | OFF | 启用 Web/WASM 构建 |
| `VBL_LIBRARY_MODE` | SHARED | 库类型（`SHARED` 或 `STATIC`） |
| `VBL_ENABLE_TESTS` | ON | 构建测试（带 GoogleTest） |
| `FILMORA_DEBUG` | OFF | Release 配置保留调试信息 |
| `VBL_ENABLE_install` | ON | 生成 install 目标 |
| `VBL_ENABLE_packaging` | ON | 生成打包目标 |
| `VBL_TESTS_DYNAMIC` | OFF | 测试编译为动态库（而非可执行文件） |
| `VBL_AUTO_GENERATE_TESTS` | OFF | 自动生成 gtest 用例（实验性） |
| `VBL_ENABLE_COLLECT_PERFORMANCE` | ON | 收集性能数据 |

### 10.3 FILMORA_DEBUG 模式

用于 Release 包调试：关闭优化（`/Od` 或 `-O0`），保留调试符号，可选重定向输出目录：

```cmake
-DFILMORA_DEBUG=ON -DFILMORA_DEBUG_OUTPUT_DIRECTORY=<path>
```

### 10.4 ccache 加速

非 CICD 且非 Web 构建时，CMake 自动检测 ccache 并启用：

| 平台 | 缓存目录 | 缓存大小 |
|---|---|---|
| macOS | `/Users/ws/cache/fvbl/.ccache/arm` | 10 GB |
| Android | `/Users/ws/cache/fvbl/.ccache/android` | 10 GB |
| iOS | `/Users/ws/cache/fvbl/.ccache/ios` | 10 GB |

---

## 十一、CMake 构建体系

### 11.1 构建体系架构

```
CMakeLists.txt（根）
├── tools/cmake/ 包含的辅助模块
│   ├── modules_basic.cmake     ← AppendModulesBasic 函数（核心）
│   ├── vbl_dir.cmake           ← 目录辅助函数（SUBDIRLIST 等）
│   ├── vbl_file.cmake          ← 文件辅助函数
│   ├── vbl_functions.cmake     ← 通用函数（add_subdirectory_split 等）
│   ├── vbl_3rdparyty.cmake     ← 三方库路径配置
│   ├── vbl_install.cmake       ← install() 规则
│   ├── vbl_packaging.cmake     ← CPack 打包配置
│   ├── vbl_config_version.cmake← 版本配置文件生成
│   ├── vbl_tests_operations.cmake ← 测试操作辅助
│   ├── mobile.cmake            ← 移动端工具链配置
│   └── MSVCPCH.cmake           ← Windows 预编译头配置
│
├── modules/CMakeLists.txt
│   ├── 定义 group_private_link_libs（全模块共享依赖）
│   └── 遍历子目录 → add_subdirectory_split()
│
└── modules/<Name>/CMakeLists.txt
    ├── 设置模块特定依赖变量
    └── AppendModulesBasic(Name, Group, Folders)
            ├── 生成 version.h（configure_file）
            ├── Glob 收集头文件与源文件
            ├── add_library(Name SHARED)
            ├── add_library(VBL::Name ALIAS Name)
            ├── target_include_directories(...)
            ├── target_link_libraries(...)
            ├── install(TARGETS ...)
            └── 若 tests/modules/<Group>/ 存在：
                    add_executable(Name_test ...)
                    target_link_libraries(Name_test PRIVATE ${GTEST_LIBS} Name)
                    add_test(NAME Name_test COMMAND ...)
```

### 11.2 输出目录

| 平台 | 运行时输出（DLL/EXE） | 库输出（仅 Windows .lib） | 安装目录 |
|---|---|---|---|
| Windows | `bin/` | `lib/` | `build/install/VBL/win/` |
| macOS | `bin/` | `bin/` | `build/install/VBL/mac/` |
| Android（arm64） | `bin/` | `bin/` | `build/install/VBL/ANDROID_arm64-v8a/` |

---

## 十二、测试体系

### 12.1 测试组织结构

测试代码位于 `tests/modules/`，目录结构与 `modules/` 完全对应：

```
tests/modules/
├── Adapter/
│   ├── AdapterFactory_test.cpp
│   ├── AudioCaptureAdapter_test.cpp
│   ├── TimelineVideoAnalyser_test.cpp
│   └── ...
├── BaseService/
│   ├── BackgroundTaskManager/
│   │   ├── BsAudioWaveExtractTask_test.cpp
│   │   ├── BsFaceDetectTask_test.cpp
│   │   └── ...
│   ├── BsCloudDisk/
│   ├── BsCloudResource/
│   ├── BsAI/
│   ├── BSWsid/
│   └── ...
└── DataModel/（若存在）
```

### 12.2 测试自动发现机制

`AppendModulesBasic` 函数自动为每个模块注册测试：

1. 检测 `tests/modules/<group>/` 目录是否存在
2. 若存在，创建 `<Name>_test` 可执行目标
3. 链接 `GoogleTest` + 被测模块
4. 通过 `add_test()` 注册到 CTest
5. 测试结果输出：`build/vbl_gtest_output/<Name>_test.json`

### 12.3 运行测试

```bash
# 构建所有测试（默认启用）
cmake -G "Ninja Multi-Config" -S . -B build
cmake --build build --config Debug

# 运行全部测试
cd build
ctest -C Debug --output-on-failure

# 运行单个模块测试（正则过滤）
ctest -C Debug -R DataModel_test -V
ctest -C Debug -R BackgroundTaskManager_test -V

# 直接运行测试可执行文件
bin/Debug/DataModel_test.exe
bin/Debug/DataModel_test.exe --gtest_filter="SomeTestSuite.SomeTestCase"

# 禁用测试构建
cmake -G "Ninja Multi-Config" -S . -B build -DVBL_ENABLE_TESTS=OFF
```

### 12.4 测试公共源（AppendCommonSource）

测试目标通过 `AppendCommonSource(${name})` 获取测试公共头和辅助库（`COMMONSOURCE_INCLUDE` / `COMMONSOURCE_LIBS`），避免每个测试重复定义 Mock 和工具类。

---
