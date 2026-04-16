09-property-panel-technical-details.md

# 属性面板模块技术实现细节

> 项目：Wondershare Filmora（C++17 + Qt 5.15）
> 模块：属性面板（FPropertyPanelView）

---

## 一、模块概述

属性面板（Property Panel）是 Filmora 编辑器的参数编辑入口，当用户在时间线上选中 Clip 后，面板动态展示该 Clip 的全部可编辑属性（位置/旋转/缩放、色彩调校、音频增益、特效参数、蒙版、关键帧曲线等），用户在此完成参数调整并实时预览效果。面板需要同时支持视频、音频、文字、贴纸、特效等十余种 Clip 类型的差异化渲染，且所有修改须与撤销/重做、关键帧系统深度联动。

模块主要分为以下层次：

| 层次 | 路径 | 职责 |
|---|---|---|
| `FPropertyPanelView`（UI + 控制） | `Src/FPropertyPanelView` | 属性控件、分组组织、工厂构建、事件调度 |
| `FPropertyControl`（业务逻辑） | `Src/FPropertyPanelView` | VBL 读写、关键帧操作、撤销/事件合并 |
| `FFVBLModel`（数据层，外部） | `3rdparty/FilmoraFrameworkPlatform` | `IFFClip / IFFEffect / IFFUndoRedoService` |

---

## 二、整体架构

属性面板采用 **分层 MVP + 工厂动态构建** 架构：

```
┌─────────────────────────────────────────────────────────────┐
│                       View 层（UI）                           │
│  FPropertyView（主面板，含 Tab/ListWidget）                    │
│  FPropertyGroup（可折叠组 = Header + Body）                   │
│  FPropertyBaseWidget（组 Body 基类，实现 Observer 接口）        │
│  FPropertyComponent（单行属性：Label + 控件 + 关键帧按钮）      │
│  各类具体控件：Slider / ColorPicker / Combo / Toggle / LUT... │
└──────────────────────────┬──────────────────────────────────┘
                           │ Presenter 转发 / Observer 回调
┌──────────────────────────▼──────────────────────────────────┐
│                  Presenter / 调度层                            │
│  FPropertyControlManager（事件汇聚，决定激活哪个 Control）      │
│  FPropertySelector（封装当前选中 Clip 状态）                    │
│  IFPropertyPresenter / FPropertyPresenterImpl（控件↔Control 桥）│
│  FPropertyWidgetFactory（Builder 注册 + 按需构建 UI 树）         │
└──────────────────────────┬──────────────────────────────────┘
                           │ IFFClip / IFFEffect 接口调用
┌──────────────────────────▼──────────────────────────────────┐
│                  FPropertyControl（业务核心）                   │
│  FPropertyVideoControl / FPropertyColorControl               │
│  FPropertyAudioControl / FPropertyTitlesControl ...          │
│  FPropertyBatchGetter / FPropertyBatchSetter（模板，批量读写）  │
└──────────────────────────┬──────────────────────────────────┘
                           │ VBL 接口
┌──────────────────────────▼──────────────────────────────────┐
│              VBL 底层数据层（外部子模块）                       │
│  IFFClip / IFFEffect / IFFTimeline                           │
│  IFFUndoRedoService（撤销/重做服务）                           │
└─────────────────────────────────────────────────────────────┘
```

---

## 三、主业务流程

### 3.1 选中 Clip → 属性面板动态构建与展示

```
时间线 Clip 选中变更
       │  VBL 触发 IFFTimelineEventObserver 回调
       │
FPropertyControlManager::onTimelineChangedEvent()
       │  QMetaObject::invokeMethod(..., Qt::QueuedConnection) 调度至主线程
       │
FPropertySelector 更新选中 Clip 列表
       │  getSingleClip() / clipTypes() 分析选中类型
       │
FPropertyControlManager::switchPropertyView(FFPropertyType)
       │
FPropertyView::createPropertyListView(type)
       │  查询 d.listViews 缓存 → 命中则直接切换显示
       │  未命中 → FPropertyWidgetFactory::buildersForType(type)
       │           → 遍历 Builder 列表，逐一 builder->build(initData, parent)
       │           → 创建 FPropertyGroup + FPropertyBaseWidget 并 init()
       │           → 结果缓存到 d.listViews[type]
       │
每个 FPropertyBaseWidget::init(FPropertyInitData)
       │  向 FPropertyControl 注册 IFPropertyControlObserver
       │  通过 FPropertyBatchGetter 从 VBL 读取当前值
       │  检测多选多值（FMultipleValue<T>::isMultiple），UI 显示"多值"状态
       │  填充控件初始值，完成展示
```

**缓存策略**：每种 `FFPropertyType` 的 UI 树构建一次后缓存，切换回同类型时直接显示缓存视图，只执行 `refreshData()` 更新值，避免重复创建 Widget 树的开销。

### 3.2 用户修改属性 → 写回 VBL → 实时预览

```
控件事件（valueChanged / clicked）
       │
IFPropertyPresenter::onValueChanged(value)
       │  格式化/校验 → 封装为 FEffectPropertyData
       │
FPropertyControl::setEffectPropertyValue(info, value, clip)
       │  ┌────────────────────────────────────────────────┐
       │  │  FF_DECLARE_PAUSE_EVENT_COMMANDER(timeline)     │
       │  │  → 暂停 VBL 事件广播                            │
       │  │  FF_DECLARE_UNDO_COMMANDER(undoSrv)             │
       │  │  → 开启 Undo 事务                               │
       │  │                                                 │
       │  │  检查是否启用关键帧：                             │
       │  │  ├── 有关键帧 → IFFClip::keyFrameService()->    │
       │  │  │              setKeyFrameValue(frame, value)  │
       │  │  └── 无关键帧 → IFFEffect::setPropertyValue(..) │
       │  │                                                 │
       │  │  commander 析构：提交 Undo 记录 + 恢复事件广播    │
       │  └────────────────────────────────────────────────┘
       │
VBL 发出 IFFTimelineEventObserver 回调
       │  ├── FPropertyControlManager 收到 → notifyUpdateData(UpdateReasons)
       │  │   → 各 FPropertyBaseWidget::refreshData()（按需局部刷新）
       │  │
       │  └── 播放器收到 → IFFTimelineRender 重渲染当前帧 → 预览窗更新
```

### 3.3 多 Clip 批量编辑

```
用户框选多个 Clip → FPropertySelector::clipList() 返回多 Clip 列表
       │
FPropertyBatchGetter<Target, T>::get(clips)
       │  遍历所有 Clip，读取各自属性值
       │  若值不一致 → FMultipleValue<T>{ isMultiple=true }
       │  → UI 控件显示"多值"状态（输入框空白 / 滑条灰化）
       │
用户修改控件值
       │
FPropertyBatchSetter<Target>::set(clips, value)
       │  FF_DECLARE_PAUSE_EVENT_COMMANDER（暂停所有 Clip 的事件）
       │  FF_DECLARE_UNDO_COMMANDER（合并为一条 Undo 记录）
       │  for each clip:
       │      IFFEffect::setPropertyValue(clip, value)  // 批量写入
       │  commander 析构 → 一次性提交 + 统一刷新
```

### 3.4 关键帧联动

```
用户点击属性行的关键帧按钮（启用关键帧）
       │
FPropertyControl::setKeyFrameEnabled(info, true)
       │  IFFClip::keyFrameService()->enableKeyFrame(info)
       │  在当前 Playhead 帧位置写入初始关键帧
       │
播放头移动（IFFPlayheadEventObserver 回调）
       │
FPropertyControlManager::onPlayheadChangedEvent(frame)
       │  notifyUpdateData(UpdateReasons::PlayheadChanged)
       │  → 各 Widget::refreshData() 读取当前帧的插值属性值并更新控件
       │  → 关键帧按钮高亮（当前帧有关键帧时）
       │
用户在属性控件修改值（此时关键帧已启用）
       │  FPropertyControl::setEffectPropertyValue(...)
       │  → 走关键帧写入分支（keyFrameService()->setKeyFrameValue）
       │  → 时间线关键帧图元更新（Timeline Observer 回调）
```

### 3.5 AI 算法特效属性的异步处理

```
用户选中含 AI 算法特效的 Clip（如 AI 降噪/面部增强）
       │
FPropertyBaseWidget::init() 检测算法缓存状态
       │  FAlgorithmCacheQueueWorker::getStatus(clipId)
       │  ├── 已缓存 → 直接展示属性控件
       │  └── 未缓存 → 提交后台缓存任务，UI 显示进度条
       │
FAlgorithmCacheQueueWorker（后台 QThread）
       │  执行算法预处理，定期 emit sigAlgorithmCacheProgressChange(progress)
       │
主线程 slot 收到进度信号 → 更新进度条
       │
emit sigAlgorithmCacheStatusChange(Completed)
       │  → 属性面板隐藏进度条，展示可交互的属性控件
```

---

## 四、核心技术点

### 4.1 Factory + Builder 动态构建属性 UI

属性面板支持十余种 Clip 类型，每种类型的属性 UI 完全不同。采用 **注册式 Builder** 模式实现 UI 的按需构建与扩展：

```cpp
// 在各 Widget 的 .cpp 文件末尾静态注册
RegisterPropertyBuilderByType(VideoBasic, FPropertyBasicTransformWidgetBuilder);
RegisterPropertyBuilderByType(Color,      FPropertyColorBasicLightWidgetBuilder);
RegisterPropertyBuilderByType(Audio,      FPropertyAudioBasicWidgetBuilder);
// ...（每个子面板自注册，主框架无需感知）
```

`FPropertyWidgetFactory::buildersForType(FFPropertyType)` 返回该类型的 Builder 列表，`FPropertyView` 遍历列表构建 `FPropertyGroup + FPropertyBaseWidget`，整个过程对主框架透明。**新增属性子面板只需实现 Builder 并注册，不修改任何现有代码**，完全满足开闭原则。

### 4.2 多类型参数统一抽象

属性参数涉及 float、bool、color、enum、string、vector2D 等多种值类型，以及"单 Clip 单值"vs"多 Clip 多值"两种读写模式。通过模板抽象统一处理：

```cpp
// 多值检测：读取多个 Clip 的值并判断是否一致
template<typename Target, typename T>
FMultipleValue<T> FPropertyBatchGetter<Target, T>::get(
    const QList<IFFClip*>& clips,
    std::function<T(Target*)> getter) {
    T firstValue = getter(clips[0]->effect());
    for (int i = 1; i < clips.size(); i++) {
        if (getter(clips[i]->effect()) != firstValue)
            return { firstValue, true };  // isMultiple = true
    }
    return { firstValue, false };
}

// 批量写入：统一 Undo/Event 合并
template<typename Target>
void FPropertyBatchSetter<Target>::set(...) {
    FF_DECLARE_PAUSE_EVENT_COMMANDER(m_timeline);
    FF_DECLARE_UNDO_COMMANDER(m_undoSrv);
    for (auto* clip : m_clips)
        m_setter(clip->effect(), value);
}
```

**`FEffectPropertyBaseInfo`** 作为属性的"定位符"（封装主类型、子类型、属性索引），将 UI 操作精确映射到 VBL 数据层的具体字段，解耦了 UI 代码与 VBL 字段路径。

### 4.3 属性变更与 Undo/Event 的宏封装

所有属性写回均使用两个 RAII 宏包裹，利用作用域析构自动提交：

```cpp
// FF_DECLARE_PAUSE_EVENT_COMMANDER：
// 构造时暂停 VBL 事件广播；析构时恢复，触发一次统一通知
// → 防止写 N 个属性时触发 N 次 UI 刷新

// FF_DECLARE_UNDO_COMMANDER：
// 构造时开启 Undo 事务（beginMacro）；析构时关闭（endMacro）
// → 无论修改多少个属性，都合并为一条 Undo 记录

void setEffectPropertyValueList(const QList<FEffectPropertyData>& dataList) {
    FF_DECLARE_PAUSE_EVENT_COMMANDER(m_timeline);
    FF_DECLARE_UNDO_COMMANDER(m_undoSrv);
    for (const auto& data : dataList)
        doSetSingleValue(data);
    // 两个 commander 析构 → 合并 undo + 统一刷新
}
```

这种设计保证了：无论 UI 层如何调用属性写接口，undo 粒度与刷新频率都由 Control 层统一控制，UI 层无需关心。

### 4.4 实时调节与完成态的区分

拖动 Slider 期间（实时调节）与鼠标松开（操作完成）采用不同处理策略：

- **`startRealTimeUpdate()`**：进入实时模式，写 VBL 时使用 `Pause Event` 暂停部分重计算（如某些昂贵的后处理），只驱动预览窗实时刷新
- **`finishRealTimeUpdate()`**：结束实时模式，触发完整的数据刷新和 Undo 记录写入

这确保了 Slider 拖动过程中预览窗流畅响应，但不会在 Undo 栈中产生大量中间记录。

### 4.5 折叠状态的持久化

属性组（`FPropertyGroup`）的折叠/展开状态在会话间持久化：

- **加载**：`FPropertyView` 启动时读取 `loadDefaultFoldStates()`，按 groupType Key 恢复各组的默认折叠状态
- **保存**：`FPropertyView` 析构时调用 `saveDefaultFoldStates()`，将当前所有组的折叠状态写入配置
- 折叠时 Body Widget 调用 `hide()`，Header 保持可交互；布局通过监听 `sizeHint` 变化自动收紧

### 4.6 UpdateReasons 位掩码刷新控制

`notifyUpdateData(UpdateReasons reasons)` 使用位掩码精确控制刷新范围：

```cpp
enum UpdateReasons {
    ClipChanged    = 1 << 0,  // Clip 数据变更
    PlayheadMoved  = 1 << 1,  // 播放头移动
    UndoRedo       = 1 << 2,  // 撤销/重做
    EffectChanged  = 1 << 3,  // 特效变更
    // ...
};
```

每个 `FPropertyBaseWidget::refreshData(UpdateReasons)` 按需判断是否需要响应本次刷新，避免所有 Widget 在任何变更时全量重刷。

---

## 五、设计模式应用

### 5.1 工厂 + Builder（创建型）

`FPropertyWidgetFactory` 维护 `FFPropertyType → Builder列表` 的注册表，Builder 以静态宏自注册，主框架按需查询并构建。扩展新属性类型零侵入，体现了**开闭原则**。

### 5.2 观察者（Observer）

- `FPropertyControlManager` 实现 `IFFTimelineEventObserver / IFFPlayheadEventObserver / IFFUndoRedoServiceEventObserver / IFFProjectManagerEventObserver`，统一订阅外部模块事件
- `FPropertyBaseWidget` 实现 `IFPropertyControlObserver`，注册到 `FPropertyControl`，当数据变更时接收 `refreshData()` 通知

### 5.3 MVP（Model-View-Presenter）

- **View 层**（`FPropertyBaseWidget` / `FPropertyComponent`）：纯 UI 展示，通过 `IFPropertyPresenter` 将控件事件上报
- **Presenter 层**（`FPropertyPresenterImpl`）：转换控件值为 `FEffectPropertyData`，调用 `FPropertyControl` 写接口
- **Model/Control 层**（`FPropertyControl`）：持有业务逻辑，直接调用 VBL 接口

### 5.4 命令模式（Command）

所有写操作通过 `FF_DECLARE_UNDO_COMMANDER` RAII 宏封装为可撤销事务，与 `IFFUndoRedoService` 深度集成，支持精确的操作合并粒度控制。

### 5.5 策略模式（Strategy）

`FPropertyControl::setEffectPropertyValue()` 内部根据当前关键帧启用状态选择写入策略：
- 有关键帧 → 走 `keyFrameService()->setKeyFrameValue(frame, value)`（写当前帧的关键帧）
- 无关键帧 → 走 `IFFEffect::setPropertyValue(...)`（写静态属性值）

这两条路径对 UI 层完全透明，Presenter 只管调用，Control 层自动路由。

### 5.6 模板方法（Template）

`FPropertyBatchGetter / FPropertyBatchSetter` 是泛型模板，将"多 Clip 批量读/写"的公共骨架固定（遍历、多值判断、Undo 合并），具体的值读取/写入逻辑由调用方以 lambda 注入。

---

## 六、模块间通信机制

| 通信方式 | 使用场景 |
|---|---|
| `IFFTimelineEventObserver` | 时间线数据变更 → 属性面板刷新 |
| `IFFPlayheadEventObserver` | 播放头移动 → 关键帧高亮、属性值更新 |
| `IFFUndoRedoServiceEventObserver` | 撤销/重做完成 → 属性面板全量刷新 |
| `IFPropertyControlObserver` | Control 通知各 Widget 刷新（内部通信） |
| `FFMessage / sendMessageToScene` | 属性变更后通知播放器刷新预览帧 |
| Qt 信号/槽 | 控件值变化、AI 算法进度通知 |
| `QMetaObject::invokeMethod`（QueuedConnection） | VBL 回调安全切换回主线程 |

---

## 七、关键类职责速查

| 类名 | 职责 |
|---|---|
| `FPropertyView` | 主面板，Tab 切换、list view 缓存、折叠状态持久化 |
| `FPropertyControlManager` | 事件汇聚，监听 Timeline/Playhead/Undo，调度 Control 切换与刷新 |
| `FPropertySelector` | 封装选中 Clip 列表，分析类型（单选/多选/类型集合） |
| `FPropertyControl`（及派生类） | VBL 读写核心，关键帧操作，Undo/Event 事务管理 |
| `FPropertyBaseWidget` | 属性组 Body 基类，实现 Observer，refreshData，initPropertyInfos |
| `FPropertyGroup` | 可折叠组容器（Header + Body），折叠动画与状态管理 |
| `FPropertyComponent` | 单行属性行（Label + 控件 + 关键帧按钮） |
| `FPropertyWidgetFactory` | Builder 注册表，按 FFPropertyType 返回 Builder 列表 |
| `FPropertyBatchGetter/Setter` | 模板化批量读写，多值检测，Undo/Event 合并 |
| `IFPropertyPresenter` | 控件 ↔ Control 的 Presenter 接口，值转换与格式化 |
| `FAlgorithmCacheQueueWorker` | AI 算法缓存后台队列，进度/状态信号通知主线程 |
