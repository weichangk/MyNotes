40-download-center-interview-qa.md
# 素材下载中心模块面试问答

---

## 一、架构设计类

### Q1：素材下载中心的整体架构是怎样的？为什么要用独立子进程？

**回答：**

架构分四层：
- **UI 层**：`FResourceDownloadCenterDlg`（弹窗）+ 双 `FDownloadResourceModel`（Downloading/Downloaded）+ `FDownloadResourceDelegate`（自定义绘制）
- **调度层**：`FDownloadPackManager` 单例，路由旧路径（`FFNetwork` + ZIP）或新路径（VBL OMP）
- **下载执行层**：`FFDownloadCenterResourcePackage`（包协调）→ `FFDownloadCenterResourceItem`（单文件下载，实现 `IBsProgressCallback`）
- **VBL 接口层**：`IBsCloudResourceLoader`（下载）+ `IBsLocalResourceLoader`（SQLite 持久化）

使用独立子进程（`FDownloadCenter.exe`）的原因有三：
1. **崩溃隔离**：VBL 下载引擎（特别是加密解密、网络 SSL 路径）在某些机器上可能崩溃，子进程崩溃不影响用户的编辑状态
2. **资源隔离**：下载操作产生大量网络 IO 和磁盘写入，子进程独立，不与主进程的渲染、UI 线程竞争 CPU 和内存带宽
3. **后台持续运行**：Filmora 编辑窗口最小化或切换到其他应用时，子进程仍可继续下载，不依赖主进程的事件循环

---

### Q2：新旧两条下载路径有什么区别？如何决定走哪条？

**回答：**

| 维度 | 旧路径（`FDownloadPackTask`）| 新路径（`FFDownloadCenterResource`）|
|---|---|---|
| 网络层 | `FFNetwork`（自研 HTTP 客户端）| VBL `IBsCloudResourceLoader`（OMP 协议）|
| 传输格式 | ZIP 压缩包 | 每个文件独立下载（颗粒化）|
| 进度粒度 | 包级别 | 文件级别 |
| 失败处理 | 整包重试 | 仅失败文件重试 |
| 优先级 | 无 | `FFMediaDownloadLevel` 三档 |

路由逻辑在 `FDownloadPackManager::startDownload()` 中：判断资源包的元数据中是否有 OMP 协议标记（`hasOmpFlag`）。早期上线的素材包无标记，走旧路径保持兼容；新上线素材包均使用新路径。这是一个典型的策略模式——调度层根据数据特征选择执行策略，两条路径互不影响，可以独立演进。

---

### Q3：颗粒化下载的串行队列是如何实现的？为什么不并发下载所有文件？

**回答：**

`FFDownloadCenterResourcePackage` 维护一个 `QQueue<FFDownloadCenterResourceItem*>`，每次只启动一个 Item：

```
start() → dequeue 第一个 Item → item->start()
item onFinish → slotResourceItemFinish()
  → dequeue 下一个 Item → 启动
  → 直到队列为空 → 发射 sigPackageFinished
```

优先级通过入队位置实现：缩略图用 `prepend()` 插队首，完整资源用 `append()` 追加。

不并发下载所有文件的原因：
1. **服务器并发限制**：OMP 服务对单个客户端的并发连接数有限制，盲目并发可能触发限速甚至封禁
2. **磁盘 IO 竞争**：多文件同时写入同一目录，SSD 的随机写性能不如顺序写
3. **用户体验优先级**：串行保证缩略图最先完成（用户立即看到预览图），而不是所有文件同时"50%"进度

---

## 二、核心机制类

### Q4：下载 URL 是怎么加密的？为什么用 XOR 而不是 AES？

**回答：**

加密方案是**双层编码**：
1. 原始 HTTPS URL 逐字节 XOR 密钥 `"erahsrednow"`（循环使用）
2. XOR 结果 Base64 编码，加 `fmrespack://` 前缀

```cpp
QString decryptUrl(const QString& encrypted) {
    // 去 fmrespack:// 前缀
    QString base64Part = encrypted.mid(QString("fmrespack://").length());
    // Base64 解码
    QByteArray xorBytes = QByteArray::fromBase64(base64Part.toUtf8());
    // XOR 解密
    QByteArray key = "erahsrednow";
    for (int i = 0; i < xorBytes.size(); ++i) {
        xorBytes[i] ^= key[i % key.size()];
    }
    return QString::fromUtf8(xorBytes);
}
```

用 XOR 而非 AES 的原因：
- **目的是混淆而非加密**：加密的目的是防止普通用户从客户端反编译后直接获取 CDN URL 绕过授权下载，而不是防止专业破解者；XOR 足以应对大多数场景
- **性能**：XOR 是 O(n) 无额外内存分配，AES 需要块加密、填充、密钥派生，对高频 URL 解密场景开销不必要
- **无密钥管理问题**：密钥硬编码在可执行文件中，XOR 和 AES 安全性都依赖密钥不泄露；在这个场景下 AES 的安全优势无法体现

---

### Q5：OAuth Token 是如何管理的？Token 过期期间的下载请求如何处理？

**回答：**

Token 通过两个机制维护：

**主动刷新（定期）**：
`FFDownloadCenterResource` 构造时启动 30 分钟 `QTimer`，触发时调用 `VBL::getBsTokenUpdater()->updataAccessToken()`，Token 在过期前主动续期，正常情况下下载请求不会遇到 Token 过期。

**被动恢复（容错）**：
若因时钟误差或网络延迟导致请求到达服务器时 Token 已过期，服务器返回 401，`FFDownloadCenterResourceItem::onFinish(kErrorCode_401)` 触发：立即等待 Token 刷新完成后重新发起请求，不计入用户可见的重试次数，不删除已下载部分。

当前实现的一个已知问题：若大文件下载超过 30 分钟，Token 刷新后 VBL 内部已持有的下载句柄不会自动更新 Token，需要整个文件从头重下。理想方案是 VBL 提供 `resumeWithNewToken(token)` 接口支持带 Range 的续传。

---

### Q6：任务持久化是如何实现的？为什么不用数据库而用 JSON 文件？

**回答：**

持久化使用两个 JSON 文件：
- `download.josn`：进行中/失败的任务，Base64 编码避免中文路径问题，最多 100 条
- `done.josn`：已完成任务记录，用于 Downloaded Tab 历史展示

不用 SQLite 的原因：
1. **简单性**：下载任务记录数量有限（最多 100 条），JSON 读写足够高效，无需 Schema 管理
2. **SQLite 已被 VBL 占用**：`IBsLocalResourceLoader` 已用 SQLite 存储资源元数据（资源 ID、路径、时间），两套 SQLite 写入需要考虑锁竞争；JSON 文件写入完全独立
3. **调试友好**：Base64 解码后可直接用文本编辑器查看任务状态，SQLite 需要专用工具

Filmora 启动时读取 `download.josn`，筛选出 `state != Finished` 的任务重新入队，延迟 3 秒（等待启动期高优先级网络请求完成）后恢复下载。

---

### Q7：IBsProgressCallback 回调是在哪个线程？如何保证 UI 线程安全？

**回答：**

`IBsProgressCallback::onProgress / onFinish` 由 VBL 内部下载线程触发，不在主线程。

线程安全保证通过 Qt 信号/槽机制：

```cpp
// FFDownloadCenterResourceItemPrivate（PIMPL Private，在 VBL 工作线程）
void onProgress(int64_t bytes, int64_t total) override {
    // 直接 emit Qt 信号，Qt 的 QueuedConnection 自动切主线程
    emit q_ptr->sigProgress(bytes, total);
}

// FFDownloadCenterResourceItem（主线程）
// 信号默认 AutoConnection → 跨线程时自动 QueuedConnection
connect(this, &FFDownloadCenterResourceItem::sigProgress,
        this, [this](int64_t bytes, int64_t total) {
    // 此处已在主线程，安全更新 Model
    m_package->updateItemProgress(m_resourceId, bytes, total);
});
```

VBL 工作线程只调用 `emit`（线程安全），实际 Model 更新全在主线程，UI 层无需加锁。

---

## 三、业务与产品类

### Q8：下载完成后如何通知素材库刷新？整条链路是什么？

**回答：**

下载完成集成链路（全部异步，零主线程阻塞）：

```
IBsProgressCallback::onFinish(成功)
  ↓ [VBL 工作线程]
IBsLocalResourceLoader::postRequest(resourceId, localPath)
  → SQLite 写入（资源元数据持久化）
  ↓ [emit QueuedConnection → 主线程]
IPC 信号 → 主进程（若在子进程中）
  ↓
FFMediaFolderRedDotManager::addRedDot(categoryType)
  → 分类文件夹角标 +1
  ↓
若为字体（path 含 "/Font/"）
  → QFontDatabase::addApplicationFont(localPath)
  → 字体即时生效
  ↓
emit sigResourceDownloaded(resourceId)
  → FMediaLibraryView::onResourceDownloaded()
  → 刷新对应分类的 QAbstractListModel
  → 资源从"未下载/可下载"状态变为"已就绪/可使用"状态
```

每个节点职责单一，SQLite 写完才通知 UI，保证 UI 展示的永远是持久化成功的数据。

---

### Q9：批量下载（同时下载多个素材包）是如何管理的？

**回答：**

`FDownloadPackManager` 维护所有活跃包的 `QMap<QString, FFDownloadCenterResourcePackage*>`，多个包可以**并发下载**（包间并发），包内串行（包内文件顺序下载）：

```
包 A（10个文件，串行）：item1 → item2 → item3 → ...
包 B（5个文件，串行） ：item1 → item2 → ...
包 C（8个文件，串行） ：item1 → item2 → ...
[A/B/C 三包并发，互不等待]
```

UI 展示方面，`FDownloadResourceModel` 的每一行对应一个包，`FDownloadResourceDelegate` 绘制该包的聚合进度（`finishedCount / totalCount`）和速度（基于最近 1 秒的字节增量计算）。

并发包数量没有硬限制，由用户行为决定——用户连续点击多个素材包的"下载"按钮就会有多个并发。考虑到 OMP 服务的并发限制，实际可以在 `FDownloadPackManager` 中加一个最大并发包数量（如 5），超出的包进入等待队列。

---

### Q10：红点是怎么实现的？用户打开素材库后红点还会记录上次未看到的吗？

**回答：**

`FFMediaFolderRedDotManager` 以分类名称为键，维护未读计数和持久化：

```cpp
// 下载完成时
FFMediaFolderRedDotManager::instance()->addRedDot("贴纸");
// → m_redDotCount["贴纸"]++
// → 持久化到本地配置（config.ini）

// UI 渲染
bool hasRedDot = FFMediaFolderRedDotManager::instance()->hasRedDot("贴纸");
// → 分类 Tab 或文件夹图标右上角渲染红点圆圈

// 用户打开分类
FFMediaFolderRedDotManager::instance()->clearRedDot("贴纸");
// → m_redDotCount["贴纸"] = 0
// → 清除持久化记录
```

**跨会话持久**：红点状态写入本地配置，下次启动 Filmora 仍然可见——如果用户下载素材后关闭 Filmora 没有查看，下次启动素材库文件夹仍有红点。

**粒度**：红点是分类级别的（"贴纸"、"转场"、"音效"），而不是单个素材级别，防止红点泛滥影响 UI 整洁性。

---

### Q11：如果用户在下载过程中卸载 Filmora，会发生什么？

**回答：**

卸载触发 `FDownloadCenter.exe` 的 `Quit` IPC 事件，子进程收到后：

1. **停止所有正在进行的下载**：调用 `IBsCloudResourceLoader::cancel()` 取消所有活跃请求
2. **清理未完成文件**：对 `state != Finished` 的资源项，删除其本地临时文件（避免留下损坏文件）
3. **保留已完成文件**：`done.josn` 中记录的已下载资源本地文件由卸载程序决定是否删除（通常提供"删除用户数据"选项）
4. **子进程退出**

若子进程异常被强制终止（任务管理器 kill），未完成的临时文件留在磁盘。下次安装 Filmora 后，`FDownloadPackManager::loadPendingTasks()` 读取 `download.josn`，发现未完成任务，会删除旧临时文件（避免残片）后重新发起下载。

---

### Q12：素材缩略图和完整资源是分开下载的吗？这样设计的好处是什么？

**回答：**

是的，缩略图和完整资源是两个独立的 `FFDownloadCenterResourceItem`，并且有明确的优先级区分：

| 资源类型 | 入队方式 | VBL 下载级别 | 目的 |
|---|---|---|---|
| 缩略图（thumbnail URL）| `prepend()` 队首 | `Middle` | 快速视觉反馈 |
| 完整资源（download URL）| `append()` 队尾 | `High` | 实际使用 |
| VCG 预览水印图 | `append()` 队尾 | `Low` | 非必要预览 |

这种设计的好处：

1. **即时视觉反馈**：用户点击下载后，缩略图通常 <1 秒就出现在下载列表中，用户可以确认下载的是正确素材
2. **渐进式体验**：即使完整资源还在下载中，素材库列表已经可以展示缩略图；用户在等待期间可以继续浏览其他素材
3. **失败解耦**：若缩略图 URL 失效（CDN 迁移），不影响完整资源下载；反之亦然，两者可以独立重试

`FFMediaDownloadLevel` 的三档设计来自 VBL：VBL 内部网络调度器在带宽受限时优先处理 `High` 级别的请求（完整资源），降级处理 `Low` 级别（VCG 预览）。
