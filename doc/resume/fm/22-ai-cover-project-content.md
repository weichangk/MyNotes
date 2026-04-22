22-ai-cover-project-content.md

# 项目内容

- 负责 **AI 封面制作模块**的设计与实现，基于 MVP 架构将 UI（`FCoverSettingDialog / FCoverRecommendationWidget / FCoverFromLocalView`）、业务编排（`FCoverSettingPresenter / FProjectCoverPresenter`）、VBL 适配层（`FFCoverRecommendationService / IFFCoverProject`）三层解耦，支持 AI 智能推荐、时间线截帧、本地图片上传三种封面来源，三路入口统一收敛到同一套封面保存逻辑。

- 设计并实现 **`FFCoverRecommendationService` VBL AI 推荐适配器**：私有实现类继承 `IVbVideoCoverRecommendCallback`，在 `generateCoverFinished(VBLInt index)` 回调中 `emit coverReady(index)` 将 COM 风格 VBL 接口转换为 Qt 信号，上层 Presenter 完全无感知 VBL 细节；`getCover(i)` 内部从 VBL 原始像素 buffer 拷贝构建 `QPixmap`，并通过 `VBL::frameFromCnsTime(frameRate, vblPos)` 将 CNS 时间单位转换为帧索引，保证多处帧计算一致性。

- 实现**本地图片双通道解码**：优先通过 `QImageReader::setAutoTransform(true)` + `scaledSize` 限制最大像素的 Qt 原生解码（防 OOM），Qt 失败时通过 `FFLocalMediaItemFactory::createTempLocalMediaItem` + `createBsThumbManager()->getThumbnailSync` 兜底解码，支持 HEIC/WEBP 等 Qt 原生不支持的格式；两路均通过 `QtConcurrent::run` 异步执行，配合 `CoverUtility::waitWithProgressDialog` 提供非阻塞加载进度反馈。

- 实现**封面模板应用假进度条（乐观 UI）**：VBL 模板应用接口不提供真实进度，`FProjectCoverEditDialog` 收到 `progress < 0` 时启动 `QTimer`（200ms 间隔）驱动假进度递增，收到 `progress >= 100` 时关闭；用户感知连续平滑进度条而非 0% 突跳到完成，同时进度对话框阻塞导出防止用户在模板未完成时提前操作。

- 实现**封面编辑临时 Timeline 隔离与工程持久化**：`FProjectCoverCache` 维护独立的临时编辑 timeline，封面编辑期间的文字/贴纸/模板修改不影响主工程 timeline，取消时直接丢弃无需 Undo；保存时通过 `IFFCoverProject::setCover + archive` 将封面写入工程包，`FFProjectCoverHelper` 负责临时目录（`app_temp/cover/{uuid}/`）管理与 `setProjectThumbnail` 写入 VBL DmProject。

- 实现**导出前付费模板资源检查**：`FProjectCoverPresenter::checkActivatedTemplate()` 在保存/导出前遍历已应用模板元素，对接 `FF_APP_LICENSE` 检查是否含未激活付费资源，若存在则阻断导出并提示用户购买或移除，防止付费内容未经授权被嵌入导出文件。
