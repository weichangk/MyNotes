# AIManager 模块详细设计文档

> 所属层：BusinessLayer
> 接口头：`Interface/BusinessLayer/AIManager/IVbAIManager.h`
> 工厂函数：`createAIManager(AITaskType type, VBLBool toTaskCenter)`
> 命名空间：`VBL::`

---

## 1. 模块职责

AIManager 是 VBL 中所有 AI 功能的**统一任务调度与管理中心**，负责：

- **任务生命周期**：创建、启动、暂停、取消、查询 AI 任务
- **云端/本地执行**：支持本地算法和云端 AI 服务两种执行路径
- **进度与结果回调**：通过 `IVblTaskCallBack` / `IVblQueryCallBack` 异步返回
- **任务中心集成**：支持进入全局任务中心（UI 任务面板展示）
- **凭据与限流管理**：消费凭据（`consumeLogNo`）、流量控制（`allowRoll`）

---

## 2. AI 任务类型（AITaskType）

| 类别 | 功能 |
|---|---|
| 语音处理 | STT（语音转文字）、TTS（文字转语音）、语音克隆、声纹分离、翻译 |
| 画面处理 | 智能抠图/背景替换、AI 视频风格化、智能马赛克、超清修复 |
| 创作辅助 | AI 剪同款、AI 智能剪辑、Text To Video、Idea To Video |
| 音频处理 | AI 智能配乐、节拍检测、音频修复 |
| 字幕处理 | 场景字幕、字幕翻译 |
| 内容生成 | AI 文案生成、AI Copilot、Long To Shorts |
| 文件处理 | 文件上传（IVbUploadFileManager） |

---

## 3. 核心接口说明

```cpp
class IVbAIManager : virtual public IDmBaseObj {
    // 配置
    virtual Result setManagerConfig(VBLConstPChar key, Property val) = 0;
    virtual Result getManagerConfig(VBLConstPChar key, Property& val) = 0;
    virtual Result setServerParam(IBsPubInfo* pubParam, IBsServerProxyInfo* proxy) = 0;

    // 数据查询（任务前置查询，如查询剩余额度）
    virtual Result queryData(TransferDataQueryType type, IVblParam* queryParam = nullptr) = 0;

    // 任务操作
    virtual Result addTask(IDmBaseObj* transferObj, IVblParam* taskParam, VBLInt& taskid) = 0;
    virtual Result delTask(VBLInt taskid) = 0;
    virtual Result startTask(VBLInt taskid = -1) = 0;  // -1 = 启动全部
    virtual Result cancelTask(VBLInt taskid, VBLBool forceDeleteTask = false) = 0;
    virtual Result stopTask(VBLInt taskid = -1) = 0;

    // 任务查询
    virtual IVbAITaskInfo* getTaskInfo(VBLInt taskid) = 0;
    virtual VBLInt taskCount() = 0;
    virtual VBLInt getTaskId(VBLInt idx) = 0;
    virtual VBLVoid waitForFinish(VBLInt taskId) = 0;

    // 回调注册
    virtual VBLVoid setQueryCallback(IVblQueryCallBack* pCallback) = 0;
    virtual VBLVoid setTaskCallback(IVblTaskCallBack* pCallback) = 0;
};

// 任务信息
class IVbAITaskInfo : virtual public IDmBaseObj {
    virtual AITaskStatus taskStatus() = 0;    // Pending/Running/Success/Failed/Cancelled
    virtual Result taskErrorCode() = 0;
    virtual VBLInt taskProgress() = 0;        // 0~100
    virtual AITaskStageData stageData() = 0;  // 执行阶段（上传/处理/下载）
    virtual IBsErrorInfo* errorInfo() = 0;
    virtual IBsInfoReader* extraDataInfo() = 0; // 云端任务 ID、等待位置等
};
```

---

## 4. 云端任务额外信息（AITaskInfoExtraDataKey）

| Key | 类型 | 说明 |
|---|---|---|
| `taskUseTime` | int(ms) | 任务耗时 |
| `cloudTaskId` | string | 云端平台任务 ID |
| `cloudTaskStatus` | int | 云端任务状态 |
| `cloudTaskPosition` | int | 队列等待位置 |
| `cloudTaskWaitSeconds` | int | 预计等待时间（秒）|
| `cloudTaskPriority` | int | 0=免费用户 / 1=付费用户 |

---

## 5. 依赖关系

```
AIManager
  ├── 依赖 → BsAI              （底层 AI 算法调用）
  ├── 依赖 → BsNet             （网络请求：上传文件、调用云 AI API）
  ├── 依赖 → BSWsid            （用户授权校验、消费凭据）
  ├── 发布 → IMsEventBus       （task.ai.* 进度/完成事件）
  └── 可选 → 全局任务中心       （toTaskCenter=true 时注册到 UI 任务面板）
```

---

## 6. 时序图

### 6.1 本地 AI 任务（语音转文字 STT）

```
UI 层          AIManager        BsAI（本地算法）   DataModel      EventBus
  │                │                  │                │              │
  │ createAIManager(STT, false)       │                │              │
  ├──────────────► │                  │                │              │
  │ setServerParam(pubInfo, proxy)    │                │              │
  ├──────────────► │                  │                │              │
  │ addTask(audioClip, taskParam, id) │                │              │
  ├──────────────► │                  │                │              │
  │ startTask(id)  │                  │                │              │
  ├──────────────► │                  │                │              │
  │                │ BsAI::runSTT(audioClip)           │              │
  │                ├─────────────────►│                │              │
  │                │                  │ 本地识别处理    │              │
  │                │ onProgress(50%)  │                │              │
  │                │◄─────────────────┤                │              │
  │                │ callback->onTaskProgress(id, 50%) │              │
  │ progressBar 50%│◄──────────────────────────────────────────────   │
  │                │                  │ onComplete(result)            │
  │                │◄─────────────────┤                │              │
  │                │ callback->onTaskCompleted(id, STTResult)         │
  │                │ result → 创建字幕 Clip              │              │
  │                │ timeline->addSubtitleClip(clip)   │              │
  │                ├────────────────────────────────── ►│             │
  │                │ postEvent("task.ai.stt.done")      │              │
  │                ├──────────────────────────────────────────────────►│
  │ onEvent() 刷新字幕列表          │                │              │ → UI
  │◄──────────────────────────────────────────────────────────────────────┤
```

### 6.2 云端 AI 任务（AI 超清修复）

```
UI 层          AIManager        BsNet         云端 AI 服务    EventBus
  │                │               │                │              │
  │ addTask(videoClip, param, id)  │                │              │
  ├──────────────► │               │                │              │
  │ startTask(id)  │               │                │              │
  ├──────────────► │               │                │              │
  │                │ 阶段1: 上传文件│                │              │
  │                │ BsNet::upload(videoFile)        │              │
  │                ├──────────────►│                │              │
  │                │               │ HTTP POST 上传  │              │
  │                │               ├───────────────► │              │
  │                │ stageData(Upload, 60%)          │              │
  │◄───────────────────────────────────────────────────────────────────│
  │                │               │                │              │
  │                │ 阶段2: 创建云端任务             │              │
  │                │ BsNet::createCloudTask(params)  │              │
  │                ├──────────────►│                │              │
  │                │               │ HTTP POST 创建  │              │
  │                │               ├───────────────► │              │
  │                │               │  cloudTaskId    │              │
  │                │               │◄───────────────┤              │
  │                │               │                │              │
  │                │ 阶段3: 轮询状态 (cloudTaskStatus)              │
  │                │ BsNet::queryTask(cloudTaskId)   │              │
  │                ├──────────────►│                │              │
  │                │               │ HTTP GET 查询   │              │
  │                │               ├───────────────► │              │
  │                │               │  Status_Done    │              │
  │                │               │◄───────────────┤              │
  │                │ 阶段4: 下载结果│                │              │
  │                │ BsNet::download(resultUrl)      │              │
  │                ├──────────────►│                │              │
  │                │               │ HTTP GET 下载   │              │
  │                │               ├───────────────► │              │
  │                │ callback->onTaskCompleted(id, result)          │
  │                │ postEvent("task.ai.uhd.done")   │              │
  │                ├──────────────────────────────────────────────── ►│
  │ onEvent() 替换 clip 为超清版本  │                │              │ → UI
  │◄──────────────────────────────────────────────────────────────────────┤
```

### 6.3 取消 AI 任务

```
UI 层          AIManager        BsNet         云端 AI 服务
  │                │               │                │
  │ cancelTask(id) │               │                │
  ├──────────────► │               │                │
  │                │ taskStatus = Cancelling         │
  │                │ BsNet::cancelCloudTask(cloudTaskId)
  │                ├──────────────►│                │
  │                │               │ HTTP DELETE     │
  │                │               ├───────────────►│
  │                │               │  OK             │
  │                │               │◄───────────────┤
  │                │ callback->onTaskCancelled(id)   │
  │◄───────────────┤               │                │
  │ 任务取消提示   │               │                │
```

---

## 7. 子管理器列表

| 接口 | 功能 |
|---|---|
| `IVbAISpeechToTextManager` | 语音转文字 |
| `IVbAITextToSpeechExManager` | 文字转语音（含多语言）|
| `IVbAITranslationManager` | 字幕翻译 |
| `IVbAIVoiceCloneManager` | 声音克隆 |
| `IVbAIVoiceSeparationManager` | 人声分离 |
| `IVbAIVoiceCorrectionManager` | 语音修正 |
| `IVbAISmartCutManager` | AI 智能剪辑 |
| `IVbAILongToShortsTaskManager` | 长视频转短视频 |
| `IVbAIReelsMakerManager` | Reels 自动生成 |
| `IVbAICutSameMakerManager` | 剪同款 |
| `IVbAIUltraHDManager` | 超清修复 |
| `IVbAIReplaceSkyManager` | 天空替换 |
| `IVbAIImageStylizerManager` | 图像风格化 |
| `IVbAITextToVideoManager` | 文字转视频 |
| `IVbAIIdeaToVideoTaskManager` | 创意转视频 |
| `IVbAISmartBGMManager` | 智能配乐 |
| `IVbAISmartBeatsManager` | 节拍检测 |
| `IVbSmartRemoveObjManager` | 智能去除对象 |
| `IVbAIComboAIGCManager` | 组合 AIGC 任务 |

---

## 8. 设计要点

| 要点 | 说明 |
|---|---|
| 统一入口 | 所有 AI 功能通过 `IVbAIManager::addTask/startTask` 统一调度，不直接调用 BsAI |
| 任务中心 | `toTaskCenter=true` 时注册到全局任务中心，UI 可统一展示所有 AI 任务进度 |
| 阶段回调 | `AITaskStageData` 区分上传/处理/下载三阶段，UI 可展示更细粒度进度 |
| 排队执行 | `queued_execution=true` 时任务串行排队，避免并发过多 |
| 消费回滚 | `allowRoll=true` 时任务失败自动退款消费额度 |
