35-player-optimization.md
# 播放器模块技术优化方案

---

## 一、seek 完成后静帧预览模式未能及时退出

### 问题描述

`exitPreviewImageMode()` 在 `pctSeekFinished` 回调中被调用，将 `QStackedWidget` 从静帧视图（`FFPreviewImageWidget`）切换回 VBL 渲染视图（`FFVBLMediaPlayerWidget`）。但由于 VBL 引擎在 seek 完成后到首帧渲染完成之间存在约 20~80ms 的空白期，`exitPreviewImageMode()` 过早切换导致这段时间内渲染窗口显示黑帧，用户看到"静帧→黑帧→新帧"的闪烁，特别是在快速连续 seek（拖动时间线）时频繁出现。

### 优化方案

等待首帧渲染完成后再退出静帧预览模式：

```cpp
// 新增：监听首帧渲染完成事件
bool FFMediaPlayerPresenter::onPlayerChangedEvent(FFPlayerChangedType type, ...) {
    switch (type) {
    case pctSeekFinished:
        // 不立即退出静帧，仅标记"等待首帧"
        m_bWaitingFirstFrame = true;
        break;

    case pctProgress:
        // seek 后的第一个 pctProgress 事件代表首帧已渲染
        if (m_bWaitingFirstFrame) {
            m_bWaitingFirstFrame = false;
            // 首帧已在 VBL 渲染窗口显示，此时再切换视图（无黑帧）
            QMetaObject::invokeMethod(this, "exitPreviewImageMode",
                Qt::QueuedConnection); // 下一帧事件循环执行，确保帧已呈现
        }
        break;
    }
}
```

"静帧→黑帧→新帧"三步闪烁变为"静帧→新帧"无缝过渡，快速拖动时间线时视觉体验显著提升。

---

## 二、缩略图提取任务无优先级控制，长时间线滚动卡顿

### 问题描述

`IFFBsThumbManager` 按提交顺序串行提取帧缩略图（FIFO 队列），当用户打开一条有 200 个 Clip 的长时间线时，所有 Clip 的缩略图任务按序提交。用户快速滚动时间线到第 150 个 Clip，仍需等待前 149 个 Clip 的缩略图提取完成后才能看到当前可见区域的缩略图，可见区域缩略图延迟长达数十秒。

### 优化方案

按可见区域动态调整提取优先级（LIFO 优先可见帧），与 FFAsync 的 `TaskDequeueMethod::LIFO` 理念一致：

```cpp
class FFTimelineThumbnailPresenter {
    // 可见范围变化时（滚动、缩放），重新排队可见区域任务
    void onVisibleRangeChanged(int visibleStartClip, int visibleEndClip) {
        // 取消尚未开始的可见区域之外的任务
        m_thumbManager->cancelPendingTasks([&](int clipIndex) {
            return clipIndex < visibleStartClip || clipIndex > visibleEndClip;
        });

        // 重新提交可见区域任务（高优先级）
        for (int i = visibleStartClip; i <= visibleEndClip; ++i) {
            if (!m_oThumbnails.contains(i)) {
                m_thumbManager->submitTask(i, TaskPriority::kHigh);
            }
        }

        // 将可见区域外的任务以低优先级补充提交（后台预取）
        for (int i = 0; i < totalClips; ++i) {
            if (!m_oThumbnails.contains(i)
                && (i < visibleStartClip || i > visibleEndClip)) {
                m_thumbManager->submitTask(i, TaskPriority::kLow);
            }
        }
    }
};
```

可见区域缩略图从"等待整条时间线"缩短到 <500ms（可见区域通常只有 10~20 个 Clip），长时间线滚动体验接近即时。

---

## 三、多实例播放器音量同步使用轮询而非事件驱动

### 问题描述

`sycnMainPlayerVolume()` 在 `FFMediaPlayerPresenter` 构造时和每次媒体切换时被调用，读取 `SystemConfig::MainPlayerVolume` 并 `setVolume`。但当用户在主播放器调整音量时，其他播放器实例（如高级字幕编辑播放器）的音量不会立即同步——只有在这些实例的下次媒体切换时才会读取新音量，存在"音量已改但旧实例仍使用旧音量"的不一致窗口。

### 优化方案

使用单一音量信号总线，所有实例实时同步：

```cpp
// 全局音量管理器（单例）
class FFPlayerVolumeManager : public QObject {
    Q_OBJECT
    int m_globalVolume{100};

public:
    static FFPlayerVolumeManager* instance();

    void setVolume(int volume) {
        if (m_globalVolume == volume) return;
        m_globalVolume = volume;
        SystemConfig::setMainPlayerVolume(volume); // 持久化
        emit sigVolumeChanged(volume);             // 广播
    }

Q_SIGNALS:
    void sigVolumeChanged(int volume);
};

// 每个 FFMediaPlayerPresenter 在构造时注册
FFMediaPlayerPresenter::FFMediaPlayerPresenter() {
    connect(FFPlayerVolumeManager::instance(),
            &FFPlayerVolumeManager::sigVolumeChanged,
            this, [this](int vol) { setVolume(vol); });
    // 初始化时同步当前音量
    setVolume(FFPlayerVolumeManager::instance()->getVolume());
}
```

用户调整主播放器音量后，所有在线播放器实例在同一帧内完成同步，音量不一致窗口缩短为 0。

---

## 四、全屏切换时 VBL SwapChain 与 Qt 窗口尺寸不同步

### 问题描述

`enterFullScreen()` 中，Qt `setGeometry(screen->geometry())` 和 `vblPlayer->setFullScreen(true)` 是顺序调用的，但 `setGeometry` 是异步的（Qt 不保证立即生效，需等下一帧 `resizeEvent`）。若 `setFullScreen(true)` 在 `resizeEvent` 之前执行，VBL 的 SwapChain 尺寸仍是窗口化尺寸，导致全屏后视频只在屏幕左上角渲染，其余区域黑色，直到下次 `resizeEvent` 才修正。

### 优化方案

在 `resizeEvent` 中通知 VBL 而非在 `enterFullScreen` 中：

```cpp
// FFVBLMediaPlayerWidget
void resizeEvent(QResizeEvent* event) override {
    QWidget::resizeEvent(event);
    // 每次尺寸变化均通知 VBL 调整 SwapChain（包括全屏切换后的尺寸）
    if (m_vblPlayer) {
        m_vblPlayer->setRenderSize({width(), height()});
    }
}

// FFPlayerFullScreenPreviewHelper::enterFullScreen()
void enterFullScreen() {
    // 1. 仅设置 Qt 窗口状态（异步）
    widget->setWindowFlags(fullScreenFlags);
    widget->setGeometry(screen->geometry());
    // 2. 不调用 setFullScreen(true)，改为在 resizeEvent 中响应
    //    VBL 会在收到新 size 后自动调整 SwapChain
}
```

全屏切换后 VBL SwapChain 与窗口尺寸严格同步（通过 `resizeEvent` 驱动），消除全屏后黑边问题，同时窗口拖拽改变尺寸时也能实时适配。

---

## 五、Quick Preview 的 QEventLoop 阻塞时间过长

### 问题描述

`QEventLoop::exec(ExcludeUserInputEvents)` 阻塞期间，虽然用户输入被过滤，但若 VBL Quick Preview 由于复杂特效（粒子/3D 等）耗时超过 500ms，用户感知到明显的卡顿（工具栏按钮无法点击、滚动无响应）。更严重的情况：若 VBL 引擎在 Quick Preview 期间发生错误未触发完成回调，`QEventLoop` 永久阻塞，程序表现为"无响应"。

### 优化方案

引入超时保护和进度反馈：

```cpp
void FFMediaPlayerPresenter::waitQuickPreviewFinished() {
    QEventLoop loop;

    // 超时保护（3s 未完成则强制退出）
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(3000);
    connect(&timeout, &QTimer::timeout, &loop, [&loop]() {
        qWarning() << "[Player] QuickPreview timeout, force exit loop";
        loop.quit();
    });

    connect(this, &FFMediaPlayerPresenter::sigWaitQuickPreviewFinished,
            &loop, &QEventLoop::quit);

    timeout.start();
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    // 超时后继续，不永久阻塞
}
```

Quick Preview 超时（复杂特效 / VBL 错误）最长阻塞 3s 后自动恢复，程序不会无响应，同时 `qWarning` 留下可调试的日志。

---

## 六、截帧输出路径选择每次弹框，打断工作流

### 问题描述

`FFSnapshotFormatDialog` 在"首次截帧时弹出格式选择"，用户选择格式和路径后保存为偏好。但当用户批量截帧（连续点击多次截帧按钮）时，每次的输出文件名自动递增（`snapshot_001.jpg → snapshot_002.jpg`），这是正确的。然而若用户修改过工程后切换到另一个工程，截帧路径仍指向上一工程的目录，新工程的截帧文件落在错误目录里，用户需要手动去旧目录寻找文件。

### 优化方案

截帧路径与当前工程关联，工程切换时自动更新：

```cpp
class FFMainPlayerPresenter {
    void onProjectChanged(IFFProject* newProject) {
        if (!newProject) return;

        // 默认截帧目录 = 工程文件所在目录/snapshots/
        QString projectDir = QFileInfo(newProject->filePath()).absolutePath();
        QString snapshotDir = projectDir + "/snapshots/";
        QDir().mkpath(snapshotDir);

        // 更新截帧配置（不弹框，静默更新）
        FFMainPlayerConfig::setSnapshotDir(snapshotDir);
    }

    void snapshot() {
        QString dir = FFMainPlayerConfig::snapshotDir();
        // 自动生成不冲突的文件名
        QString path = generateUniqueSnapshotPath(dir, "snapshot", ".jpg");
        m_editorPresenter->snapshot(path);
        // 截帧完成后在通知栏显示"已保存到 [path]"（可点击打开目录）
    }
};
```

截帧文件自动落在当前工程目录下的 `snapshots/` 子目录，工程切换后自动更新，用户无需手动管理路径，批量截帧工作流不被弹框打断。
