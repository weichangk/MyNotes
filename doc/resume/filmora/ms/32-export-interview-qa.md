32-export-interview-qa.md
# 导出模块面试问答

---

## 一、架构设计类

### Q1：导出模块整体架构是怎样的？为什么用责任链而不是 if-else？

**回答：**

导出模块分五层：
1. **触发层**：`FTimelineView` 等调用 `exportTimeline(FExportParam)`
2. **责任链代理层**：8 个 `FAbstractExportProxy` 子类串联，依次执行前置检查
3. **流程控制层**：`FLocalProcessPresenter / FSocialMediaProcessPresenter / FDVDProcessPresenter`，按导出类型分流
4. **VBL Pipeline 层**：`IFFExportManager → IFFExportPipeline → IFFExportJob` 三层任务图
5. **编码引擎层**：`FProcessEncoder`（独立子进程）+ `IFFEncoder`（线程模式降级）

使用责任链的原因：导出前置检查有 8 个独立条件（授权拉取、文件丢失、格式选择、版权检查、激活检查、执行导出、升级引导），每个条件逻辑独立，任意失败短路。如果写成 if-else，单个函数会膨胀到数百行，且每新增一个检查都要修改该函数（违反开闭原则）。责任链中每个 Proxy 只管一件事，新增检查只需插入新节点，移除检查只需从链中拆除，调整顺序只需改链接关系，修改互不影响。

---

### Q2：Pipeline 的任务依赖图是如何实现的？为什么不用简单的顺序执行？

**回答：**

`IFFExportManager` 内部维护了有向无环图（DAG）调度引擎。`addDependency(jobA, jobB)` 声明 A 依赖 B（B 完成才启动 A）。VBL 层按拓扑排序执行：没有依赖的 Job 立即启动，依赖已满足的 Job 等待依赖方完成后再启动。

典型依赖链：
```
encodeJob ─→ autoReframeJob ─→ uploadCloudJob
```

不用顺序执行的原因：当有多条时间线需要批量导出时，每条时间线有自己的 Pipeline，多个 Pipeline 之间完全独立，可以并发执行（Pipeline A 在编码的同时 Pipeline B 也可以编码）。若用顺序执行，10 个视频只能串行处理；DAG 调度器可以让多个 Pipeline 并发，充分利用多核 CPU/GPU，总耗时大幅缩短。同时 DAG 还支持"编码完成后才上传"这种跨 Job 的数据依赖，简单顺序执行无法表达这种逻辑。

---

### Q3：独立子进程编码是怎么实现的？为什么要用子进程而不是线程？

**回答：**

`FProcessEncoder` 启动 `FExportExe` 独立子进程，通过 IPC（进程间通信）传递编码参数和状态：

```
主进程发送：StartMsg（FFEncodeParam JSON 序列化）
子进程执行：IFFEncoder::start()，调用 VBL 编码引擎
子进程回传：ProgressMsg（进度）/ StateMsg（完成/错误）
主进程控制：PauseMsg / ResumeMsg / StopMsg
```

用子进程而不用线程的原因有三：
1. **崩溃隔离**：VBL 编码引擎（特别是 GPU 编解码路径）在某些机器上可能崩溃，崩溃若在线程中会导致整个主进程退出，用户丢失编辑状态；子进程崩溃只影响该编码任务，主进程捕获 `esExeCrash` 后自动降级线程模式重试。
2. **GPU 上下文隔离**：子进程独占 GPU 编码上下文，不与主进程的渲染进程竞争显存和 D3D/OpenGL 上下文。
3. **未来可扩展**：子进程模式天然支持远程编码（未来可将子进程替换为远程编码服务器），而线程模式只能本地执行。

---

## 二、核心机制类

### Q4：双重保活机制是怎么实现的？如何区分"慢速编码"和"子进程冻结"？

**回答：**

`FProcessEncoder` 维护两个定时器：

**启动超时**（`m_pProcressInitTimer`，120s）：
子进程启动后 120s 内未收到第一个 ProgressMsg，认为初始化失败，直接终止子进程并降级到线程模式。

**心跳监控**（`m_pMonitorTimer`，500ms 轮询）：
每 500ms 检查最新进度值，若 15s 内进度值未发生任何变化，认定为冻结（`esExeFreeze`），切换线程模式降级。

**"慢速编码"的问题**：4K HDR 首帧编码本身可能耗时 >15s，会被错误认定为冻结。这是当前实现的一个已知缺陷。理想方案是子进程主动发送心跳（每 3s 一次，即使进度不变也发送），主进程检测心跳超时（30s）而非进度停滞，两者语义不同：心跳超时说明子进程失去响应，进度停滞只是编码慢。

---

### Q5：GPU 降级是怎么触发的？降级后的行为是什么？

**回答：**

GPU 降级有两条触发路径：

**路径一：GPU 编码错误**（`reStartCloseGpu()`）：
```cpp
m_encodeParam.bGpu = false;  // 关闭 GPU 参数
stopCurrentProcess();         // 终止当前子进程
restartProcess();             // 用软编参数重启子进程
```
整个流程对进度对话框透明——进度条会短暂停顿（子进程重启约 1~2s），然后继续从 0% 重新编码（因为子进程是新的）。

**路径二：子进程崩溃/冻结**（`esExeCrash / esExeFreeze`）：
不重启子进程，直接切换到线程模式（`IFFEncoder` 直接在主进程线程中执行），以 `m_encodeParam.bGpu = false` 参数重新发起编码。

两种降级均保证用户不会看到"导出失败"弹窗（除非软编也失败），体验上是"导出变慢了"而非"导出失败了"。

---

### Q6：VBL 的进度回调是在哪个线程触发的？如何保证 UI 安全？

**回答：**

`IFFExportManagerEventObserver` 定义了两个语义不同的回调：

**`onStatusChangedInWorkThread()`**：在 VBL 工作线程直接回调（不切主线程）。用途：精确时间戳埋点——编码完成的确切时刻在工作线程记录，比切到主线程后再记录精确数十毫秒。`FExportProgressTrackingHelper` 实现该接口，只做数据记录（时间戳、状态码），不触碰任何 Qt 对象。

**`onProgressChanged()` / `onStatusChanged()`**：通过 Qt 信号槽 `QueuedConnection` 自动切主线程，`FLocalProcessPresenterPrivate` 实现该接口并转发到 `FExportProgressPresenter`，后者更新进度条和状态标签。

这种设计的核心原则：**需要精确时间戳的用工作线程直接回调，需要更新 UI 的走 Qt 信号自动切主线程**，两者职责清晰，互不干扰。

---

## 三、业务与产品类

### Q7：批量导出是如何管理多个任务的？失败了某一个怎么处理？

**回答：**

批量导出的数据由 `FExportDataManager` 管理，持有 `QList<FExportParamInfo>`（每个任务的时间线 + 授权资源 + 输出路径）。

执行时 `FLocalProcessPresenter` 为每条时间线创建一个 `IFFExportPipeline`，所有 Pipeline 加入同一个 `IFFExportManager`，VBL 层并发调度。每个 Pipeline 有独立的状态机（`FFExportStatus: Waiting/Busying/Completed/Failed`）。

某个 Pipeline 失败时：
1. `onStatusChanged(pipelineIdx, Failed)` 回调通知 Presenter
2. Presenter 更新该 Pipeline 对应的任务行状态为"Failed"（红色）
3. 其他 Pipeline 继续执行，不受影响
4. 全部执行完后，Presenter 统计失败任务列表，显示"X 个任务导出失败"提示
5. 用户可选择"重试失败任务"，只重新提交失败的 Pipeline，不重试已完成的

文件名冲突通过 `checkAndGetFileName()` 自动追加数字后缀（`video(1).mp4`），避免覆盖已有文件。

---

### Q8：水印是如何注入的？哪些情况会有水印？

**回答：**

水印通过 `FFWatermarkParam` 字段携带，在 `FAppLicenseExportProxy` 中根据授权状态设置：

```cpp
// 三种情况注入水印：
// 1. 试用版（未购买激活）
// 2. 素材版权未购买（背景音乐/贴纸等付费素材）
// 3. 特定付费功能（如 HDR/杜比音效）未购买但已使用
```

`FUnLicenseTimelineData` 在责任链运行前扫描整个时间线，收集所有未授权的媒体资源，`FFWatermarkParam` 携带水印类型（版权水印/试用水印/功能水印）和位置参数传递给 VBL 编码层，VBL 在渲染合成阶段将水印叠加到视频帧上，导出文件中包含永久水印。水印参数在编码参数中而非 UI 层处理，用户无法绕过。

---

### Q9：文件大小预估的精度如何？是实时计算的吗？

**回答：**

`FExportSettingSizeAndDurationPresenter` 使用公式：

```
预估大小 ≈ (video_bitrate + audio_bitrate) × duration ÷ 8
```

精度：**约 ±15%**。误差来源：
1. VBR 模式下实际码率随内容复杂度波动（运动多的场景码率更高）
2. 容器格式头部开销（MP4 box 结构约占 0.1~1%）
3. 封面帧和片尾视频增加的体积未计入

是实时计算的：用户调整码率 SpinBox、分辨率下拉框、时长裁剪区间时，`QTimer::singleShot(0)` 触发延迟计算（防止拖动 SpinBox 时每帧都计算），计算完成后立即更新预估标签。

社交媒体导出场景还有平台限制检查（如 TikTok 最大 1GB，YouTube Shorts 最大 60s），预估大小超过平台限制时，Widget 显示警告色并提示用户降低码率。

---

### Q10：格式配置是硬编码的还是可配置的？支持新格式怎么扩展？

**回答：**

格式配置来自 XML 文件（`Format_H.dat`），通过 `FExportFormatInfo::ParseFormatInfo()` 解析，不硬编码在代码中。每个格式条目包含：
- 格式名称和 FourCC
- 支持的编码器列表（`video_fourcc / audio_fourcc`）
- 各编码器的默认参数（分辨率列表、帧率列表、码率范围）
- 平台特定限制（如 DVD 只支持 MPEG-2，GIF 禁用音频）
- 功能开关（是否支持 HDR / GPU / 透明通道 / Dolby 等）

扩展新格式：在 XML 中新增格式条目，无需修改任何 C++ 代码。例如新增 AV1 格式：在 XML 中添加 `<format fourcc="MP4" videoFourcc="AV1" ...>`，设置默认参数，下次启动自动生效。VBL 层只需支持该编码器即可。这种数据驱动设计使运营/产品可以独立于开发发布格式配置更新。

---

### Q11：导出过程中用户关闭应用，会发生什么？

**回答：**

`FExportGuard` 在导出期间阻止自动保存（`FFProjectAutoSaveEnabledWraper`），确保工程状态不在导出中途被意外修改。

用户关闭应用时触发 `QCloseEvent`：
1. 主窗口检测到导出进行中，弹出"导出尚未完成，确定退出吗？"确认框
2. 用户确认退出：调用 `IFFExportManager::stop()` → 所有 Pipeline 发送 `StopMsg` → 子进程收到后调用 `IFFEncoder::stop()` → 子进程退出
3. 主进程等待子进程退出（超时 5s 后强制 `terminate()`）
4. `FExportGuard` 析构，恢复所有全局状态
5. 已编码的部分文件被删除（不留不完整文件）

子进程意外被系统 kill 时（如 OOM Killer），`FProcessEncoder` 的进程监控检测到进程退出，根据退出码判断是否需要清理临时文件，并向 UI 发送 `sigExportFailed(esDiskSpaceError / esAborted)` 信号提示用户。

---

### Q12：社交媒体上传的两阶段是如何设计的？上传失败怎么处理？

**回答：**

`FSocialMediaProcessPresenter` 管理两阶段状态机：

**阶段一：本地编码**
复用 `FLocalProcessPresenter` 的编码逻辑，生成本地临时文件，进度显示"Exporting... 0%~100%"。

**阶段二：平台上传**
编码完成后，Pipeline 中的 `UploadThirdPartyPlatformJob` 自动启动（依赖 encodeJob），调用平台 SDK（YouTube/TikTok/Vimeo 等各自 SDK），上传进度通过 Observer 回调更新 UI（显示"Uploading... 0%~100%"）。

**两阶段进度合成**：总进度 = 编码权重（70%）+ 上传权重（30%）加权合成，进度条连续增长不跳变。

**上传失败处理**：
- 网络中断：`UploadThirdPartyPlatformJob` 进入 `Failed` 状态，但本地临时文件保留
- UI 显示"上传失败，本地文件已保存到 [路径]"，给用户兜底
- 提供"重试上传"按钮，跳过编码阶段直接重新上传本地文件（节省重新编码时间）
- 若用户选择定时发布但网络失败，保存预约信息，下次联网时自动补传
