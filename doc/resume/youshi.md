youshi.md

 个人优势总结（完整版 - 包含全部 5 个项目）

  1. 扎实的全栈架构设计能力

  - 深度参与 Filmora 视频编辑器(C++17 + Qt)、VBL 音视频中间层(C++)、UniConverter 多媒体平台(C# + WinForms)、Presentory 演示创作工具(C++ + Qt)、AniRemover Mobile(Flutter)五个千万级用户量的商业产品研发
  - 精通桌面端与移动端全栈开发,具备从底层引擎适配到业务逻辑封装再到UI呈现的完整架构把控能力
  - 擅长分层架构设计(MVP/MVC/纯虚接口体系):主导 PBL 中间层设计实现 UI 与引擎完全解耦;设计 VBL 30+ 纯虚接口体系支撑多产品线;构建 AniRemover 统一任务处理框架支持 5+ 业务类型

  2. 跨平台技术广度与深度

  - C++:精通 C++17/20 现代特性(智能指针、lambda、移动语义、RAII),深度掌握内存管理、多线程并发;设计 SafePtr + IRef 引用计数体系实现零内存泄漏
  - C#/.NET:熟练掌握 WinForms 开发,MVC 架构实践,DllImport/COM 互操作,支撑 UniConverter 80+子工程架构
  - Qt 框架:深度掌握信号槽机制、元对象系统、QGraphicsView、多线程模型、高DPI适配,实现 Presentory 复杂画布系统(渲染层+悬浮层+绘图板)
  - Flutter/Dart:精通跨平台移动开发,GetX 状态管理,自定义 Canvas 绘制,实现 60 FPS 流畅交互;设计策略模式+模板方法的任务处理引擎,新增业务开发效率提升 80%

  3. 复杂业务场景架构能力

  - 统一任务处理框架(AniRemover):设计基于策略模式+模板方法的异步任务引擎,抽象「上传 → 轮询 → 下载」完整流程,支持前后台无缝切换(detach/reattach),任务成功率 95%+,前后台切换成功率 99.9%
  - 云端 AI 任务处理框架(UniConverter):抽象多模型串联与任务一致化调度,支持 12+ AI 功能模块,显著降低 AI 功能接入成本
  - 中间层接口体系(PBL/VBL):通过工厂方法+门面模式+观察者模式实现业务层与引擎层完全解耦,引擎切换时 UI 层零修改
  - 多进程/IPC 架构:设计导出进程隔离方案,通过 IPC 通信保障主进程稳定性;Presentory 跨进程 Broker 架构支持主进程与录制/启动进程无缝通信

  4. 音视频技术领域专精

  - 熟悉视频编解码流程(H.264/H.265),掌握 FFmpeg、WES/NLE/tlb 引擎对接与优化
  - 精通时间线编辑系统架构设计:VBL DataModel 层统一管理项目/时间线/轨道/Clip 全套数据模型;实现 PBL Clone Timeline 异步缩略图抓取方案消除主线程阻塞
  - 具备音视频渲染管线优化经验:GPU 加速(OpenCL/Metal)、多线程解码、帧缓存管理;AniRemover 图片编辑 FPS 从 25 提升至 60(+140%),涂抹延迟从 50ms 降至 16ms(-68%)
  - 深度参与AI 能力集成:视频增强、智能剪辑、智能总结、水印去除、AI 文字辅助等;熟悉本地 SDK + 云端 API 双路执行架构

  5. 跨平台支付系统设计能力

  - iOS StoreKit 双栈兼容:运行时自动判断系统版本(iOS 12-14 使用 SK1,iOS 15+ 使用 SK2),统一验证流程(本地 → Apple 服务器 → 自有后端),支付成功率 96%+
  - Android Google Play 集成:服务端验证模式(更安全),订阅优惠支持,支付成功率 94%+
  - 跨平台统一架构:平台工厂模式 + 统一基类抽象实现业务层无感知支付;完善的补偿机制(未验证购买持久化与自动重试),验证失败补偿率 99%
  - 授权轮询机制:验证提交后轮询后端授权状态(最多 120 秒),确保权益生效;恢复购买成功率 98%+

  6. 性能优化实战经验

  - 桌面端优化:录制模块重构(COM 工厂封装),活跃用户数提升 16.28%,转化率提升 37.48%;坐标变换缓存(QHash)避免每帧重算
  - 移动端优化:
    - 图片编辑:离屏渲染 + Dirty Rect,FPS 提升 140%
    - 视频上传:并发限流 + 智能分片,速度提升 300%
    - 内存优化:流式解码 + Delta-based Undo,OOM 崩溃率降低 83%,内存占用从 100MB 降至 20MB
  - 虚拟进度算法:服务端无细粒度进度时使用平滑虚拟进度,用户感知处理时间缩短 30%

  7. 生命周期管理与内存安全

  - 智能指针体系设计:PBL SafePtr + IRef 引用计数智能指针,对象内嵌引用计数,支持跨接口安全转换;tlb 原生对象通过 std::shared_ptr + 自定义 deleter 确保正确释放,实现零内存泄漏
  - Undo/Redo 系统:Command 模式 + Lambda + SafePtr 封装,支持宏命令组合、对象引用保活,彻底消除悬空指针风险
  - 多线程安全:熟练运用 QMutex/QAtomicInteger/std::atomic、线程池调优、跨线程信号槽等并发控制手段;AniRemover Runtime Slot 并发控制机制防止 OOM

  8. 设计模式与系统思维

  - 精通并实践 15+ 设计模式:工厂方法、策略模式、模板方法、命令模式、观察者模式、门面模式、适配器模式、单例模式、对象池模式、依赖倒置等
  - 架构模式实践:MVP(Presentory/Filmora)、MVC(UniConverter)、分层架构(VBL 五层架构)、事件驱动架构(ISignal* 信号总线)
  - 接口设计能力:VBL 30+ 纯虚接口体系、PBL 12 个子 Manager 门面统一访问、AniRemover TaskProcessor 泛型接口等接口级解耦实践

  9. 工程化与质量保障经验

  - CMake 工程化:构建大型跨平台项目(80+子工程),掌握增量编译、依赖管理、多配置构建
  - 质量保障体系:单元测试(GoogleTest)、代码规范(clang-format)、内存泄漏检测(VLD)、崩溃上报(BugSplat/Firebase Crashlytics),AniRemover Crash Rate < 0.1%
  - 移动端 CI/CD:GitHub Actions + Fastlane 自动化发布,单元测试覆盖率 60%+
  - 线上问题定位:基于崩溃日志快速排查多线程竞争、内存越界、引擎崩溃等复杂缺陷



    一、跨平台架构设计能力

  - 精通多语言技术栈：C++17/20（结构化绑定、std::optional、if constexpr、Fold expressions）、C#/.NET、Dart/Flutter、Qt 5.15，具备快速切换技术栈的能力
  - 智能指针与内存管理：深度应用 SafePtr 侵入式引用计数、std::shared_ptr/unique_ptr、WeakPtr 循环引用打破，实现零内存泄漏、零悬空指针（经 Valgrind 验证）
  - 设计模式工程化应用：熟练应用策略模式、模板方法、工厂模式、观察者模式、对象池模式等 10+ 种设计模式，提升代码复用率 60%，新增业务开发效率提升 80%
  - 多进程/多线程架构：设计并实现 Filmora 多进程架构（Main Process + Backend Service）、VBL 任务队列 + 线程池并发模型，熟练运用 condition_variable、std::atomic、packaged_task/future、读写锁等并发技术

  ---
  二、音视频处理与图形渲染技术

  - 多媒体处理技术栈：熟练使用 FFmpeg 进行 H.264/H.265 硬件加速编解码、视频分片处理、智能分片上传（并发限流 + 断点续传）
  - 渲染引擎架构：深度参与 NLE（Non-Linear Editor）+ WES（Wondershare Engine System）+ tlb（Timeline Backend）三层渲染架构设计与实现
  - 图形渲染优化：实现 PaintCache 状态缓存、FreezeMode 拖拽冻结、Substitute 虚影拖拽、Dirty Rect 增量渲染等优化技术，单帧绘制时间减少 30%，拖拽帧率提升 50%
  - Qt 图形框架：熟练使用 QGraphicsScene/View/Item 架构、QPainter 自定义绘制、RenderHints 动态调整

  ---
  三、性能优化实战经验

  桌面端优化（Filmora）

  - 时间线渲染优化：通过 PaintCache、FreezeMode、Substitute 虚影拖拽等 7 项优化，拖拽时延从 800ms 降至 110ms（降低 86%），冗余回调减少 95%
  - 属性面板优化：通过 QueuedConnection 批量刷新、UpdateReasons 过滤、Tab 懒创建等 10 项优化，Tab 切换时间从 ~200ms 降至 ~5ms（降低 96%），播放时 CPU 占用率降低 60%

  多线程与并发优化（VBL）

  - 任务队列 + 线程池：设计 BsBackgroundTaskManager（condition_variable + 谓词模式）、StdThreadPool（packaged_task + future 异步模式），实现优先级队列、任务取消机制
  - 对象池优化：实现 MediaInfoAdapterObjectPool RAII 式自动归还机制，减少 new/delete 开销 30%

  移动端优化（AniRemover Mobile）

  - 图片编辑性能：离屏渲染 + Dirty Rect 优化，FPS 从 25 提升至 60（+140%），绘制延迟从 50ms 降至 16ms（-68%）
  - 视频上传优化：并发上传限流 + 智能分片，上传速度从 500KB/s 提升至 2MB/s（+300%）
  - 内存优化：Undo/Redo 增量存储（内存占用从 100MB 降至 20MB，-80%）、大图分块处理（OOM 崩溃率从 3% 降至 0.5%，-83%）
  - 架构优化：任务优先级队列、自适应轮询（服务端 QPS -30%）、任务持久化缓冲（数据库写入 QPS -70%）、动态 Slot 调整

  ---
  四、业务架构设计能力

  - 统一任务处理引擎：设计基于策略模式 + 模板方法的任务框架，支持图片擦除、视频增强等 5+ 种业务类型，业务逻辑与调度逻辑完全解耦，任务成功率 95%+
  - 跨平台 IAP 架构：设计平台工厂模式 + 统一基类抽象的支付系统，兼容 iOS StoreKit 1/2 + Android Google Play，实现未验证购买持久化 + 自动重试机制，支付成功率 iOS 96%+、Android 94%+
  - 前后台无缝切换：实现 detach/reattach 机制，任务在后台继续执行，完成后通过系统通知提醒用户，前后台切换成功率 99.9%

  ---
  五、问题分析与解决能力

  - 性能分析工具链：熟练使用 Qt Creator Profiler、Valgrind、Visual Studio Profiler、Flutter DevTools、Dart Observatory 进行性能瓶颈定位
  - 多层次日志系统：设计包含时间戳、线程 ID、类别、source_location 的结构化日志系统，支持条件断点、内存泄漏定位
  - 典型问题解决：
    - 图片编辑卡顿：通过离屏渲染 + Dirty Rect，FPS 从 25 提升至 60
    - 视频上传慢：通过并发限流 + 智能分片，速度提升 300%
    - Tab 切换卡顿：通过懒创建 + 缓存，时间从 200ms 降至 5ms
  - 优化方法论：遵循"先测量后优化"原则，运用 80/20 法则，分层优化（架构层 → 算法层 → 代码层）

  ---
  六、技术亮点总结

  - C++ 现代特性：深度应用 C++17/20 核心特性，智能指针内存管理实现零泄漏
  - RAII 设计模式：系统性应用 RAII 进行资源管理（锁、信号、状态保护、对象池）
  - 并发编程：熟练掌握多线程同步原语（mutex、condition_variable、atomic、shared_mutex）、线程池模式、packaged_task/future 异步模式
  - 性能优化：具备丰富的桌面端/移动端性能优化实战经验，熟悉缓存策略、懒计算、批量更新、异步加载等优化技术
  - 跨平台开发：具备 Qt/C++、Flutter/Dart、C#/.NET 多平台开发经验，熟悉平台差异处理与统一抽象设计



  5. AI 工程化与研发效能建设能力