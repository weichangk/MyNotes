21-ai-cover-technical-details.md

# AI 封面制作模块技术实现细节

> 项目：Wondershare Filmora（C++17 + Qt 5.15）
> 模块：FProjectCover / AICoverTimeline（`Src/FProjectCover`、`Src/FTimelineView/AICoverTimeline`）

---

## 一、模块概述

AI 封面制作模块为 Filmora 项目提供智能封面生成与编辑能力，支持三种封面来源：**AI 智能推荐**（VBL 引擎分析视频内容自动推荐多个候选帧）、**时间线截帧**（用户在时间线上手动选帧）、**本地图片上传**（支持 Qt 原生解码与 VBL 兜底解码双通道）。选取的封面可进入独立的封面编辑器，叠加文字/贴纸/模板，最终保存到工程并在导出时嵌入。

---

## 二、整体架构

```
┌──────────────────────────────────────────────────────────────┐
│                         UI 层                                 │
│  FCoverSettingDialog（主对话框：三种来源 Tab）                  │
│  FCoverRecommendationWidget（AI 推荐缩略图列表）               │
│  FCoverFromTimelineView（时间线截帧视图）                      │
│  FCoverFromLocalView（本地图片选取+裁剪视图）                  │
│  FProjectCoverEditDialog（封面编辑：文字/贴纸/模板）            │
│  FExportPanelCoverSettingWidget（导出面板封面预览入口）         │
└──────────────────────────────┬───────────────────────────────┘
                               │ 信号/槽
┌──────────────────────────────▼───────────────────────────────┐
│                       业务编排层（Presenter）                   │
│  FCoverSettingPresenter（封面选取逻辑：推荐/截帧/本地图片）       │
│  FProjectCoverPresenter（封面保存/模板应用/导出到本地）          │
│  FAICoverTimelinePresenter（临时 Timeline 构建，预览用）         │
└──────────────────────────────┬───────────────────────────────┘
                               │ VBL 接口
┌──────────────────────────────▼───────────────────────────────┐
│                     服务/平台层（VBL Adapter）                  │
│  FFCoverRecommendationService（封装 VBL AI 推荐管理器）         │
│  IFFCoverProject（封面工程接口：setCover / updateCover）        │
│  FFProjectCoverHelper（临时目录管理 / setProjectThumbnail）     │
│  IVbVideoCoverRecommendManager（VBL AI 推荐引擎，底层）         │
└──────────────────────────────────────────────────────────────┘
```

### 模块间数据流

```
用户操作（选帧/选图/点击推荐项）
     │ emit sigSaveCoverToProject(QImage, localPath, templateMedia)
FCoverSettingPresenter
     │ FProjectCoverPresenter::updateCover(CoverInfo)
     │      IFFCoverProject::updateCover(image, localFile)
     │      applyCoverTemplateItem(templateMedia)  ← 若有模板
     └── IFFCoverProject::archive()  → 持久化到工程包
```

---

## 三、核心数据结构

### 3.1 FFCoverRecommendationService::CoverData（AI 推荐结果）

```cpp
struct CoverData {
    QPixmap                  image;          // 推荐帧的缩略图
    qlonglong                frame;          // 对应的时间线帧索引
    IFFAbstractMediaItemPtr  templateMedia;  // 若有配套模板素材则非空
};
```

### 3.2 FFCoverRecommendationService::InitParam（推荐服务配置）

```cpp
struct InitParam {
    int interval{0};    // 缩略图提取间隔（传给 VBL manager）
    int maxCount{3};    // 最多推荐几个候选封面
};
```

### 3.3 FProjectCoverPresenter::CoverInfo（封面保存载体）

```cpp
struct CoverInfo {
    QImage                coverImage;    // 封面图像数据
    QString               localCoverFile; // 本地图片来源路径（截帧时为空）
    IFFAbstractMediaItem* mediaItem = nullptr; // 配套模板媒体项（推荐来源时非空）
};
```

---

## 四、主要业务流程

### 4.1 用户入口

```
[导出面板入口]
FExportPanelCoverSettingWidget（封面区域点击）
     → SendCustomEvent<OpenCoverSettingInExportDialog>()
     → FCoverSettingDialog 打开（三 Tab：From Video / From Local / AI 推荐）

[主界面入口]
菜单/工具栏 → RegisterPopupProcDialog → FCoverSettingDialog
```

### 4.2 AI 推荐封面生成流程

```
FCoverSettingPresenter 构造时：
     d->m_pService = new FFCoverRecommendationService(this)
     d->m_pService->init({interval, maxCount=3})
     d->m_pService->setTimeline(m_pTimeline)
          → FFTimeline::dmTimeline() 获取 VBL DmTimeline
          → templateApplyManager->init(undoTemplateStack, pDmTimeline)
          → manager->setTimeline(pDmTimeline)
          → 保存 frameRate 用于帧索引计算

用户切换到"AI 推荐"Tab：
FCoverSettingPresenter::startRecommendation()
     │  d->m_oTimer.start()   // 记录推荐耗时（埋点用）
     └──→ d->m_pService->generate()
              → d->undoTemplateStack->clear()
              → d->manager->generateCover()   // 交给 VBL AI 引擎，异步执行

[VBL 异步回调]
IVbVideoCoverRecommendCallback::generateCoverFinished(VBLInt index)
     → FFCoverRecommendationServicePrivate 实现该接口
     → emit q->coverReady(index)    // Qt 信号

FCoverSettingPresenter 槽函数（connect 于构造时）：
     nCnt = d->m_pService->getCount()
     for i in [0, min(nCnt, maxCoverCount)):
         CoverData oData = d->m_pService->getCover(i)
              // getCover 内部：
              // 1. manager->getCover(i) 取 VBL 原始像素 buffer
              // 2. new unsigned char[] 拷贝像素 → QImage → QPixmap
              // 3. frame = VBL::frameFromCnsTime(frameRate, vblCoverData->pos)
              // 4. templateMedia = FMediaFactory::getOrCreateMedia(vblMediaItem)
         d->m_oCoverMap.insert(i, oData)
         emit coverReady(i, oData.image)    // → FCoverRecommendationWidget::updateCover

     emit coverRecommendationFinished(nCnt) // → UI 更新推荐项总数
     记录埋点：FCoverRecommendationDurationEvent::send(timer.elapsed()/1000)
```

### 4.3 时间线截帧流程

```
FCoverFromTimelineView 构造时：
     presenter->openTimeline()
          → 创建缩略图缓存路径：app_temp/cover/{uuid}/
          → m_pThumbManager->startFetchTimelineThumbnailTask(cachePath)
          → IFFBsThumbManager 异步提取帧缩略图
          → 回调 onThumbnailReady(index, path, pixmap)
               → d->m_oThumbnailMap.insert(index, pixmap)
               → emit sigThumbnailChanged()  → UI 更新缩略图条

用户拖动/点击缩略图条选帧：
     → m_pPlayerPresenter->seek(targetFrame)
     → emit sigPlayHeadPosChanged() → UI 高亮选中项

用户点击 Save：
     FCoverSettingPresenter::timelineCoverImg()
          → d->m_pTimeline->getImage(currentFrame)  // 取当前帧图像
     emit sigSaveCoverToProject(image, "", nullptr)
```

### 4.4 本地图片上传流程

```
用户点击"Browse"选择图片：
FCoverSettingPresenter::openLocalFile(path)
     │  QtConcurrent::run(readPhoto, path)   // 异步加载，避免主线程 IO
     │    readPhotoByQt：
     │        QImageReader::setAutoTransform(true)
     │        限制最大像素（kMaxImageSize），设置 scaledSize
     │        QPixmap::fromImageReader（高效，不全量解码）
     │    readPhotoByVBL（Qt 失败时兜底）：
     │        FFLocalMediaItemFactory::createTempLocalMediaItem(path)
     │        createBsThumbManager()->getThumbnailSync(...)
     │        支持 HEIC/WEBP 等 Qt 原生不支持的格式
     │  CoverUtility::waitWithProgressDialog(future)  // 阻塞式进度对话框
     │  d->m_oLocalPicture = loaded QPixmap
     │  emit sigLocalFileOpened()
     └──→ FCoverLocalPreviewWidget 显示裁剪预览

用户调整裁剪框：
     FCoverSettingPresenter::setCropRect(rect)
     localCoverImg() 根据 cropRect + previewRect 计算最终 QImage
     emit sigSaveCoverToProject(localCoverImg(), localPath, nullptr)
```

### 4.5 封面模板应用与进度反馈

```
FProjectCoverPresenter::applyCoverTemplateItem(mediaItem, rankId)
     → m_ApplyCoverTemplateManager->applyCoverTemplateItem(...)
     → 回调 templateApplyCallback(progress, mediaItem)

templateApplyCallback(progress, mediaItem):
     if (progress < 0):
         // VBL 层没有真实进度，UI 启动假进度条
         FProjectCoverEditDialog::showApplyProgressDialog()
             → QTimer(200ms interval) 每次递增进度显示
     if (progress >= 100):
         updateTimelineAllClips()   // 刷新 timeline 内容
         sendEventUpdatePlayer()    // 刷新预览播放器
         closeApplyProgressDialog()
         emit sigApplyCoverProgressChanged(100)
```

### 4.6 封面保存与持久化

```
FProjectCoverPresenter::saveProjectCover(CoverInfo& info)
     │  m_CoverProject->setCover(info.coverImage, info.localCoverFile)
     │  if (info.mediaItem) → applyCoverTemplateItem(mediaItem, -1)
     └──→ m_CoverProject->archive(updateProjectCover=true)

FFProjectCoverHelper::addMediaIntoProject(cover, originPath)
     → 保存图像：app_temp/cover/{uuid}/{IFFProject::getCoverFileName()}.jpg
     → sourceManager->addMediaItems({tempPath})  → 加入媒体库

FFProjectCoverHelper::setCoverThumbnail(QByteArray data)
     → m_pDmProject->setProjectThumbnail(data)  // VBL 层写入工程 thumbnail
```

---

## 五、核心技术点

### 5.1 VBL AI 推荐的 Adapter 封装

`FFCoverRecommendationService` 将 VBL 的 COM 风格接口（`IVbVideoCoverRecommendManager`）适配为 Qt 信号接口。其私有实现类 `FFCoverRecommendationServicePrivate` 继承 `IVbVideoCoverRecommendCallback`，在 `generateCoverFinished(VBLInt index)` 回调中 `emit q->coverReady(index)`，对上层完全透明——上层 Presenter 只处理 Qt 信号，不感知任何 VBL 接口细节。

### 5.2 本地图片双通道解码（Qt + VBL 兜底）

```cpp
// 优先 Qt 原生解码（快速，低内存）
QPixmap readPhotoByQt(const QString& path) {
    QImageReader reader(path);
    reader.setAutoTransform(true);    // 自动处理 EXIF 旋转
    if (reader.size().width() * reader.size().height() > kMaxImageSize)
        reader.setScaledSize(scaledSize); // 限制最大像素，防止 OOM
    return QPixmap::fromImageReader(&reader);
}

// Qt 失败时 VBL 兜底（支持 HEIC/WEBP 等格式）
QPixmap readPhotoByVBL(const QString& path) {
    auto mediaItem = FFLocalMediaItemFactory::createTempLocalMediaItem(path);
    return createBsThumbManager()->getThumbnailSync(mediaItem);
}
```

两段代码通过 `QtConcurrent::run` 在线程池执行，配合 `CoverUtility::waitWithProgressDialog` 展示非阻塞进度对话框。

### 5.3 模板应用假进度条（乐观 UI）

VBL 的封面模板应用接口不提供实际进度，`FProjectCoverEditDialog` 用 `QTimer`（200ms 间隔）驱动假进度：收到 `progress < 0` 时开始推进，收到 `progress >= 100` 时关闭。用户感知到平滑的进度条，而非突然从 0% 跳到完成。

### 5.4 推荐结果帧索引计算

VBL 推荐结果的时间位置以 CNS 时间（内部时间单位）表示，通过 `VBL::frameFromCnsTime(d->frameRate, vblCoverData->pos)` 转换为帧索引。`frameRate` 在 `setTimeline` 时从 timeline 对象提取并缓存，保证后续所有帧索引计算使用一致的帧率。

### 5.5 封面编辑的临时 Timeline 隔离

`FProjectCoverCache` 维护一个"临时编辑 timeline"，封面编辑器使用该临时 timeline（而非主工程 timeline），保证封面编辑期间的文字/贴纸/模板修改不影响主时间线，用户取消编辑时直接丢弃临时 timeline，无需 Undo。

### 5.6 模板付费资源检查

`FProjectCoverPresenter::checkActivatedTemplate()` 在保存/导出前遍历已应用的模板元素，检查是否含未激活的付费资源（对接 `FF_APP_LICENSE`），若有则提示用户购买或移除，防止付费内容未经授权被导出。

---

## 六、设计模式应用

### 6.1 MVP 模式

三个 Presenter（`FCoverSettingPresenter` / `FProjectCoverPresenter` / `FAICoverTimelinePresenter`）各自对应独立的业务边界，UI 类不直接调用 VBL 接口，所有操作通过 Presenter 方法或信号完成。

### 6.2 Facade / Adapter 模式

`FFCoverRecommendationService` 是 VBL AI 推荐引擎的门面，将复杂的 COM 风格 VBL 接口封装为 Qt 信号接口；`FFProjectCoverHelper` 封装 VBL 工程的 thumbnail 写入与媒体资源注册，对上层屏蔽 VBL 底层细节。

### 6.3 Observer 模式

- `IVbVideoCoverRecommendCallback`：VBL 层回调接口，`FFCoverRecommendationServicePrivate` 实现
- Qt 信号/槽：贯穿 VBL 回调 → Service → Presenter → UI 的整条通知链

### 6.4 Builder 模式

`FAICoverTimelinePresenter` 使用 `FFTimelineBuilder` 动态构建封面预览专用的临时 timeline，根据传入的媒体项按需组装，构建完成后交给 UI 使用。

### 6.5 命令/撤销模式

封面模板应用操作通过 `IBsUndoTemplateStack` 维护 undo 快照，在重新 `generate()` 前执行 `undoTemplateStack->clear()` 清除历史，保证推荐切换不累积 undo 记录。

---

## 七、关键类职责速查

| 类名 | 职责 |
|---|---|
| `FCoverSettingDialog` | 封面选取主对话框，管理三种来源 Tab，触发保存或进入编辑 |
| `FCoverSettingPresenter` | 封面选取业务逻辑：推荐服务管理、截帧、本地图片加载、裁剪 |
| `FProjectCoverPresenter` | 封面持久化：保存到工程、模板应用、导出到本地、付费资源检查 |
| `FAICoverTimelinePresenter` | 构建封面预览用临时 Timeline，驱动封面编辑器中的播放器 |
| `FCoverRecommendationWidget` | AI 推荐缩略图列表 UI，展示推荐项并响应用户点击应用 |
| `FCoverFromTimelineView` | 时间线截帧视图，管理缩略图条和帧选择交互 |
| `FCoverFromLocalView` | 本地图片选取+裁剪视图 |
| `FProjectCoverEditDialog` | 封面编辑对话框，集成文字/贴纸/模板叠加，管理假进度条 |
| `FFCoverRecommendationService` | VBL AI 推荐引擎的 Qt Adapter，提供 `generate/stop/getCover` 接口 |
| `IFFCoverProject` | 封面工程接口：`setCover / updateCover / archive` |
| `FFProjectCoverHelper` | VBL 工程 thumbnail 写入与媒体资源注册工具类 |
| `FProjectCoverCache` | 临时封面编辑 timeline 缓存，隔离主工程与封面编辑状态 |
