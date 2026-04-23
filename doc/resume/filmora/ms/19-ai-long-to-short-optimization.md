19-ai-long-to-short-optimization.md

# AI 长视频转短视频模块技术优化方案

---

## 一、任务失败后重试丢失原始参数问题

### 问题描述

用户点击"重试"时，`startRetryTask()` 直接重发上一次任务请求，但 `FFAILongToShortGenerateTaskParam` 中的部分参数（如 `templateContent`、`smartMusic`）在生成任务完成后会被清空或覆盖，导致重试时参数与首次提交不一致，可能产生和用户期望不同的结果。此外，若用户在失败对话框中修改了参数（如换了主题/比例）后重试，当前实现无法区分"原参数重试"和"新参数重试"。

### 优化方案

在 Presenter 内部保存"任务快照"，重试时从快照恢复：

```cpp
class FAILongToShortPresenterPrivate {
    // 每次 startGenerateTask() 前拍快照
    FFVBLMODEL::FFAILongToShortGenerateTaskParam m_lastGenerateParam;

    void startGenerateTask() {
        m_lastGenerateParam = generateTaskInfo.param; // 保存快照
        service->addGenerateTask(generateTaskInfo.param);
        service->startTask(generateTaskInfo.taskId);
    }

    void startRetryTask(bool useNewParam = false) {
        auto param = useNewParam ? generateTaskInfo.param : m_lastGenerateParam;
        service->addGenerateTask(param);
        service->startTask(generateTaskInfo.taskId);
    }
};
```

重试对话框新增"修改参数重试"和"原参数重试"两个按钮，分别传入 `useNewParam = true / false`。

---

## 二、生成完成后脚本列表冷启动加载慢

### 问题描述

`getGeneratedScripts(taskId)` 在生成完成信号触发后同步调用，脚本中含有每个候选视频的封面缩略图（`projectThumbnail()`）。若同时生成了多个候选（通常 3~5 个），封面图片的解码与 UI 渲染在主线程串行执行，导致结果页从"加载中"到"显示结果"之间有明显的白屏/卡顿（测试中约 300~800ms）。

### 优化方案

脚本列表先呈现骨架屏，封面缩略图异步加载：

```cpp
void FAILongToShortResultWidget::showScripts(
    const QList<FFVBLObjectRefPtr<IFFAILongToShortScript>>& scripts) {

    // 第一步：立即渲染骨架屏（评分、时长、字幕文字）
    for (int i = 0; i < scripts.size(); ++i) {
        auto* item = addResultItem(scripts[i]);
        item->showSkeleton(); // 封面位置显示灰色占位图
    }

    // 第二步：异步加载封面（不阻塞主线程）
    FFAsync::postTask([this, scripts]() {
        for (int i = 0; i < scripts.size(); ++i) {
            QPixmap thumb = scripts[i]->projectThumbnail(); // 可能有 IO/解码
            QMetaObject::invokeMethod(this, [this, i, thumb]() {
                m_resultItems[i]->setThumbnail(thumb);
            }, Qt::QueuedConnection);
        }
    }, FFTasksFeature(TaskPriority::kNormal));
}
```

用户看到结果列表的延迟从 800ms 降为 <50ms（骨架屏几乎即时），封面图随后逐个填入。

---

## 三、并发云盘下载缺乏超时与队列管理

### 问题描述

`PresenterPrivate::downloadCloudMedia()` 将每个下载任务的 ID 存入 `m_downloadTaskIds`，通过 `downloadFinished / taskFailed` 信号管理。但当前实现：
1. **无超时**：若云盘服务端卡住，下载任务永不返回，`m_downloadTaskIds` 永远不清空，用户界面一直显示"下载中"，无法操作。
2. **无并发上限**：用户可能在历史页快速点击多个视频触发多个下载，每个都进入队列但无法控制并发数量。

### 优化方案

引入超时定时器和并发限制：

```cpp
class FAILongToShortPresenterPrivate {
    QMap<QString, QTimer*> m_downloadTimers; // taskId → 超时定时器
    static constexpr int kDownloadTimeoutMs = 60000; // 60s 超时

    void downloadCloudMedia(FFVBLObjectRefPtr<IFFAbstractMediaItem> mediaItem) {
        if (m_downloadTaskIds.size() >= 3) {
            // 超过并发上限，提示用户等待
            emit q_ptr->sigShowMessage(tr("当前下载任务已满，请稍候"));
            return;
        }

        QString taskId = FF_CLOUD_DISK->cloudDiskViewPresenter()
                            ->downloadCloudMedia(mediaItem);
        m_downloadTaskIds.insert(taskId);

        // 创建超时定时器
        auto* timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, [this, taskId]() {
            m_downloadTaskIds.remove(taskId);
            m_downloadTimers.remove(taskId);
            emit q_ptr->sigShowError(tr("下载超时，请检查网络后重试"));
        });
        timer->start(kDownloadTimeoutMs);
        m_downloadTimers[taskId] = timer;
    }
};
```

---

## 四、生成参数设置页缺乏实时预览反馈

### 问题描述

`FAILongToShortParamSettingView` 提供模板选择、比例选择、时长选择等参数，用户需要先配置完所有参数后点击"生成"，才能看到 AI 的输出结果。若参数选择不当（如选错比例），需要重新生成（耗时 30s+）才能看到差异，调试成本极高。

### 优化方案

对部分参数提供快速预览能力，无需重新生成：

```cpp
// 比例切换只需对已有脚本做帧裁剪，不需要重新推理
void FAILongToShortResultPresenter::onRatioChanged(const QString& newRatio) {
    // 已有脚本的时间线不变，只修改导出配置
    FF_AI_LONG_TO_SHORT_EDITOR->setAspectRatio(newRatio);
    // 预览播放器立即切换裁剪模式
    emit sigPreviewRatioChanged(newRatio);
}

// 时长调整可在已生成脚本中动态截断
void FAILongToShortResultPresenter::onDurationChanged(int targetSeconds) {
    auto truncatedScript = scriptManager->truncateTo(
        currentScript, targetSeconds); // 在已有片段中截取
    playerView->loadScript(truncatedScript);
}
```

这样"比例"和"时长"参数的调整几乎是即时的，只有"主题/模板/语种"等核心参数变更才需要重新触发完整的 AI 生成流程。

---

## 五、多候选脚本切换时预览播放器重复初始化

### 问题描述

`FAILongToShortPlayerView` 在用户切换不同候选脚本时每次都重新调用 `loadScript(script)`，该函数内部会销毁旧的播放器实例并重新创建（包括 VBL 播放器上下文初始化），导致脚本切换耗时 200~500ms，有明显的视觉闪烁。

### 优化方案

复用播放器实例，切换时只更新时间线数据：

```cpp
class FAILongToShortPlayerView {
    // 持有长期存在的播放器实例
    FFVBLObjectRefPtr<IFFMediaPlayer> m_player;

public:
    void switchScript(FFVBLObjectRefPtr<IFFAILongToShortScript> newScript) {
        // 不销毁播放器，只更新时间线
        m_player->pause();
        m_player->seekTo(0);
        m_player->loadTimeline(newScript->timeline()); // 只替换时间线数据
        // 更新封面和元信息
        m_thumbnailLabel->setPixmap(newScript->projectThumbnail());
        m_scoreLabel->setText(scoreToString(newScript->score()));
        // 播放器恢复预览（自动 seek 到首帧）
        m_player->previewFirstFrame();
    }
};
```

脚本切换延迟从 ~400ms 降到 <30ms（只有时间线数据更新，播放器上下文复用）。

---

## 六、错误状态缺乏分级处理与恢复引导

### 问题描述

当前 `onGenerateTaskStateChanged` 将非"余额不足"的所有错误统一处理为"生成失败，请重试"，但实际错误类型很多：网络超时、服务端繁忙（重试可解决）、视频格式不支持（重试无效，需换格式）、视频内容违规（重试无效，需换素材）。统一提示导致用户即使重试也无法解决问题，体验差。

### 优化方案

对错误码进行分级，提供差异化的引导：

```cpp
void FAILongToShortPresenterPrivate::onGenerateTaskFailed(const FFAITaskStateInfo& info) {
    switch (info.errorCode) {
    case kNetworkTimeout:
    case kServerBusy:
        // 可重试：自动重试一次，失败后提示手动重试
        if (m_autoRetryCount < 1) {
            ++m_autoRetryCount;
            startRetryTask();
        } else {
            emit q_ptr->sigShowRetryError(tr("网络异常，请检查网络后重试"));
        }
        break;

    case kUnsupportedFormat:
        // 不可重试：提示更换视频格式
        emit q_ptr->sigShowNonRetryError(
            tr("当前视频格式暂不支持，请转换为 MP4/MOV 后重试"));
        break;

    case kContentViolation:
        // 内容违规：提示更换素材
        emit q_ptr->sigShowNonRetryError(
            tr("视频内容不符合规范，请更换素材"));
        break;

    default:
        emit q_ptr->sigShowRetryError(tr("生成失败（%1），请重试").arg(info.errorCode));
    }
}
```
