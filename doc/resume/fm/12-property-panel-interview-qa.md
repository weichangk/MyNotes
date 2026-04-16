12-property-panel-interview-qa.md
# 属性面板模块面试问答

---

## 一、架构设计类

### Q1：属性面板支持十余种 Clip 类型，如何做到扩展新类型不修改现有代码？

**回答：**

通过**注册式 Builder 工厂**实现。每个属性子面板在自己的 `.cpp` 文件末尾用静态宏自注册：

```cpp
RegisterPropertyBuilderByType(VideoBasic, FPropertyBasicTransformWidgetBuilder);
RegisterPropertyBuilderByType(Color,      FPropertyColorBasicLightWidgetBuilder);
```

宏本质上是静态对象的构造函数在程序启动时向 `FPropertyWidgetFactory` 单例插入 `FFPropertyType → Builder` 映射。主框架 `FPropertyView` 只调用 `FPropertyWidgetFactory::buildersForType(type)` 获取 Builder 列表并遍历构建，完全不感知具体子面板的存在。

**新增属性类型的步骤**：实现新的 `FPropertyBaseWidget` 派生类 + 对应 `Builder`，在 `.cpp` 末尾加一行注册宏，其余代码零修改。这是**开闭原则**的典型实践。

---

### Q2：`FPropertyControlManager` 与 `FPropertyControl` 的职责如何划分？

**回答：**

两者分工明确：

- **`FPropertyControlManager`（调度层）**：实现多种外部 Observer 接口（Timeline / Playhead / UndoRedo / Project），负责"感知外部变化、决策是否切换 Control 或触发刷新"。它不直接操作 VBL，只管理当前激活的 `FPropertyControl` 实例，并通过 `notifyUpdateData(UpdateReasons)` 驱动 UI 刷新。

- **`FPropertyControl`（业务层）**：负责对 VBL 的实际读写（`IFFEffect::setPropertyValue / keyFrameService()->setKeyFrameValue`）、Undo 事务管理、关键帧操作全套 API。它不感知 UI 控件，只提供"属性读写"接口供上层调用。

**分工本质**：Manager 管"何时刷新、展示什么"，Control 管"怎么读写数据"，两者通过 Observer 接口解耦。

---

### Q3：属性面板在选中不同 Clip 时如何避免 Widget 树频繁重建？

**回答：**

使用**类型级别的 Widget 缓存**（`d.listViews`，以 `FFPropertyType` 为 Key）：

- 首次展示某类型时，调用 `createPropertyListView(type)` 完整构建 Widget 树并缓存
- 再次切换到同类型时，直接取缓存的视图，只调用 `refreshData()` 更新控件值，不重建任何 Widget

这使得"视频 Clip ↔ 视频 Clip"之间的切换开销极低（只有数据读取），只有"视频 ↔ 音频"等跨类型切换才会触发构建（且只构建一次）。

---

## 二、批量编辑与多值处理类

### Q4：多选多个 Clip 时，属性面板如何展示和处理"多值"状态？

**回答：**

通过模板类 `FMultipleValue<T>` 统一抽象：

```cpp
template<typename T>
struct FMultipleValue {
    T value;         // 第一个 Clip 的值（或代表值）
    bool isMultiple; // 是否存在多值
    bool isValid;    // 是否有效
};
```

**读取阶段**：`FPropertyBatchGetter` 遍历所有选中 Clip，逐一读取属性值，若发现不同则返回 `isMultiple = true`。UI 控件检查该标志：
- `isMultiple = false`：显示实际值，正常可编辑
- `isMultiple = true`：输入框置空、Slider 置灰，提示用户当前为多值状态

**写入阶段**：用户修改控件后，`FPropertyBatchSetter` 将新值写入所有选中 Clip，多值状态自动消除。整个过程合并为一条 Undo 记录、一次 UI 刷新。

---

### Q5：批量修改多个 Clip 的属性时，如何保证 Undo 只记录一条、UI 只刷新一次？

**回答：**

核心是两个 RAII 宏的配合使用：

```cpp
void FPropertyBatchSetter::set(const QList<IFFClip*>& clips, const T& value) {
    FF_DECLARE_PAUSE_EVENT_COMMANDER(m_timeline); // ① 暂停 VBL 事件广播
    FF_DECLARE_UNDO_COMMANDER(m_undoSrv);         // ② 开启 Undo 事务（beginMacro）

    for (auto* clip : clips)
        doSetValue(clip, value);  // 写 N 个 Clip，期间无事件广播

    // ③ 两个 commander 按构造逆序析构：
    //    UNDO_COMMANDER 析构 → endMacro，合并为一条 Undo
    //    PAUSE_COMMANDER 析构 → 恢复广播，触发一次统一通知
}
```

两个宏利用 C++ RAII 的作用域析构顺序保证：无论 `set()` 中途是否抛出，事务都能正确关闭。UI 层调用 `set()` 无需关心任何事务细节。

---

## 三、关键帧联动类

### Q6：属性修改是否写关键帧，是怎么决定的？

**回答：**

`FPropertyControl::setEffectPropertyValue()` 内部做策略路由：

```cpp
void setEffectPropertyValue(const FEffectPropertyBaseInfo& info,
                            const FEffectPropertyValue& value,
                            IFFClip* clip) {
    if (isKeyFrameEnabled(info, clip)) {
        // 有关键帧 → 写当前 Playhead 帧位置的关键帧值
        clip->keyFrameService()->setKeyFrameValue(
            m_playhead->currentFrame(), info, value);
    } else {
        // 无关键帧 → 写静态属性值
        clip->effect()->setPropertyValue(info, value);
    }
}
```

这个路由对 UI 层（Presenter/Widget）完全透明——UI 只管调用 `setEffectPropertyValue()`，是否走关键帧完全由 Control 内部决定。用户感知到的区别只是：启用关键帧后，同一个控件的修改会在时间线上生成关键帧图标。

---

### Q7：播放头移动时，属性面板如何同步更新各控件的值？

**回答：**

`FPropertyControlManager` 实现 `IFFPlayheadEventObserver`：

```cpp
void onPlayheadChangedEvent(qlonglong frame) {
    // 安全切回主线程
    QMetaObject::invokeMethod(this, [this, frame]() {
        notifyUpdateData(UpdateReasons::PlayheadMoved);
    }, Qt::QueuedConnection);
}
```

`notifyUpdateData(PlayheadMoved)` 广播后，每个 `FPropertyBaseWidget::refreshData()` 判断原因包含 `PlayheadMoved` 后，从 VBL 重新读取**当前帧的插值属性值**（VBL 渲染引擎按关键帧曲线插值计算）并更新控件。同时，各属性行的关键帧按钮检查当前帧是否有关键帧，有则高亮，无则正常显示。

---

## 四、Qt 技术类

### Q8：VBL 回调可能在非主线程触发，属性面板如何保证 UI 安全？

**回答：**

`FPropertyControlManager` 在所有 VBL Observer 回调的入口处统一使用 `QMetaObject::invokeMethod` 切换到主线程：

```cpp
void onTimelineChangedEvent(const FFTimelineChangeEvent& event) {
    // VBL 回调可能在任意线程
    QMetaObject::invokeMethod(this, [this, event]() {
        // 以下逻辑保证在主线程执行
        processTimelineChange(event);
    }, Qt::QueuedConnection);
}
```

`FPropertyControlManager` 构造时记录 `d.mainThread = QThread::currentThread()`，关键路径上通过 `Q_ASSERT(QThread::currentThread() == d.mainThread)` 做线程一致性断言，在开发阶段捕获线程安全问题。

---

### Q9：`FPropertyGroup` 的折叠/展开是如何实现的？折叠状态如何跨会话持久化？

**回答：**

**折叠实现**：`FPropertyGroup` 由 Header（`FPropertyGroupHeader`）和 Body（`FPropertyBaseWidget`）组成。折叠时调用 `body->hide()`，Header 保持显示和可交互；展开时 `body->show()`。布局层通过 `QWidget::adjustSize()` 或 `QSizePolicy` 自动收紧/撑开高度，`QListWidget` 对应的 `QListWidgetItem` 更新 `sizeHint` 响应高度变化。

**持久化**：

```cpp
// 加载（FPropertyView 构造时）
void loadDefaultFoldStates() {
    for (auto* group : m_groups) {
        bool folded = QSettings().value("fold/" + group->groupKey(), false).toBool();
        group->setFold(folded);
    }
}

// 保存（FPropertyView 析构时）
void saveDefaultFoldStates() {
    for (auto* group : m_groups)
        QSettings().setValue("fold/" + group->groupKey(), group->isFolded());
}
```

以 `groupKey`（通常是 `groupType` 的字符串表示）为 Key 存储，不同 Clip 类型的同名属性组共享折叠状态（符合用户预期：折叠了"基础变换"组，下次打开任何 Clip 的基础变换组都默认折叠）。

---

## 五、业务与产品类

### Q10：属性面板如何处理 AI 特效参数需要后台算法预处理的场景？

**回答：**

通过 `FAlgorithmCacheQueueWorker` 异步队列处理：

1. 用户选中含 AI 特效的 Clip，`FPropertyBaseWidget::init()` 查询 `FAlgorithmCacheQueueWorker::getStatus(clipId)`
2. 若状态为 `Pending/Processing`，属性面板显示进度条占位，禁用相关控件
3. `FAlgorithmCacheQueueWorker` 在后台 QThread 执行算法预处理（模型推理/特征提取），定期 `emit sigAlgorithmCacheProgressChange(progress)` 更新进度条
4. 完成后 `emit sigAlgorithmCacheStatusChange(Completed)` → 主线程 slot 收到 → 隐藏进度条，展示可交互的属性控件

**快速切换 Clip 的处理**：用户切换 Clip 时旧任务通过 token 机制失效（见优化文档），避免旧 Clip 的任务完成后错误刷新新 Clip 的 UI。

---

### Q11：如果用户在拖动 Slider 调节参数时频繁撤销，会有什么问题？你是如何解决的？

**回答：**

**问题**：如果拖动 Slider 的每次 `valueChanged` 都写 VBL 并记录 Undo，用户拖动一次产生数十条 Undo 记录，Ctrl+Z 只能一格一格撤销，无法回到拖动前的状态，体验极差。

**解决方案**：区分"实时调节"和"完成态"：

```cpp
// 鼠标按下（开始拖动）
void onSliderPressed() {
    m_control->startRealTimeUpdate(m_info);
    // 记录"拖动前"的值，开启 Undo macro
    m_undoMacro = new FUndoMacroScope(m_undoSrv);
}

// 拖动中（高频触发）
void onSliderValueChanged(float value) {
    // 写 VBL 但不生成新 Undo 记录（在 macro 内）
    m_control->setEffectPropertyValueRealTime(m_info, value);
    // 只驱动预览窗刷新，不触发其他模块的完整更新
}

// 鼠标释放（拖动结束）
void onSliderReleased() {
    m_control->finishRealTimeUpdate(m_info);
    delete m_undoMacro; // endMacro：将整个拖动合并为一条 Undo
}
```

整个拖动过程合并为一条 Undo 记录，Ctrl+Z 直接回到拖动前的值，符合用户直觉。

---

### Q12：多选不同类型的 Clip（如同时选中视频和贴纸），属性面板如何处理？

**回答：**

`FPropertySelector::clipTypes()` 返回所有选中 Clip 的类型集合。当类型集合包含多种不兼容类型时：

1. `FPropertyWidgetFactory` 根据类型集合查询**各 Builder 的 `supportedClipTypes()` 掩码**，只展示"所有选中类型都支持"的属性组（取交集）。例如"基础变换"支持视频和贴纸，可以显示；"色彩调校"只支持视频，不显示。

2. 可编辑的公共属性以正常方式展示（支持批量多值逻辑）；不公共的属性组整体隐藏，避免误操作。

3. 如果选中类型完全不兼容（如视频+字幕），面板展示空状态提示"已选中多种不兼容的素材类型"，引导用户单独编辑。

这保证了多选操作下属性面板不会展示无意义或危险的属性，用户可以安全地批量修改公共参数。
