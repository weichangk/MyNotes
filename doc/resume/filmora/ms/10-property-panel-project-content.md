10-property-panel-project-content.md

# 项目内容

负责属性面板（Property Panel）模块的开发与维护，该模块是 Filmora 编辑器的参数编辑入口，当用户在时间线选中 Clip 后动态展示并编辑该 Clip 的全部属性（变换、色彩、音频、特效、蒙版、关键帧等），需同时支持十余种 Clip 类型的差异化渲染，且所有修改须与撤销/重做及关键帧系统深度联动。

- 设计并实现**注册式 Builder 工厂**动态构建属性 UI：各属性子面板以 `RegisterPropertyBuilderByType` 宏静态自注册到 `FPropertyWidgetFactory`，主框架按 `FFPropertyType` 查询 Builder 列表按需构建 `FPropertyGroup + FPropertyBaseWidget`；新增属性类型零侵入现有代码；构建结果按类型缓存（`d.listViews`），切换同类型时只执行 `refreshData()` 更新值，避免重复构建 Widget 树。

- 实现**多 Clip 批量编辑框架**：以模板类 `FPropertyBatchGetter / FPropertyBatchSetter` 统一批量读写，读取时聚合多 Clip 值并检测是否多值（`FMultipleValue<T>::isMultiple`），UI 自动呈现"多值"状态；写入时内嵌 `FF_DECLARE_PAUSE_EVENT_COMMANDER`（暂停 VBL 事件广播）与 `FF_DECLARE_UNDO_COMMANDER`（开启 Undo 事务），无论修改多少 Clip，都合并为一次 UI 刷新和一条 Undo 记录，UI 层无需关心事务管理。

- 完善**属性修改与关键帧系统的自动路由**：`FPropertyControl::setEffectPropertyValue()` 根据当前是否启用关键帧自动选择写入路径——有关键帧时调用 `keyFrameService()->setKeyFrameValue(frame, value)` 写当前帧，无关键帧时调用 `IFFEffect::setPropertyValue()` 写静态值；监听 `IFFPlayheadEventObserver` 在播放头移动时自动刷新各控件为当前帧的插值属性值并高亮关键帧按钮，实现属性面板与时间线关键帧编辑的无缝联动。

- 区分**实时调节与完成态**的处理策略：Slider 拖动期间调用 `startRealTimeUpdate()` 进入实时模式，暂停部分昂贵后处理只驱动预览窗刷新；鼠标松开时调用 `finishRealTimeUpdate()` 触发完整刷新并写入 Undo 记录，保证拖动过程中预览流畅且 Undo 栈不产生大量中间记录。

- 引入 **`UpdateReasons` 位掩码精细化刷新控制**：`notifyUpdateData(UpdateReasons)` 向各 `FPropertyBaseWidget` 广播刷新原因（ClipChanged / PlayheadMoved / UndoRedo / EffectChanged 等），各 Widget 按需判断是否响应本次刷新，避免任意变更触发全量重刷；`FPropertyControlManager` 统一实现 `IFFTimelineEventObserver / IFFPlayheadEventObserver / IFFUndoRedoServiceEventObserver` 三类观察者，通过 `QMetaObject::invokeMethod(..., Qt::QueuedConnection)` 保证所有 VBL 回调安全切换至主线程处理。
