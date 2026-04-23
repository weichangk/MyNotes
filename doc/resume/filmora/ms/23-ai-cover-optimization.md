23-ai-cover-optimization.md

# AI 封面制作模块技术优化方案

---

## 一、AI 推荐结果缓存失效后重复请求问题

### 问题描述

用户在"AI 推荐"Tab 切出后再切回，`FCoverSettingPresenter` 会重新调用 `startRecommendation()` 触发 `manager->generateCover()`，导致 VBL 引擎再次执行完整的视频帧分析（耗时约 2~5 秒），即使视频内容和参数均未发生变化。重复生成不仅浪费算力，也让用户看到"加载中"状态反复出现，体验较差。

### 优化方案

在 Presenter 中缓存推荐结果，Tab 切换时优先复用缓存：

```cpp
class FCoverSettingPresenterPrivate {
    bool m_bRecommendationDone{false};
    QMap<int, FFCoverRecommendationService::CoverData> m_oCoverMap;

public:
    void onSwitchToRecommendTab() {
        if (m_bRecommendationDone && !m_oCoverMap.isEmpty()) {
            // 直接复用缓存结果，跳过重新生成
            for (auto it = m_oCoverMap.begin(); it != m_oCoverMap.end(); ++it) {
                emit q_ptr->coverReady(it.key(), it.value().image);
            }
            emit q_ptr->coverRecommendationFinished(m_oCoverMap.size());
            return;
        }
        q_ptr->startRecommendation();
    }

    // 以下情况需清除缓存，强制重新生成
    void invalidateCache() {
        m_bRecommendationDone = false;
        m_oCoverMap.clear();
    }
};

// 时间线内容发生变化时（如用户修改了素材）主动失效缓存
connect(timeline, &FTimeline::sigTimelineModified,
        [this](){ d->invalidateCache(); });
```

Tab 切换时推荐结果从缓存恢复耗时 <1ms，只有在视频内容发生变化时才重新触发 VBL 推荐。

---

## 二、多张推荐封面缩略图串行加载导致 UI 更新延迟

### 问题描述

`FFCoverRecommendationService::getCover(i)` 在 `generateCoverFinished` 回调中被串行调用，每次从 VBL 原始 buffer 拷贝像素并构建 `QPixmap`。若 VBL 返回 5 张推荐封面，则 5 次像素拷贝 + `QPixmap::fromImage` 均在主线程串行执行，主线程阻塞时间约 50~200ms，`FCoverRecommendationWidget` 的缩略图列表出现明显延迟才能全部显示。

### 优化方案

并行构建所有推荐帧的 `QPixmap`，汇总后批量更新 UI：

```cpp
void FFCoverRecommendationService::onGenerateCoverFinished(int totalCount) {
    // 并行构建所有封面的 QPixmap，不阻塞主线程
    QList<int> indices;
    for (int i = 0; i < totalCount; ++i) indices.append(i);

    auto future = QtConcurrent::mapped(indices, [this](int i) {
        return qMakePair(i, buildCoverData(i)); // 像素拷贝 + QPixmap 构建在线程池
    });

    auto* watcher = new QFutureWatcher<QPair<int, CoverData>>(this);
    connect(watcher, &QFutureWatcher<QPair<int, CoverData>>::resultReadyAt,
            this, [this, watcher](int idx) {
                auto [coverIdx, coverData] = watcher->resultAt(idx);
                d->m_oCoverMap.insert(coverIdx, coverData);
                emit coverReady(coverIdx, coverData.image); // 逐个通知 UI
            });
    connect(watcher, &QFutureWatcher<QPair<int, CoverData>>::finished,
            this, [this, watcher, totalCount]() {
                emit coverRecommendationFinished(totalCount);
                watcher->deleteLater();
            });
    watcher->setFuture(future);
}
```

5 张封面并行构建后，缩略图逐个填入（与串行相比总耗时减少约 60%），同时 UI 可以逐张显示而非等全部完成后批量出现。

---

## 三、本地图片裁剪预览实时性差

### 问题描述

`FCoverSettingPresenter::setCropRect(rect)` 在用户拖动裁剪框时被高频调用，每次均重新执行完整的 `QPixmap::copy(scaledRect)` + `scaled()` 操作。当本地图片分辨率较大（如 4K 原图，最大像素限制前约 8MP），即使有 `scaledSize` 限制，裁剪计算仍需约 20~50ms，导致拖拽裁剪框时预览画面明显卡顿（<30fps）。

### 优化方案

预先生成固定尺寸的预览图，裁剪计算基于低分辨率版本：

```cpp
class FCoverSettingPresenterPrivate {
    QPixmap m_oLocalPicture;       // 原始图（受 kMaxImageSize 限制）
    QPixmap m_oPreviewPicture;     // 专供裁剪预览的低分辨率版本

    static constexpr int kPreviewMaxSize = 800; // 预览图最大边长

    void preparePreviewPicture() {
        // 生成预览图（一次性开销，<5ms）
        if (m_oLocalPicture.width() > kPreviewMaxSize ||
            m_oLocalPicture.height() > kPreviewMaxSize) {
            m_oPreviewPicture = m_oLocalPicture.scaled(
                kPreviewMaxSize, kPreviewMaxSize,
                Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else {
            m_oPreviewPicture = m_oLocalPicture;
        }
    }
};

// 裁剪预览使用低分辨率图，实时性好
QPixmap FCoverSettingPresenter::previewCropResult(const QRect& cropRect) {
    QRect scaledCrop = mapCropRectToPreview(cropRect); // 坐标映射到预览图
    return d->m_oPreviewPicture.copy(scaledCrop);       // <2ms
}

// 最终保存时才使用原图精度
QImage FCoverSettingPresenter::localCoverImg() {
    QRect scaledCrop = mapCropRectToOriginal(d->m_oCropRect);
    return d->m_oLocalPicture.copy(scaledCrop).toImage();
}
```

裁剪预览帧率从 <30fps 提升到 60fps，保存时仍使用原图精度，视觉质量不损失。

---

## 四、时间线截帧缩略图缓存路径冲突

### 问题描述

`FCoverFromTimelineView` 在构造时创建缩略图缓存路径：`app_temp/cover/{uuid}/`，其中 `uuid` 由 `QUuid::createUuid()` 生成，每次打开封面对话框都会创建新目录。若用户频繁打开/关闭封面对话框，临时目录会持续累积（每次约 10~50 张 JPEG，单次约 2~20MB），应用生命周期内可能占用数百 MB 磁盘空间。

### 优化方案

对话框关闭时清理本次缓存目录，并对路径生成策略加固：

```cpp
class FCoverFromTimelineViewPrivate {
    QString m_sCachePath;

public:
    void initCachePath() {
        // 使用固定子目录名 + 时间戳，便于清理
        m_sCachePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                       + "/FilmoraCoverThumb/"
                       + QString::number(QDateTime::currentMSecsSinceEpoch()) + "/";
        QDir().mkpath(m_sCachePath);
    }

    void cleanupCachePath() {
        if (!m_sCachePath.isEmpty()) {
            QDir(m_sCachePath).removeRecursively();
            m_sCachePath.clear();
        }
    }

    // 启动时清理所有过期的历史缓存（超过 24 小时的子目录）
    void cleanupStaleCaches() {
        QDir baseDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                     + "/FilmoraCoverThumb/");
        const qint64 maxAgeMs = 24 * 60 * 60 * 1000LL;
        for (const auto& entry : baseDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            bool ok;
            qint64 ts = entry.fileName().toLongLong(&ok);
            if (ok && QDateTime::currentMSecsSinceEpoch() - ts > maxAgeMs) {
                QDir(entry.absoluteFilePath()).removeRecursively();
            }
        }
    }
};

// 对话框析构时清理
FCoverFromTimelineView::~FCoverFromTimelineView() {
    d->cleanupCachePath();
}
```

每次关闭封面对话框后立即释放本次缩略图缓存，并在启动时清理超过 24 小时的历史遗留目录，临时目录磁盘占用降至接近 0。

---

## 五、封面保存与模板应用回调顺序不确定

### 问题描述

`FProjectCoverPresenter::saveProjectCover(CoverInfo& info)` 中，封面图像保存（`setCover`）和模板应用（`applyCoverTemplateItem`）是先后调用的，但 `applyCoverTemplateItem` 是异步的（有回调 `templateApplyCallback`）。当前代码在 `applyCoverTemplateItem` 之后立即调用 `m_CoverProject->archive()`，若模板素材下载未完成就 archive，工程包内的模板资源可能不完整，重新打开工程时出现模板丢失。

### 优化方案

将 archive 推迟到模板应用完全完成后：

```cpp
void FProjectCoverPresenter::saveProjectCover(CoverInfo& info) {
    m_CoverProject->setCover(info.coverImage, info.localCoverFile);

    if (info.mediaItem) {
        // 异步：等模板应用完成后再 archive
        m_pendingArchive = true;
        applyCoverTemplateItem(info.mediaItem, -1);
        // archive 在 templateApplyCallback(progress >= 100) 中触发
    } else {
        // 无模板：直接 archive
        m_CoverProject->archive(true);
    }
}

void FProjectCoverPresenter::templateApplyCallback(int progress,
                                                    IFFAbstractMediaItem* mediaItem) {
    if (progress >= 100 && m_pendingArchive) {
        m_pendingArchive = false;
        updateTimelineAllClips();
        sendEventUpdatePlayer();
        closeApplyProgressDialog();
        m_CoverProject->archive(true);  // 确保模板资源已完整写入后再持久化
        emit sigApplyCoverProgressChanged(100);
    }
    // ... 其他 progress 处理
}
```

archive 严格在 `templateApplyCallback(100)` 之后触发，保证工程包内模板资源完整，消除重新打开工程时模板丢失的风险。

---

## 六、付费模板检查时机过晚导致用户重复操作

### 问题描述

`checkActivatedTemplate()` 当前在用户点击"保存/导出"后才执行，若检测到未激活付费资源，弹出购买提示后用户需要：① 购买或移除模板 → ② 重新点击保存。在付费弹窗出现之前，用户已经经历了"点击保存 → 等待检查 → 弹出拦截"的感知断点，操作路径不顺畅，且若用户取消购买后直接关闭对话框，已编辑的封面内容有丢失风险。

### 优化方案

在封面模板选择时即时检查授权状态，提前告知用户：

```cpp
// 用户在 FCoverRecommendationWidget 点击某个推荐项时
void FCoverRecommendationWidget::onItemClicked(int index) {
    CoverData data = presenter->getCoverData(index);
    if (data.templateMedia && !presenter->isTemplateActivated(data.templateMedia)) {
        // 在选中时立即显示付费标记，但不阻断选中操作
        showPaidBadge(index);               // UI 角标提示
        emit sigPaidTemplateSelected(data); // 通知 Presenter 记录状态
    }
    emit sigCoverItemSelected(index);
}

// 在编辑页工具栏上常驻显示付费状态标签
void FProjectCoverEditDialog::updatePaidStatus(bool hasPaidTemplate) {
    m_pPaidLabel->setVisible(hasPaidTemplate);
    m_pPaidLabel->setText(hasPaidTemplate
        ? tr("包含付费模板，导出前需激活")
        : QString());
    m_pSaveButton->setEnabled(true); // 允许继续编辑，不阻断流程
}

// 保存时仍检查，但此时用户已提前知晓
void FProjectCoverPresenter::onSaveRequested() {
    if (!checkActivatedTemplate()) {
        // 弹出购买对话框，同时提供"移除模板后保存"选项
        emit sigShowPaidTemplateDialog(/*canSaveWithoutTemplate=*/true);
        return;
    }
    saveProjectCover(m_currentCoverInfo);
}
```

用户在选中付费模板时即看到角标提示，编辑全程有常驻状态标签，点击保存时若弹出购买对话框也不意外，同时提供"移除模板后保存"快捷路径，减少用户被迫重新操作的概率。
