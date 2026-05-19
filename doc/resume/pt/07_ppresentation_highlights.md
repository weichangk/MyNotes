07_ppresentation_highlights.md

# PPresentation UI 层 — 功能技术亮点文档

---

## 一、三层 QPixmap 合成解决高亮笔透明度失真

### 问题背景
标注画板的高亮笔（半透明黄色笔迹）在多次叠加绘制时，由于默认 Alpha 混合模式（`CompositionMode_SourceOver`）会将每次绘制的透明度累积叠加，导致颜色越画越深，视觉上严重失真。

### 解决方案
设计三层 `QPixmap` 合成架构：

| 层 | 名称 | 职责 |
|----|------|------|
| 实时绘制层 | `m_pixmap` | 当前笔迹从鼠标按下到释放期间的增量绘制 |
| 稳定底层 | `m_overlapTempPixmap` | 上次鼠标释放后的已完成笔迹合成图 |
| 输出合成层 | `m_overlapPixmap` | 最终向屏幕输出的合成结果 |

关键在于 `m_pixmap` 绘制时使用 `QPainter::CompositionMode_Source` 代替默认混合模式——`Source` 模式直接替换目标像素，不叠加透明度，使得同一笔迹区域无论经过多少次绘制，颜色始终保持一致。

### 效果
高亮笔多次叠加颜色不变深，演示标注视觉效果准确稳定。

---

## 二、Clone Timeline 异步缩略图抓帧

### 问题背景
大量幻灯片（>10 页）场景下，缩略图生成需要对时间轴逐页 seek + 渲染，若在主预览 Timeline 上操作，会污染预览画面（跳帧），且 tlb 不支持多线程操作同一 Timeline。

### 解决方案
```
触发 updateThumbnail(indices)
  ├── ≤10 页：直接用主 Timeline 当前帧快速抓取
  └── >10 页：
        ├── WESManager::CloneTimeline()  // 克隆独立 tlb 时间轴副本
        ├── QThread::create(lambda) → 后台线程
        │     └── 逐页 GetFrame(clonedTimeline, pos) → emit sigPageThumbnail(i, img)
        └── cancelUpdateThumbnail(wait=true)  // 支持安全取消+阻塞等待
```

克隆的 Timeline 共享 Clip 数据但 seek 位置独立，后台线程可随意 seek 而不影响主预览；`cancelUpdateThumbnail(wait=true)` 在页面删除前确保后台线程完全退出，消除竞态崩溃。

### 效果
主线程渲染不被干扰，幻灯片列表滚动与缩略图更新并行进行，无论多少页均流畅。

---

## 三、SafePtr + Command 模式的无悬空指针 Undo/Redo

### 问题背景
Undo/Redo 系统中，命令对象可能在 Clip 已被删除后才执行，若 lambda 捕获裸指针会导致悬空访问崩溃。

### 解决方案
`pushCmd(IBaseClipPtrs clips, redoFn, undoFn)` 接口设计：
- 调用方传入 `SafePtr` 列表（侵入式引用计数智能指针）
- `BaseCommand` 内部持有这些 `SafePtr`，在命令存活期间保持对象引用计数 ≥ 1
- lambda 通过 `SafePtr` 访问对象，永远不会拿到已析构的裸指针

宏命令组合：
- `beginComposite(name)` / `endComposite()` 对应 `QUndoStack::beginMacro / endMacro`
- 批量操作（拖拽排序、批量删除页面）作为单一 Undo 步骤，保证操作粒度符合用户直觉

### 效果
彻底消除多线程场景下 Undo 悬空指针崩溃；宏命令提供专业级 Undo 粒度。

---

## 四、信号驱动的异步进度对话框模式

### 问题背景
工程打开/保存/模板加载均是重型 IO 操作，若在主线程同步执行会冻结 UI；若完全异步又难以保证操作顺序和错误处理。

### 解决方案
统一「信号驱动的进度对话框」模式：

```cpp
PProgressDialog dlg;
// 连接进度信号 → 实时更新进度条
connect(&ISignalProject, sigProgress, &dlg, [&](int p){ dlg.SetProgress(p); });
// 连接完成信号 → 关闭对话框
connect(&ISignalProject, sigFinished, &dlg, [&](bool ok){
    ok ? dlg.done(0) : dlg.setErrorTip(tr("Operation failed..."));
});
// 对话框出现时触发实际操作
connect(&dlg, sigBegin, [&]{ PRESENTATION_PROJECT->loadProject(path); });
dlg.DoModal();  // 局部事件循环，不阻塞主线程事件处理
```

`DoModal()` 内部进入局部事件循环（非系统模态阻塞），主线程仍可处理信号和绘制事件，进度实时更新，操作完成后信号驱动关闭。

### 效果
所有重型操作统一体验，UI 不冻结，进度可见，错误可处理，代码模式可复用。

---

## 五、PSlidesView 流畅拖拽排序动画

### 问题背景
幻灯片列表需要支持拖拽重排，且排序过程中其他项目需要平滑动画移动到新位置，而非瞬间跳变。

### 解决方案
逻辑位置与渲染位置分离：
1. `item->setProperty("virtualPosX", targetY)` 存储目标逻辑位置，与当前渲染位置解耦
2. 拖动时实时计算与相邻项重叠比例，超过 50% 触发 `m_tabs.swap()` 交换逻辑顺序
3. 被移位的项通过 `QVariantAnimation + QEasingCurve::Linear` 平滑动画移动到新目标位置
4. 鼠标释放时播放 100ms 归位动画，保证拖动结束有明确的视觉反馈

### 效果
拖拽体验流畅，视觉反馈清晰，与主流演示软件（PowerPoint/Keynote）体验一致。

---

## 六、动画序号角标悬浮层（AnimationSNWdg）

### 问题背景
动画属性面板切换到「序号视图」时，需要在每个 Clip 的左上角显示动画序号按钮，且需随 Clip 在画布上的实际渲染位置精确对齐。

### 解决方案
`AnimationSNWdg` 设计为透明叠加层（`WA_NoSystemBackground + WA_TranslucentBackground`）：
1. 序号按钮 `AnimationSNItem` 通过 `btn->setParent(parentWidget())` 跳出父容器，放置到 `PPlayerRenderWdg` 的父级
2. 根据 PBL 返回的 Clip 变换信息（`PBL::ISignalManager::sigTransform`）实时计算屏幕坐标：
   ```cpp
   int x = clip.l / devicePixelRatio - btn->width() - 8;  // Clip 左边缘向左 8px
   int y = clip.t / devicePixelRatio + panelOffset;
   btn->move(x, y);
   ```
3. 拖动 Clip 时隐藏角标（`sigTransform isBegin=true`），松开后重新定位显示

### 效果
序号角标精确跟随 Clip 位置，不影响渲染画布，交互体验干净。

---

## 七、浮动工具条对象池（PPlayerToolbarMgr）

### 问题背景
画布上可能同时存在多个 Clip，每次 Hover/Select 时需要弹出对应工具条。频繁创建/销毁 Qt 控件开销大，且可能出现短暂的空白闪烁。

### 解决方案
`PPlayerToolbarMgr` 实现工具条对象池：
- 以 Clip UUID 为 key，维护 `QHash<QString, PPlayerToolBar*>` 缓存
- 首次需要时创建，后续复用同一实例，只更新状态和位置
- 当前非活跃的工具条隐藏（`hide()`），不销毁
- 工程关闭或页面切换时统一清理池中所有实例

### 效果
工具条出现无闪烁，内存占用稳定，性能开销最小化。

---

## 八、AI 积分体系集成

### 设计
AI 功能统一经过三步校验：

```
1. checkAICredits()
   ├── 非收费环境（PAppTempSettings::getPrChargeStatus() == false）→ 直接放行
   └── 积分 ≤ 0 → 弹出积分购买对话框（IAIGC::showCreditsDlg）

2. 发起 AI 请求（IAIGC::requestTxtPrompts）
   └── 展示 NormalLoadingWidget（可取消）

3. 完成后刷新积分
   └── PBrokerGlobalClient::refreshCredits()  // 跨进程通知所有窗口同步积分显示
```

`PFloatSubEdit`（AI 输入对话框）内置实时字符计数器（上限 150 字），通过 QSS 对象名切换实现达到上限后输入框变色提示，`QTimer::singleShot(0)` 延迟处理确保布局刷新在当前事件处理后执行。

---

## 九、跨平台渲染画布差异处理

### Windows 底层渲染接管
```cpp
// PPlayerRenderWdg.cpp（Win）
QPaintEngine* PPlayerRenderWdg::paintEngine() const {
    return nullptr;  // 禁用 Qt 软件渲染
}
```
配合 `setAttribute(Qt::WA_NativeWindow)` + `setAttribute(Qt::WA_PaintOnScreen)`，将 HWND 句柄直接传给 PBL/WES 引擎，由引擎通过 DirectX 渲染到窗口，消除 Qt 与 DirectX 双重渲染的 z-order 冲突。

### macOS Retina/高 DPI 坐标换算
所有 Clip 坐标换算统一使用 `devicePixelRatioF()`：
```cpp
QRect screenRect = QRect(
    clip.l / devicePixelRatioF(),
    clip.t / devicePixelRatioF(),
    clip.w / devicePixelRatioF(),
    clip.h / devicePixelRatioF()
);
```

---

## 十、用户引导状态机系统

引导系统通过 `PUserGuideManager` 状态机控制三个阶段的引导时机：

| 状态 | 引导组件 | 触发时机 |
|------|---------|---------|
| 创建阶段 | `PCreateGuideWdg` | 首次进入新建工程页 |
| 编辑阶段 | `PEditGuideWdg` | 首次打开主编辑窗口 |
| 推流阶段 | `PStreamGuideWdg` | 首次点击推流按钮 |

气泡提示组件 `PTriangleBubble` 自绘带三角指示箭头的圆角矩形气泡，三角朝向（上/下/左/右）通过枚举参数控制，自动计算锚点偏移量使气泡精准指向目标控件。

---

## 十一、埋点事件追踪体系

代码全面覆盖用户行为追踪，关键埋点包括：

| 事件类型 | 追踪点 |
|---------|-------|
| 工程操作 | 新建/打开/保存/工程类型（视频/PPT） |
| AI 功能 | 每类 AI 请求成功/失败/耗时 |
| 录制 | 录制时长、录制模式 |
| 模板 | 模板使用 ID、主题类型 |
| 付费 | 积分不足触发、升级引导点击 |
| 自动保存 | 自动保存成功/失败/间隔设置 |

通过 `WONDERSHARE_TRACKER_INFO->SendClickEvent(...)` 和 `PEventTrackingHelper::GetInstance()->sendEvent(...)` 两套埋点 SDK 上报，覆盖 A/B 测试数据收集需求。

---

## 十二、磁盘空间保护机制

`DCFreeSpaceChecker` 在录制/推流期间定期检测剩余磁盘空间，空间不足时：
1. 触发 `onNotEnoughSpace()` 弹出警告对话框
2. 给出明确的剩余空间数值与操作建议（删除文件 / 更换输出路径）
3. 可选择继续录制（风险自担）或立即停止

防止录制中途因磁盘满导致输出文件损坏的用户体验问题。
