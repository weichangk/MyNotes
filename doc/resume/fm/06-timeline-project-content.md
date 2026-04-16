06-timeline-project-content.md
# 项目内容

负责时间线（Timeline）模块的开发与维护，该模块是 Filmora 编辑器的核心编辑区域，承载多轨视频、音频、特效等全类型轨道的可视化编排，用户在此完成全部剪辑操作并实时预览合成结果。

- 基于 Qt Graphics View Framework 深度定制时间线 UI，构建 **FGraphicsTimelineScene**（QGraphicsScene）管理全部轨道/片段图元，设计帧坐标系与像素坐标系的双向映射（`frameToScenePos / scenePosToFrame`），支持 0.5px ～ 80px/帧的缩放范围；UI 层通过 Observer 接口订阅 VBL 数据变更，保证数据与图元的一致性。

- 设计并实现 **Substitute（替身）拖拽机制**：拖拽移动/裁剪时构造轻量替身图元代替真实 ClipItem，全程不触碰 IFFClip 数据，鼠标释放时一次性 `commit` 到 VBL 层并写入 undo 栈；ESC 取消则销毁替身、零副作用；消除了逐帧修改数据层触发 VBL 回调的性能瓶颈，保证拖拽帧率流畅。

- 实现**吸附对齐系统**（SnapService）：实时扫描所有 Clip 入出点、Playhead、Marker 作为吸附候选点，拖拽时计算最近偏移量实现磁性对齐；主轨支持磁性排布，插入片段时 `rearrangeMainTrack()` 自动令相邻 Clip 位移让位。

- 对接 VBL 层 **Command 模式撤销/重做**：所有修改操作封装为 `IFFUndoableOp` 压栈，引入 `FF_DECLARE_UNDO_COMMANDER` 宏配合 `beginMacro / endMacro` 将多个子操作合并为单条 undo 记录；`FF_DECLARE_PAUSE_EVENT_COMMANDER` 在合并期间暂停 VBL 事件广播，杜绝 UI 中间状态的无效刷新。

- 负责**音频波形与视频缩略图异步生成**：音频波形由 `FAudioThumbnailWorker` 在独立 QThread 内解码 PCM 并绘制 QImage，`QMetaObject::invokeMethod` 跨线程触发，完成后回主线程更新缓存并局部刷新 ClipItem；缓存以 `clipId + 入出点 + 可视宽度` 为参数 Key，缩放或裁剪后参数变化时精准失效重建，参数不变时直接复用。

- 实施**时间线渲染性能优化**：引入冻结绘制模式（`openFreezeDrawMode`）在批量操作期间暂停 Scene 刷新、批量完成后统一重绘；可视区域裁剪保证只调度视口内图元参与 `scheduleRepaint`；ClipItem 缓存已渲染的缩略图/波形图像，数据不变时不重解码。
