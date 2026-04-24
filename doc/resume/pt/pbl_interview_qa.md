pbl_interview_qa.md
# PBL 模块 — 面试高频问题与答案

---

## 架构设计类

### Q1：PBL 作为中间层，它解决了什么核心问题？为什么不让 UI 直接调用渲染引擎？

**答**：

直接耦合有三个致命问题：
1. **可测试性差**：UI 直接依赖 WES/tlb 意味着跑任何单元测试都要初始化重量级渲染引擎，测试成本极高
2. **引擎不可替换**：tlb 是内部 C 引擎，若未来切换引擎（如改用 FFmpeg pipeline），所有 UI 代码都要改
3. **多平台差异**：Win/Mac 引擎 API 略有差异，差异代码会散落在整个 UI 层

PBL 的解法是**依赖倒置**：UI 层只依赖 `Include/` 下的纯虚接口（`IBaseClip / IPagesEditor / IPresentationEditor` 等）；`ISignal*` 单例作为事件总线，UI 订阅信号而不直接调用引擎方法。这样 WESManager 完全封装了与 tlb 的所有交互，更换底层引擎时只需重写 WESManager 和 Clip 实现，UI 零修改。

---

### Q2：IPresentationEditor 作为门面暴露了 12 个子 Manager，这种设计有什么取舍？

**答**：

**优点**：
- UI 只需一个入口点（`PRESENTATION_EDITOR`）即可访问所有功能，不用管理多个全局对象
- 每个 Manager 职责单一（CameraManager 只管摄像头），边界清晰，易于扩展
- 门面对象可以统一管理 Manager 的生命周期（创建顺序、依赖注入）

**取舍**：
- 当 Manager 数量增长（目前 12 个），门面接口文件本身变成变更热点
- 若两个 Manager 之间有协作（如 CameraManager 需要通知 BackgroundManager 更新），会出现 Manager 间隐式耦合

**改进思路**：可以将 Manager 进一步分组（`ICaptureFacade` 包含 Camera/Screen/Window，`IMediaFacade` 包含 Background/Watermark），减少单一门面的膨胀压力。

---

### Q3：SafePtr 与 std::shared_ptr 都是智能指针，为什么选择自研 SafePtr？有什么缺点？

**答**：

**选择 SafePtr 的原因**（推测历史决策）：
1. 项目启动时 C++11 的 shared_ptr 在某些平台（尤其 MSVC 早期版本）有性能问题
2. 侵入式引用计数（对象内部存储计数）比非侵入式（shared_ptr 的控制块）缓存友好，减少一次指针间接
3. 与 tlb（C 风格 COM-like 接口）的 AddRef/Release 模型天然兼容

**缺点**：
1. **无 weak_ptr**：循环引用无法自动检测，需要手动打破（比如 PageClip 与子 Clip 的父子引用）
2. **侵入式**：只能包装继承了 `IRef` 的对象，无法包装第三方 C++ 对象
3. **与标准库割裂**：代码中同时存在 `SafePtr` 和 `std::shared_ptr`（tlb clip 用后者），维护两套语义
4. **异常安全**：没有 `make_safe_ptr` 等价物，`new T` + `SafePtr(ptr, false)` 之间若抛异常会泄漏

---

## 技术实现类

### Q4：缩略图抓取为什么要 Clone Timeline？直接在主 timeline 上抓帧有什么问题？

**答**：

主 timeline 同时被预览（渲染线程）和编辑（主线程 WES API 调用）使用。若后台缩略图线程也在同一 timeline 上 seek 到不同位置抓帧：
1. **渲染状态污染**：seek 会改变 timeline 的当前播放位置，导致用户看到预览画面跳帧
2. **线程竞态**：tlb 的 timeline seek + render 不是线程安全操作，两个线程并发操作同一 timeline 会崩溃
3. **性能互斥**：缩略图抓帧是 GPU 渲染操作，与主预览渲染竞争 GPU，导致预览卡顿

Clone Timeline 创建了一个独立副本（共享 clip 数据，但 seek 位置独立），后台线程在副本上随意 seek 和渲染，完全不影响主 timeline。

---

### Q5：BaseCommand 的 lambda 捕获有什么风险？你们是如何处理的？

**答**：

**风险**：若 lambda 按值捕获裸指针（`IBaseClip*`），undo 时对象可能已被析构，导致悬空指针访问。

**PBL 的处理**：`pushCmd(IBaseClipPtrs clips, redoFn, undoFn)` 接口要求传入 `SafePtr` 列表，`BaseCommand` 内部持有这些 SafePtr，在命令存活期间保持对象引用计数 ≥ 1，保证对象不被释放。

**进一步风险**：若 lambda 捕获的是对象的某个**状态值**（如 `int oldOpacity`），而非对象本身，redo 时仍然安全。但若 lambda 捕获了其他会被析构的对象（如已删除的 Page 对象），仍然有问题——因此规范要求 lambda 只通过 SafePtr 持有需要访问的对象。

---

### Q6：WESManager 初始化放在后台线程，UI 线程如何知道初始化完成？初始化完成前如果用户操作怎么办？

**答**：

**通知机制**：WESManager 初始化完成后通过 `DCSyncInitHelper` 的回调或直接 emit 一个信号（如 `sigWESReady`），主线程收到后解锁 UI。

**空窗期处理**（当前方案）：代码中多处用 `if(!WESManager::instance().getTlbEditing()) return;` 防御性 null 检查。这是防御但不彻底的做法。

**更好的做法**：
1. 主窗口启动时展示加载态（skeleton screen），订阅 `sigWESReady`，收到后移除加载遮罩
2. 引入操作队列：初始化完成前的 UI 操作暂存到 `pendingOps`，ready 后按序重放
3. 或直接用 `std::future<void>` + `get()` 在特定关键路径上等待（但主线程阻塞需谨慎）

---

### Q7：ISignal* 信号总线与直接函数调用相比，性能差异如何？在高频场景（如预览每帧回调）是否有问题？

**答**：

**性能差异**：Qt 的跨线程信号/槽使用 `QMetaObject::invokeMethod` 加消息队列，单次调用约有 1-2μs 额外开销。同线程直连（`Qt::DirectConnection`）开销更小，接近函数调用。

**预览帧回调的处理**：`sigPositionChanged`（每帧触发，约 30-60Hz）是高频信号，若连接了多个 Slot，累积开销可观。PBL 的做法是在预览回调中**只 emit 必要信号**，UI 层的 Slot 应尽量轻量（仅更新数值，不做重渲染）。

**优化建议**：
- 高频信号改为 `Qt::QueuedConnection` + 节流（`QTimer::singleShot(0)` 合并连续信号）
- 或完全绕过 Qt 信号，改为回调函数/atomic 状态读取（UI 用定时器主动 poll）

---

### Q8：PageClip 内部的 Clip 层序（layerIndex）是如何实现的？添加/删除/移动 Clip 时如何保证层序正确？

**答**：

`PageClip` 内部维护一个有序列表（track index → Clip），层序即列表顺序（index 越大渲染越在上层）。

- `addClip(clip)` → 追加到末尾（最上层），调用 `tlb::AddTrack / SetTrackClip`
- `removeClip(clip)` → 找到对应 track，移除，后续 track index 依次前移
- `moveClipLayer(clip, LayerAction)` → `LayerAction` 枚举（Up/Down/Top/Bottom），调整 clip 在列表中的位置，并同步更新 tlb track 顺序
- `clipLayerIndex(clip, action)` → 预计算操作后 clip 的目标 index，供 UI 预览显示用

多操作时用 `IUndoManager::beginComposite/endComposite` 包裹，保证层序调整作为一个原子 undo 步骤。

---

## 业务场景类

### Q9：用户在演示中途切换摄像头设备，整个调用链是什么？

**答**：

```
用户在 UI 选择新摄像头
  → PCameraPanel::slotDeviceChanged(newDeviceId)
  → PRESENTATION_EDITOR->getCameraManager()->switchDevice(newDeviceId)
  → CameraManager 内部：
      1. 停止旧 CaptureClip 的采集流（旧 PDevice::stop()）
      2. 创建新 CaptureClip（ClipFactory::createCaptureClip(newStreamId)）
      3. 替换 PageClip 中的摄像头 Clip（PageClip::removeClip(old) + addClip(new)）
      4. 启动新流（新 PDevice::start()）
      5. WESManager::beginEditTimeline / endEditTimeline 包裹修改
  → ISignalPage::sigPageClipsChanged emit
  → UI PMainSlides / PPlayerEditor 响应更新预览
```

如果设备断开（热插拔），`ISignalDevice::sigDeviceRemoved` 先触发 → `CameraManager` 捕获信号 → 自动回退到默认设备或显示 `DeviceDisconnectTipWdg`。

---

### Q10：一个典型的 Undo 操作（比如撤销「移动 Clip 位置」）的完整流程是什么？

**答**：

```
用户拖动 Clip 修改位置
  → UI mouseRelease
  → UndoManager::pushCmd(
        clips: [clipSafePtr],
        redo: [clip, newPos]{ clip->setTLBegin(newPos); clip->setTLEnd(newPos+dur); },
        undo: [clip, oldPos]{ clip->setTLBegin(oldPos); clip->setTLEnd(oldPos+dur); }
    )
  → BaseCommand 压栈 QUndoStack

用户按 Ctrl+Z
  → QUndoStack::undo()
  → BaseCommand::undo()
      1. m_undoFn() → clip->setTLBegin(oldPos) → tlb clip setTLBegin → WES 重算渲染
      2. ProjectInfo::setChanged(true)
      3. emit sigPropertyChanged
  → ISignalUndoManager::sigUndoAvailable(false) → UI 工具栏刷新
  → ISignalPage::sigPageClipsChanged → PMainSlides / PPlayerEditor 刷新显示
```

---

### Q11：项目文件的序列化和反序列化是如何处理跨版本兼容的？

**答**：

PBL 的 `ITimelineCallback::onPrjVersion(version)` 回调在加载项目时先返回工程文件版本，`ProjectManager` 根据版本号执行迁移逻辑：

1. **版本字段**：工程文件内嵌版本号（如 `"version": "1.5.0"`），加载时由 `onPrjVersion` 通知 PBL
2. **升级迁移**：`ProjectManager` 维护 migration 函数表（`v1.0 → v1.1 → v1.5`），按版本链式升级
3. **Clip 重建容错**：`onClip(WesOnClipCallBackInfo)` 回调中，若某个 Clip 类型在新版本中已弃用，ClipFactory 可返回一个 `NullClip`（不渲染但保留数据），避免整个项目加载失败
4. **向后兼容**：旧版客户端打开新版工程文件时，未知字段被忽略（JSON/XML 宽松解析），只恢复已知字段

---

### Q12：多页面场景下，"应用转场到所有页面"的操作性能如何保证？

**答**：

`IPagesEditor::changeTransition(path, applyToAll=true)` 遍历全部 pages 设置 TransClip，朴素实现每页都触发一次 `beginEditTimeline/endEditTimeline`，N 页就是 N 次 WES 重算，性能很差。

**优化方案**：

1. **批量包裹**：整个 applyToAll 操作用一次 `beginEditTimeline / endEditTimeline` 包裹，WES 只在 `endEditTimeline` 时做一次统一重算
2. **只更新 TransClip 数据，延迟渲染**：设置 TransClip 时标记 dirty，不立即触发渲染；所有页面设置完后统一触发 `updateCanvas()`
3. **Undo 合并**：applyToAll 用 `beginComposite("Apply Transition To All") / endComposite()` 包裹为单条宏命令，undo 时一次回退所有页面

---

## 扩展思考类

### Q13：如果需要支持「实时协作编辑」（多用户同时编辑同一个 Presentory 文件），PBL 现有架构需要哪些改造？

**答**：

现有架构的主要障碍：

1. **操作不是纯函数**：当前 Undo/Redo 的 lambda 捕获本地状态，无法序列化传输
2. **无 Operation ID**：操作没有全局唯一 ID，无法做冲突检测（OT/CRDT）
3. **状态在内存**：PageClip/BaseClip 状态只在内存，没有可同步的持久化格式

**改造方向**：

1. **操作序列化**：将所有修改操作从 lambda 改为**可序列化的 Operation 对象**（类型 + 参数），通过网络广播给其他客户端
2. **引入 CRDT 或 OT**：对并发操作冲突（如两人同时移动同一 Clip）引入 Operational Transformation 或 CRDT 解决
3. **Clip 状态版本化**：每个 Clip 维护 `version: int`，操作带版本前提条件（CAS 语义），冲突时拒绝并要求 rebase
4. **PBL 信号总线扩展**：`ISignalProject` 增加 `sigRemoteOperationApplied` 信号，本地 UI 在收到远端操作后通过此信号刷新，与本地操作路径统一

---

### Q14：PBL 的 taskProcessor 与 Qt 的 QThreadPool 有什么区别？为什么要自研？

**答**：

| 特性 | taskProcessor | QThreadPool |
|------|--------------|-------------|
| 任务依赖 | 支持 `waitSigAfter`（任务间依赖序列化） | 不支持，需手动 QFutureSynchronizer |
| 完成通知 | `taskSignal` / `taskFuture` 返回 future | `QFuture<T>` via `QtConcurrent::run` |
| 取消 | 支持任务级取消标志 | 不支持已提交任务取消（只能 cancel future）|
| 线程数控制 | 自定义 worker 数量 | `setMaxThreadCount`，全局共享 |

自研的核心动机是**任务依赖**（如：InitGpuService 必须在 InitDecMgr 之前完成）和**任务可取消性**，这两点 `QThreadPool` 原生不支持。现代 C++17 也可以用 `std::execution` 或第三方 Taskflow 库替代自研实现。
