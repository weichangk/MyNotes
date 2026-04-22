26-ai-highlight-project-content.md
# 项目内容

- 负责 **AI 精彩集锦（Auto Beat Sync）模块**的设计与实现，基于 MVP 架构将 UI（`FFAutoHighlightWidget / FFBeatSyncWidget / FFBeatVideoEffectWidget / FFMontageWidget`）、业务编排（四个 Presenter）、VBL 适配层（`FFAutoHighlightMontage`）三层解耦，`FFAutoHighlightMontageViewPrivate` 作为总协调者聚合四个 Presenter，实现素材管理、BGM 配置、特效权重、AI 分析四大功能域的独立演化；使用 PIMPL 模式封装主对话框，对外头文件零私有成员暴露。

- 设计并实现**双层 Adapter 接口适配**：`FFAutoHighlightMontage` 同时实现 `IFFAutoHighlightMontage`（FF 层纯虚接口，供 Presenter 调用）与 `VBL::IVblCallBack`（VBL 回调接口），在 `processChanged(status, data)` 中将 VBL 状态码（`ctsProcessing / ctsFINISH / ctsERROR / ctsUSER_STOP`）分发到 `IFFAutoHighlightMontageEventObserver` 的四个语义方法，VBL 引擎替换不影响任何上层代码；参数通过 `Property` 结构体（union 值 + ptInt/ptLonglong 类型标记）以键值方式传递，时间单位统一使用百纳秒（CNS）保证音视频精度。

- 实现 **VBL 异步回调的主线程安全切换**：VBL AI 引擎在工作线程触发 `IVblCallBack::processChanged`，`FFMontagePresenter` 通过 `QMetaObject::invokeMethod(this, "updateProgress", Qt::AutoConnection, Q_ARG(int, progress))` 将进度更新切回主线程；`onExtractFinished` 在 VBL 线程直接调用 `generateTimeline()`（纯数据操作），再通过 Qt 信号（自动 `QueuedConnection`）将完成事件投递主线程，分析完成后延迟 500ms 关闭进度对话框避免 UI 闪烁。

- 实现**分段进度反馈与分析按钮三状态机**：`FProgressComposeHelper` 按进度值 30% 为阈值切换 Step 1（"Analyze beats and rhythm"）和 Step 2（"AI highlights best moments"）提示文字，为用户提供语义化阶段感知；`AnalysisBtnState_e` 三状态机（`ZERO_TIMES / AND_KEEP / BUT_EDIT`）通过 `sigEditOptionChanged` 信号监听全部参数变化（素材/BGM/节拍/特效权重），参数变化后强制提示重新分析，防止用户使用过期结果导出。

- 实现**双线程异步媒体处理**：`AutoHighLightImportHelper`（`QThread`）逐文件异步导入媒体并通过信号回传进度；`ThumbnailHelper`（`QThread`）维护生产消费队列（`QMutex` 保护 `m_todo`），异步提取各素材首帧缩略图并逐项填充列表 UI；`FFBeatSyncPresenter` 通过 `createAudioWaveService()` 异步提取 PCM 样本，使用 `QPolygonF` 对称双路波形 + `QPainterPath::fillPath` 绘制主题自适应波形图。

- 实现**分析结果导出到时间线的跨模块解耦**：`generateTimeline()` 将 VBL 生成的 `IDmTimeline` 通过 `FFTimelineBuilderPrivate` 转换为三轨 `IFFTimeline`（BGM 轨 / 视频片段轨 / 特效轨）；导出时通过 `sendMessageToScene(&FFTimelineAddHighLightMontageMediaMessage)` 消息总线将克隆后的 Clip 列表传递给时间线模块，时间线模块不依赖分析模块任何具体类型；素材同步通过 `addMediaItems` 注册到项目媒体库，保证时间线引用合法性。
