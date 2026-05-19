08_ppresentation_optimization.md

# PPresentation UI 层 — 技术优化建议文档

---

## 一、信号总线数量膨胀与连接追踪困难

### 问题

当前系统存在 10+ 个全局信号总线单例（`SignalMainWindowToolBar` / `SignalProject` / `ISignalPage` / `ISignalManager` 等），随着功能增长信号数量持续膨胀：
- 单个信号总线（如 `ISignalManager`）挂载了 Clip 变换、焦点、Hover、动画、摄像头等几十个信号，职责混杂
- 无法快速定位「哪些模块订阅了某个信号」，出现 bug 时信号链路难以复现
- 非 `QObject` 的工具类订阅信号时若忘记 disconnect，会产生野指针回调

### 优化方案

1. **细粒度拆分信号总线**：将 `ISignalManager` 按业务域拆分为 `ISignalClipTransform`（变换）、`ISignalClipFocus`（焦点/Hover）、`ISignalAnimation`（动画序号），各域信号数量可控
2. **引入 SignalConnectionTracker**：Debug 构建中记录每个信号的所有连接（信号名、sender、receiver 类名），提供 `dumpConnections()` 接口辅助排查
3. **强制 QObject 订阅方**：规范要求所有 connect 的 receiver 必须是 `QObject` 子类，利用 Qt 自动断连机制；lambda 捕获非 QObject 指针时通过代码审查拦截

---

## 二、PPlayerRenderWdg 坐标缓存失效时机粗放

### 问题

`CalcClipTransform()` 计算并缓存当前页所有 Clip 的屏幕矩形（`QHash<UUID, CLIPINFO>`），用于 Hover/Focus 的命中判断。但当前触发时机粗放：
- 每次鼠标移动事件都可能触发重算（Clip 未变化时做无效计算）
- 窗口 resize 时所有坐标失效，但部分路径未能及时触发重算，导致短暂 Hover 判断偏移

### 优化方案

1. **脏标记缓存**：引入 `m_transformCacheDirty` 标志，只在 Clip 属性变更（`sigPageClipsChanged`）或窗口 resize 时置 dirty；鼠标事件先检查标志，脏时才重算
2. **增量更新**：单 Clip 变换时（`sigTransform`）只更新该 Clip 对应的缓存条目，不重算全页
3. **延迟批量更新**：批量操作期间（`beginEditTimeline`）屏蔽 dirty 传播，`endEditTimeline` 后统一重算一次

---

## 三、PSlidesView 大量页面下的性能瓶颈

### 问题

`PSlidesView` 使用 `QGraphicsScene` + 每页一个 `SlideItem`（完全自绘），存在：
- 100+ 页时 `QGraphicsScene` 的碰撞检测和场景更新开销显著增大
- 缩略图 `QPixmap` 全部常驻内存，100 页时内存占用可达数百 MB
- 拖拽排序动画对所有 item 同时运行 `QVariantAnimation`，100+ 页时 CPU 峰值明显

### 优化方案

1. **虚拟化列表（VirtualList）**：改用 `QListView + QAbstractListModel`，只渲染可视区域内的 item（视口外 item 仅保留逻辑数据，不创建 `SlideItem` 对象），渲染数量从 N 降至常数
2. **缩略图 LRU 缓存**：维护最大 50 张缩略图的 LRU 缓存，超出时释放最久未显示的 `QPixmap`，滚动到对应位置时按需重新抓帧
3. **动画批量限制**：拖拽时只对相邻受影响的 ±3 个 item 播放移动动画，其他 item 直接跳位

---

## 四、PMainBottomToolBar 录制状态管理散乱

### 问题

底部工具栏的录制状态机通过多个布尔变量和条件判断分散管理（`m_bRecording` / `m_bPause` / `m_bStream` / `m_bPreview` 等），缺乏统一状态对象：
- 状态组合爆炸（录制中+推流、预览+暂停等）难以枚举和覆盖测试
- 新增模式（如边录边推）时需要在多处条件分支中同步修改
- 按钮 enabled/visible 的切换逻辑散落在各个 slot 中，状态与 UI 同步依赖人工维护

### 优化方案

1. **显式状态机对象**：定义 `RecordingStateMachine { state, transition(event) }`，状态枚举覆盖所有合法状态（Idle/Countdown/Recording/Paused/Streaming/Preview），非法状态转移在此拦截
2. **状态驱动 UI**：`onStateChanged(old, new)` 中统一更新所有按钮的 enabled/visible/text，消除散落的 UI 更新代码
3. **事件化操作**：所有用户操作（点击录制/暂停/停止）改为发送 `RecordEvent` 枚举到状态机，状态机决定是否接受并驱动 UI 更新

---

## 五、AITextMgr 功能耦合过重

### 问题

`AITextMgr` 当前同时承担：
- AI 请求管理（积分检查、请求发起、Loading、成功/失败回调）
- 提词器 AI 入口（持有 `DCTeleprompterContrl` 引用）
- 文字 Clip AI 入口（与 `PFloatToolBarMgr` 交互）
- 积分刷新跨进程通知（`PBrokerGlobalClient`）

职责混杂，单元测试困难，新增 AI 功能时改动范围大。

### 优化方案

1. **拆分 AIRequestService**：抽离 `AIRequestService { request(funcName, text) → Future<result> }`，统一管理积分校验、HTTP 请求、Loading 状态、成功/失败回调，不依赖任何具体 UI 模块
2. **各 AI 入口自持逻辑**：`DCTeleprompterContrl` 和浮动工具条各自持有 `AIRequestService` 实例，独立处理自己的结果回写逻辑
3. **积分刷新通过信号总线**：`AIRequestService` 成功后 emit `sigCreditsChanged`，感兴趣的模块（包括跨进程的 Broker）订阅该信号，避免 `AIRequestService` 直接依赖 `PBrokerGlobalClient`

---

## 六、属性面板 DataMgr 与 Panel 强耦合

### 问题

`NormalPropertyDataMgr` 通过枚举 key（`NormalProertyClipType`）硬编码 ClipType → Panel 的映射关系，新增 Clip 类型时需要：
1. 在枚举中添加新类型
2. 在 DataMgr 的 switch/case 中添加映射
3. 实现新 Panel 类
4. 在 `PStatckedWidget` 中注册

四处改动，容易遗漏且不符合开闭原则。

### 优化方案

1. **注册式 Panel 工厂**：定义 `IPanelFactory { bool canHandle(ClipType); PBasePanel* create(); }`，各 Panel 对应一个 Factory 类，在模块初始化时注册到 `NormalPropertyDataMgr`
2. **反射式匹配**：`DataMgr` 遍历已注册的 Factory 列表，第一个 `canHandle()` 返回 true 的 Factory 负责创建对应 Panel
3. **效果**：新增 Clip 类型只需实现新 Panel 类 + 对应 Factory 并注册，主流程零改动

---

## 七、绘图板多页面内存未及时释放

### 问题

`PDrawBoardHelper` 为每页维护一个 `PDrawBoard` 实例，每个实例持有三层 `QPixmap`（分辨率等于画布尺寸）。页面被删除后，对应 `PDrawBoard` 若未及时清理，所有 `QPixmap` 内存持续占用。在大量页面（50+）且每页都有标注时，累计占用可能超过 500MB。

### 优化方案

1. **订阅页面删除信号**：监听 `ISignalPage::sigPageRemoved(index)`，及时销毁对应 `PDrawBoard` 及其 `QPixmap`
2. **按需懒创建**：`PDrawBoard` 在用户首次在该页进行标注时才创建（而非页面加载时预创建），空白页面零内存占用
3. **序列化持久化**：标注数据序列化为 JSON 保存到工程文件，切换页面时可释放内存中的 `QPixmap`，切回时重建（以 CPU 换内存）

---

## 八、WESManager 初始化空窗期的非法调用风险

### 问题

WESManager 在后台线程异步初始化，主线程不阻塞即返回。`PPlayerRenderWdg` 和多处代码通过 `if (!WESManager::instance().getTlbEditing()) return;` 防御性检查，但：
- 检查不彻底（部分调用路径未守卫）
- 用户在初始化完成前的操作（如快速点击打开工程）可能触发空指针崩溃
- 没有明确的「初始化中」状态，UI 无法准确反映当前系统状态

### 优化方案

1. **启动遮罩（Skeleton Screen）**：`PMainWindow` 在 WESManager 初始化完成（`sigWESReady`）前展示骨架屏/加载动画，禁止所有编辑操作，从源头消除非法调用
2. **操作队列（Pending Queue）**：`Initializing` 阶段收到的 UI 操作放入延迟队列，`Ready` 后按序重放（适用于启动时命令行传入工程路径等场景）
3. **WES 状态机**：定义 `WESState { Uninit, Initializing, Ready, Error }`，所有依赖 WES 的方法统一检查状态，非 Ready 时拒绝并日志记录，而非静默 return

---

## 九、PPlayerToolbarMgr 池容量无上限

### 问题

`PPlayerToolbarMgr` 按 UUID 缓存工具条实例，但没有最大容量限制。在极端情况（一页内有大量 Clip 且频繁切换页面）时，池中工具条数量线性增长，造成内存泄漏类似的问题，且大量隐藏的 Qt 控件仍然占用 GDI 资源（Win）。

### 优化方案

1. **LRU 容量限制**：限制池最大容量（如 20 个），超出时析构最久未使用的工具条实例（LRU 淘汰）
2. **页面切换时清空**：切换页面时（`sigCurrentPageChanged`）清空整个池，因为上一页的 Clip UUID 在新页面几乎不复用
3. **延迟销毁**：淘汰时不立即销毁，而是放入回收队列，通过 `QTimer::singleShot(5000)` 延迟销毁，避免频繁切换时重复创建销毁

---

## 十、埋点代码侵入业务逻辑

### 问题

埋点调用（`WONDERSHARE_TRACKER_INFO->SendClickEvent` / `PEventTrackingHelper::sendEvent`）直接散落在业务逻辑代码中，与业务代码强耦合：
- 业务代码与追踪代码交织，可读性下降
- 更换或增加埋点 SDK 时需要全局搜索修改
- 单元测试业务逻辑时埋点代码难以 Mock

### 优化方案

1. **埋点装饰器/拦截器**：在业务信号总线的 emit 路径上挂载追踪中间件，信号触发时自动记录事件，业务代码不感知埋点存在
2. **统一追踪接口**：定义 `IEventTracker { trackEvent(name, properties) }` 接口，业务代码只依赖接口，具体实现（Wondershare SDK / Firebase 等）通过注入提供，易于替换和 Mock
3. **声明式埋点**：在类元数据或配置文件中声明「当 X 信号发生时，上报 Y 事件」，追踪引擎在运行时解析配置并自动连接，彻底从业务代码中剥离追踪逻辑

---

## 十一、跨进程 IPC 错误处理不完善

### 问题

Broker 架构（`PBrokerRecorderClient` / `PBrokerGlobalClient` 等）在进程间通信时缺乏健壮的错误处理：
- 子进程崩溃后主进程未能检测到，继续向不存在的进程发送消息
- IPC 超时没有明确机制，某些操作（如刷新积分）在子进程无响应时会静默失败
- 没有重连机制，一次连接失败后后续所有 IPC 调用均失效

### 优化方案

1. **进程心跳检测**：子进程定期发送心跳包，主进程超时未收到心跳时标记连接断开，并触发重启或降级处理
2. **IPC 调用超时**：所有 IPC 方法设置超时（如 3 秒），超时后回调错误处理（而非永久等待）
3. **重连队列**：连接断开后将待发送消息放入队列，连接恢复后自动重发；对于幂等操作（刷新积分）合并多条消息只发一次
