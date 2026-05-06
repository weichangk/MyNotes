# Presentory — AI 演示文稿创作工具（跨平台）

**技术栈：** C++17、Qt 5、CMake、PBL 中间层（SafePtr/IRef/ISignal*）、WES/tlb 渲染引擎、IAIGCData AI 接口、QUndoStack、Win/Mac 双平台

---

## 项目介绍

Presentory 是万兴科技面向演示场景的 AI 驱动演示文稿创作工具，支持 Windows / macOS 双平台、10 种语言，覆盖幻灯片编辑、AI 内容生成、录制、推流、提词器、屏幕标注等全套能力。

项目基于 **C++17 + Qt** 开发，采用四层模块化架构：

- **UI 层（PPresentation）：** 以 Navigator 模式管理启动页、模板创建页、主编辑窗口三个顶层页面；主窗口采用五区布局（幻灯片缩略图面板、顶部工具栏、中央编辑画布区、底部录制控制栏、右侧属性面板）；编辑画布区以分层叠加架构管理渲染画布、透明悬浮层、浮动工具条、标注画板等多个子组件；属性面板按素材类型动态切换，涵盖普通属性与动画属性两大分支
- **业务中间层（PBL）：** 以纯虚接口体系将 WES/tlb 渲染引擎与 UI 完全解耦；UI 层只依赖 `Include/` 接口头文件，不引用任何实现；11 个 `ISignal*` / `Signal*` QObject 单例信号总线覆盖工程、页面、录制、设备、AI 等全域事件，实现模块间发布-订阅解耦
- **渲染引擎层：** WES/tlb 为 C++ native 渲染引擎，由 WESManager 适配层封装，负责 GPU 渲染、时间轴编辑与帧输出；Windows 下直接接管 HWND（DirectX 渲染），macOS 下统一使用 `devicePixelRatioF()` 处理 Retina 坐标换算
- **AI 接入层：** 通过 `IAIGCData` 统一接口对接，支持文字润色（6 类场景）、提词器 AI 优化、Web 端 AI 模板生成三路功能；AI 请求生命周期统一管理积分校验、HTTP 请求、Loading 动画、成功/失败回调

---

## 核心工作与技术亮点

### 一、PBL 中间层架构设计与实现

主导设计并实现贯穿整个产品的核心业务中间层 PBL，将上层 Qt UI 与下层 WES/tlb 渲染引擎彻底解耦，承担项目生命周期管理、页面与素材操作、设备管理、Undo/Redo、预览录制控制等全套业务逻辑。

**关键设计：**
- `Include/` 目录下全部以纯虚接口暴露，UI 层只依赖接口头文件，不 include 任何实现；顶层编辑器接口作为门面统一暴露 12 个子管理器（摄像头、背景、水印、麦克风、屏幕/窗口捕获、动画等），职责单一、边界清晰
- 工厂模式统一创建 14+ 种素材类型，创建链路串联媒体信息缓存（避免重复解析）→ WES 引擎原生素材创建 → PBL 包装层 → `SafePtr` 安全返回；所有素材统一支持时间轴坐标、裁剪点、渲染↔素材时间双向映射、变换/遮罩/透明度及自定义扩展数据
- `SafePtr<T>` + `IRef` 全系侵入式引用计数（`QAtomicInteger` 原子操作），tlb 原生素材通过 `std::shared_ptr` 自定义 deleter 经引擎工厂释放，在多线程环境下实现内存安全的对象生命周期管理；`pushCmd(IBaseClipPtrs, redoFn, undoFn)` 将 redo/undo lambda 与 `SafePtr` 列表打包压入 `QUndoStack`，命令存活期间引用计数 ≥ 1，彻底消除 undo 时悬空指针崩溃
- 11 个 `QObject` 单例信号总线（项目/页面/录制/设备/预览/AI 等域）覆盖全部核心事件，UI 层只订阅信号、零依赖实现头文件，实现彻底的发布-订阅解耦

**效果：**
- UI 与渲染引擎彻底解耦，更换底层引擎时 UI 层零修改
- SafePtr 保活机制彻底消除多线程 undo 悬空指针崩溃；宏命令组合提供专业级撤销粒度

---

### 二、幻灯片页面编辑

实现完整的幻灯片页面管理能力，涵盖页面生命周期操作、拖拽排序动画，以及大量页面场景下的异步缩略图生成。

**关键设计：**
- `PSlidesView` 基于 `QGraphicsView + QGraphicsScene`，每页渲染为一个 `SlideItem`（`QGraphicsObject` 完全自绘），承载缩略图、页码标签、Hover/Selected 高亮（紫色主题）、转场按钮、More 菜单等完整交互；支持新建、复制、粘贴、拖拽排序、多选（Ctrl/Shift/Ctrl+A）、删除等全套页面操作
- 拖拽排序实现逻辑位置与渲染位置分离：`property("virtualPosX")` 存储目标逻辑位置，拖动超过相邻项 50% 时触发 `m_tabs.swap()` 交换逻辑顺序，被置换项通过 `QVariantAnimation + QEasingCurve::Linear` 平滑动画移到新位置，鼠标释放后执行 100ms 归位动画，视觉反馈与 PowerPoint/Keynote 体验一致
- 页面数量超过 10 页时，通过 `WESManager::CloneTimeline()` 克隆独立 tlb 时间轴副本，在后台线程逐页调用 `GetFrame` 抓帧；克隆 Timeline 共享 Clip 数据但 seek 位置独立，随意 seek 不污染主预览渲染状态；`cancelUpdateThumbnail(wait=true)` 在页面删除前确保后台线程完全退出，消除竞态崩溃

**效果：**
- 拖拽排序动画流畅，多选批量操作覆盖全场景
- 大量页面下缩略图生成与主线程完全并行，面板滚动不卡顿；竞态问题归零

---

### 三、可视化编辑画布

设计并实现编辑区多层叠加架构，解决 GPU 原生渲染画布与多种浮动 UI 组件共存的 z-order 冲突问题，实现精确的 Clip 坐标追踪与上下文工具条管理。

**关键设计：**
- 编辑区以 `PPlayerEditorPrivate` 为核心协调者，管理三个分层：底层 `PPlayerRenderWdg`（渲染画布）、中层 `PPlayerMgrMask`（透明悬浮层）、顶层各浮动工具条；Windows 下重写 `paintEngine()` 返回 `nullptr` 禁用 Qt 软件渲染，DirectX 直接渲染到 HWND，彻底消除双重渲染的 z-order 冲突；macOS 下统一使用 `devicePixelRatioF()` 处理 Retina 高 DPI 坐标换算
- `PPlayerRenderWdg::CalcClipTransform()` 遍历当前页所有 Clip 计算并缓存 `QHash<UUID, CLIPINFO>` 屏幕矩形映射，供 Hover/Focus 命中判断使用；`PPlayerMgrMask` 以 `WA_TranslucentBackground` 透明叠加层统一承载动画序号角标、形状引导遮罩、设备断开提示等悬浮 UI，子组件以相对坐标（0.0~1.0）定位，主窗口 resize 时统一重算
- 浮动工具条（`PPlayerToolbarMgr`）以 Clip UUID 为 key 维护对象池，Hover 时按类型弹出对应工具条（视频 Clip → `PPlayerToolBar`，PPT Clip → `PPptToolBar`），首次创建后持续复用，非活跃工具条仅 `hide()` 不销毁；位置算法跟随 Clip 矩形底边，超出编辑区时自动上移
- 动画序号角标 `AnimationSNWdg` 通过 `setParent(parentWidget())` 跳出父容器约束，订阅 `ISignalManager::sigTransform` 实时计算 Clip 屏幕坐标定位，拖动期间隐藏、松手后重新对齐

**效果：**
- GPU 渲染与 Qt 控件层完全隔离，无 z-order 冲突；跨平台坐标换算统一，Retina 显示精确
- 浮动工具条出现无闪烁，动画序号精确跟随 Clip 位置，画布交互体验干净流畅

---

### 四、素材资源属性面板

实现按素材类型动态切换的属性面板体系，覆盖摄像头、视频、文字、场景布局、背景、动画等全类型属性配置，支持多选批量编辑与完整 Undo/Redo。

**关键设计：**
- `NormalPropertyDataMgr` 按 Clip 焦点变化识别当前 ClipType，动态切换 `PStatckedWidget` 到对应属性卡片（`PCameraPanel` / `PVideoPanel` / `PTextPanel` / `PSceneLayoutPanel` 等）；所有属性修改通过 `IUndoManager::pushCmd()` 推送命令，支持撤销/重做；文字属性面板聚合字体、描边、填充色三类选项，统一色板选择组件保持跨面板视觉一致
- 动画属性面板 `PAnimationView` 基于 `QListView + DataModel + Delegate` 实现三栏列表（进场/强调/出场），Delegate 自绘动画预览格（含下载进度条、VIP 标记、选中高亮）；`PAnimationOrderView` 展示当前页所有动画执行序号，支持拖拽调整播放顺序
- 动画序号角标 `AnimationSNWdg` 作为独立叠加层覆盖于渲染画布上，按 Clip 实时坐标动态定位序号按钮，动画属性面板切换序号视图时自动显示/隐藏

**效果：**
- 属性面板自动跟随素材焦点切换，无需手动导航；多类型 Clip 统一属性编辑入口
- 动画排序与序号角标联动，所见即所得地管理动画播放顺序

---

### 五、素材资源管理

实现顶部工具栏入口下的文字、背景、形状、转场、模板等多类素材资源的浏览、预览与应用管理，覆盖从资源选取到作用于时间轴的完整流程。

**关键设计：**
- **文字素材**：支持将文字 Clip 添加到时间轴，`PTextPanel` 属性面板聚合字体、描边、填充色三类选项，统一色板选择组件保持跨面板视觉一致；监听文字素材编辑状态追踪当前选中文本区间，属性修改精准作用于选中段落；所有样式变更通过 `IUndoManager::pushCmd()` 推入命令栈，完整支持 Undo/Redo
- **背景素材**：`PSceneLayoutPanel` 背景面板支持纯色、图片、视频三类背景，以及模糊、虚化等视觉效果；背景切换通过 PBL 接口驱动底层渲染引擎即时生效，预览与最终输出保持像素级一致
- **形状素材**：创建形状时在编辑画布上覆盖 `ShapeCreateMask` 引导遮罩辅助对齐，形状创建完成后自动切换到属性面板编辑变换/填充/描边参数；形状 Clip 与视频/文字 Clip 共享相同的时间轴坐标、变换、遮罩统一契约
- **转场动效**：`SlideItem` 每页底部内置转场入口（`PTransitionWidget`），动效列表基于 `QAbstractListModel + QStyledItemDelegate` 实现，每条动效携带名称、预览图、效果参数；支持「仅当前页」与「应用到全部」两种应用模式；选中动效时自动触发预览播放，播放结束后自动回归编辑态
- **模板系统**：新建工程页（`PCreateProjectPage`）模板数据异步加载期间展示占位骨架屏（Skeleton Screen），数据就绪后平滑切换至真实模板列表，按主题/行业/场景分类提供 Tab 导航，支持分页加载；模板预览一键应用，PPT 导入前预检磁盘可用空间防止大文件失败；Web 端 AI 模板通过 `PTBrokerGlobal` 跨进程触发，经 `IAIGC::pickThemeTemplate` 弹出主题选择后应用到当前工程

**效果：**
- 五类素材资源统一挂载顶部工具栏入口，操作路径一致；所有变更均可 Undo/Redo
- 骨架屏模板加载与分页机制保证大量模板下的浏览流畅度，转场预览即点即看无需额外操作

---

### 六、录制/推流实时流管理

实现完整的录制状态机与多进程实时流管理，通过 IPC Broker 架构拉起独立录制进程，并建立信号驱动的异步 IO 模式统一所有重型操作。

**关键设计：**
- `PMainBottomToolBar` 维护完整录制状态机，涵盖编辑态 / 倒计时态 / 录制中 / 录制暂停 / 推流态 / 预览态六个状态；`PCountCtrl` 倒计时模态窗口结束后自动进入录制中；录制结束后通过 `PBrokerRecorderClient`（进程间 IPC）拉起独立 `DemoCreatorRecorder` 进程处理后期合成，主进程不阻塞
- IPC Broker 架构支持多向通信：`PTBrokerGlobal` 接收 Web 端/外部进程发来的打开工程、AIGC 模板触发请求；`PBrokerGlobalClient` 向其他进程广播积分刷新、录制文件列表更新；`PBrokerStartupClient` 与启动进程握手完成冷启动协调
- 所有重型 IO 操作（工程打开/保存/模板加载）统一使用信号驱动的进度对话框模式：`PProgressDialog::DoModal()` 内部进入局部事件循环（非系统模态阻塞），`sigProgress` 驱动进度条实时更新，`sigFinished` 驱动对话框关闭或展示错误提示，主线程全程不阻塞

**效果：**
- 录制状态机覆盖全部合法状态转移，非法操作序列在状态层拦截
- IPC Broker 多进程架构实现录制与编辑完全隔离；信号驱动进度模式统一全局重型操作体验

---

### 七、AI 辅助功能

实现演示文稿中文字素材的 AI 辅助改写、AI 提词器优化与 Web 端 AI 模板生成三路功能，覆盖从触发到结果回填的完整交互闭环。

**关键设计：**
- `AITextMgr` 单例统一管理完整 AI 请求生命周期：预检积分（`checkAICredits`，非收费环境直接放行，积分不足弹出购买对话框）→ 发起异步 HTTP 请求（`IAIGC::requestTxtPrompts`）→ 展示可取消加载动画（`NormalLoadingWidget`）→ 成功/失败回调处理 → 跨进程广播积分刷新（`PBrokerGlobalClient::refreshCredits`）；精确计时用于埋点上报
- 提供「缩短 / 扩写 / 自定义 / 专业化 / 幽默化 / 简化」六类文本改写场景，支持浮动工具条与右键菜单双入口；监听文字素材编辑状态追踪当前选中文本区间，AI 改写精准作用于选中段落而非整段；AI 提词器复用同一套管理器，两路 AI 入口共享统一的积分校验与按钮控制逻辑
- Web 端 AI 模板生成通过 `PTBrokerGlobal::sigAIGCApplyFromWeb` 跨进程触发，经登录校验 → `IAIGC::getContent` 拉取内容 → `IAIGC::pickThemeTemplate` 弹出主题选择 → 跳转主编辑窗口并应用内容，全流程状态通过信号总线驱动

**效果：**
- AI 改写能力深度嵌入文字编辑流程，双入口覆盖主要操作路径，降低演讲内容准备门槛
- 三路 AI 功能（文字/提词器/模板）共享统一积分与请求管理基础设施，扩展新 AI 场景无需重复实现鉴权与加载逻辑

---

### 八、屏幕标注画板

实现演示录制/推流过程中的实时屏幕标注工具，通过三层 QPixmap 合成架构解决高亮笔透明度失真问题，并以独立 Undo 栈保障标注操作完整可回溯。

**关键设计：**
- 三层 `QPixmap` 合成架构：`m_pixmap`（鼠标按下到释放期间的实时绘制层）+ `m_overlapTempPixmap`（上次释放后的稳定底层）+ `m_overlapPixmap`（最终合成输出层）；高亮模式将合成模式切换为 `CompositionMode_Source` 替代默认 `SourceOver`，`Source` 直接替换目标像素不叠加透明度，使同一区域无论绘制多少次颜色始终保持一致
- 支持自由笔迹、直线、矩形、椭圆等多种形状类型；聚光灯模式（`PDrawSpotlight`）在画面其余区域叠加半透明遮罩突出焦点区域；每个 `PDrawBoard` 持有独立 `QUndoStack`（上限 30 步），添加/删除/清空标注各封装为独立 `QUndoCommand`，`PDrawBoardHelper` 统一调度
- 跨平台实现差异：Windows 下标注板以独立 `Qt::Tool` 窗口叠加在主窗口上，通过 `PDrawBoardSignal::sigGeometryAdjust` 跟随主窗口移动/缩放；macOS 下嵌入 `PPlayerMgrMask` 作为子控件

**效果：**
- 高亮笔多次叠加颜色保持一致，演示标注视觉效果准确稳定
- 完整 Undo 支持，演示过程中误操作可即时回退；跨平台体验一致
