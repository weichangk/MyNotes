06_ppresentation_architecture.md
# PPresentation UI 层 — 整体架构与技术方案文档

> PPresentation 是 Presentory 演示文稿创作工具的 UI 层，基于 C++17 + Qt 5 开发，承载主窗口布局、幻灯片管理、编辑画布、属性面板、AI 功能、绘图标注等全部前端交互逻辑，通过 PBL 纯虚接口体系与底层渲染引擎解耦。

---

## 一、目录结构与模块划分

```
PPresentation/
├── Presentation/          # 主应用层：应用入口、主窗口、工具栏、启动页
├── PPlayerEditor/         # 编辑画布层：渲染区、悬浮层、AI 文字、绘图板
│   └── DrawBoard/         # 屏幕标注子模块
└── PTProperty/            # 属性面板层：普通属性、动画属性
    ├── BaseUI/            # 属性 UI 基础组件
    ├── NormalProperty/    # 各 Clip 类型属性卡片
    ├── AnimationProperty/ # 动画属性面板
    └── common/            # 公共 UI 组件（颜色选择器等）
```

---

## 二、应用启动与导航架构

### 2.1 启动流程

```
main()
  └── PApplication::Exec()
        ├── initModules()       // 加载所有 PBL 后端模块（动态库）
        ├── initLang()          // 加载国际化翻译文件（10 种语言）
        ├── initAccount()       // 账号授权体系初始化
        └── PMainWindowManager::init()
              ├── IAIGC::init()           // 初始化 AI 服务
              ├── PTSetting::Load()       // 加载用户偏好设置
              ├── initEasyEditor()        // 后台拉起轻编辑器进程
              └── showStartupPage()       // 展示启动页
```

### 2.2 页面导航（Navigator 模式）

`PMainWindowManager` 是整个 UI 的**导航控制器**，持有三个顶层页面并负责切换：

```
PMainWindowManager
  ├── PStartupPage          // 启动页（最近工程列表）
  ├── PCreateProjectPage    // 新建工程页（模板选择、PPT 导入）
  └── PMainWindow           // 主编辑窗口
```

切换方法：`gotoStartPage()` / `gotoCreateProject()` / `gotoPresentation()`，通过 `show/hide` 控制窗口可见性（非路由跳转，避免重建开销）。

---

## 三、主窗口布局架构

### 3.1 五区布局

```
PMainWindow
├── PMainTitleBar (Win) / PTTitleBar (Mac)    H=48px
└── 内容区（PHBoxLayout，margin=12，spacing=12）
    ├── PMainSlides         W=200px    左侧幻灯片缩略图面板
    ├── 中央纵向布局
    │   ├── PMainTopToolBar     H=56px    顶部工具栏（导入/背景/文字/形状等）
    │   ├── PMainPlayerEditor   Expanding 中央编辑画布区
    │   └── PMainBottomToolBar  H=72px    底部录制/推流/预览控制栏
    └── PMainProperty       W=352px    右侧属性面板
```

最小尺寸：Win 1290×690，Mac 1680×820（Mac 动态适配 Dock/MenuBar 高度）。

### 3.2 标题栏平台差异化

| 平台 | 实现类 | 特点 |
|------|--------|------|
| Windows | `PMainTitleBar` | 自绘圆角+阴影，自定义最大化/最小化/关闭按钮 |
| macOS | `PTTitleBar` | 复用系统原生标题栏，适配 macOS 交互规范 |

---

## 四、全局信号总线（Event Bus）

系统核心解耦机制，所有模块间通信通过单例信号对象（Qt Signal/Slot）完成，禁止直接引用跨层对象。

### 4.1 信号总线全览

| 信号类 | 职责范围 |
|--------|---------|
| `SignalMainWindowToolBar` | 录制/预览/倒计时/进度/设备等主窗口全局事件 |
| `SignalProject` | 新建/打开/保存/粘贴/关闭工程事件 |
| `PBL::ISignalProject` | 后端工程切换、加载完成、进度等 |
| `PBL::ISignalPage` | 页面切换、Clip 增删、缩略图更新 |
| `PBL::ISignalManager` | Clip 变换/焦点/Hover/动画/摄像头等画布交互 |
| `PBL::ISignalRecorder` | 录制开始/暂停/停止/计时 |
| `PBL::ISignalPreview` | 演示预览播放控制 |
| `IAIGCSignal` | AI 请求成功/失败/积分更新 |
| `PDrawBoardSignal` | 绘图板状态变更/几何调整 |
| `PShortcutMng` | 快捷键变更/方向键启用禁用 |
| `SignalAppSettingsChanged` | 用户设置变更（自动保存间隔等） |

### 4.2 通信模式

```
模块 A（emit）→ 信号总线单例 → 模块 B（connect/slot）
```

- UI 层各模块只 connect 信号，不 include 对方头文件
- PBL 后端事件通过 `ISignal*` 单例驱动 UI 刷新
- 支持跨线程安全传递（`Qt::QueuedConnection`）

---

## 五、编辑画布层架构（PPlayerEditor）

### 5.1 分层结构

```
PMainPlayerEditor
  └── PPlayerEditorPrivate（核心协调者）
        ├── PPlayerRenderWdg          底层渲染画布（Win：禁用 QPainter，DirectX 渲染）
        ├── PPlayerMgrMask            悬浮层管理器（OverlayManager）
        │     ├── AnimationSNWdg      动画序号角标悬浮层
        │     ├── DeviceDisconnectTipWdg  设备断开提示
        │     ├── ShapeCreateMask     形状绘制引导遮罩
        │     ├── PDrawBoardArea      绘图板容器
        │     └── ImportFileFailTip   导入失败提示
        ├── PPlayerToolbarMgr         浮动工具条池管理器
        │     ├── PPlayerToolBar[]    视频 Clip 工具条（按 UUID 索引）
        │     └── PPptToolBar[]       PPT Clip 工具条
        └── AITextMgr（单例）          AI 文字管理器
              └── DCTeleprompterContrl  AI 提词器控制器
```

### 5.2 渲染画布（PPlayerRenderWdg）

核心职责：
- **坐标变换缓存**：`CalcClipTransform()` 遍历当前页所有 Clip，计算并缓存 `QHash<UUID, CLIPINFO>` 屏幕矩形映射，供 Hover/Focus 判断使用，避免每帧重算
- **底层渲染接管**：Win 下重写 `paintEngine()` 返回 `nullptr`，禁用 Qt 的 QPainter，由 PBL/WES 引擎直接渲染到 HWND（DirectX）
- **右键菜单**：内置 Copy/Cut/Paste/Delete/SelectAll/Replace/Group/Ungroup/Layer/Flip 等完整操作
- **输入法处理**：录制/预览期间禁用 `WA_InputMethodEnabled`，修复中文输入与快捷键的冲突问题

### 5.3 悬浮层管理器（PPlayerMgrMask）

`PPlayerMgrMask` 作为透明叠加层覆盖于渲染画布之上，统一管理所有悬浮 UI 组件，支持以相对坐标（0.0~1.0）定位子组件，主窗口 resize 时统一重算所有子组件位置。

### 5.4 浮动工具条（Context Toolbar）

Clip 被 Hover/Select 时，自动在 Clip 下方浮现对应工具条：
- **视频/音频 Clip** → `PPlayerToolBar`（含播放/暂停/时间轴拖拽）
- **PPT Clip** → `PPptToolBar`（含翻页控制）
- **池化管理**：`PPlayerToolbarMgr` 按 Clip UUID 索引工具条实例，避免频繁创建销毁

位置算法：工具条跟随 Clip 矩形底边，间隔固定间距，超出编辑区时上移显示。

---

## 六、幻灯片管理层（PMainSlides / PSlidesView）

### 6.1 架构设计

```
PMainSlides
  └── PSlidesView（QGraphicsView）
        └── QGraphicsScene
              └── SlideItem（QGraphicsObject，完全自绘）× N
                    ├── 缩略图（圆角矩形）
                    ├── 序号标签
                    ├── Hover/Selected 高亮（紫色主题）
                    ├── 转场按钮 → PTransitionWidget
                    └── More 菜单按钮
```

### 6.2 拖拽排序动画

1. `property("virtualPosX")` 存储目标逻辑位置与当前渲染位置分离
2. 拖动超过相邻项 50% 时触发 `m_tabs.swap()` 交换逻辑顺序
3. 被置换项通过 `QVariantAnimation + QEasingCurve::Linear` 平滑动画移动
4. 鼠标释放后执行 100ms 归位动画

### 6.3 多选交互

| 操作 | 行为 |
|------|------|
| Ctrl+Click | 切换单个选中状态 |
| Shift+Click | 范围选中 |
| Ctrl+A | 全选所有页面 |

---

## 七、属性面板系统（PTProperty）

### 7.1 架构层次

```
PProperty（顶层，含「普通属性」/「动画属性」两个 Tab）
  ├── PNormalProperty : PBaseProperty
  │     ├── NormalPropertyDataMgr     ClipType → 属性页面映射表
  │     └── PStatckedWidget
  │           ├── PCameraPanel        摄像头属性卡片
  │           ├── PVideoPanel         视频属性卡片
  │           ├── PTextPanel          文字属性卡片（含字体/描边/填充）
  │           ├── PSceneLayoutPanel   场景布局卡片
  │           └── ... 其他 Clip 类型
  └── PAnimationPanel : PBasePanel
        ├── PAnimationView            动画资源列表（入场/强调/出场三列）
        └── PAnimationOrderView       动画执行序号视图
```

### 7.2 动态分发机制

Clip 焦点变化时：
1. `NormalPropertyDataMgr::getClipType(clip)` 识别 Clip 类型
2. 切换 `PStatckedWidget` 到对应属性面板
3. 各 `PPanelOption` 从 Clip 属性中拉取最新值刷新显示

所有属性修改通过 `IUndoManager::pushCmd()` 推送命令，支持 Undo/Redo。

### 7.3 动画属性面板

- **PAnimationView**：三栏 MVC 列表（`QListView + DataModel + Delegate`），Delegate 自绘动画预览格，含下载进度、VIP 标记、选中高亮
- **PAnimationOrderView**：展示当前页所有动画的执行序号，支持拖拽调整顺序
- 序号角标 `AnimationSNWdg` 叠加在渲染画布上，按 Clip 坐标动态定位

---

## 八、AI 功能系统

### 8.1 AI 文字辅助数据流

```
用户触发（右键菜单 / 浮动工具条）
  → AITextMgr::slotMenuTrigger(funcName)
  → checkAICredits()               // 积分校验
  → IAIGC::requestTxtPrompts()     // 异步 HTTP 请求
  → NormalLoadingWidget::start()   // 展示可取消加载动画
  → IAIGCSignal::sigSuccess/Failed
  → 成功：回写文案到提词器 / 文字 Clip
  → PBrokerGlobalClient::refreshCredits()  // 同步更新积分显示
```

### 8.2 AI 功能标识符体系

AI 功能通过字符串标识符调用后端接口：

| 标识符 | 功能 |
|--------|------|
| `"shorter"` | 缩短文本 |
| `"lengthen"` | 扩写文本 |
| `"custom"` | 自定义指令 |
| `"professional"` | 专业化改写 |
| `"humorous"` | 幽默化改写 |
| `"simpler"` | 简化文本 |
| `"ImproveWriting"` | 整体润色 |

### 8.3 AIGC 工程生成流程

```
Web 端触发（PTBrokerGlobal::sigAIGCApplyFromWeb）
  → 校验当前无运行中任务
  → 检查登录状态（未登录先登录）
  → IAIGC::getContent(ssid, template_id)
  → IAIGCSignal::sigRequestRst
  → IAIGC::pickThemeTemplate()    // 弹出主题模板选择
  → 跳转主编辑窗口并应用内容
  → PBL::ISignalProject::sigAIGCApplyFinished
```

---

## 九、屏幕标注绘图板系统（DrawBoard）

### 9.1 架构

```
PDrawBoardHelper（单例，全局管理器）
  ├── PDrawBoardToolBar    浮动工具栏（笔/高亮/橡皮/光标/聚光灯）
  ├── PDrawBoard[]         每页一个绘图板，含独立 QUndoStack
  │     ├── QPixmap m_pixmap              实时绘制层（鼠标移动中）
  │     ├── QPixmap m_overlapTempPixmap   上次释放后的稳定层
  │     └── QPixmap m_overlapPixmap       最终合成输出层
  └── PDrawSpotlight       聚光灯遮罩效果
```

### 9.2 三层 QPixmap 合成

```
鼠标按下 → 在 m_pixmap 上绘制当前笔迹（CompositionMode_Source）
鼠标移动 → m_overlapTempPixmap + m_pixmap 合成显示
鼠标释放 → m_overlapPixmap 合并 m_pixmap → 作为下一笔的稳定底层
```

`CompositionMode_Source` 替换默认 Alpha 混合，解决高亮笔多次叠加导致颜色持续加深的透明度失真问题。

### 9.3 Undo/Redo 命令

```
QUndoCommand 子类：
  ├── AddShapesCommand    压入添加的笔迹对象
  ├── DeleteShapesCommand 橡皮擦删除
  └── ClearShapesCommand  清空全部
```

每个 `PDrawBoard` 持有独立 `QUndoStack`（上限 30 步），`PDrawBoardHelper` 统一调度 undo/redo。

### 9.4 跨平台实现差异

| 平台 | 绘图板实现 |
|------|-----------|
| Windows | 独立 `Qt::Tool` 窗口叠加在主窗口上，通过 `PDrawBoardSignal::sigGeometryAdjust` 跟随主窗口移动/缩放 |
| macOS | 嵌入 `PPlayerMgrMask` 作为子控件 |

---

## 十、录制状态机

`PMainBottomToolBar` 维护完整的录制状态机：

```
编辑态
  ├──[点击推流] → 推流态
  ├──[点击预览] → 预览态
  └──[点击录制] → 倒计时态（PCountCtrl 模态窗口）
                      └──[倒计时结束] → 录制中
                                           ├──[暂停] → 录制暂停
                                           │     └──[继续] → 录制中
                                           └──[停止] → 编辑态
```

录制结束后，`slotSwitchModeSaveProject()` 通过 `PBrokerRecorderClient`（进程间 IPC）拉起独立录制进程 `DemoCreatorRecorder`。

---

## 十一、工程 IO 异步模式

所有耗时工程操作（打开、保存、模板加载）统一使用**信号驱动的进度对话框模式**：

```cpp
// 统一模式：连接信号 → 创建进度框 → DoModal 阻塞 → 信号驱动关闭
PProgressDialog dlg;
connect(ISignalProject, sigProgress, dlg, [&](int p){ dlg.SetProgress(p); });
connect(ISignalProject, sigFinished, dlg, [&](bool ok){
    ok ? dlg.done(0) : dlg.setErrorTip("...");
});
connect(&dlg, sigBegin, [&]{ PRESENTATION_PROJECT->loadProject(path); });
dlg.DoModal();
```

UI 线程不阻塞（`DoModal` 内部进入局部事件循环），进度信号实时更新进度条，操作完成后自动关闭。

---

## 十二、跨进程通信（IPC Broker 架构）

| Broker 类 | 职责 |
|-----------|------|
| `PBrokerPresentoryService` | 主进程服务端，接收外部请求 |
| `PBrokerStartupClient` | 与启动进程通信 |
| `PBrokerRecorderClient` | 启动/通信录制子进程 |
| `PTBrokerGlobal` | 接收 Web/其他进程发来的打开工程/AIGC 触发请求 |
| `PBrokerGlobalClient` | 向其他进程广播（刷新积分、刷新录制文件列表）|

---

## 十三、关键设计模式汇总

| 设计模式 | 在 PPresentation 中的应用 |
|---------|--------------------------|
| **观察者/事件总线** | 全局 `ISignal*` / `Signal*` 单例，模块间零直接依赖 |
| **导航器（Navigator）** | `PMainWindowManager` 管理三个顶层页面的切换 |
| **门面（Facade）** | `PPlayerEditorPrivate` 协调编辑区所有子组件 |
| **命令模式（Command）** | `QUndoCommand` 子类封装标注操作与属性修改 |
| **对象池（Pool）** | `PPlayerToolbarMgr` 池化浮动工具条，按 UUID 复用 |
| **策略模式** | 标注工具类型（自由/直线/矩形/椭圆）运行时切换 |
| **MVC** | `PSlidesView`（View）+ `SlideItem`（Item）+ PBL（Model）|
| **单例** | `AITextMgr` / `PDrawBoardHelper` / 各 `ISignal*` 总线 |
| **依赖倒置** | UI 层只依赖 PBL `Include/` 纯虚接口，不包含任何实现 |
