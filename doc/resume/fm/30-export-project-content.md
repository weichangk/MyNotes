30-export-project-content.md
# 项目内容

- 负责**导出模块**的设计与实现，采用**责任链（Proxy Chain）**驱动八级前置检查（资源授权 → 文件丢失 → 格式选择 → 素材版权 → 程序激活 → 导出执行），每个 `FAbstractExportProxy` 子类封装单一检查逻辑，通过 `m_pNextExportProxy` 链式组合，任意环节失败短路不影响其他节点，新增检查步骤只需插入新 Proxy，符合开闭原则；`FExportGuard` RAII 对象在整个导出生命周期内保护自动保存、颜色对比模式、AI 算法缓存、License 刷新四个全局状态，任何退出路径均自动还原，无资源泄漏。

- 设计并实现 **Pipeline 任务依赖图（DAG）**：`FLocalProcessPresenter` 通过 `IFFExportManager` 为每条时间线构建 `IFFExportPipeline`，在 Pipeline 内声明 Job 依赖（`addDependency(autoReframeJob, encodeJob)` 确保 AutoReframe 在 Encode 完成后执行，`uploadJob` 依赖 AutoReframe），VBL 层按拓扑排序并发调度独立 Job，支持 Encode / AutoReframe / UploadCloudDisk / UploadThirdPartyPlatform 四类 Job 组合，批量导出场景下多个 Pipeline 并发执行，最大化吞吐量。

- 实现**独立子进程编码隔离**（`FProcessEncoder` + `FExportExe`）：编码引擎运行在独立子进程中，主进程通过 IPC 发送序列化的 `FFEncodeParam JSON` 启动编码，双重保活机制——启动超时定时器（120s）监控子进程初始化，心跳监控定时器（500ms 轮询，15s 无进度认定冻结）自动切换线程模式降级；GPU 编码失败时 `reStartCloseGpu()` 将 `bGpu = false` 后原地重启子进程以软编模式继续，用户无感知；子进程崩溃（`esExeCrash`）同样自动降级到线程模式重试。

- 实现 **GPU 硬件加速多厂商适配与软编降级**：`FFEncodeParam::bGpu` 控制 GPU 开关，VBL 层按显卡类型选择 NVENC（NVIDIA）/ AMF（AMD）/ QSV（Intel），`FExportSettingDataMgr` 检测驱动可用性决定 UI 入口是否展示；GPU 错误时 `reStartCloseGpu()` 无缝切换软编，保证导出任务在任何硬件环境下均能完成；`FFBsEncodeParam` 携带 `video_fourcc`（H264/HEVC/AV1）、`video_bitrate_mode`（VBR/CBR/ABR）、`video_colorspace`（SDR/HDR）等 VBL 层参数，覆盖主流编码场景。

- 实现**进度管理与线程安全回调**：`IFFExportManagerEventObserver` 定义两个回调——`onStatusChangedInWorkThread` 在 VBL 工作线程直接执行（用于精确时间戳埋点，零延迟），`onProgressChanged` 通过 Qt 信号 `QueuedConnection` 自动切主线程后更新进度 UI；进度对话框（MVP：`FExportProgressPresenter` + `FExportProgressDialog`）的活动区（`IFExportProgressActivityWidget`）在导出等待期间按运营策略切换展示 NPS 评分、付费引导、裂变活动等内容，导出等待时间被充分利用。

- 实现**多格式多场景完整覆盖**：本地导出支持 MP4/MOV/AVI/GIF/MP3/WAV，GIF 特化 `gif_quality_mode`（调色板/抖动算法）并禁用音频轨，MP3/WAV 禁用视频轨走纯音频流；社交媒体导出经编码→上传两阶段状态机，支持定时发布；`FExportSettingSizeAndDurationPresenter` 实时预估输出文件大小（`(video_bitrate + audio_bitrate) × duration ÷ 8`），用户调整码率即时反馈，辅助质量与体积决策。
