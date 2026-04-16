20-ai-long-to-short-interview-qa.md

# AI 长视频转短视频模块面试问答

---

## 一、架构设计类

### Q1：AI 长视频转短视频模块整体架构是怎样设计的？为什么这样分层？

**回答：**

模块分三层：

- **UI 层**（`FAILongToShortDialog / View / ResultWidget`）：只负责展示和用户输入转发，不直接调用任何服务接口。
- **业务编排层**（`FAILongToShortPresenter`）：协调导入、三阶段任务流水线、结果管理、导出逻辑，通过信号驱动 UI。
- **AI 任务服务层**（`IFFAILongToShortTaskService / FFAILongToShortTaskService`）：封装 VBL Task Manager，将上层参数转换为 VBL 对象并提交，对上层屏蔽推理细节。

**分层原因**：AI 推理是否在本地或云端、VBL 接口版本升级，都不影响 UI 和 Presenter；UI 改版不影响任务逻辑；Presenter 是唯一知道业务规则的地方，便于测试和维护。

---

### Q2：Presenter 内部的三阶段任务是如何串联的？如何保证 Generate 一定在 Transfer 完成之后执行？

**回答：**

`FAILongToShortPresenterPrivate` 实现 `IFFAIServiceEventObserver`，在 `onTaskStateChanged(info)` 回调中按 `taskType` 分发：

```
onTransferTaskStateChanged(info)
    if (info.state == Finished) {
        fileId = service->getTransferFileId(taskId)
        generateTaskInfo.param.fileId = fileId  // 填充依赖数据
        q_ptr->generate()                        // 才触发下一阶段
    }
```

Generate 不是"定时触发"，而是严格等待 Transfer 回调 `Finished` 后才开始，保证 `fileId` 已存在。这个串联完全在回调中完成，无需外部调度器或额外同步。

---

### Q3：`FAILongToShortPresenter` 和 `FAILongToShortResultPresenter` 职责是如何划分的？

**回答：**

- **`FAILongToShortPresenter`（总控制器）**：管理整个任务生命周期——从导入、AI 生成、到触发编辑/导出，以及错误处理、埋点上报、云盘集成。它不关心结果列表的内部展示逻辑。

- **`FAILongToShortResultPresenter`（结果逻辑）**：管理生成后的脚本列表——排序、筛选、选中状态、候选视频预览切换。它不感知任务的生成过程，只消费已完成的脚本数据。

这种划分使"任务生成"和"结果消费"两个生命周期独立演化：结果页可以展示历史任务的旧脚本（不需要重新生成），也可以响应新任务完成后的刷新。

---

## 二、核心机制类

### Q4：进度条是如何做到从 0% 到 100% 连续增长的，不会因为阶段切换而跳动？

**回答：**

通过 `FProgressComposeHelper` 将三个子任务的进度按权重（Language:Transfer:Generate = 10:30:60）加权合成：

```
totalProgress = (lang_progress×10 + transfer_progress×30 + generate_progress×60) / 100
```

各子任务在 `on*TaskStateChanged` 中调用 `progressComposeHelper->setSubProgressValue(subType, info.progress)`，Helper 内部计算加权总值并触发 `sigTaskProgressChanged`。

UI 只订阅一个信号，对子任务划分完全透明。由于每个阶段的权重是预设的、进度是 0→1 单调递增的，总进度始终连续增长，不会因为"语种检测结束、转码开始"而产生跳变。

---

### Q5：用户从在线 URL 导入视频时，解析真实下载地址的流程是怎样的？

**回答：**

1. `presenter->queryDownloadUrl(request)` → `service->queryDownloadUrl(request)` 向 VBL HTTP 服务发请求
2. VBL 层 `onQueryFinished()` 回调中，使用 `QMetaObject::invokeMethod(mainThread, Qt::QueuedConnection, ...)` 将结果切回主线程
3. 主线程 `onQueryDownloadUrlSuccess(reply)` → `d_ptr->importByUrl(url)` 填充任务参数

这里有个细节：`PresenterPrivate` 缓存了 `queryVideoUrlReply`，若用户快速连续提交同一 URL，第二次请求会检查缓存直接复用结果，避免重复网络请求。

---

### Q6：AI 推理是本地执行还是云端？Presenter 是如何与推理层交互的？

**回答：**

Presenter 对推理方式完全无感知。所有任务通过 `IFFAILongToShortTaskService` 接口提交，实现类 `FFAILongToShortTaskService` 将参数通过 `toAILongToShortObj()` 转为 VBL 对象，调用 `vblTaskManager()->addTask(vblTask)`，具体是本地引擎还是云端 API 由 VBL 层内部决定（取决于运行时配置）。

Presenter 只关心三件事：**提交参数**（`addTask`）、**接收状态回调**（`IFFAIServiceEventObserver`）、**读取结果**（`getGeneratedScripts`）。这是 Facade + Adapter 模式的典型应用，推理引擎的替换或升级不影响上层任何代码。

---

## 三、异步与线程安全类

### Q7：VBL 任务回调是在什么线程触发的？属性面板如何保证 UI 安全？

**回答：**

VBL Task Manager 的任务状态回调（`IFFAIServiceEventObserver::onTaskStateChanged`）可能在非主线程触发。`FFAILongToShortTaskService` 在 `onQueryFinished` 和状态变更回调中使用 `QMetaObject::invokeMethod(mainThread, ..., Qt::QueuedConnection)` 将调用切回主线程后才通知上层 Observer。

因此 `FAILongToShortPresenterPrivate::onTaskStateChanged` 的实际执行始终在主线程，Presenter 内部不需要额外加锁，也不需要在每个信号处理中做线程检查。

---

### Q8：用户在生成过程中关闭对话框，任务会怎样处理？

**回答：**

`FAILongToShortPresenter::stopTask()` 会调用 `service->stopTask(taskId)`，通知 VBL 层停止任务。同时：
- `currentTaskType` 状态重置为 None
- `progressComposeHelper` 清零
- 若任务已进入 Generate 阶段（消耗了 AI 积分），会向用户提示"积分已消耗但任务已取消"

对于正在进行中的云盘下载，`d_ptr->cancelAllDownloads()` 遍历 `m_downloadTaskIds`，逐一调用云盘服务的取消接口。

对话框关闭后，`PresenterPrivate` 中所有 Observer 解注册，后续 VBL 回调不会再触发任何 UI 更新，避免悬空指针访问。

---

### Q9：历史任务列表恢复时，脚本文件 IO 是如何避免阻塞主线程的？

**回答：**

通过 `QtConcurrent::run` 在线程池后台执行加载：

```cpp
d_ptr->resultFuture = QtConcurrent::run([this, taskInfo]() {
    d_ptr->scriptManager->openResults(taskInfo); // 文件 IO 在后台线程
});
```

`resultFuture` 由 Presenter 持有，析构时不会强制等待任务完成（`QFuture` 默认行为）。加载完成后通过 `scriptManager` Observer 回调通知 Presenter，Presenter 再通过信号刷新 UI。

这里有个潜在问题：若用户在加载完成前快速切换到另一个历史任务，可能出现竞态（旧任务结果更新了新任务的 UI）。防御方案是在 Presenter 中维护 `currentHistoryTaskId`，回调时检查 taskInfo 是否匹配当前选中项，不匹配则丢弃结果。

---

## 四、业务与产品类

### Q10：生成多个候选短视频脚本后，AI 是如何评分排序的？评分字段怎么使用？

**回答：**

`IFFAILongToShortScript::score()` 返回 AI 对该候选视频的综合评分（浮点数），评分算法由 VBL AI 引擎决定，综合考量了内容完整性、画面质量、语义连贯性等维度，`scoreDescription()` 提供文字说明（如"情感丰富，节奏紧凑"）。

`FAILongToShortResultPresenter` 拿到脚本列表后默认按 `score()` 降序排列，第一个即为 AI 推荐的最优版本。结果 Widget 展示评分标签（如星级或百分比），用户也可以按评分或时长手动重排。

---

### Q11：批量导出多个候选视频时，如何管理工程配置的切换与恢复？

**回答：**

`doBatchExport` 在导出前动态修改工程配置：

```cpp
auto savedConfig = FF_AI_LONG_TO_SHORT_EDITOR->currentProjectConfig(); // 保存原始配置

// 修改为短视频配置
config.aspectRatio = FFProjectAspectRatioType::art9_16;
config.resolution  = QSize(1080, 1920);
FF_AI_LONG_TO_SHORT_EDITOR->applyProjectConfig(config);

emit sigExport(scriptList); // 触发导出管线

// 导出管线完成后（通过回调）恢复原始配置
FF_AI_LONG_TO_SHORT_EDITOR->applyProjectConfig(savedConfig);
```

如果用户中途取消导出，通过 `sigExportCancelled` 回调同样触发配置恢复，保证编辑器状态不被污染。多个脚本的导出是批量提交的（`scriptList`），由导出管线按序处理，Presenter 不需要逐一监控每个视频的导出状态。

---

### Q12：用户修改了生成参数后重新生成，旧的历史结果会如何处理？

**回答：**

`scriptManager` 在每次生成任务完成后以 `taskId` 为 Key 保存脚本列表，不同 `taskId` 的历史结果互不覆盖。`FAILongToShortHistoryPresenter` 展示所有历史 `taskId` 对应的生成记录，用户可以切换查看不同参数版本的结果。

重新生成时分配新的 `taskId`，旧 `taskId` 的脚本保留在历史列表中，用户可以随时回到旧版结果进行编辑或导出。历史记录通过 `scriptManager->saveScripts` 持久化到本地，跨会话可恢复，历史记录达到上限时按时间顺序淘汰最旧的记录。
