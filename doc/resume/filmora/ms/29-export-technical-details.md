29-export-technical-details.md
# 导出模块技术实现细节

> 项目：Wondershare Filmora（C++17 + Qt 5.15）
> 模块：FExportView / FExportExe（`Src/FExportView`、`Src/FExportExe`）

---

## 一、模块概述

导出模块是 Filmora 的核心功能出口，负责将用户编辑好的时间线渲染编码为目标文件。支持本地导出（MP4/MOV/AVI/GIF/MP3/WAV 等）、设备导出、社交媒体上传、DVD 刻录、批量导出五大场景。模块设计采用**责任链（Proxy Chain）**驱动前置检查，**Pipeline 任务图**管理编码 Job，编码引擎通过**独立子进程**隔离，VBL 层提供底层编码能力，全程以 MVP 模式组织进度 UI。

---

## 二、整体架构

```
┌─────────────────────────────────────────────────────────────────┐
│                       用户交互触发层                              │
│  FTimelineView / FProductHomePage → exportTimeline()             │
└───────────────────────────────┬─────────────────────────────────┘
                                │ FExportParam（IFFProject + IFFTimeline）
┌───────────────────────────────▼─────────────────────────────────┐
│                 责任链代理层（Proxy Chain）                        │
│  FASyncLicenceFetchProxy  → 异步拉取资源授权                       │
│  FCheckMissFilesProxy     → 检查素材文件丢失                       │
│  FCheckTailVideoFileProxy → 检查片尾视频文件                       │
│  FFormatSelectExportProxy → 显示导出格式面板（用户配置参数）         │
│  FMediaItemLicenseExportProxy → 素材版权检查                      │
│  FAppLicenseExportProxy   → 程序激活授权检查（水印控制）             │
│  FExportProgressProxy     → 执行导出（核心触发点）                  │
└───────────────────────────────┬─────────────────────────────────┘
                                │ FExportDataDelegate（聚合全部参数）
┌───────────────────────────────▼─────────────────────────────────┐
│              导出流程控制层（Process Presenter）                    │
│  FLocalProcessPresenter（本地/设备）                               │
│  FSocialMediaProcessPresenter（社交媒体）                          │
│  FDVDProcessPresenter（DVD）                                      │
└─────────────────┬──────────────────────────┬────────────────────┘
                  │ IFFExportManager          │ 进度回调（Observer）
┌─────────────────▼─────────────┐  ┌─────────▼──────────────────┐
│    VBL Pipeline 引擎层         │  │  进度 UI 层（MVP）            │
│  IFFExportManager              │  │  FExportProgressPresenter   │
│  IFFExportPipeline             │  │  FExportProgressDialog      │
│  IFFExportJob                  │  │  IFExportProgressView       │
│  （Encode / AutoReframe / Upload）│  └────────────────────────────┘
└─────────────────┬─────────────┘
                  │ FFEncodeParam（bGpu / video_fourcc / bitrate…）
┌─────────────────▼─────────────────────────────────────────────┐
│                   VBL 编码引擎层                                 │
│  线程模式：IFFEncoder（VBL 内部工作线程）                          │
│  进程模式：FProcessEncoder → FExportExe 独立子进程（IPC 通信）      │
└───────────────────────────────────────────────────────────────┘
```

---

## 三、核心数据结构

### 3.1 FFBsEncodeParam（VBL 层编码核心参数）

```cpp
struct FFBsEncodeParam {
    // 视频
    bool    disable_video;
    QString video_fourcc;       // "H264" / "HEVC" / "AV1" / "GIF" 等
    int     video_width;
    int     video_height;
    int     video_bitrate;
    int     video_bitrate_mode; // FFBitrateMode: VBR / CBR / ABR
    QPoint  video_framerate;    // {分子, 分母}，如 {30000, 1001} ≈ 29.97fps
    int     video_colorspace;   // SDR-Rec.709 / HDR
    bool    support_alpha;      // 透明通道（ProRes 4444 等）

    // 音频
    bool    disable_audio;
    QString audio_fourcc;       // "AAC" / "MP3" / "PCM"
    int     audio_channels;     // 1 / 2 / 6（环绕声）
    int     audio_bitrate;
    int     audio_samplerate;
    int     audio_bit_depth;    // 16 / 24 / 32

    // 高级
    bool              has_h264_extend_param;
    FFH264ExtendParam h264_extend_param; // profile / b_frame_num / quality_level
    FFLongLongRange   export_range;      // 导出帧区间
    int               gif_quality_mode;
    int               dolby_value;       // 杜比音效（需授权）
    QByteArray        cover;             // 封面图数据
};
```

### 3.2 FFEncodeParam（完整编码参数，含 GPU 等控制字段）

```cpp
struct FFEncodeParam {
    bool                 bGpu;                 // GPU 硬件加速开关
    bool                 bAddCoverToBeginning;  // 封面帧加入视频首帧
    FFEncodeType         encodeType;            // etTimeline / etClips
    FFWatermarkParam     waterMarkParam;        // 水印参数（试用版/未授权）
    FFEncodeOutputParam  outputParam;           // 输出路径（支持多路输出）
    FFEncodeTailVideoParam tailVideoParam;      // 片尾视频拼接参数
    FFBsEncodeParam      bsEncodeParam;         // 核心编码参数
};
```

### 3.3 FFExportPipelineParam（Pipeline 构建参数）

```cpp
struct FFExportPipelineParam {
    QList<FFVBLObjectRefPtr<IFFAbstractResourceInfo>> authResList; // 授权资源列表
    FFVBLObjectRefPtr<IFFTimeline> timeline;
    FFVBLObjectRefPtr<IFFClip>    clip;          // Clip 导出模式时使用
    bool                          bAddCoverToBeginning;
    FFWatermarkParam              waterMarkParam;
    FFEncodeTailVideoParam        tailVideoParam;
    FFBsEncodeParam               bsEncodeParam;
    struct ExternParam {
        QString title;
        bool    bIsUploadCloud;
        bool    bCanUploadCloud;
        bool    bSaveCoverToLocal;
        int     pipelineIndex;
        bool    isTemplateMode;
    } externParam;
};
```

### 3.4 任务状态枚举

```cpp
// VBL 底层编码状态
enum FFEncodeState {
    esFinished, esAborted, esError,
    esExeCrash,       // 子进程崩溃 → 自动切线程模式重试
    esExeFreeze,      // 子进程冻结 → 自动切线程模式重试
    esDiskSpaceError  // 磁盘空间不足
};

// 新架构 Pipeline 任务状态
enum class FFExportStatus {
    Waiting, Starting, Busying, Paused,
    Stopped, Completed, Failed, Resuming
};

// Pipeline Job 类型
enum class FFExportJobType {
    Encode,                  // 编码核心 Job
    AutoReframe,             // AI 自动重构画面比例
    Highlight,               // AI 精彩集锦
    UploadCloudDisk,         // 上传云盘
    UploadThirdPartyPlatform // 上传社交媒体
};
```

---

## 四、主要业务流程

### 4.1 导出触发与责任链执行

```
FTimelineView / FProductHomePage
     └── exportTimeline(FExportParam{project, timeline, parent, ...})
          │
          ├── SCOPED_PREPARE_EXPORT()   // RAII：退出预览模式，备份工程
          ├── FExportDataDelegate::initExportParam()  // 初始化全部参数
          │
          └── 责任链顺序执行（每个 Proxy::execute() 检查条件，通过则调用下一层）
               [1] FASyncLicenceFetchProxy   → 异步拉取素材版权授权
               [2] FCheckMissFilesProxy      → 检查素材文件是否丢失
               [3] FCheckTailVideoFileProxy  → 检查片尾视频文件
               [4] FFormatSelectExportProxy  → 弹出格式面板（用户选参数，阻塞）
               [5] FMediaItemLicenseExportProxy → 素材版权检查（背景音乐/贴纸）
               [6] FAppLicenseExportProxy    → 程序激活检查（试用版注入水印）
               [7] FExportProgressProxy      → 创建 ProcessPresenter，触发导出
               [8] FExportUpgradeProxy       → 付费升级引导弹窗（导出完成后）
```

### 4.2 Pipeline 构建（FLocalProcessPresenter::init()）

```
FLocalProcessPresenter::init()
     │
     ├── createExportManager()  → IFFExportManager（VBL 导出管理器）
     │
     └── 遍历每个待导出时间线：
          pipeline = exportManager->addExportPipeline(pipelineParam)

          [必选] encodeJob = pipeline->addEncodeExportJob(encodeBuildParam)

          [可选] if (bEnabledAutoReframe):
                   reframeJob = pipeline->addAutoReframeExportJob(...)
                   pipeline->addDependency(reframeJob, encodeJob)
                   // AutoReframe 依赖 Encode 完成后才执行

          [可选] if (bIsUploadCloud):
                   uploadJob = pipeline->addUploadCloudDiskJob(...)
                   pipeline->addDependency(uploadJob, encodeJob)
                   // 上传依赖编码完成

     exportManager->start()  → 按依赖图调度所有 Pipeline 执行
```

### 4.3 编码引擎两路策略（进程模式 / 线程模式）

```
exportManager->start() 触发编码
     │
     ├── [优先] 子进程模式（FProcessEncoder）
     │    ├── 启动 FExportExe 独立子进程
     │    ├── IPC 发送 StartMsg（含序列化的 FFEncodeParam JSON）
     │    ├── m_pProcressInitTimer（120s）监控启动超时
     │    ├── m_pMonitorTimer（15s）监控进度心跳，无进度→认定冻结
     │    │
     │    ├── [正常] IPC 接收 ProgressMsg → 更新进度
     │    │         IPC 接收 StateMsg(esFinished) → 完成
     │    │
     │    ├── [异常：GPU 错误] reStartCloseGpu()
     │    │    → isDisableGPU = true → 关闭子进程 → 软编重启
     │    │
     │    └── [异常：esExeCrash / esExeFreeze]
     │         → 切换到线程模式重试（降级）
     │
     └── [降级] 线程模式（直接调用 IFFEncoder，VBL 内部线程）
```

### 4.4 子进程 IPC 通信

```
主进程（FProcessEncoder）              子进程（FExportExe）
         │                                      │
         ├── IPC StartMsg ──────────────────→   FExportServerClient
         │    （FFEncodeParam 序列化 JSON）        └── FExportVideoTask::start()
         │                                          └── IFFEncoder::start()
         │
         │← IPC ProgressMsg（progress 0.0~1.0）─── onProgressChanged()
         │← IPC StateMsg（esFinished/esError…）─── onStateChanged()
         │
         ├── IPC PauseMsg ───────────────────→   IFFEncoder::pause()
         ├── IPC ResumeMsg ──────────────────→   IFFEncoder::resume()
         └── IPC StopMsg ────────────────────→   IFFEncoder::stop()
```

### 4.5 进度回调链

```
[VBL 工作线程 / 子进程]
IFFExportManagerEventObserver::onProgressChanged(pipeline, progress)
     │
     └── FLocalProcessPresenterPrivate（observerHelper 注册）
          │ Qt 信号 QueuedConnection（自动切主线程）
          └── FExportProgressPresenter::onTaskProgressChanged()
               └── IFExportProgressStatusWidget::setProgress(progress)
                    └── 进度条 UI 更新（平滑动画，精度 0.01）

IFFExportManagerEventObserver::onStatusChangedInWorkThread()
     └── FExportProgressTrackingHelper（工作线程直接执行，精确埋点时间戳）
```

### 4.6 社交媒体导出流程

```
FSocialMediaProcessPresenter::startExport()
     │
     ├── [第一阶段] 本地编码（复用 FLocalProcessPresenter 逻辑）
     │    → 生成临时本地文件
     │
     └── [第二阶段] 上传
          pipeline->addUploadThirdPartyPlatformJob(uploadParam)
          │  platform SDK 异步上传（YouTube / TikTok / Vimeo…）
          │  支持：定时发布（FSocialMediaCalendarPopWidget 选择时间）
          └── onUploadFinished → 通知 UI 更新状态
```

---

## 五、核心技术点

### 5.1 责任链模式驱动前置检查

所有导出前置条件通过 `FAbstractExportProxy` 子类串联，每个 Proxy 持有 `IFExport* m_pNextExportProxy` 指针：

```cpp
class FAbstractExportProxy : public IFExport {
    IFExport* m_pNextExportProxy{nullptr};
protected:
    virtual void doExecute() = 0; // 子类实现检查逻辑
    void executeNext() { if (m_pNextExportProxy) m_pNextExportProxy->execute(); }
public:
    void execute() override { doExecute(); } // 子类决定是否调用 executeNext()
};
```

不通过检查的代理直接短路（如文件丢失则弹窗提示并返回），不调用 `executeNext()`。这种设计使新增检查步骤只需插入新 Proxy 而不修改已有代码，符合开闭原则。

### 5.2 独立子进程编码隔离

编码引擎运行在独立的 `FExportExe` 子进程中，与主进程完全隔离：

**好处**：
- 编码崩溃不影响主进程（VBL 渲染崩溃会导致主进程退出）
- 子进程独占 GPU 上下文，不与渲染进程竞争
- 可并发多个子进程实现多任务批量编码

**双重保活机制**：
```cpp
m_pProcressInitTimer->start(120000); // 启动超时：120s 内未收到首个 ProgressMsg
m_pMonitorTimer->start(500);         // 心跳监控：轮询 VBL 层最新进度

// 无进度超过 15s → esExeFreeze → 切线程模式降级
if (lastProgress == currentProgress && elapsed > 15000)
    switchToThreadMode();
```

### 5.3 GPU 硬件加速与软编降级

GPU 编码通过 `FFEncodeParam::bGpu = true` 启用，VBL 层按 GPU 类型选择编码器：
- NVIDIA → NVENC（H.264 / H.265）
- AMD → AMF（H.264 / H.265）
- Intel → QSV（H.264 / H.265）

降级逻辑：
```cpp
void FProcessEncoder::reStartCloseGpu() {
    // GPU 编码失败（驱动异常/显存不足）时：
    isDisableGPU = true;
    m_encodeParam.bGpu = false;   // 关闭 GPU
    stopCurrentProcess();          // 终止子进程
    restartProcess();              // 软编重启，全部流程复用，无感知切换
}
```

### 5.4 Pipeline 任务依赖图

`IFFExportManager` 内部实现了有向无环图（DAG）调度引擎，Job 之间可声明依赖关系：

```
encodeJob ─────────────────┬──────────────────────────
                            ▼
                    autoReframeJob（等 Encode 完成后执行）
                            ▼
                    uploadCloudJob（等 AutoReframe 完成后执行）
```

VBL 层按拓扑排序调度 Job，满足依赖的 Job 立即启动，不满足依赖的 Job 等待，实现最优并发。

### 5.5 RAII 导出环境守卫（FExportGuard）

导出期间需要保护多个全局状态，使用 RAII 确保任何路径退出后均自动恢复：

```cpp
class FExportGuard {
    // 析构时自动还原：
    ~FExportGuard() {
        FFProjectAutoSaveEnabledWraper::restore(); // 恢复自动保存
        disableColorCompareMode(false);             // 恢复颜色对比模式
        FFAlgorithmCacheTaskGuard::resume();        // 恢复 AI 算法缓存任务
        FLicenseRefreshBlocker::unblock();          // 恢复 License 刷新
    }
};
```

### 5.6 多格式特殊处理

| 格式 | 特殊逻辑 |
|---|---|
| **GIF** | `disable_audio = true`；`gif_quality_mode`（调色板大小/帧率/抖动算法）|
| **MP3/WAV** | `disable_video = true`；纯音频流，无需 GPU |
| **DVD** | 独立 `FDVDProcessPresenter`；H.262/MPEG-2 编码；DVD 目录/ISO/刻录三种输出模式 |
| **HDR** | `video_colorspace = HDR`；需要 HDR 显示器或 HDR 兼容播放器 |
| **透明通道** | `support_alpha = true`；需选择支持 Alpha 的格式（如 ProRes 4444）|
| **杜比音效** | `dolby_value` 传给 VBL；入口通过 `FF_CBS_SWITCH` 功能开关控制显隐 |

### 5.7 文件大小预估

`FExportSettingSizeAndDurationPresenter` 实现实时预估：

```
预估大小 ≈ (video_bitrate + audio_bitrate) × duration ÷ 8
```
用户调整码率/时长时，Widget 实时更新预估文件大小，帮助用户在质量和体积间权衡。

### 5.8 进度对话框活动区（营销位）

`IFExportProgressActivityWidget` 在导出等待期间展示运营内容，按策略切换：
- `FExportProgressPurchaseWidget`：付费功能购买引导
- `FExportSceneNPSWidget`：用户满意度 NPS 评分
- `FResourceFreeTrialWidget`：付费资源 0 元试用
- `FFissionActivityWidget`：裂变活动（分享得权益）

设计为接口 + 工厂，运营可灵活配置显示策略，导出等待时间被充分利用。

---

## 六、设计模式应用

### 6.1 责任链模式（Chain of Responsibility）

`FAbstractExportProxy` 子类链式组合，各自处理单一前置条件，无需大型 if-else 逻辑块，扩展性强。

### 6.2 Pipeline 模式

`IFFExportManager → IFFExportPipeline → IFFExportJob` 三层任务图，Job 间声明依赖关系，VBL 层按 DAG 拓扑排序并发调度。

### 6.3 MVP 模式

进度面板：`FExportProgressPresenter`（逻辑）+ `IFExportProgressView`（接口）+ `FExportProgressDialog`（实现）三层解耦，进度 Presenter 通过接口驱动 View，View 可独立替换。

### 6.4 工厂方法

`FExportProcessPresenter::createExportProcessPresenter(type)` 按导出类型返回对应 Presenter（本地/社媒/DVD），调用方不依赖具体类。

### 6.5 观察者模式

`IFFExportManagerEventObserver`：`onProgressChanged / onStatusChanged / onStatusChangedInWorkThread` 三个回调解耦编码引擎与 UI，工作线程回调不切线程（用于低延迟埋点），主线程回调通过 Qt 信号自动切线程。

### 6.6 RAII 模式

`FExportGuard` 在整个导出生命周期内保护全局状态，析构时自动还原，无论正常完成、用户取消还是异常退出均不遗漏。

---

## 七、关键类职责速查

| 类名 | 职责 |
|---|---|
| `FAbstractExportProxy` | 责任链代理基类，持有下一节点，子类实现单一前置检查 |
| `FFormatSelectExportProxy` | 弹出格式选择面板，用户配置所有导出参数 |
| `FAppLicenseExportProxy` | 程序激活检查，试用版注入水印参数 |
| `FExportProgressProxy` | 责任链末端，创建 ProcessPresenter 触发真实导出 |
| `FExportDataDelegate` | 聚合全部导出参数，在责任链各层间传递 |
| `FExportDataManager` | 批量导出场景下多任务参数管理 |
| `FLocalProcessPresenter` | 本地/设备导出流程控制：构建 Pipeline、管理进度回调、GPU 降级 |
| `FSocialMediaProcessPresenter` | 社媒导出+上传流程控制，管理两阶段（编码→上传）状态机 |
| `FExportProgressPresenter` | 进度对话框 Presenter（MVP），将 VBL 进度转换为 UI 更新 |
| `FExportProgressDialog` | 进度对话框 View（`FFFramelessDialog`），含状态区 + 活动区 |
| `FProcessEncoder` | 子进程编码器，管理 FExportExe 生命周期、IPC 通信、双重保活 |
| `FExportExe` | 独立编码子进程，运行 VBL `IFFEncoder`，通过 IPC 与主进程通信 |
| `IFFExportManager` | VBL 导出管理器接口，管理所有 Pipeline 的生命周期与调度 |
| `IFFExportPipeline` | 单条时间线的编码 Pipeline，包含 Job 依赖图 |
| `IFFExportJob` | 单个导出任务（编码/AutoReframe/上传），有状态机和进度 |
| `FExportFormatInfo` | 从 XML 配置文件解析支持的格式、编码器、默认参数列表 |
| `FExportSettingDataMgr` | 导出参数持久化单例，保存/恢复用户上次设置 |
| `FExportGuard` | RAII 导出环境守卫，保护自动保存/颜色模式/AI 任务/License 刷新 |
| `FExportSettingSizeAndDurationPresenter` | 实时预估输出文件大小和时长 |
| `FExportProgressTrackingHelper` | 埋点助手，在工作线程直接回调以获取精确时间戳 |
