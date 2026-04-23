18-ai-long-to-short-project-content.md

# 项目内容

- 负责 **AI 长视频转短视频模块**的设计与实现，基于 MVP 架构将 UI（`FAILongToShortDialog / ResultWidget`）、业务编排（`FAILongToShortPresenter`）、AI 任务服务（`IFFAILongToShortTaskService`）三层解耦，Presenter 通过信号驱动视图，完全屏蔽 AI 推理细节，支持本地文件、云盘媒体、在线 URL 三种视频来源接入。

- 设计并实现**三阶段串行 AI 任务流水线**（语种检测 → 视频转码 → 内容理解与脚本生成），`FAILongToShortPresenterPrivate` 实现 `IFFAIServiceEventObserver` 接收 VBL Task Manager 的异步状态回调，按 `taskType` 分发到三个 handler，每阶段完成后自动触发下一阶段（Transfer 完成后取 `fileId` 触发 Generate），严格保证数据依赖关系，AI 推理细节由 `FFAILongToShortTaskService` 适配封装，上层对本地/云端推理引擎无感知。

- 实现 **`FProgressComposeHelper` 多任务进度加权合成**：将语种检测（10%）、转码（30%）、生成（60%）三个子任务的原始进度按权重线性合成统一进度值，通过单一 `sigTaskProgressChanged` 信号驱动进度条，进度始终连续增长不发生跳变，用户体验流畅。

- 实现**多来源视频导入与云盘异步下载集成**：本地文件直接填充任务参数；URL 来源通过 `queryDownloadUrl` 调用 VBL HTTP 服务解析真实地址，`QMetaObject::invokeMethod` 切回主线程后触发导入；云盘媒体通过 `FF_CLOUD_DISK->cloudDiskViewPresenter()` 异步下载，`m_downloadTaskIds` 集合追踪进行中任务，`downloadFinished` 信号回调后无缝衔接本地导入流程；三路来源统一收敛到同一套任务参数填充逻辑。

- 实现**结果编辑与批量导出流程**：生成完成后通过 `getGeneratedScripts(taskId)` 获取多个候选短视频脚本（含 AI 评分、字幕、时间线片段）；单个编辑通过 `FF_AI_LONG_TO_SHORT_EDITOR->setScript(script)` 打开脚本工程进入编辑器；批量导出时动态修改工程配置（宽高比 9:16、分辨率 1080×1920），`emit sigExport(scriptList)` 触发导出管线，导出完成后恢复原始工程配置；历史任务结果通过 `QtConcurrent::run` 异步加载，避免脚本文件 IO 阻塞主线程。
