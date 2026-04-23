# Filmora 整体设计架构文档

> 版本：14.7.x | 平台：Windows / macOS | 技术栈：C++17 + Qt 5.15

---

## 目录

1. [整体架构概览](#1-整体架构概览)
2. [分层架构](#2-分层架构)
3. [启动与生命周期](#3-启动与生命周期)
4. [核心框架层（FilmoraFrameworkPlatform）](#4-核心框架层-filmoraframeworkplatform)
5. [后端服务层（FF* 模块）](#5-后端服务层-ff-模块)
6. [数据模型层（VBL Model）](#6-数据模型层-vbl-model)
7. [前端视图层（F*View 模块）](#7-前端视图层-fview-模块)
8. [页面管理与场景系统](#8-页面管理与场景系统)
9. [模块间通信机制](#9-模块间通信机制)
10. [异步任务系统](#10-异步任务系统)
11. [AI 功能模块](#11-ai-功能模块)
12. [工具与辅助模块](#12-工具与辅助模块)
13. [平台差异处理](#13-平台差异处理)
14. [目录结构总览](#14-目录结构总览)

---

## 1. 整体架构概览

Filmora 是一款跨平台桌面视频编辑器，整体采用**模块化分层架构**，通过 CMake 将几十个独立子模块组织成一个应用。

```
┌─────────────────────────────────────────────────────────────┐
│                    Filmora 应用进程                           │
│                                                              │
│  ┌─────────────────────────────────────────────────────┐    │
│  │             前端视图层（F*View 模块）                  │    │
│  │  FTimelineView  FMediaPlayerView  FPropertyPanelView │    │
│  │  FMediaLibraryView  FExportView  FCommonView  ...    │    │
│  └────────────────────────┬────────────────────────────┘    │
│                            │ 接口调用 / Qt 信号               │
│  ┌─────────────────────────▼────────────────────────────┐   │
│  │             后端服务层（FF* 模块）                      │   │
│  │  FFMediaLibrary  FFAudioMixer  FFAdvancedEdit         │   │
│  │  FFAppLicense  FFCloudDisk  FFSpeechService  ...      │   │
│  └────────────────────────┬────────────────────────────┘   │
│                            │ VBL/WES 渲染引擎接口             │
│  ┌─────────────────────────▼────────────────────────────┐   │
│  │          数据模型层（FFVBLModel / IFF* 接口）           │   │
│  │  IFFProject  IFFTimeline  IFFAbstractClip             │   │
│  │  IFFMediaLibrary  IFFMediaPlayer  ...                 │   │
│  └────────────────────────┬────────────────────────────┘   │
│                            │                                  │
│  ┌─────────────────────────▼────────────────────────────┐   │
│  │        核心框架层（FilmoraFrameworkPlatform）           │   │
│  │  FFCore  FFAsync  FFFilmoraCore  FFSchedule           │   │
│  │  Qt 工具集  日志  网络  皮肤  单例  观察者  ...         │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. 分层架构

### 层级说明

| 层级 | 命名惯例 | 位置 | 职责 |
|------|----------|------|------|
| **核心框架层** | `FF*`（双F前缀） | `3rdparty/FilmoraFrameworkPlatform` | 基础设施：Qt扩展、工具类、日志、网络、异步、观察者 |
| **数据模型层** | `IFF*`（接口）/ `FFVBLModel` | `FilmoraFrameworkPlatform/Include/FFVBLModel` | 纯虚接口定义：项目、时间线、轨道、剪辑、媒体 |
| **后端服务层** | `FF*`（双F前缀） | `Src/FF*` | 功能服务实现：媒体库管理、混音、授权、云盘、AI等 |
| **前端视图层** | `F*View` / `F*` | `Src/F*View` | Qt UI 面板：时间线、播放器、属性面板、媒体库、导出 |
| **应用启动层** | `FilmoraApp` | `Src/Filmora` | 进程入口、初始化编排、生命周期管理 |

### 命名前缀约定

- `IFF*` — 纯虚接口类（Interface），定义模块对外契约
- `FF*` — 框架/服务的具体实现类（Framework Feature）
- `F*` — 前端 UI 类（Frontend）
- `FFAsync::` — 异步任务命名空间
- `ffcore::` — 核心工具命名空间（ObserverContainer 等）
- `FFVBLMODEL::` — VBL 模型数据类型命名空间

---

## 3. 启动与生命周期

### 启动流程

```
main()
  │
  ├── FilmoraApp::initHighDPI()        // 高DPI初始化（静态，Qt创建前）
  │
  └── FilmoraApp::run()                // 继承自 FApplication → FFApplication
        │
        ├── init()                     // 环境初始化阶段
        │     ├── initDirInfo()        // 目录路径初始化（Win/Mac 分支）
        │     ├── initFFEnv()          // VBL/WES 底层引擎环境初始化
        │     ├── initSingleton()      // 单例服务注册
        │     ├── initGPUConfig()      // GPU 配置检测
        │     ├── initDecoder()        // 解码器初始化
        │     └── FFAsyncInitializer   // 启动异步任务引擎
        │
        ├── prepareRun()               // 预启动阶段（异步并行任务）
        │     ├── initLicense()        // 授权验证
        │     ├── initEventTrackingProxy()  // 埋点
        │     ├── initABTestService()  // A/B 测试
        │     ├── initWsPush()         // 消息推送
        │     ├── initCloudDisk()      // 云盘
        │     ├── initNetworkProxy()   // 网络代理
        │     └── showSplashScreen()   // 启动页
        │
        ├── doRun()                    // 主编辑器启动
        │     ├── initProjectEditor()  // 创建项目/编辑器实例
        │     ├── FilmoraPageMgr::run()// 页面管理器启动
        │     └── QApplication::exec() // 进入 Qt 事件循环
        │
        └── runFinished()              // 退出清理
              ├── uninitFFEnv()
              └── 各服务反初始化
```

### 继承链

```
QApplication
    └── FFApplication          (FilmoraFrameworkPlatform/FFCore)
            └── FApplication   (Src/FCore)
                    └── FilmoraApp  (Src/Filmora)
```

`FFApplication` 提供：
- 高DPI配置、顶层窗口管理
- 应用启动/退出 Action 注册宏（`OnApplicationStart` / `OnApplicationQuit`）
- 全局通知观察者 `FFApplicationNotifyObserver`

`FApplication` 提供：
- 皮肤/主题管理（`FFSkinManager`）
- 单应用实例锁（`FFSingleApplication`）
- 崩溃上报（`FBugSplatProxy`）
- CEF 初始化（Windows）

---

## 4. 核心框架层（FilmoraFrameworkPlatform）

子模块 `3rdparty/FilmoraFrameworkPlatform`，独立的 git 子模块，为整个项目提供基础设施。

### 主要子命名空间/模块

| 目录 | 功能 |
|------|------|
| `Include/FFCore` | 基础工具：观察者模式、单例、日志、网络、JSON、异步 |
| `Include/FFAsync` | 异步任务框架（FFTask、FFTaskRunner、Promise/Future）|
| `Include/FFFilmoraCore` | Filmora 特有的应用基础设施（配置、网络、域名、Token）|
| `Include/FFVBLModel` | 数据模型接口（IFFProject、IFFTimeline 等）|
| `Include/FFSchedule` | 有向无环图任务调度（FFScheduleProject）|
| `Src/FFCore` | FFApplication、FFUIScene 等实现 |
| `Src/FFAsync` | 异步引擎实现 |
| `3rdparty/VBL` | VBL 渲染/编辑引擎（底层 C++ 库，二进制形式） |
| `3rdparty/WES` | WES 特效引擎（底层 C++ 库，二进制形式） |

### 核心工具类（FFCore）

```cpp
// 观察者容器 — 线程安全的观察者管理
ffcore::ObserverContainer<IObserverType>
  // attach / detach / invoke

// 懒汉单例
FFLazySingleton<T>   // 继承后获得 T::instance() 方法

// UI 场景管理
FFUIScene            // 持有多个 FFUIView，管理消息分发和撤销/重做

// Presenter 基类
FFPresenter          // MVP 模式 Presenter 基类

// 日志
IFFBaseLog           // 统一日志接口
```

---

## 5. 后端服务层（FF* 模块）

所有后端服务位于 `Src/FF*`，通过 `IFF*` 接口对外暴露能力，前端视图和应用层只依赖接口，不直接依赖实现。

### 核心服务模块

#### FFMediaLibrary — 媒体库
- 管理本地/在线素材的导入、索引、缩略图生成
- 提供 `IFFMediaLibrary` 接口，支持按分类/关键词/过滤器检索
- 内部采用 Presenter + Model 架构（`FFMediaLibraryPresenter` + `FFMediaDataModel`）
- 支持两种搜索模式：本地过滤（`LocalFilter`）和在线搜索（`OnlineFilter`）

#### FFAudioMixer — 混音面板
- 管理所有轨道的音量/平衡/Surround 设置
- 组件：`FFAudioMixerPresenter`（逻辑）+ `FFAudioMixerTracksModel`（数据）+ `FFAudioMixerPanel`（UI）
- 依赖 `IFFTimeline` 获取轨道数据

#### FFAppLicense — 授权管理
- 管理软件订阅/授权状态
- 接口：`IFFAppLicense`、`IFFWSIDLicense`（万兴账号授权）
- 与 CBS（配置服务）、在线授权服务器通信

#### FFCloudDisk — 云盘
- 管理用户云端素材存储
- 接口：`IFFCloudDisk`、`IFFCloudResourceUserActionCollector`
- 通过 `FFCloudDiskItemManagerProxy` 代理访问

#### FFSpeechService — 语音/字幕服务
- AI 语音识别、字幕生成、字幕翻译
- 依赖 `IFFAITranslationService`、`IFFSpeechServiceAITranslatePresenter`

#### FFAdvancedEdit — 高级编辑
- 高级文字编辑器（`FFAdvanceTextMediaPlayer`）
- 复杂特效操作封装

#### 其他服务模块

| 模块 | 职责 |
|------|------|
| `FFAppSettings` | 应用全局设置持久化 |
| `FFWsUpgrade` | 软件自动更新 |
| `FFLossless` | 无损导出 |
| `FFWsPush` / `FFWsMsgPush` | WebSocket 消息推送 |
| `FFSmartKeying` | AI 智能抠图 |
| `FFSilenceDetection` | 静音检测 |
| `FFRecord` | 屏幕录制 |
| `FFAutoHighlightMontage` | AI 自动剪辑/高光 |
| `FFCropAndZoom` | 关键帧裁剪缩放 |
| `FWsSurvey` | 用户问卷 |
| `FFUpgrade` | 升级弹窗 |

---

## 6. 数据模型层（VBL Model）

核心数据模型定义在 `FilmoraFrameworkPlatform/Include/FFVBLModel`，采用**纯虚接口**设计，实现在 VBL/WES 底层引擎中。

### 核心接口关系

```
IFFProject                          ← 工程根对象
  ├── config() → FFProjectConfig    ← 工程配置（分辨率/帧率/长度）
  ├── mainTimeline() → IFFTimeline  ← 主时间线
  ├── mediaLibrary() → IFFMediaLibrary  ← 媒体库
  ├── player() → IFFMediaPlayer     ← 播放器
  └── undoRedoService() → IFFUndoRedoService  ← 撤销/重做

IFFTimeline                         ← 时间线
  ├── tracks[]                      ← 轨道集合（视频/音频/字幕/特效）
  └── duration()

IFFAbstractClip                     ← 剪辑基类（所有轨道元素）
  ├── start() / end() / duration()  ← 时间属性
  ├── type()                        ← 剪辑类型（视频/音频/文字/贴纸...）
  ├── resourceInfo()                ← 关联的媒体资源
  └── isSelected() / enabled()

IFFMediaLibrary                     ← 媒体库模型接口
  └── 素材搜索/过滤/分类树
```

### VBL 与 WES

- **VBL**（Video Base Library）：核心视频编辑引擎，处理时间线、轨道、剪辑的数据结构和渲染调度
- **WES**（Wondershare Effect System）：特效渲染引擎，处理滤镜、转场、动画等视觉特效
- 两者均以预编译二进制库形式集成，通过 `IFF*` 接口层访问
- 换库时需运行 `Build/CopyBat/copy_vbl.bat` 同步运行时文件

### Helper 工具类

FFVBLModel 目录下还提供大量无状态工具类（`FF*Helper`），封装对 VBL 接口的常见操作：

```
FFClipHelper        // 剪辑通用操作
FFTimelineHelper    // 时间线辅助操作
FFTrackHelper       // 轨道操作
FFKeyFrameHelper2   // 关键帧操作
FFSubtitleHelper    // 字幕操作
FFTextClipHelper    // 文字剪辑操作
FFMediaItemHelper   // 媒体项操作
FFClipFinder        // 剪辑查找
```

---

## 7. 前端视图层（F*View 模块）

所有 UI 面板位于 `Src/F*`，均基于 Qt Widgets 构建，遵循 **MVP（Model-View-Presenter）** 模式。

### 主要视图模块

#### FTimelineView — 时间线视图
- 最复杂的 UI 模块，基于 `QGraphicsScene` / `QGraphicsView` 实现
- `FGraphicsAbstractClipItem`：所有轨道元素的基类（继承 `QGraphicsRectItem`）
- 支持多选、拖拽、裁剪（Trim In/Out）、关键帧编辑
- 子系统：AI 背景音乐（`FAIBackgroundMusicPresenter`）、自动重构（`FAIAutoReframeHelper`）、音频闪避（`FAudioDuckService`）

#### FMediaPlayerView — 播放器视图
- 预览播放器，通过 IPC 与独立播放进程通信（`FIPCMediaPlayer`）
- 实现 `IFFAbstractMediaPlayer` 接口
- 支持时间码显示、帧步进、全屏预览

#### FPropertyPanelView — 属性面板
- 根据时间线选中的剪辑类型，动态切换显示对应属性 UI
- 子目录结构：`Audio/`、`AITools/`、`EffectPanel/`、`Core/`、`CoverProperty/`
- 与时间线通过消息总线 / 信号同步选中状态

#### FMediaLibraryView — 媒体库视图
- 显示本地导入素材、云端素材、效果资源
- 与 `FFMediaLibrary` 后端服务配合
- 支持拖拽到时间线

#### FExportView — 导出视图
- 多格式导出面板（本地/云端/DVD 等）
- 采用工厂模式创建不同格式的导出面板：`FExportCommonApi::createExportPanel()`
- 子目录：`ExportPanel/`、`ExportDataModel/`、`ExportProgress/`、`DVDBuilder/`

#### 其他视图

| 模块 | 职责 |
|------|------|
| `FCommonView` | 公共 UI 控件（按钮、对话框、Toast、进度条）|
| `FProductHomePage` | 首页/欢迎页 |
| `FAdvancedEdit` | 高级编辑 UI（文字、遮罩、混合模式）|
| `FAutoReframe` | 自动重构预览和操作面板 |
| `FUserGuide` | 用户引导/新手教程 |
| `FThirdPartyCloud` | 第三方云盘集成 UI |
| `FProjectCover` | 项目封面设置 |
| `FDownloadCenter` | 素材下载管理器 |

---

## 8. 页面管理与场景系统

### FilmoraPageMgr — 页面管理器

`FilmoraPageMgr`（`Src/Filmora/FilmoraPageMgr.h`）是应用主界面的**状态机控制器**，继承自多个 Observer 接口，统一监听并响应各业务系统的事件。

```cpp
class FilmoraPageMgr : public QObject
    , public FFLazySingleton<FilmoraPageMgr>
    , public IFFWSIDEventObserver        // 账号事件
    , public IFFCloudDiskEventObserver   // 云盘事件
    , public IFFIPCProcessEventObserver  // 进程间通信事件
    , public IFFProjectManagerEventObserver  // 项目事件
    , public IFUserGuideEventObserver    // 引导事件
    , public IFFAITaskManagerObserver    // AI任务事件
```

**页面枚举：**
```
fpiProductHomePage  // 首页
fpiMainForm         // 主编辑器
```

### FFUIScene — UI 场景

`FFUIScene`（`FilmoraFrameworkPlatform/Include/FFCore/FFUIScene.h`）是视图聚合容器，管理一组 `FFUIView`：

```cpp
class FFUIScene {
    void addView(FFUIView*)
    void removeView(FFUIView*)
    void setActive(bool)
    void sendMessage(...)            // 向场景内所有 View 广播消息
    IFFUndoRedoService* undoRedoService()  // 统一的撤销/重做
    void subscribe(event, handler)   // 事件订阅
}
```

场景类型：
- **主编辑场景**（`FilmoraMainForm`）：包含时间线、播放器、属性面板、媒体库
- **模板场景**（`TemplateModeScene`）：模板预览/编辑

---

## 9. 模块间通信机制

项目采用**多层次解耦通信**策略，不同场景使用不同机制：

### 1. Qt 信号/槽（同进程，跨线程安全）

适用于 UI 事件传递、状态变更通知。

```cpp
// 视图内部
connect(clipItem, &FGraphicsAbstractClipItem::selected,
        propertyPanel, &FPropertyPanelView::onClipSelected);
```

### 2. 观察者模式（ffcore::ObserverContainer）

适用于一对多的业务事件广播，支持运行时动态注册/注销。

```cpp
// FilmoraApp 中
ffcore::ObserverContainer<IFStartedAppObserver> m_startedAppObserverList;

// 注册
addStartedAppObserver(observer);

// 广播（模板遍历）
m_startedAppObserverList.invoke(&IFStartedAppObserver::onAppStarted);
```

常见 Observer 接口：
- `IFFProjectManagerEventObserver` — 项目打开/关闭/保存事件
- `IFFCloudDiskEventObserver` — 云盘状态变化
- `IFFAITaskManagerObserver` — AI 任务进度
- `IFFWSIDEventObserver` — 账号登录/登出

### 3. 纯虚接口（IFF* 接口调用）

适用于跨层调用，视图层通过接口访问后端能力，不依赖具体实现。

```cpp
// 视图通过接口操作时间线
IFFTimeline* timeline = project->mainTimeline();
timeline->addClip(clip, trackIndex, position);
```

### 4. IPC（进程间通信）

播放器采用独立进程，通过 IPC 通信：
- `FIPCMediaPlayer`：IPC 播放器客户端实现
- `FPlayServer`（仅 Windows）：播放服务进程
- `IFFIPCServer` / `FilmoraIPCServer`：IPC 服务端

### 5. 消息总线（FFUIScene::sendMessage）

场景内 View 之间通过消息总线解耦通信，避免直接依赖。

### 6. CBS URL 服务（配置服务）

运行时从 CBS 服务拉取动态配置 URL：
- `FCBSUpdater` / `IFCBSUpdater`：管理各种功能接口的 URL
- `FFCBSUrlReader`：URL 读取器

---

## 10. 异步任务系统

项目有两套互补的异步系统：

### FFAsync — 轻量异步框架

```
Include/FFAsync/
  FFTask.h          // 任务单元
  FFTaskRunner.h    // 任务执行器
  FFAsync.h         // 异步函数组合
  promise/          // QtPromise（Promise/Future 模式）
  FFAsyncInitializer.h  // 异步初始化器（FilmoraApp 使用）
```

`FFAsync::FFAsyncInitializer` 用于应用启动阶段的并行初始化：
- 将多个无依赖的初始化任务并行执行
- 合并完成后才进入下一启动阶段

### FFSchedule — 有向无环图（DAG）任务调度

```
FFScheduleProject       // 任务项目（DAG 根节点）
  └── FFScheduleTask[]  // 各任务节点
        └── after()     // 建立依赖关系

FFScheduleProjectBuilder  // Builder 模式构建任务图
  .add("task1", func1)
  .add("task2", func2)
  .after("task1")          // task2 依赖 task1
  .create()
```

`FilmoraApp` 中 `m_taskWatcher` 和 `m_pPrepareRunTask` 使用此系统管理启动时的任务依赖链。

---

## 11. AI 功能模块

AI 功能是 Filmora 的重要组成部分，分布在多个层次：

### AI 服务接口（FFVBLModel）

所有 AI 服务均通过 `IFF*` 接口暴露（位于 `Include/FFVBLModel/IFFAi*.h`）：

| 接口 | 功能 |
|------|------|
| `IFFAITaskManager` | AI 任务统一管理器，监控所有 AI 任务进度 |
| `IFFAIService` | AI 服务基类 |
| `IFFAISmartShortService` | AI 智能短片 |
| `IFFAIErasingService` | AI 对象消除 |
| `IFFAIGCService` | AI 生成内容（AIGC）|
| `IFFAITranslationService` | AI 翻译 |
| `IFFAITextToVideoManager` | 文字生成视频 |
| `IFFAIImageStylizerService` | AI 图片风格化 |
| `IFFAIVoiceCloneService` | AI 声音克隆 |
| `IFFAIUltraHDService` | AI 超分辨率 |
| `IFFAIIntelligentMusicService` | AI 智能配乐 |
| `IFFAILongToShortService` | AI 长视频转短视频 |
| `IFFAIAudioToVideoService` | 音频驱动视频生成 |
| `IFFAIIdeaToVideoService` | 创意转视频 |
| `IFFAITextMusicService` | 文字生成音乐 |

### AI Copilot（FCore）

`FAICopilotService`（`Src/FCore`）：AI 助手服务，实现 `IFAICopilotService` 接口，通过 WebSocket/HTTP 与 AI 后端通信。

### AI 语音服务（FFSpeechService）

`Src/FFSpeechService`：语音识别、字幕生成、AI 翻译的统一入口。

### AI 视觉功能

- `FFSmartKeying`：AI 智能绿幕/抠图
- `FFAutoHighlightMontage`：AI 高光剪辑
- `FFSilenceDetection`：静音检测（自动分割）
- `FFCropAndZoom`：AI 裁剪缩放（人脸追踪）

---

## 12. 工具与辅助模块

### 埋点统计（FEventTracking）

`Src/FEventTracking`：统一用户行为埋点，通过 `IFFEventTrackingProxy` 接口发送事件。UI 层通过 `FUiEventTrack` 工具类快速埋点。

### 下载中心（FDownloadCenter）

`Src/FDownloadCenter`：素材/效果包的下载队列管理。

### 性能监控

- `FSystemMonitor`（`FSystemMonitorThread`）：内存占用监控线程
- `WSP::FPerformanceMain`：性能 SDK 接入
- `FFPerformanceTrack`、`FFPerformancePileInsetor`：性能打点工具

### 崩溃上报

- `FBugSplatProxy`：集成 BugSplat 崩溃收集（Win/Mac）
- `FDmpSender`（仅 Windows）：Dump 文件发送

### 调试工具（仅 Debug 构建）

- `QSpy` / `QSpyServer`：UI 组件树远程检查工具
- `Tools/WidgetsTool`：控件预览工具

---

## 13. 平台差异处理

### Windows 专属模块

| 模块 | 功能 |
|------|------|
| `FWebBrowser`（QCefView）| CEF 内嵌浏览器 |
| `FPlayServer` | 独立播放服务进程 |
| `FUninstaller` | 卸载程序 |
| `FFWsRegister` | Windows 注册表操作 |
| `FFWsAP` | 防盗版（AntiPiracy）|
| `FDmpSender` | Minidump 上传 |
| `FEffectInstaller` | 效果包安装程序 |

### macOS 专属模块

| 模块 | 功能 |
|------|------|
| `FNativeSettings` | 系统偏好/权限设置 |
| `FFMacToQtSyncData` | macOS 原生层 → Qt 数据同步 |
| `FilmoraWidget`（仅 App Store）| macOS 小组件扩展 |
| `FFCocoaAppDelegateProxy` | Cocoa AppDelegate 桥接 |

### CMake 条件编译宏

```cpp
#ifdef Q_OS_WIN    // Windows 代码
#ifdef Q_OS_MACOS  // macOS 代码
#ifdef APP_STORE   // App Store 版本
#ifdef FILMORA_FOR_MIAOYING  // 喵影版本
#ifdef BETA        // Beta 版本
```

---

## 14. 目录结构总览

```
filmora11-win/
├── CMakeLists.txt              # 顶层 CMake（定义所有模块）
├── cmake-vs2017.bat            # Windows VS 工程生成脚本
├── xcode.sh                    # macOS Xcode 工程生成脚本
├── .clang-format               # 代码格式化规则
│
├── 3rdparty/
│   └── FilmoraFrameworkPlatform/   # 核心框架（git 子模块）
│       ├── Include/
│       │   ├── FFCore/         # 基础工具（观察者、单例、日志...）
│       │   ├── FFAsync/        # 异步任务框架
│       │   ├── FFFilmoraCore/  # 应用基础设施
│       │   ├── FFVBLModel/     # 数据模型接口（IFFProject 等）
│       │   └── FFSchedule/     # DAG 任务调度
│       ├── Src/
│       │   ├── FFCore/         # FFApplication、FFUIScene 实现
│       │   └── FFAsync/        # 异步引擎实现
│       └── 3rdparty/
│           ├── VBL/            # VBL 渲染引擎（二进制）
│           └── WES/            # WES 特效引擎（二进制）
│
├── Src/
│   ├── Filmora/                # 应用入口（main.cpp、FilmoraApp、FilmoraPageMgr）
│   ├── FCore/                  # FApplication 基类、IPC、AI Copilot
│   │
│   ├── FF*/                    # 后端服务模块（无 UI）
│   │   ├── FFMediaLibrary/     # 媒体库
│   │   ├── FFAudioMixer/       # 混音
│   │   ├── FFAppLicense/       # 授权
│   │   ├── FFCloudDisk/        # 云盘
│   │   ├── FFSpeechService/    # 语音/字幕
│   │   ├── FFSmartKeying/      # AI 抠图
│   │   └── ...
│   │
│   ├── F*View/                 # 前端视图模块（Qt UI）
│   │   ├── FTimelineView/      # 时间线（最核心的 UI）
│   │   ├── FMediaPlayerView/   # 播放器
│   │   ├── FPropertyPanelView/ # 属性面板
│   │   ├── FMediaLibraryView/  # 媒体库面板
│   │   └── FExportView/        # 导出面板
│   │
│   └── F*/                     # 其他前端模块
│       ├── FCommonView/        # 公共控件
│       ├── FEventTracking/     # 埋点
│       ├── FDownloadCenter/    # 下载中心
│       └── ...
│
├── Include/                    # 项目公共头文件（按模块分目录）
├── Lib/                        # 预编译库
├── Bin/                        # 构建产物输出（Bin/x64/Debug|Release）
├── Tests/                      # 单元测试
├── Tools/                      # 开发工具（FigmaPlugin 等）
├── Resource/                   # 应用资源
├── I18n/                       # 国际化文件
└── Doc/                        # 文档（快捷键、类图、性能优化）
```

---

*文档生成时间：2026-03-27 | 基于源码静态分析*
