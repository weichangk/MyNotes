27-ai-highlight-optimization.md
# AI 精彩集锦（Auto Beat Sync）模块技术优化方案

---

## 一、素材缩略图提取阻塞 UI 响应

### 问题描述

`ThumbnailHelper` 线程通过 `IFFVideoThumbnailService` 逐文件串行提取缩略图，当用户一次性导入大量素材（10~20 个文件）时，列表项缩略图需要数秒才能全部填充完毕。更关键的是：若用户在导入后立即滚动列表或点击其他控件，`SigFetchListItemInfoDone` 信号的 `QueuedConnection` 槽函数会在事件队列中积压，导致 UI 响应延迟明显。

### 优化方案

改为优先加载可见区域的缩略图，不可见项延迟加载：

```cpp
class ThumbnailHelper : public QThread {
    // 新增：可见优先队列（高优先级前置）
    struct ThumbnailTask {
        ListItemInfo* item;
        bool visible = false;
        int  rowIndex = 0;
    };

    void prioritize(int visibleStart, int visibleEnd) {
        QMutexLocker locker(&m_dataMtx);
        // 将可见范围内的任务移到队列头部
        std::stable_partition(m_todo.begin(), m_todo.end(),
            [&](const ThumbnailTask& t) {
                return t.rowIndex >= visibleStart && t.rowIndex <= visibleEnd;
            });
    }
};

// 列表滚动时通知 Helper 更新优先级
connect(listWidget, &QListWidget::verticalScrollBarValueChanged, [this](int val) {
    int start = listWidget->rowAt(0);
    int end   = listWidget->rowAt(listWidget->height());
    m_thumbnailHelper->prioritize(start, end);
});
```

可见区域缩略图优先填充，首屏显示延迟从 ~2s 降至 <200ms（只需加载 4~6 个可见项）。

---

## 二、每次重新分析前参数未全量重置导致脏数据

### 问题描述

`FFMontagePresenter::startAnalysis()` 在用户修改参数后重新分析时，复用了上次分析的 `m_pAutoHighlightMontage` 实例，仅调用 `clearMaterials()` 和 `clearEffects()`，但 `setHighlightStep`、`setBeatLevel`、`setBackgroundMusic` 等参数若本次未被用户修改，则不会重新 set，依赖上一次残留的 VBL Property 状态。若 VBL 引擎内部在 `stopAnalysis()` 后不清除全部参数，重新分析的结果可能与预期不一致（如使用了上次的 BGM 起止点而非新选区）。

### 优化方案

重新分析前重建 `IFFAutoHighlightMontage` 实例，保证全量初始化：

```cpp
void FFMontagePresenter::startAnalysis() {
    // 重建实例，彻底清除所有上次的 VBL 内部状态
    if (m_pAutoHighlightMontage) {
        m_pAutoHighlightMontage->setEventObserver(nullptr);
    }
    m_pAutoHighlightMontage.reset(IFFAutoHighlightMontage::createInstance());
    m_pAutoHighlightMontage->setEventObserver(this);

    // 全量设置参数（每次分析均完整提交）
    m_pAutoHighlightMontage->clearMaterials();
    for (auto& media : m_videoMediaList)
        m_pAutoHighlightMontage->addMaterial(media);

    m_pAutoHighlightMontage->clearEffects();
    for (auto& effect : m_effectList)
        m_pAutoHighlightMontage->addEffect(effect.media, effect.weight);

    m_pAutoHighlightMontage->setBackgroundMusic(
        m_bgmMedia, m_bgmCnsStart, m_bgmCnsEnd);
    m_pAutoHighlightMontage->setBeatLevel(m_beatLevel);
    m_pAutoHighlightMontage->setHighlightStep(m_highlightStep);

    m_pAutoHighlightMontage->startAnalysis();
}
```

每次分析均从干净实例出发，彻底消除残留参数的影响，分析结果与当前 UI 配置严格对应。

---

## 三、波形图在高 DPI 屏幕上显示模糊

### 问题描述

`FFBeatSyncPresenter::createWaveThumbnail()` 生成的 `QPixmap` 宽度固定为 Widget 的逻辑像素宽度（如 600px），在 2x 高 DPI 显示器上实际渲染区域为 1200 物理像素，导致波形图被放大后显示模糊，特别是在 macOS Retina 屏和 Windows 200% 缩放下问题明显。

### 优化方案

按设备像素比（DPR）生成高分辨率 Pixmap：

```cpp
QPixmap FFBeatSyncPresenter::createWaveThumbnail(
    const QVector<float>& samples, QWidget* targetWidget) {

    qreal dpr = targetWidget->devicePixelRatioF(); // 获取 DPR（通常 1.0 / 1.5 / 2.0）
    QSize logicalSize = targetWidget->size();
    QSize physicalSize = logicalSize * dpr;         // 实际物理像素尺寸

    QPixmap pixmap(physicalSize);
    pixmap.setDevicePixelRatio(dpr);  // 告诉 Qt 该 Pixmap 的 DPR
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 以物理像素坐标绘制波形
    int w = physicalSize.width(), h = physicalSize.height();
    // ... 波形绘制逻辑使用 physicalSize 尺寸 ...

    return pixmap; // Widget 按逻辑尺寸显示，自动映射到物理像素（清晰）
}
```

高 DPI 屏上波形图清晰度与文字/图标一致，不再出现模糊问题，且对低 DPI 屏无副作用（DPR=1.0 时行为不变）。

---

## 四、分析过程中用户无法预览已选素材

### 问题描述

`FFAutoHighlightMontageView` 中的预览播放器（`FFBeatSyncPlayerView`）仅在 BGM 预览时使用，素材区没有内嵌播放器。用户导入素材后无法在对话框内直接预览单个视频内容，需要记住哪个文件是什么内容，列表中只有缩略图和时长，用户体验不佳，特别是导入多个相似文件时容易选错。

### 优化方案

为 `FFAutoHighlightWidget` 添加悬浮预览能力：

```cpp
// 鼠标悬浮在列表项超过 500ms 触发小窗预览
void FFAutoHighlightWidget::onItemHovered(QListWidgetItem* item, int row) {
    if (m_hoverTimer) m_hoverTimer->stop();

    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    connect(m_hoverTimer, &QTimer::timeout, [this, item, row]() {
        ListItemInfo* info = getItemInfo(row);
        if (!info || !info->media) return;

        // 弹出小悬浮窗（200×150，无边框，临近列表项位置）
        if (!m_pHoverPreview) {
            m_pHoverPreview = new FFHoverPreviewWidget(this->window());
        }
        m_pHoverPreview->setMedia(info->media);
        m_pHoverPreview->show(mapToGlobal(item->pos()) + QPoint(itemWidth + 4, 0));
    });
    m_hoverTimer->start(500);
}

void FFAutoHighlightWidget::onItemLeft() {
    if (m_hoverTimer) m_hoverTimer->stop();
    if (m_pHoverPreview) m_pHoverPreview->hide();
}
```

用户悬浮 0.5s 即可看到视频首几秒的自动预览（GIF 循环效果），无需离开对话框确认素材内容，减少用户误选概率。

---

## 五、取消分析后进度对话框残留

### 问题描述

用户点击进度对话框的"取消"按钮后，`FFProgressDialog::rejected` 触发 `cancelAnalysis()` → VBL `cancelAnalysis()`，但 VBL 的取消操作是异步的：VBL 工作线程需要等当前帧分析完成才响应停止信号，期间可能经过 500ms~2s。当前实现中 `closeProgressDlgDelay(0)` 立即关闭对话框，若此时 VBL 延迟触发 `ctsUSER_STOP` 回调，`FFMontagePresenter` 可能已经析构或 UI 已切换状态，产生野指针访问风险。

### 优化方案

取消后保持进度框"取消中"状态，等待 VBL 确认停止后才关闭：

```cpp
void FFMontagePresenter::cancelAnalysis() {
    m_bCanceling = true;
    // 更新进度框提示文字，禁用取消按钮（防止重复点击）
    m_pProgressDlg->SetProgressTitle(tr("Canceling..."));
    m_pProgressDlg->SetCancelButtonEnabled(false);
    m_pAutoHighlightMontage->cancelAnalysis();
    // 不立即关闭对话框，等待 onExtractStop() 回调
}

bool FFMontagePresenter::onAutoHighlightMontageExtractStop() {
    // VBL 确认停止后，安全关闭对话框
    QMetaObject::invokeMethod(this, [this]() {
        m_bCanceling = false;
        if (m_pProgressDlg) {
            m_pProgressDlg->close();
            m_pProgressDlg = nullptr;
        }
        emit sigAnalysisCanceled();
    }, Qt::QueuedConnection);
    return true;
}
```

取消操作的完整生命周期：用户点击取消 → 进度框显示"Canceling..." → VBL 确认停止 → 进度框关闭 → `sigAnalysisCanceled` 更新 UI 状态，消除异步回调引发的野指针风险。

---

## 六、多播放器互斥逻辑在极端情况下失效

### 问题描述

`FFAutoHighlightMontageView` 内嵌三个播放器实例（BGM 波形预览、素材悬浮预览、结果 Timeline 预览），通过 `sigPauseOtherPlayer` 信号实现互斥：A 播放器开始播放时向 B、C 发信号暂停。但在快速切换场景下（用户在 A 未完全暂停前立即点 B 播放），因信号是 `QueuedConnection` 异步投递的，可能出现 A 的"暂停完成"回调晚于 B 的"开始播放"执行，导致 A 重新盖过 B 的音频输出。

### 优化方案

引入统一的播放器状态管理器，串行化播放请求：

```cpp
class FFPlayerMutexManager : public QObject {
    enum class PlayerType { BGM, Material, Timeline };
    QMap<PlayerType, FFMediaPlayerPresenter*> m_players;
    PlayerType m_activePlaying{PlayerType::BGM};

public:
    void requestPlay(PlayerType type) {
        if (m_activePlaying == type) return;

        // 先同步暂停当前播放器（直接调用，不通过信号）
        if (auto* current = m_players.value(m_activePlaying)) {
            current->pause();  // 同步调用，立即生效
        }
        m_activePlaying = type;

        // 再启动新播放器
        if (auto* next = m_players.value(type)) {
            next->play();
        }
    }
};
```

将"先停后播"从异步信号改为同步调用顺序，彻底消除竞态窗口，任意时刻最多一个播放器处于活跃状态。
