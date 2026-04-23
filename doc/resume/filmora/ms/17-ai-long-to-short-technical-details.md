17-ai-long-to-short-technical-details.md
# AI 长视频转短视频模块技术实现细节

> 项目：Wondershare Filmora（C++17 + Qt 5.15）
> 模块：AILongToShort（`Src/Filmora/AILongToShort`）

---

## 一、模块概述

AI 长视频转短视频（AILongToShort）是 Filmora 的 AI 智能剪辑功能，支持用户输入一段长视频（本地文件、云盘媒体或在线 URL），由 AI 自动完成语种识别、视频转码、内容理解与评分、短视频脚本生成，最终输出多个评分排序的短视频片段，用户可一键编辑或批量导出。

整个模块横跨 UI 展示、业务编排、VBL 任务引擎三层，AI 推理与生成由底层 VBL Task Manager 驱动，上层通过统一的 Observer 回调接口感知进度与状态。

---

## 二、整体架构

```
┌────────────────────────────────────────────────────────────┐
│                     UI 层（View / Dialog）                   │
│  FAILongToShortDialog（主对话框）                            │
│  FAILongToShortView / ImportPage / ParamSettingView          │
│  FAILongToShortResultWidget / LoadingPanel / StatusView      │
│  FAILongToShortPlayerView（预览播放控件）                    │
└────────────────────────────┬───────────────────────────────┘
                             │ 信号/槽
┌────────────────────────────▼───────────────────────────────┐
│                  业务编排层（Presenter）                      │
│  FAILongToShortPresenter（总控制器）                         │
│  ├── FAILongToShortPresenterPrivate                         │
│  │     startLanguageDetectTask()                            │
│  │     startTransferTask()                                  │
│  │     startGenerateTask()                                  │
│  │     onTaskStateChanged()（Observer 回调）                 │
│  │     progressComposeHelper（进度合成）                     │
│  ├── FAILongToShortResultPresenter（结果列表逻辑）            │
│  ├── FAILongToShortHistoryPresenter（历史记录）               │
│  └── FAILongToShortTemplateManager（模板管理）               │
└────────────────────────────┬───────────────────────────────┘
                             │ VBL 抽象接口
┌────────────────────────────▼───────────────────────────────┐
│                  AI 任务服务层（VBL Task）                    │
│  IFFAILongToShortTaskService（接口）                         │
│  FFAILongToShortTaskService（实现：封装 VBL Task Manager）   │
│  IFFAILongToShortScript（脚本结果对象）                      │
│  IFFAudioLanguageRecognitionService（语种检测服务）          │
│  VBL Task Manager（本地/云端 AI 推理引擎）                   │
└────────────────────────────────────────────────────────────┘
```

### 生命周期中的四个任务阶段

```
用户导入视频
     │
[1] Language Detect Task  ←── audioLanguageRecognitionService
     │ 完成后自动触发
[2] Transfer Task          ←── addTransferTask → VBL（转码/上传）
     │ 完成后获取 fileId，自动触发
[3] Generate Task          ←── addGenerateTask → VBL（AI 分析+脚本生成）
     │ 完成后
[4] 获取脚本列表            ←── getGeneratedScripts(taskId)
     │
[5] 用户选择 → edit() 或 doBatchExport()
```

---

## 三、核心数据结构

### 3.1 FFAILongToShortGenerateTaskParam（生成任务参数）

```cpp
struct FFAILongToShortGenerateTaskParam {
    QString     fileId;             // Transfer 任务完成后返回的文件 ID
    QString     fileName;           // 原始文件名（展示用）
    QString     language;           // 语种（语种检测结果或用户手动指定）
    QString     ratio;              // 目标宽高比（如 "9:16"）
    QString     theme;              // 主题风格
    int         targetDuration;     // 目标短视频时长（秒）
    QString     templateContent;    // 模板内容（若使用了 AI 模板）
    bool        smartMusic;         // 是否启用 AI 配乐
};
```

### 3.2 IFFAILongToShortScript（脚本结果对象）

脚本是 AI 生成的核心输出，每条脚本代表一个候选短视频：

```cpp
class IFFAILongToShortScript {
    virtual float  score() const = 0;              // AI 综合评分（排序依据）
    virtual QString scoreDescription() const = 0;  // 评分说明
    virtual QList<SttItem> sttList() const = 0;    // 字幕/语音分段列表
    virtual TimelineData timeline() const = 0;     // 时间线数据（片段起止点）
    virtual QPixmap projectThumbnail() const = 0;  // 封面缩略图
};
```

### 3.3 FFAITaskStateInfo（任务状态回调信息）

```cpp
struct FFAITaskStateInfo {
    int         taskId;
    FFAITaskType taskType;       // AudioLanguageRecognition / Transfer / Generate
    FFAITaskState state;         // Processing / Finished / Error
    float       progress;        // 0.0 ~ 1.0
    int         errorCode;       // 错误码（余额不足/网络错误等）
    QString     errorMessage;
};
```

---

## 四、主要业务流程

### 4.1 视频导入流程

```
[本地文件导入]
FAILongToShortPresenter::importByLocalFile(filePath)
     │  设置 transferTaskInfo.param.videoPath / fileType
     │  设置 generateTaskInfo.param.fileName
     └──→ emit sigImportByLocalFileFinished()

[云盘导入]
importByCloudDisk(mediaItem)
     │  d_ptr->downloadCloudMedia(mediaItem)
     │  FF_CLOUD_DISK->cloudDiskViewPresenter()->downloadCloudMedia(...)
     │  connect(cloudPresenter, &downloadFinished, ...) // 下载完成后走本地导入
     └──→ m_downloadTaskIds 追踪进行中的下载任务

[URL 导入]
queryDownloadUrl(request)
     │  service->queryDownloadUrl(request)
     │  → FFAILongToShortTaskService::onQueryFinished()
     │    QMetaObject::invokeMethod(mainThread, ...)
     └──→ onQueryDownloadUrlSuccess → importByUrl(url)
```

### 4.2 AI 任务执行流程

```
FAILongToShortPresenter::transfer()
     │  d_ptr->addSubProgressParam(Language, weight=10%)
     │  d_ptr->addSubProgressParam(Transfer, weight=30%)
     │  d_ptr->startLanguageDetectTask()
     │       audioLanguageRecognitionService->addTask(param)
     │       audioLanguageRecognitionService->startTask(taskId)
     │
     │  [回调] onLanguageDetectTaskStateChanged(info)
     │       progressComposeHelper->setSubProgressValue(Language, info.progress)
     │       emit sigTaskProgressChanged(totalProgress)
     │       if (Finished) → 获取语种结果 → d_ptr->startTransferTask()
     │
     │  startTransferTask()
     │       service->addTransferTask(transferTaskInfo.param)
     │       service->startTask(taskId)
     │
     │  [回调] onTransferTaskStateChanged(info)
     │       if (Finished) → service->getTransferFileId(taskId)
     │                       → 填充 generateTaskInfo.param.fileId
     │                       → FAILongToShortPresenter::generate()

FAILongToShortPresenter::generate()
     │  d_ptr->addSubProgressParam(Generate, weight=60%)
     │  d_ptr->startGenerateTask()
     │       service->addGenerateTask(generateTaskInfo.param)
     │       service->startTask(taskId)
     │
     │  [回调] onGenerateTaskStateChanged(info)
     │       progressComposeHelper->setSubProgressValue(Generate, info.progress)
     │       emit sigTaskProgressChanged(totalProgress)
     │       if (Finished) →
     │           scripts = service->getGeneratedScripts(taskId)  // 获取脚本列表
     │           scriptManager->saveScripts(scripts)
     │           emit sigTaskStateChanged(Finished)
     │       if (Error) →
     │           if (errorCode == NoEnoughCredits) → 弹余额不足对话框
     │           else → emit sigTaskStateChanged(Error)
```

### 4.3 进度合成机制

```
FProgressComposeHelper
     │  setTotalProgressDuration(100)
     │  addSubProgress(Language, weight=10)
     │  addSubProgress(Transfer, weight=30)
     │  addSubProgress(Generate, weight=60)
     │
各子任务回调中：
     │  progressComposeHelper->setSubProgressValue(subType, rawProgress)
     │      → 内部计算：totalProgress = Σ(subProgress[i] × weight[i]) / Σweight[i]
     └──→ emit sigTaskProgressChanged(totalProgress, subType)
     
UI 层：
     connect(presenter, &sigTaskProgressChanged, progressBar, &setValue)
     // 用户看到的进度条是语种检测+转码+生成的加权合成
```

### 4.4 结果展示与编辑导出流程

```
生成完成后：
     getGeneratedScripts(taskId)
     → FAILongToShortResultWidget 显示脚本列表（评分、封面、时长、字幕预览）

[单个编辑]
FAILongToShortPresenter::edit(index, openSpeechEnhance)
     │  script = scriptManager->getScript(index)
     │  FF_AI_LONG_TO_SHORT_EDITOR->setScript(script)
     │  FF_AI_LONG_TO_SHORT_EDITOR->setEditMode(EditMode::LongToShort)
     │  if (openSpeechEnhance) → setSpeechEnhanceChecked(true)
     └──→ emit sigEdit()  // 主窗口响应，打开编辑器

[批量导出]
FAILongToShortPresenter::doBatchExport(indexList)
     │  scriptList = scriptManager->getScripts(indexList)
     │  FF_AI_LONG_TO_SHORT_EDITOR->openProject(scriptList.first())
     │  修改工程配置：
     │      config.aspectRatio = FFProjectAspectRatioType::art9_16
     │      config.resolution  = QSize(1080, 1920)
     └──→ emit sigExport(scriptList)
          // FExport pipeline 订阅该信号，触发导出流程
          // 导出完成后恢复原始工程配置
```

---

## 五、核心技术点

### 5.1 三阶段任务流水线与状态机

Presenter 内部维护 `currentTaskType` 状态变量，Observer 回调 `onTaskStateChanged(info)` 按 `info.taskType` 分发到三个 handler：

```
onTaskStateChanged(info)
  ├── taskType == AudioLanguageRecognition → onLanguageDetectTaskStateChanged
  ├── taskType == Transfer                → onTransferTaskStateChanged
  └── taskType == Generate               → onGenerateTaskStateChanged
```

三阶段严格串行（Language → Transfer → Generate），每阶段完成才触发下一阶段，保证数据依赖（Generate 依赖 Transfer 返回的 fileId）。

### 5.2 VBL Task Manager 抽象层

`FFAILongToShortTaskService` 将上层参数转换为 VBL 对象，通过 `vblTaskManager()->addTask(vblTask)` 提交，将 AI 推理的具体实现（本地引擎 / 云端 API）完全封装在 VBL 层内部，上层对推理方式无感知。

任务回调通过 VBL Observer 机制返回，`onQueryFinished` 内使用 `QMetaObject::invokeMethod(mainThread, ..., Qt::QueuedConnection)` 确保状态更新在主线程执行，保证 UI 安全。

### 5.3 进度合成加权策略

`FProgressComposeHelper` 将三个子任务的进度按权重（Language:Transfer:Generate = 10:30:60）线性加权合成总进度，UI 层只订阅一个 `sigTaskProgressChanged` 信号，对子任务划分完全透明，同时进度条始终连续增长，不会因为阶段切换而跳动。

### 5.4 云盘导入与任务追踪

云盘导入通过 `FF_CLOUD_DISK->cloudDiskViewPresenter()` 异步触发下载，`PresenterPrivate` 维护 `m_downloadTaskIds` 集合追踪进行中的下载，并连接 `downloadFinished / taskFailed` 信号进行状态同步，下载完成后无缝衔接本地文件导入流程。

### 5.5 AI 积分余额不足的差异化错误处理

`onGenerateTaskStateChanged` 中对错误码进行细分判断：

```cpp
if (info.errorCode == kNoEnoughAICredits) {
    // 弹出余额不足对话框，引导用户购买积分
    SendCustomEvent<NotifyShowNoEnoughAICreditsDlg>();
} else {
    // 通用错误提示，附带重试按钮
    emit sigTaskStateChanged(Error, info);
}
```

保证用户在不同错误场景下获得有针对性的引导，而不是统一的"生成失败"提示。

### 5.6 历史任务结果的异步加载

用户从历史记录恢复某次任务结果时，通过 `QtConcurrent::run` 在后台线程加载：

```cpp
d_ptr->resultFuture = QtConcurrent::run([this, taskInfo]() {
    d_ptr->scriptManager->openResults(taskInfo);
});
```

避免历史脚本文件 IO 阻塞主线程，加载完成后通过信号通知 UI 切换到结果视图。

---

## 六、设计模式应用

### 6.1 MVP 模式

`FAILongToShortPresenter` 与 `FAILongToShortDialog / View` 严格分离，Presenter 通过信号（`sigTaskStateChanged / sigEdit / sigExport`）驱动 UI 更新，UI 只负责展示和用户输入转发，不直接调用任何 AI 服务接口。

### 6.2 Observer 模式

- `IFFAIServiceEventObserver`：PresenterPrivate 实现该接口，由 VBL 任务服务在任务状态变更时回调
- `IFFAILongToShortScriptManagerObserver`：脚本管理器的事件观察者，脚本列表变更时通知 Presenter
- 解耦了 AI 服务层与业务层，服务层无需知道上层是 Presenter 还是其他消费者

### 6.3 Facade / Adapter 模式

`FFAILongToShortTaskService` 是 VBL 引擎的适配器：将上层的 `FFAILongToShortGenerateTaskParam` 通过 `toAILongToShortObj()` 转换为 VBL 内部对象，对外暴露简洁的任务接口，屏蔽 VBL 底层细节。

### 6.4 工厂模式

```cpp
// 服务实例通过工厂函数创建，不直接 new 实现类
audioLanguageRecognitionService = FFVBLMODEL::createAudioLanguageRecognitionService();
```

上层依赖接口而非实现，VBL 层可在不同平台/版本切换不同实现。

### 6.5 Command / Task 模式

每个 AI 任务（addTransferTask / addGenerateTask）通过 `addTask → startTask → Observer 回调` 三步模式执行，任务对象封装了完整的执行参数，支持 `stopTask / retryTask` 操作，是典型的任务命令模式。

---

## 七、关键类职责速查

| 类名 | 职责 |
|---|---|
| `FAILongToShortPresenter` | 总控制器，协调导入、任务流水线、结果编辑、导出、埋点 |
| `FAILongToShortPresenterPrivate` | Presenter 内部实现，实现 Observer 接口，管理任务状态机与进度合成 |
| `FAILongToShortDialog` | 主对话框，承载所有子 View，连接 Presenter 信号驱动页面切换 |
| `FAILongToShortResultWidget` | 结果列表展示，显示脚本评分、封面、字幕预览 |
| `FAILongToShortResultPresenter` | 结果页业务逻辑（排序、筛选、选中状态管理） |
| `FAILongToShortHistoryPresenter` | 历史任务管理，支持恢复/删除历史生成结果 |
| `FAILongToShortTemplateManager` | AI 模板管理，提供主题/风格模板供参数设置页使用 |
| `IFFAILongToShortTaskService` | AI 任务服务接口，定义转码/生成/查询/重试 API |
| `FFAILongToShortTaskService` | 任务服务实现，封装 VBL Task Manager 调用 |
| `IFFAILongToShortScript` | 脚本结果对象，包含评分、字幕列表、时间线、封面缩略图 |
| `FProgressComposeHelper` | 多子任务进度加权合成，对 UI 暴露统一进度值 |
| `FAILongToShortRelocateHelper` | 处理脚本引用的媒体资源重定位（路径变更/云端下载） |
| `IFFAudioLanguageRecognitionService` | 语种识别服务接口 |
