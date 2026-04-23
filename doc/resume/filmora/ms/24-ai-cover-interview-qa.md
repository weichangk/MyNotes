24-ai-cover-interview-qa.md

# AI 封面制作模块面试问答

---

## 一、架构设计类

### Q1：AI 封面制作模块整体架构是怎样的？为什么设计三个 Presenter？

**回答：**

模块分三层：
- **UI 层**：`FCoverSettingDialog`（三 Tab 主对话框）、`FCoverRecommendationWidget`（AI 推荐列表）、`FCoverFromTimelineView`（截帧视图）、`FCoverFromLocalView`（本地图片裁剪）、`FProjectCoverEditDialog`（编辑器）——只负责展示，不调用任何 VBL 接口。
- **业务编排层**：三个 Presenter，各司其职：
  - `FCoverSettingPresenter`：封面来源选择逻辑（推荐服务管理、截帧、本地图片加载与裁剪）
  - `FProjectCoverPresenter`：封面持久化（保存到工程、模板应用、付费检查、导出到本地）
  - `FAICoverTimelinePresenter`：封面编辑器专用临时 Timeline 构建
- **VBL 适配层**：`FFCoverRecommendationService`（AI 推荐 Adapter）、`IFFCoverProject`（工程接口）、`FFProjectCoverHelper`（thumbnail 写入工具）

三个 Presenter 的原因：封面"选取"和封面"保存/编辑"是两个不同的生命周期，强行合并会造成职责混乱。`FCoverSettingPresenter` 只关心"选哪张"，`FProjectCoverPresenter` 只关心"怎么存"，`FAICoverTimelinePresenter` 只关心"编辑预览的 Timeline 怎么构建"，每个 Presenter 的变更都不影响其他两个。

---

### Q2：`FFCoverRecommendationService` 是如何将 VBL COM 风格回调适配为 Qt 信号的？

**回答：**

这是经典的 Adapter 模式。VBL 的 `IVbVideoCoverRecommendManager` 用 COM 风格接口（注册回调对象），不能直接 connect Qt 信号槽。

`FFCoverRecommendationService` 的私有实现类 `FFCoverRecommendationServicePrivate` 继承 `IVbVideoCoverRecommendCallback`，实现其 `generateCoverFinished(VBLInt index)` 方法：

```cpp
void generateCoverFinished(VBLInt index) override {
    emit q->coverReady(index);  // 转换为 Qt 信号
}
```

`FFCoverRecommendationService` 将 `d_ptr`（即 private 实现）注册为 VBL 的回调对象，上层 `FCoverSettingPresenter` connect `coverReady` 信号，完全不感知任何 VBL 接口。这种封装保证：VBL AI 引擎版本升级或回调接口变化，只需修改 `FFCoverRecommendationService` 内部，上层代码零修改。

---

### Q3：三种封面来源（AI 推荐/截帧/本地图片）最终如何统一收敛到保存逻辑？

**回答：**

三路来源最终都通过同一个信号 `sigSaveCoverToProject(QImage, localPath, templateMedia)` 收敛：

- AI 推荐：`emit sigSaveCoverToProject(coverImage, "", coverData.templateMedia)`（可能携带模板）
- 截帧：`emit sigSaveCoverToProject(timelineCoverImg(), "", nullptr)`（无模板，无本地路径）
- 本地图片：`emit sigSaveCoverToProject(localCoverImg(), localPath, nullptr)`（携带原始路径）

`FCoverSettingPresenter` 监听该信号后调用 `FProjectCoverPresenter::updateCover(CoverInfo)`，`CoverInfo` 中的三个字段（`coverImage` / `localCoverFile` / `mediaItem`）正好对应这三路来源的差异，`FProjectCoverPresenter` 根据字段是否为空决定后续操作（是否应用模板、是否注册媒体库）。这种设计使三路来源对保存逻辑完全透明。

---

## 二、核心机制类

### Q4：本地图片双通道解码的具体实现是什么？为什么需要两套解码路径？

**回答：**

**第一通道（Qt 原生）**：
```cpp
QImageReader reader(path);
reader.setAutoTransform(true);      // 自动处理 EXIF 旋转
if (像素数 > kMaxImageSize)
    reader.setScaledSize(scaledSize); // 限制最大像素，防止大图 OOM
return QPixmap::fromImageReader(&reader);
```

**第二通道（VBL 兜底）**：
```cpp
auto mediaItem = FFLocalMediaItemFactory::createTempLocalMediaItem(path);
return createBsThumbManager()->getThumbnailSync(mediaItem);
```

需要两套路径的原因：Qt 原生 `QImageReader` 不支持 HEIC（iOS 照片）和 WEBP（现代 Web 图片）格式，这两种格式在用户本地图片中占比较高。VBL 图像解码库内置了对这些格式的支持，可作为 Qt 失败时的后备。实际实现中，Qt 通道优先（速度快、内存效率高），只有在 Qt 返回空图时才触发 VBL 通道。两路均在 `QtConcurrent::run` 线程池中异步执行，避免阻塞主线程。

---

### Q5：封面模板应用的假进度条是如何实现的？为什么不等 VBL 回调真实进度？

**回答：**

VBL 的 `IApplyCoverTemplateManager::applyCoverTemplateItem` 是异步执行的，但回调参数 `progress` 只会传两个值：`-1`（开始执行，无进度）和 `100`（执行完成）。不提供 0~100 的真实中间进度。

为了避免用户看到进度条从 0% 突跳到 100%，`FProjectCoverEditDialog` 在收到 `progress < 0` 时启动一个 `QTimer`（200ms 间隔），每次 tick 将进度值递增（如每次 +3%~+5%，接近 90% 时减速），制造平滑递增的假进度。收到 `progress >= 100` 时立即停止 timer 并关闭进度对话框。

这是典型的"乐观 UI"策略——用伪进度提升用户感知，而不是让用户面对一个静止的转圈动画。关键点是假进度不会超过 95%（给真实完成信号留出跳变空间），且不影响功能正确性（archive 始终在真实完成后执行）。

---

### Q6：`FProjectCoverCache` 的临时 Timeline 隔离机制是什么？有什么好处？

**回答：**

封面编辑器（`FProjectCoverEditDialog`）使用的是 `FProjectCoverCache` 维护的**临时 Timeline**，而非主工程的 Timeline。

工作原理：
- 用户进入封面编辑时，`FAICoverTimelinePresenter` 用 `FFTimelineBuilder` 基于选中帧/图片构建一个只包含封面内容的临时 Timeline。
- 封面编辑期间，文字叠加、贴纸、模板应用等所有操作都发生在临时 Timeline 上。
- 若用户取消编辑，直接丢弃临时 Timeline，主工程 Timeline 无任何变化，无需 Undo。
- 若用户确认保存，才将临时 Timeline 的结果提取为图像，通过 `IFFCoverProject::updateCover` 写入主工程。

好处：
1. **无污染**：封面编辑不影响主工程 Timeline 的 Undo 历史
2. **简单清理**：取消编辑无需复杂回滚，直接销毁临时 Timeline
3. **独立渲染**：封面预览播放器使用临时 Timeline，与主播放器互不干扰

---

## 三、异步与线程安全类

### Q7：本地图片加载是在什么线程执行的？如何避免主线程阻塞？

**回答：**

本地图片的 IO + 解码通过 `QtConcurrent::run` 在线程池执行：

```cpp
QFuture<QPixmap> future = QtConcurrent::run([path]() {
    QPixmap result = readPhotoByQt(path);
    if (result.isNull()) result = readPhotoByVBL(path);
    return result;
});
```

主线程调用 `CoverUtility::waitWithProgressDialog(future)` 展示一个模态进度对话框，内部通过 `QEventLoop` + `QFutureWatcher` 实现：`future` 未完成时循环处理 Qt 事件（保持 UI 响应），`resultReadyAt` 信号触发时退出循环。

这种"阻塞式但不冻结"的方案比完全异步的方案（需要更复杂的状态机处理"加载中时用户能做什么"）更简单，对用户来说进度对话框是明确的等待反馈，避免了模糊的"UI 卡死"感。

---

### Q8：AI 推荐的 VBL 回调是在什么线程触发的？如何保证 UI 安全？

**回答：**

`IVbVideoCoverRecommendCallback::generateCoverFinished` 是由 VBL AI 引擎内部线程触发的，**可能不在主线程**。

`FFCoverRecommendationServicePrivate` 实现该接口，在 `generateCoverFinished` 中执行：
```cpp
emit q->coverReady(index); // Qt 信号
```

由于 `FFCoverRecommendationService` 是在主线程创建的 QObject，Qt 信号的默认连接方式（`AutoConnection`）会检测发送方线程与接收方所在线程：若不同，信号会被投递为 `QueuedConnection`，即通过 Qt 事件队列切回主线程后才执行槽函数。

因此 `FCoverSettingPresenter` 的槽函数（`onCoverReady(index)`）始终在主线程执行，Presenter 内部不需要额外加锁，UI 更新绝对安全。这是利用 Qt 信号/槽的线程安全机制，而非手动 `invokeMethod` 或互斥锁。

---

## 四、业务与产品类

### Q9：帧索引是如何从 VBL 的 CNS 时间单位转换来的？为什么不直接用帧号？

**回答：**

VBL 内部使用 CNS（Canonical Native Samples，规范化采样单位）表示时间位置，这是一个与帧率无关的时间单位，保证音视频同步精度。`IVbVideoCoverRecommendData::pos` 就是 CNS 时间。

转换公式：
```cpp
qlonglong frame = VBL::frameFromCnsTime(d->frameRate, vblCoverData->pos);
```

`frameRate` 在 `setTimeline` 时从 Timeline 对象提取并缓存。

不直接用帧号的原因：VBL AI 引擎内部时间轴处理是帧率无关的（同一视频以不同帧率导出时，内容位置不变），直接传帧号会在帧率不一致时产生误差。CNS 是 VBL 全层统一的时间表示，帧索引只在 UI 层显示和 Timeline seek 时才需要，转换点集中在 `getCover` 这一处，保证整个模块所有帧索引计算使用相同的帧率和转换逻辑。

---

### Q10：用户点击"保存"后，封面是如何完整持久化到工程包的？

**回答：**

持久化分三步：

**第一步：图像写入临时目录**
```
FFProjectCoverHelper::addMediaIntoProject(cover, originPath)
   → 保存图像到 app_temp/cover/{uuid}/{IFFProject::getCoverFileName()}.jpg
   → sourceManager->addMediaItems({tempPath})  加入媒体库
```

**第二步：设置工程 thumbnail**
```
FFProjectCoverHelper::setCoverThumbnail(QByteArray data)
   → m_pDmProject->setProjectThumbnail(data)  // 写入 VBL DmProject
```
这一步让工程在文件管理器/最近项目列表中显示正确的封面预览图。

**第三步：工程 archive**
```
IFFCoverProject::archive(updateProjectCover=true)
```
将整个工程（包括上述临时图像和 thumbnail 数据）打包写入 `.wfp` 工程文件。

三步完成后，封面数据同时存在于：媒体库（可在编辑器引用）、DmProject thumbnail（工程列表预览）、工程包（跨会话持久化）。

---

### Q11：付费模板资源检查是怎么实现的？检查的时机和粒度是什么？

**回答：**

`FProjectCoverPresenter::checkActivatedTemplate()` 在用户点击"保存"或"导出"时触发，遍历封面编辑器中已应用的所有模板元素：

```cpp
bool checkActivatedTemplate() {
    auto elements = m_CoverProject->getAppliedTemplateElements();
    for (auto* element : elements) {
        if (element->isPaidContent() && !FF_APP_LICENSE->isActivated(element->productId())) {
            return false; // 发现未激活付费资源
        }
    }
    return true;
}
```

检查粒度是**模板元素级**（非整个模板），因为一个模板包可能包含免费和付费元素的混合，需要逐元素检查。

检查时机在保存/导出前（而非编辑时），这是出于产品策略——允许用户先编辑再决定是否购买，降低付费门槛感。检查失败时给出两个选项：① 购买激活；② 移除付费元素后免费保存。若用户选择②，`removePaidTemplateElements()` 从临时 Timeline 中删除付费元素，然后重新走保存流程。

---

### Q12：封面对话框中三个 Tab 切换时，各自的资源（缩略图线程、推荐服务）如何管理？

**回答：**

三个 Tab 对应三个 View，其生命周期与 Tab 激活状态解耦：

- **AI 推荐 Tab**：切换到该 Tab 时调用 `startRecommendation()`（若缓存有效则直接复用），切换离开时**不停止**推荐任务（允许后台继续生成），但 `coverReady` 信号连接到 `FCoverRecommendationWidget` 时使用 `Qt::QueuedConnection`，即使 Widget 不可见也能正常接收并更新内部状态。

- **时间线截帧 Tab**：缩略图提取任务由 `IFFBsThumbManager` 管理，切换离开时不停止（缩略图可能还在生成），缩略图就绪后通过 `sigThumbnailChanged` 更新 View 内部 map；若对话框关闭则调用 `m_pThumbManager->cancelAll()` 停止所有任务并清理临时目录。

- **本地图片 Tab**：`QtConcurrent::run` 的 `QFuture` 由 Presenter 持有；切换 Tab 时不取消加载（图片加载通常极快，<1s）；若对话框关闭时 future 尚未完成，`FCoverSettingPresenter` 析构时 `future.cancel()` 取消后续操作（已开始的解码不中断，但回调不更新 UI）。

这种策略的核心原则：**不可见不等于不活跃**，资源在对话框关闭时才真正释放，避免切 Tab 时频繁启动/停止任务造成的抖动。
