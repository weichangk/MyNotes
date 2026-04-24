summary_filmora.md
# Filmora — Win/Mac 视频编辑器

**技术栈：** C++17、Qt 5.15、CMake、Objective-C++

---

## 项目介绍

Filmora 是万兴科技旗下旗舰级桌面视频编辑产品，面向全球数百万创作者，支持 Windows 和 macOS 双平台，持续迭代至 14.x 大版本，保持行业领先的功能竞争力。

项目采用 **C++17 + Qt 5.15** 开发，基于 CMake 多配置构建体系，包含 50+ 功能模块、数十个独立共享库，涵盖时间线编辑、媒体库管理、AI 智能处理、特效素材、导出等完整视频创作流程。

**架构设计亮点：**
- 四层模块化分层架构：核心框架层（FFCore/FFAsync）→ 数据模型层（IFF* 纯虚接口）→ 后端服务层（FF* 功能服务）→ 前端视图层（F*View MVP）
- UI 层全面采用 **MVP（Model-View-Presenter）** 模式，所有面板遵循 View / Presenter / Model 三层分离
- 模块间通过 Qt 信号槽、纯虚接口、观察者模式（`ffcore::ObserverContainer`）多层次解耦通信
- 通过 **VBL（Video Business Layer）** 中间层对接 WES 底层渲染引擎，UI 层零感知引擎差异
- 多进程架构（独立导出进程 FExportExe、独立播放进程 FPlayServer），核心流程进程隔离，单模块崩溃不影响全局
- CMake 多平台多变体构建，支持 Win / Mac / Mac App Store 三平台及国际版/喵影版/设计师版条件编译

---

## 项目内容

### 一、资源面板（Media Library）

负责媒体库模块的功能开发与性能优化，该模块承载本地/云端全类型素材的导入、浏览、搜索与管理，是编辑器的核心入口面板。

- **主导 MVP 架构重构**：将原本耦合严重的代码拆分为 View / Presenter / Model 三层，View 构造注入 Presenter、Presenter 通过 Observer 接口回调通知 View，新功能接入成本显著降低
- **设计双层缩略图缓存系统**（路径缓存 + 像素缓存分离，`mediaId+size` 联合 Key），图像解码通过 FFAsync 异步框架移至线程池，以 QtPromise 链式回调安全回写主线程，全程弱指针保护对象生命周期；Hi-DPI 下以 2× 尺寸读取后缩放解决缩略图模糊；1000+ 素材场景下加载耗时下降约 **40%**
- **开发鼠标悬停视频预览**：帧解码迁移至独立 Worker 线程，复用底层 FFTimelineBuilder 逐帧解码并预缓冲，主线程以 40ms QTimer 驱动 Delegate 局部刷新，实现主线程零解码开销的流畅预览
- **引入惰性初始化 + 分页加载**：分类 Presenter 改为首次访问时按需创建，Model 层实现 `canFetchMore/fetchMore` 按需渲染，大素材库首屏速度明显提升
- **基于策略模式构建可扩展多维过滤框架**（`IFFMediaItemFilter` 接口），支持关键字、媒体类型、云端/本地多条件组合过滤，引入 100ms 合并更新定时器消除批量导入时的界面抖动

---

### 二、时间线（Timeline）

负责时间线模块的开发与维护，该模块是编辑器的核心编辑区域，承载多轨视频、音频、特效等全类型轨道的可视化编排。

- **基于 Qt Graphics View Framework 深度定制时间线 UI**，构建 `FGraphicsTimelineScene`（QGraphicsScene）管理全部轨道/片段图元，设计帧坐标系与像素坐标系的双向映射，支持 0.5px～80px/帧的缩放范围
- **设计 Substitute（替身）拖拽机制**：拖拽移动/裁剪时构造轻量替身图元代替真实 ClipItem，全程不触碰 IFFClip 数据，鼠标释放时一次性 commit 到 VBL 层并写入 undo 栈；ESC 取消则销毁替身零副作用；消除了逐帧修改数据层触发 VBL 回调的性能瓶颈，拖拽帧率流畅
- **实现吸附对齐系统**（SnapService）：实时扫描所有 Clip 入出点、Playhead、Marker 作为吸附候选点，拖拽时计算最近偏移量实现磁性对齐；主轨支持磁性排布，插入片段时自动令相邻 Clip 位移让位
- **对接 VBL Command 模式撤销/重做**：所有修改操作封装为 `IFFUndoableOp` 压栈，引入宏配合 `beginMacro/endMacro` 将多个子操作合并为单条 undo 记录，合并期间暂停 VBL 事件广播，杜绝 UI 中间状态的无效刷新
- **音频波形与视频缩略图异步生成**：波形由独立 QThread 内解码 PCM 并绘制 QImage，以 `clipId + 入出点 + 可视宽度` 为参数 Key，缩放或裁剪后精准失效重建
- **时间线渲染性能优化**：引入冻结绘制模式在批量操作期间暂停 Scene 刷新，可视区域裁剪只调度视口内图元，ClipItem 缓存已渲染图像，数据不变时不重解码

---

### 三、属性面板（Property Panel）

负责属性面板模块的开发与维护，当用户选中 Clip 后动态展示并编辑该 Clip 的全部属性（变换、色彩、音频、特效、蒙版、关键帧等），需同时支持十余种 Clip 类型的差异化渲染，且所有修改须与撤销/重做及关键帧系统深度联动。

- **设计注册式 Builder 工厂**动态构建属性 UI：各属性子面板以 `RegisterPropertyBuilderByType` 宏静态自注册到 `FPropertyWidgetFactory`，主框架按类型查询按需构建；新增属性类型零侵入现有代码；构建结果按类型缓存，切换同类型时只执行 `refreshData()` 更新值，避免重复构建 Widget 树
- **实现多 Clip 批量编辑框架**：以模板类 `FPropertyBatchGetter/FPropertyBatchSetter` 统一批量读写，读取时聚合多 Clip 值并检测多值状态，写入时内嵌 Undo 事务与 VBL 事件暂停，无论修改多少 Clip 均合并为一次 UI 刷新和一条 Undo 记录
- **完善属性修改与关键帧系统的自动路由**：`setEffectPropertyValue()` 根据是否启用关键帧自动选择写入路径，监听播放头移动时自动刷新各控件为当前帧的插值属性值，实现属性面板与时间线关键帧编辑的无缝联动
- **区分实时调节与完成态的处理策略**：Slider 拖动期间进入实时模式只驱动预览窗刷新，鼠标松开后触发完整刷新并写入 Undo 记录，保证拖动过程中预览流畅且 Undo 栈不产生大量中间记录
- **引入 `UpdateReasons` 位掩码精细化刷新控制**：各 Widget 按需判断是否响应本次刷新，避免任意变更触发全量重刷；所有 VBL 回调通过 `QueuedConnection` 安全切换至主线程处理

---

### 四、FFAsync 轻量异步框架

设计并主导实现了全项目通用的自研异步调度框架，替代 QtConcurrent 在优先级、串行队列、优雅退出方面的不足，全项目缩略图、音频波形、AI 推理等核心异步场景均基于此框架实现。

- **框架架构设计**：Facade 模式对外暴露 5 个静态 API，内部由三级线程组（Foreground / Common / Background）+ `FFTaskPagePriorityQueue` 优先队列驱动，支持 4 级 TaskPriority 与 2 级线程优先级
- **`postReplyableResultTask` Promise 回调线程自动保证**：调用时捕获调用线程引用，Worker 完成后自动检测线程差异并将 `.then()` 投递回调用线程事件循环，对上层完全透明，彻底消除手动 `QFutureWatcher` + 信号槽的样板代码
- **三种任务运行器**：`FFParallelTaskRunner`（并发执行）、`FFQueueTaskRunner`（FIFO/LIFO 串行）、`FFThreadTaskRunner`（专属独立线程），AI 算法缓存队列使用 QueueTaskRunner 保证同一 Clip 多次参数变更按序处理
- **`stateFlag` 原子位域优雅退出机制**：退出标志与阻塞任务计数合并到单个 `std::atomic<quint32>`，三种退出策略（忽略/忽略未运行/必须执行）精细控制退出时任务行为，无锁等待安全销毁线程池
- **`buildAutoRelease` 跨线程堆对象安全释放**：将需要释放的调用线程堆对象包装为 `AutoReleaseWrapper`，析构时通过 `invokeMethod` 调度到原始分配线程执行，保证框架内所有内存操作无 UAF 风险

---

### 五、播放器（Player）

负责播放器模块的设计与实现，承担视频实时预览、多渲染后端适配、关键帧可视化编辑等核心职责。

- **三级 Presenter 继承体系**（`FFMediaPlayerPresenter` → `FFMediaPlayerEditorPresenter` → `FFMainPlayerPresenter`）按职责分层，PIMPL 模式隐藏 VBL 接口，Public 类头文件零 VBL 类型暴露，VBL 引擎升级无需重编上层模块
- **原生窗口（Native Window）嵌入渲染**：通过 `Qt::WA_NativeWindow` 创建真实 HWND，VBL 引擎将 D3D11 SwapChain 直接绑定到该 HWND 进行 Present，Qt 合成器完全不介入视频渲染路径，消除 Qt 与 D3D 的合成冲突
- **多渲染后端适配与 GPU 硬解码**：支持 D3D11（Windows 推荐）/ D3D9 / OpenGL（macOS）/ GDI（软件降级），启用 DXVA2/NVDEC/QSV 硬解码，4K 视频预览 CPU 占用显著降低
- **Quick Preview 同步等待机制**：启动 `QEventLoop::exec(ExcludeUserInputEvents)` 阻塞用户输入但保持事件循环（定时器和绘制正常运转），VBL 完成后精确恢复 UI 响应，防止画面撕裂
- **多显示器全屏与多实例播放器管理**：跨显示器全屏精确还原窗口状态，同时管理多个独立播放器实例（主播放器、高级字幕/分屏/文字特效编辑各一），工厂方法支持产品差异化替换

---

### 六、导出模块（Export）

负责导出模块的设计与实现，覆盖本地多格式导出、社交媒体直传、GPU 硬件加速编码等完整导出流程。

- **责任链（Proxy Chain）驱动八级前置检查**（资源授权 → 文件丢失 → 格式选择 → 素材版权 → 程序激活 → 导出执行），任意环节失败短路，新增检查步骤只需插入新 Proxy；`FExportGuard` RAII 对象自动保护自动保存、颜色对比模式等四个全局状态，任何退出路径均自动还原
- **Pipeline 任务依赖图（DAG）**：`FLocalProcessPresenter` 为每条时间线构建 `IFFExportPipeline`，在 Pipeline 内声明 Job 依赖，VBL 层按拓扑排序并发调度 Encode / AutoReframe / UploadCloudDisk 等 Job，批量导出多 Pipeline 并发执行，最大化吞吐量
- **独立子进程编码隔离**（`FProcessEncoder` + `FExportExe`）：编码引擎运行在独立子进程，双重保活机制（启动超时 120s + 心跳监控 500ms），GPU 编码失败时无感知自动降级至软编，子进程崩溃同样自动降级重试
- **GPU 硬件加速多厂商适配**：支持 NVENC（NVIDIA）/ AMF（AMD）/ QSV（Intel），运行时检测驱动可用性，GPU 错误时 `reStartCloseGpu()` 无缝切换软编，保证在任何硬件环境下均能完成导出
- **多格式多场景完整覆盖**：本地导出支持 MP4/MOV/AVI/GIF/MP3/WAV，社交媒体导出经编码→上传两阶段状态机支持定时发布，实时预估输出文件大小辅助质量与体积决策

---

### 七、AI 功能模块

深度参与多个 AI 场景的功能集成开发，涵盖视频内容理解、封面生成、精彩剪辑等核心 AI 能力。

#### AI 长视频转短视频

- **设计三阶段串行 AI 任务流水线**（语种检测 → 视频转码 → 内容理解与脚本生成），`FAILongToShortPresenterPrivate` 实现 `IFFAIServiceEventObserver` 接收 VBL Task Manager 异步状态回调，按 `taskType` 分发到三个 handler，每阶段完成后自动触发下一阶段，AI 推理细节由适配层封装，上层对本地/云端推理引擎无感知
- **多来源视频导入**统一收敛：本地文件直接填充参数；URL 来源通过 HTTP 服务解析真实地址；云盘媒体通过异步下载，三路来源统一收敛到同一套任务参数填充逻辑
- **`FProgressComposeHelper` 多任务进度加权合成**：将三个子任务的原始进度按权重线性合成（10%/30%/60%），通过单一信号驱动进度条，进度始终连续增长不发生跳变
- **结果批量导出流程**：动态修改工程配置（宽高比 9:16、分辨率 1080×1920），触发导出管线，导出完成后恢复原始工程配置

#### AI 封面制作

- **MVP 架构三层解耦**，支持 AI 智能推荐、时间线截帧、本地图片上传三种封面来源，三路入口统一收敛到同一套封面保存逻辑
- **`FFCoverRecommendationService` VBL AI 推荐适配器**：将 COM 风格 VBL 接口转换为 Qt 信号，上层 Presenter 完全无感知 VBL 细节
- **本地图片双通道解码**：Qt 原生解码优先（防 OOM），失败时通过 FFLocalMediaItemFactory 兜底，支持 HEIC/WEBP 等 Qt 原生不支持的格式
- **封面编辑临时 Timeline 隔离**：维护独立临时编辑 timeline，编辑期间的修改不影响主工程，取消时直接丢弃无需 Undo；导出前检查付费模板资源授权防止未经授权内容嵌入

#### AI 精彩集锦（Auto Beat Sync）

- **双层 Adapter 接口适配**：`FFAutoHighlightMontage` 同时实现 FF 层纯虚接口（供 Presenter 调用）与 VBL 回调接口，VBL 引擎替换不影响任何上层代码
- **双线程异步媒体处理**：独立 QThread 异步导入媒体 + 独立 QThread 维护生产消费队列提取缩略图；BGM 波形使用 `QPolygonF` 对称双路波形 + `QPainterPath::fillPath` 主题自适应渲染
- **分析结果导出到时间线的跨模块解耦**：通过消息总线将 Clip 列表传递给时间线模块，时间线模块不依赖分析模块任何具体类型

---

### 八、素材下载中心（Download Center）

- **独立子进程架构**：`FDownloadCenter.exe` 隔离下载引擎与主进程，通过 IPC 传递事件，主进程侧按素材类型路由到旧路径（FFNetwork + ZIP）或新路径（VBL OMP 协议），子进程崩溃不影响主进程稳定性
- **颗粒化串行下载队列**：缩略图优先插入队首（视觉反馈优先），完整资源追加队尾，结合 VBL 三档优先级双重保证，避免多文件并发竞争带宽与磁盘 IO
- **下载 URL 加密与 Token 自动刷新**：Base64 + XOR 双重编码保护真实 HTTPS 地址，内置 30 分钟 QTimer 定期自动续期 OAuth Token，下载中不会因 Token 过期中断
- **任务持久化与重试机制**：下载状态 Base64 编码 JSON 持久化，启动时重新入队未完成任务，失败项最多重试 3 次，按错误码分层处理（网络错误/磁盘不足/下载失败）

---

### 九、工程化与稳定性

- **CMake 多平台多变体构建**：维护 Win / Mac / Mac App Store 三平台及国际版/喵影版/设计师版条件编译，通过 CMake Generator Expressions 和模块宏统一管理 50+ 模块的编译配置
- **WinDbg 崩溃分析**：使用 WinDbg 分析线上 Dump 崩溃文件，结合 PDB 符号定位多线程竞争、空指针、内存越界等高频崩溃问题，配合 BugSplat 崩溃上报系统形成线上质量闭环
- **性能监控体系**：集成 `FSystemMonitor` 内存占用监控、`WSP::FPerformanceMain` 性能 SDK、`FFPerformanceTrack` 性能打点工具，建立系统化的性能度量与回归机制

---

## 项目成果

- 产品 DAU 覆盖全球**数百万**创作者，持续迭代至 14.x 大版本，保持行业领先的功能竞争力
- **1000+ 素材场景**缩略图加载耗时下降约 **40%**，大幅提升大素材库用户的使用体验
- 通过多进程架构有效隔离编辑、导出、播放等核心流程，单模块崩溃不影响全局，显著提升应用稳定性
- AI 功能矩阵（长视频转短视频、精彩集锦、AI 封面等）成功落地，成为产品核心差异化卖点，显著提升用户创作效率
- FFAsync 自研异步框架统一全项目缩略图、波形、AI 推理等场景的异步调度，消除竞态问题，提升多任务并发稳定性
- 崩溃率持续优化，通过 WinDbg Dump 分析与 BugSplat 埋点追踪形成闭环反馈机制，保障版本质量稳步提升
