11-property-panel-optimization.md

# 属性面板模块技术优化方案

---

## 一、属性 UI 频繁重建问题与优化

### 问题描述

当前 `d.listViews` 按 `FFPropertyType` 缓存 Widget 树，切换同类型时只刷新值，但**不同类型间切换时仍需完整重建**。典型场景：用户在时间线上快速交替选中视频 Clip 和音频 Clip，每次切换都触发 `createPropertyListView()` 构建一棵新的 Widget 树（含几十个 Widget），短时间大量 `new / delete` 造成主线程抖动。

### 优化方案

**方案 A：全类型预建 + 统一缓存（空间换时间）**

在编辑器初始化阶段（非启动路径，延后到第一次打开属性面板时）后台构建所有常用 `FFPropertyType` 的 Widget 树并缓存，切换时只做 `show / hide` + `refreshData()`：

```cpp
void FPropertyView::preloadCommonTypes() {
    static const QList<FFPropertyType> commonTypes = {
        FFPropertyType::Video, FFPropertyType::Audio, FFPropertyType::Color
    };
    for (auto type : commonTypes) {
        if (!d.listViews.contains(type))
            d.listViews[type] = createPropertyListView(type); // 后台预建
    }
}
```

**方案 B：Widget 对象池（Pool）复用**

对 `FPropertyGroup / FPropertyComponent` 等轻量 Widget 引入对象池，切换类型时回收旧 Widget、重新 init 后交给新类型使用，避免频繁分配/释放：

```cpp
class FPropertyGroupPool {
    QQueue<FPropertyGroup*> m_idle;
public:
    FPropertyGroup* acquire() {
        return m_idle.isEmpty() ? new FPropertyGroup() : m_idle.dequeue();
    }
    void release(FPropertyGroup* g) {
        g->reset(); // 清除旧状态
        m_idle.enqueue(g);
    }
};
```

---

## 二、全量刷新导致的性能开销

### 问题描述

`onUndoRedoChangedEvent` 触发后，当前实现对属性面板所有 Widget 执行 `refreshData()`，即使本次撤销/重做只影响某一个特效参数，所有不相关的属性行（如基础变换、色彩调校等）也会重新从 VBL 读值并更新控件，造成不必要的 VBL 读取与 UI 重绘开销。

### 优化方案

**方案 A：脏标记 + 按需刷新**

为每个 `FPropertyBaseWidget` 引入脏标记，只有关联的 `FEffectPropertyBaseInfo` 对应的属性发生变化时才标记脏，`refreshData()` 时跳过非脏 Widget：

```cpp
void FPropertyControlManager::onEffectPropertyChanged(
    const FEffectPropertyBaseInfo& changedInfo) {
    for (auto* widget : m_observers) {
        if (widget->hasProperty(changedInfo)) {
            widget->markDirty(); // 只标记受影响的 Widget
        }
    }
    notifyUpdateData(UpdateReasons::EffectChanged);
}

void FPropertyBaseWidget::refreshData(UpdateReasons reasons) {
    if (!(reasons & UpdateReasons::EffectChanged)) return;
    if (!m_dirty) return;      // 非脏，跳过
    m_dirty = false;
    doRefreshData();           // 真正从 VBL 读值更新控件
}
```

**方案 B：UpdateReasons 细化到属性 ID 级别**

将 `UpdateReasons` 扩展为携带变更的 `FEffectPropertyBaseInfo` 集合，Widget 只在自己的属性 ID 在集合内时才刷新：

```cpp
void notifyUpdateData(const QSet<FEffectPropertyBaseInfo>& changedInfos);
```

---

## 三、多 Clip 批量写入的事件风暴问题

### 问题描述

`FPropertyBatchSetter` 虽然使用 `FF_DECLARE_PAUSE_EVENT_COMMANDER` 暂停了事件广播，但 `PAUSE` 宏实现为"暂停期间的事件在恢复后统一触发一次"——若批量修改了 100 个 Clip，恢复时会触发 100 个 `onClipPropertyChanged` 事件，属性面板仍需处理 100 次回调才能合并为一次刷新，事件队列依然较长。

### 优化方案

在 `PAUSE` 恢复时，VBL 层对同类型的重复事件进行**去重合并**，只发出一次 `onBatchPropertyChanged(clips, info)` 批量事件，属性面板注册该批量 Observer 直接接收合并后的通知：

```cpp
class IFFBatchPropertyChangedObserver {
public:
    virtual void onBatchPropertyChanged(
        const QList<IFFClip*>& clips,
        const FEffectPropertyBaseInfo& info) = 0;
};

// 属性面板实现：
void FPropertyControlManager::onBatchPropertyChanged(...) {
    notifyUpdateData(UpdateReasons::BatchChanged);
    // 一次刷新，覆盖所有 Clip 的新值
}
```

---

## 四、关键帧启用/禁用切换时的 UI 状态同步延迟

### 问题描述

用户点击"启用关键帧"按钮后，`FPropertyControl::setKeyFrameEnabled()` 写入 VBL，VBL 回调 `onTimelineChanged`，经 `FPropertyControlManager` → `notifyUpdateData` → 各 `FPropertyBaseWidget::refreshData()` 链路，整个链路有 2~3 个异步跳转（`QueuedConnection`），导致关键帧按钮状态与控件可交互状态之间存在短暂不一致（按钮已高亮但控件还未切换到关键帧模式）。

### 优化方案

**直接在 UI 层同步更新视觉状态，不等待 VBL 回调：**

```cpp
void FPropertyComponent::onKeyFrameButtonClicked(bool enabled) {
    // 1. 立即同步 UI 状态（视觉反馈即时）
    m_keyFrameButton->setHighlight(enabled);
    setControlEnabled(enabled); // 控件立即切换状态

    // 2. 异步写 VBL（不阻塞 UI）
    m_presenter->setKeyFrameEnabled(m_info, enabled);
}
```

UI 先行更新、VBL 确认后若出现错误再回滚，符合"乐观 UI 更新"原则，消除用户感知到的延迟。

---

## 五、AI 算法任务缺少取消机制

### 问题描述

`FAlgorithmCacheQueueWorker` 的任务队列在用户快速切换 Clip 时可能积压大量过期任务——用户选中 Clip A 触发算法缓存，未完成时选中 Clip B 又触发，旧的 Clip A 任务仍在后台执行，浪费 CPU/GPU 且可能在完成后触发 Clip A 的 UI 更新（此时 Clip A 已不是选中状态）。

### 优化方案

引入任务 Token 机制，切换 Clip 时使旧任务失效：

```cpp
class FAlgorithmCacheQueueWorker {
    std::atomic<uint64_t> m_currentToken{0};

    void submitTask(IFFClip* clip) {
        uint64_t token = ++m_currentToken; // 使旧 token 失效
        FFAsync::postTask([this, clip, token]() {
            if (m_currentToken.load() != token) return; // 任务已过期
            runAlgorithmCache(clip);
            if (m_currentToken.load() == token)
                emit sigAlgorithmCacheStatusChange(Completed);
        });
    }

    void cancelAllPending() {
        ++m_currentToken; // 使所有当前任务失效
    }
};

// 切换 Clip 时：
void FPropertyControlManager::switchPropertyView(...) {
    m_algorithmWorker->cancelAllPending(); // 取消旧任务
    // ...
}
```

---

## 六、属性面板启动时注册开销问题

### 问题描述

`RegisterPropertyBuilderByType` 宏使用静态对象在程序启动时自动注册所有 Builder，若属性面板子面板数量持续增长（目前已有 30+），静态注册阶段的初始化时间会累积到主程序启动耗时中，且所有 Builder 对象常驻内存，即使用户从未使用某些 Clip 类型。

### 优化方案

将注册策略从**静态自动注册**改为**按需延迟注册**，只有用户首次访问该 Clip 类型时才触发对应 Builder 的注册与实例化：

```cpp
class FPropertyWidgetFactory {
    // 存储工厂函数而非 Builder 实例
    QMap<FFPropertyType, std::function<FPropertyBaseWidgetBuilder*()>> m_factories;

public:
    template<typename BuilderType>
    void registerLazy(FFPropertyType type) {
        m_factories[type] = []() { return new BuilderType(); };
    }

    QList<FPropertyBaseWidgetBuilder*> buildersForType(FFPropertyType type) {
        if (!m_builders.contains(type)) {
            // 首次访问时才实例化 Builder
            m_builders[type] = m_factories[type]();
        }
        return m_builders[type];
    }
};
```
