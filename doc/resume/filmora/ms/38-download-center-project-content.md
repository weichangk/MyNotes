38-download-center-project-content.md
# 项目内容

- 负责**素材下载中心模块**的设计与实现，采用独立子进程架构（`FDownloadCenter.exe`）隔离下载引擎与主进程，通过 IPC（Named Pipe/Socket）传递 `DownloadEffect`/`FIPCDownloadCenterPackageDetail` 等事件；主进程侧 `FDownloadPackManager` 单例按素材类型路由到旧路径（`FFNetwork` + ZIP）或新路径（VBL `IBsCloudResourceLoader` OMP 协议），子进程崩溃不影响 Filmora 主进程稳定性，下载操作不占用主进程 CPU/网络资源。

- 实现**颗粒化串行下载队列**：`FFDownloadCenterResourcePackage` 维护包内资源项的串行执行队列，缩略图使用 `prepend()` 插入队首（视觉反馈优先），完整资源使用 `append()` 追加队尾，结合 VBL 三档优先级（`FFMediaDownloadLevel: Low/Middle/High`）双重保证；`FFDownloadCenterResourceItem` 实现 `IBsProgressCallback`，单项完成后回调触发下一项，避免多文件并发竞争带宽与磁盘 IO。

- 设计并实现**下载 URL 加密与 Token 刷新机制**：资源 URL 以 `fmrespack://` 协议前缀标识，Base64 + 逐字节 XOR（密钥 `"erahsrednow"`）双重编码保护真实 HTTPS 地址；`FFDownloadCenterResource` 内置 30 分钟 `QTimer` 定期调用 `VBL::getBsTokenUpdater()->updataAccessToken()`，OAuth Token 自动续期，下载中不会因 Token 过期中断。

- 实现**下载完成多链路集成**：`IBsProgressCallback::onFinish` 完成后依次触发：`IBsLocalResourceLoader::postRequest()` 写 SQLite 持久化元数据 → IPC 通知主进程 → `FFMediaFolderRedDotManager::addRedDot(categoryType)` 分类红点更新 → 字体素材调用 `QFontDatabase::addApplicationFont()` 即时生效（无需重启）→ `sigResourceDownloaded` 驱动 `FMediaLibraryView` 刷新列表；整条链路零主线程阻塞，全部通过 Qt `QueuedConnection` 切回主线程执行。

- 实现**任务持久化与重试机制**：下载状态写入 `download.josn`/`done.josn`（Base64 编码 JSON，最多 100 条）；Filmora 启动时重新入队未完成任务；失败项最多重试 3 次，每次重试删除旧不完整文件从头重下（非断点续传，避免残片污染）；按错误码分层处理：`dftNetworkErr_0` 不重试提示检查网络，`dftDiscSpaceNotEnough` 弹出磁盘空间不足提示，`dtfDownloadEffectFailed` 最多 3 次重试后标记包失败。

- 所有 VBL 接口通过 **PIMPL 模式**封装在 `FFDownloadCenterResourcePrivate` 中，`FFDownloadCenterResource` Public 头文件零 VBL 类型暴露；`FResourceDownloadCenterDlg` 使用双 `QAbstractListModel`（Downloading/Downloaded Tab）+ `FDownloadResourceDelegate` 自定义绘制进度条与操作按钮，Model/View 分离使下载逻辑变更无需修改 UI 代码。
