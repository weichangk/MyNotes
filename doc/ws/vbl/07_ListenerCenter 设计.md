# ListenerCenter 模块详细设计文档

> 所属层：ListenerCenter
> 接口头：`Interface/ListenerCenter/IMsEventBus.h`
> 初始化入口：`Interface/ListenerCenter/MsEventBusLib.h`
> 命名空间：`VBL::`

---

## 1. 模块职责

ListenerCenter（监听中心）是 VBL 的**全局事件总线**，基于发布-订阅模式实现模块间解耦通信：

- **事件注册**：模块向 EventBus 注册事件类型及对应的数据结构
- **事件订阅**：UI 层或其他模块订阅感兴趣的事件
- **事件发布**：数据变更方通过指定 ThreadMode 投递事件
- **粘性事件**：新订阅者立即接收最后一次发布的值（类似 LiveData）
- **线程调度**：支持主线程/异步线程/后台线程/当前线程多种投递模式

---

## 2. 核心接口说明（IMsEventBus）

```cpp
class IMsEventBus {
public:
    // ===== 事件注册（数据提供方调用）=====
    // 向总线注册一种事件类型，并定义该事件的数据结构原型
    virtual Result registerEvent(VBLConstPChar eventName, IDmBaseObj* protoObj) = 0;
    virtual Result unregisterEvent(VBLConstPChar eventName) = 0;

    // ===== 事件订阅（UI/消费方调用）=====
    virtual Result subscribe(VBLConstPChar eventName, IEventObserver* observer) = 0;
    virtual Result unsubscribe(VBLConstPChar eventName, IEventObserver* observer) = 0;
    virtual Result setSubscriberActive(IEventObserver* observer, VBLBool active) = 0;

    // ===== 事件发布（数据变更方调用）=====
    virtual Result postMainThreadEvent(VBLConstPChar eventName, IDmBaseObj* data) = 0;
    virtual Result postAsyncEvent(VBLConstPChar eventName, IDmBaseObj* data) = 0;
    virtual Result postBackgroundEvent(VBLConstPChar eventName, IDmBaseObj* data) = 0;
    virtual Result postCurrentEvent(VBLConstPChar eventName, IDmBaseObj* data) = 0;
    virtual Result postEvent(VBLConstPChar eventName, ThreadMode mode, IDmBaseObj* data) = 0;

    // ===== 粘性事件 =====
    virtual Result postStickyEvent(VBLConstPChar eventName, IDmBaseObj* data) = 0;
    virtual IDmBaseObj* fetchStickEvent(VBLConstPChar eventName) = 0;
};
```

---

## 3. ThreadMode 说明

| ThreadMode | 值 | 说明 | 典型用途 |
|---|---|---|---|
| `MAINTHREAD` | 0 | 切换到主线程执行回调 | UI 刷新（必须主线程） |
| `ASYNC` | 1 | 新开一个异步线程执行回调 | 耗时的异步处理 |
| `CURRENT` | 2 | 在发布线程同步执行回调 | 同步通知、单元测试 |
| `BACKGROUND` | 3 | 固定后台线程执行回调 | 批量写文件等串行操作 |
| `USER` | 4~6 | 用户自定义线程池（3个）| 特定优先级任务 |

> 桌面端：`MAX_THREAD = 7`（MAINTHREAD + ASYNC + CURRENT + BACKGROUND + USER×3）
> Web/Mobile：`MAX_THREAD = 2`（精简为 MAINTHREAD + CURRENT）

---

## 4. 事件命名规范

| 命名空间 | 示例事件名 | 说明 |
|---|---|---|
| `timeline.*` | `timeline.clip.added` / `timeline.clip.removed` / `timeline.clip.changed` | 时间线数据变更 |
| `timeline.*` | `timeline.track.added` / `timeline.track.removed` | 轨道变更 |
| `media.*` | `media.items.added` / `media.items.removed` / `media.item.replaced` | 媒体区变更 |
| `project.*` | `project.created` / `project.loaded` / `project.saved` / `project.closed` | 工程生命周期 |
| `task.*` | `task.progress` / `task.done` / `task.failed` | 后台任务状态 |
| `task.ai.*` | `task.ai.stt.done` / `task.ai.uhd.done` | AI 任务完成 |
| `error.*` | `error.decode` / `error.export` / `error.save` | 错误通知 |
| `service.*` | `service.network.changed` / `service.gpu.changed` | 服务状态变更 |
| `wave.*` | `wave.extract.done` | 音频波形提取完成 |

---

## 5. 依赖关系

```
ListenerCenter
  ├── PRIVATE → TBB（桌面端多线程事件队列）
  ├── 被所有模块依赖（DataModel PUBLIC、各业务层 PRIVATE）
  └── 采用全局单例模式（通过 MsEventBusLib 获取实例）
```

---

## 6. 时序图

### 6.1 事件注册与订阅（初始化阶段）

```
DataModel       ListenerCenter      UI 层（订阅方）
    │                 │                    │
    │ registerEvent("timeline.clip.changed", protoObj)
    ├────────────────►│                    │
    │                 │ 注册事件，记录原型数据类型
    │                 │                    │
    │                 │ subscribe("timeline.clip.changed", observer)
    │                 │◄───────────────────┤
    │                 │ 记录订阅者与回调函数│
    │                 │                    │
    │ registerEvent("project.loaded", ...)│
    ├────────────────►│                    │
    │                 │ subscribe("project.loaded", observer)
    │                 │◄───────────────────┤
```

### 6.2 主线程事件发布（时间线变更）

```
TimelineEditor  ListenerCenter    主线程队列      UI（回调）
     │                │                │               │
     │ postMainThreadEvent("timeline.clip.added", data)
     ├───────────────►│                │               │
     │                │ 封装 Event 对象 │               │
     │                │ 投递到主线程队列│               │
     │                ├───────────────►│               │
     │  (异步)         │               │ 主线程 pump    │
     │                │               │ 取出 Event     │
     │                │               │ 回调所有订阅者  │
     │                │               ├──────────────── ►│
     │                │               │               │ onEvent("timeline.clip.added")
     │                │               │               │ 刷新时间线 UI
```

### 6.3 后台线程任务完成通知

```
BackgroundTask（后台线程）  ListenerCenter   主线程队列   UI（回调）
         │                      │                │           │
         │ 任务执行完成           │                │           │
         │ postMainThreadEvent("task.wave.done", result)     │
         ├─────────────────────►│                │           │
         │                      │ 投递主线程队列  │           │
         │                      ├───────────────►│           │
         │                      │               │ 主线程处理 │
         │                      │               ├───────────►│
         │                      │               │           │ 更新波形 UI
```

### 6.4 粘性事件（新订阅者立即获取）

```
DataModel       ListenerCenter      UI（后初始化订阅）
    │                 │                    │
    │ postStickyEvent("project.loaded", projectData)
    ├────────────────►│                    │
    │                 │ 保存最后一次 sticky 数据
    │                 │                    │
    │                 │（UI 延迟初始化后订阅）│
    │                 │ subscribe("project.loaded", observer)
    │                 │◄───────────────────┤
    │                 │ 检查 sticky 缓存    │
    │                 │ 立即回调 onEvent()  │
    │                 ├────────────────────►│
    │                 │                    │ 立即获取已有 project 数据
```

### 6.5 取消订阅

```
UI 层          ListenerCenter
  │                 │
  │ unsubscribe("timeline.clip.changed", observer)
  ├────────────────►│
  │                 │ 从订阅列表移除 observer
  │                 │ 后续不再回调
  │                 │
  │ setSubscriberActive(observer, false)  // 暂时停用（不取消，保留注册）
  ├────────────────►│
  │                 │ 标记 observer 为 inactive，跳过回调
```

---

## 7. 多平台线程模型差异

| 平台 | MAX_THREAD | BACKGROUND 线程 | USER 线程 |
|---|---|---|---|
| Windows / macOS | 7 | 1 条专用后台线程 | USER0/USER1/USER2 各 1 条 |
| iOS / Android | 2 | — | — |
| Web（WASM）| 2 | — | — |

移动端和 Web 端简化为 MAINTHREAD（主线程回调）和 CURRENT（同步回调）两种模式，减少线程开销。

---

## 8. 设计要点

| 要点 | 说明 |
|---|---|
| 解耦 | 发布方和订阅方只依赖 EventBus 接口，不直接引用对方 |
| 数据所有权 | `postEvent` 传入的 `IDmBaseObj*` 数据由发布方创建，EventBus 内部 AddRef，所有回调完成后 Release |
| 线程安全 | 内部使用 TBB concurrent_queue（桌面端）保证多线程并发投递安全 |
| 粘性事件 | `postStickyEvent` 仅保留最后一次值，适合状态类事件（非流式事件）|
| Observer 生命周期 | Observer 在销毁前必须调用 `unsubscribe`，否则野指针 |
| 批量事务 | `DataModel::endEdit()` 后才统一 postEvent，避免中间态广播 |
