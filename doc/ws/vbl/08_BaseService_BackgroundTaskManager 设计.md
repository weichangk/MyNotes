# BackgroundTaskManager 模块详细设计文档

> 所属层：BaseService
> 接口头：`Interface/BaseService/BackgroundTaskManager/IBsBackgroundTaskManager.h`
> 工厂函数：`createBackgroundTaskManager()`
> 命名空间：`VBL::`

---

## 1. 模块职责

BackgroundTaskManager 是 VBL 的**后台任务调度中心**，负责：

- **任务队列管理**：维护待执行的 `IVblBaseTask` 任务队列
- **并发控制**：通过 `SetCocurrentCount` 限制同时执行的任务数量
- **任务分组**：支持 `bgTaskMediaGroup`（媒体区任务）和 `bgTaskTimelineGroup`（时间线任务）两组，组间互斥
- **生命周期控制**：支持 start / pause / resume / stop 整体调度
- **进度与回调**：任务执行过程中通过 `IVblBaseTask` 的回调通知调用方

---

## 2. 任务类型

| 接口 | 说明 | 触发场景 |
|---|---|---|
| `IBsAudioWaveExtractTask` | 音频波形提取 | 导入音频/视频后自动触发 |
| `IBsMediaAnalysisTask` | 素材分析任务 | MaterialAnalysis::start() 调用 |
| `IBsMediaScenceDetectTask` | 场景检测 | 素材分析（场景检测算法）|
| `IBsMediaContentReconizeTask` | 媒体内容识别 | 素材智能标签识别 |
| `IBsVideoFrameExtractTask` | 视频帧提取 | 缩略图生成 |
| `IBsTimelineRenderTask` | 时间线后台渲染 | 预渲染/PreRender |
| `IBsAudioRemixTask` | 音频 Remix 处理 | BGM 自动伸缩 |

---

## 3. 核心接口说明

```cpp
class IBsBackgroundTaskManager : virtual public IDmBaseObj {
    // 并发配置
    virtual Result SetCocurrentCount(VBLInt count) = 0;
    virtual VBLInt cocurrentCount() = 0;

    // 任务管理
    virtual Result addBackgroundTask(IVblBaseTask* pTask) = 0;
    virtual Result removeBackgroundTask(IVblBaseTask* pTask) = 0;
    virtual IVblBaseTask* backgrondTask(VBLInt idx) = 0;

    // 任务分组切换
    virtual Result changeCurrentWorkingType(VBLInt group) = 0;

    // 调度控制
    virtual Result start() = 0;
    virtual Result pause() = 0;
    virtual Result resume() = 0;
    virtual Result stop() = 0;
};

// IVblBaseTask 基类（所有任务均实现此接口）
class IVblBaseTask : virtual public IDmBaseObj {
    virtual Result start() = 0;
    virtual Result stop() = 0;
    virtual Result pause() = 0;
    virtual Result resume() = 0;
    virtual VBLInt progress() = 0;         // 0~100
    virtual TaskState state() = 0;         // Pending/Running/Paused/Done/Failed
    virtual Result setCallback(IVblBaseTaskCallback*) = 0;
};
```

---

## 4. 依赖关系

```
BackgroundTaskManager
  ├── 执行 → Adapter（各 XxxExtractAdapter / XxxAnalysisAdapter）
  ├── 发布 → IMsEventBus（task.* 进度/完成/失败事件）
  └── 被依赖 → MaterialAnalysis / DataModel（提交任务）
```

---

## 5. 时序图

### 5.1 音频波形提取任务

```
DataModel/UI     BackgroundTaskMgr   IBsAudioWaveExtractTask  Adapter(Wave)  EventBus
    │                   │                      │                   │             │
    │ createAudioWaveTask(clip)                │                   │             │
    ├──────────────────►│                      │                   │             │
    │ addBackgroundTask(task)                  │                   │             │
    ├──────────────────►│                      │                   │             │
    │ start()           │                      │                   │             │
    ├──────────────────►│                      │                   │             │
    │                   │ 按并发数取出 task     │                   │             │
    │                   │ task->start()         │                   │             │
    │                   ├─────────────────────►│                   │             │
    │                   │                      │ WaveExtractAdapter::extract()   │
    │                   │                      ├──────────────────►│             │
    │                   │                      │                   │ 解码音频帧   │
    │                   │                      │                   │ 计算振幅数据  │
    │                   │                      │ onProgress(50%)   │             │
    │                   │                      │◄──────────────────┤             │
    │ 更新波形进度条      │◄─────────────────────┤                   │             │
    │                   │                      │ onComplete(waveData)            │
    │                   │                      │◄──────────────────┤             │
    │                   │                      │ postMainThreadEvent("wave.extract.done")
    │                   │                      ├──────────────────────────────── ►│
    │ 刷新时间线波形显示  │                      │                   │             │ → UI
    │◄──────────────────────────────────────────────────────────────────────────────┤
```

### 5.2 任务分组切换（媒体区 ↔ 时间线）

```
UI 层           BackgroundTaskMgr     Media任务    Timeline任务
  │                    │                  │               │
  │ 用户开始播放        │                  │               │
  │ changeCurrentWorkingType(bgTaskTimelineGroup)         │
  ├──────────────────► │                  │               │
  │                    │ 暂停所有 MediaGroup 任务          │
  │                    │ pause(task) for each media task  │
  │                    ├──────────────────►│              │
  │                    │ 切换到 TimelineGroup              │
  │                    │ resume()/start() timeline tasks  │
  │                    ├───────────────────────────────── ►│
  │                    │                  │               │ 优先执行时间线预渲染
  │ 用户停止播放        │                  │               │
  │ changeCurrentWorkingType(bgTaskMediaGroup)            │
  ├──────────────────► │                  │               │
  │                    │ 暂停 TimelineGroup│               │
  │                    │ 恢复 MediaGroup   │               │
  │                    ├──────────────────►│              │
```

### 5.3 并发限制

```
BackgroundTaskMgr
  │
  │ SetCocurrentCount(2)    // 同时最多 2 个任务
  │
  ├── [Task A: 波形提取]  running
  ├── [Task B: 场景检测]  running
  ├── [Task C: 缩略图]    pending（等待 A 或 B 完成）
  ├── [Task D: 人脸检测]  pending
  │
  │ Task A 完成
  ├── [Task B: 场景检测]  running
  ├── [Task C: 缩略图]    running  ← 自动调度
  ├── [Task D: 人脸检测]  pending
```

---

## 6. 设计要点

| 要点 | 说明 |
|---|---|
| 任务优先级 | 通过 `changeCurrentWorkingType` 动态切换组别，高优先级组（时间线）优先运行 |
| 并发数 | 默认 CPU 核数的一半，可通过 `SetCocurrentCount` 调整 |
| 任务复用 | 同一 mediaId 的波形任务若已在队列中，不重复添加 |
| 主线程通知 | 所有完成/进度回调通过 `postMainThreadEvent` 切回主线程，保证 UI 安全更新 |
