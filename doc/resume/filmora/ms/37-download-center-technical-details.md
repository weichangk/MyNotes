37-download-center-technical-details.md
# 素材下载中心模块技术详解

---

## 一、模块架构概览

```
┌─────────────────────────────────────────────────────────────┐
│                     Filmora 主进程                           │
│                                                             │
│  FResourceDownloadCenterDlg（Dialog UI 层）                  │
│       ├── FDownloadResourceModel × 2（Downloading/Done）     │
│       └── FDownloadResourceDelegate（自定义 Item 渲染）       │
│                                                             │
│  FDownloadPackManager（单例调度器）                           │
│       ├── FDownloadPackTask（旧路径：FFNetwork + ZIP）        │
│       └── FFDownloadCenterResource（新路径：VBL OMP）         │
│             └── FFDownloadCenterResourcePackage（包协调者）   │
│                   └── FFDownloadCenterResourceItem × N      │
│                         （颗粒化下载，实现 IBsProgressCallback）│
│                                                             │
│  IPC 通信层（Named Pipe / Socket）                           │
└────────────────────┬────────────────────────────────────────┘
                     │ IPC
┌────────────────────▼────────────────────────────────────────┐
│               FDownloadCenter.exe（独立子进程）               │
│    DownloadEffect / BrowserCommand / PackageDetail / Quit   │
└─────────────────────────────────────────────────────────────┘
```

### 分层职责

| 层 | 类 | 职责 |
|---|---|---|
| UI 层 | `FResourceDownloadCenterDlg` | 弹窗主体，Downloading/Downloaded 双 Tab |
| 模型层 | `FDownloadResourceModel` | `QAbstractListModel`，管理下载队列 Item |
| 渲染层 | `FDownloadResourceDelegate` | 自定义绘制进度条/缩略图/操作按钮 |
| 调度层 | `FDownloadPackManager` | 单例，分发下载请求到正确路径 |
| 旧路径 | `FDownloadPackTask` | `FFNetwork` HTTP + ZIP 压缩包解压 |
| 新路径 | `FFDownloadCenterResource` | VBL `IBsCloudResourceLoader`（OMP 协议）|
| 包协调层 | `FFDownloadCenterResourcePackage` | 串行队列，单包内资源逐项下载 |
| 项下载层 | `FFDownloadCenterResourceItem` | 单个资源项下载，`IBsProgressCallback` 回调 |
| VBL 持久化 | `IBsLocalResourceLoader` | 下载完成后写 SQLite |
| 进程通信 | IPC 层 | 主进程 ↔ `FDownloadCenter.exe` 通信 |

---

## 二、核心数据结构

### 资源信息

```cpp
struct FResourceInfo {
    QString resourceId;       // 资源唯一 ID
    QString resourceName;     // 显示名称
    QString packageId;        // 所属包 ID
    QString categoryType;     // 分类（贴纸/转场/音效/字体...）
    QString thumbnailUrl;     // 缩略图 URL（加密）
    QString downloadUrl;      // 资源 URL（加密，fmrespack:// 前缀）
    int64_t fileSize{0};      // 字节数
    QString localPath;        // 下载完成后的本地路径
};

// 包下载进度聚合
struct FDownloadPackageResProgress_t {
    QString packageId;
    int totalCount{0};
    int finishedCount{0};
    int64_t totalBytes{0};
    int64_t downloadedBytes{0};
    int errorCode{0};
};

// VBL 下载优先级
enum FFMediaDownloadLevel {
    Low    = 0,  // VCG 预览水印图
    Middle = 1,  // 缩略图
    High   = 2,  // 完整资源
};

struct FFMediaDownloadOption {
    FFMediaDownloadLevel level{High};
    bool resumable{false};  // 当前实现始终 false
    int maxRetryCount{3};
};
```

### IPC 事件

```cpp
// 主进程 → 子进程
struct DownloadEffect {
    QString packageId;
    QList<FResourceInfo> items;
    FFMediaDownloadOption option;
};

// 子进程 → 主进程
struct FIPCDownloadCenterPackageDetail {
    QString packageId;
    FDownloadPackageResProgress_t progress;
    int state;  // Waiting/Downloading/Finished/Failed
};
```

---

## 三、核心业务流程

### 3.1 下载请求路由

```
用户点击"下载"（素材库/商城）
  └── FDownloadPackManager::startDownload(packageId, items, option)
        ├── 判断 package 类型
        │     ├── 旧素材（无 OMP 标记）→ FDownloadPackTask（FFNetwork + ZIP）
        │     └── 新素材（有 OMP 标记）→ FFDownloadCenterResource（VBL）
        └── 写入 download.josn 任务持久化
```

### 3.2 颗粒化下载串行队列

```
FFDownloadCenterResource::startPackage(packageInfo)
  └── FFDownloadCenterResourcePackage::start()
        └── 遍历 items，按优先级入队：
              thumbnail → prepend()（队首，优先下载）
              full resource → append()（队尾）
              ↓
        [串行执行]
        FFDownloadCenterResourceItem::start()
          └── IBsCloudResourceLoader::download(url, localPath, callback)
                ├── IBsProgressCallback::onProgress(bytes, total) → 进度回调
                └── IBsProgressCallback::onFinish(result)
                      ├── 成功 → slotResourceItemFinish()
                      │           → 启动队列下一项
                      │           → IBsLocalResourceLoader::postRequest()（写 SQLite）
                      └── 失败 → 重试（最多 3 次，删旧文件重下）
                                  超过重试次数 → 整包标记 Failed
```

### 3.3 URL 解密流程

```
fmrespack://Base64EncodedXORString
  ↓ 去除协议头
Base64EncodedXORString
  ↓ Base64 解码
XOR 加密字节串
  ↓ 逐字节 XOR（key = "erahsrednow"，循环使用）
真实 HTTPS 下载 URL
```

### 3.4 Token 定时刷新

```
FFDownloadCenterResource 构造时
  └── QTimer（30 分钟间隔）
        └── VBL::getBsTokenUpdater()->updataAccessToken()
              → VBL 内部刷新 OAuth Token
              → 后续下载请求自动使用新 Token
```

### 3.5 下载完成集成流程

```
IBsProgressCallback::onFinish(成功)
  ↓
IBsLocalResourceLoader::postRequest(resourceId, localPath)
  → 写入 SQLite（资源 ID、路径、下载时间、分类）
  ↓
IPC 信号 → 主进程
  ↓
FFMediaFolderRedDotManager::addRedDot(categoryType)
  → 对应分类文件夹显示红点角标
  ↓
若为字体文件（localPath 含 "/Font/"）
  → QFontDatabase::addApplicationFont(localPath)
  → 字体即时生效，无需重启
  ↓
发射 sigResourceDownloaded(resourceId)
  → FMediaLibraryView 刷新对应分类列表
```

### 3.6 任务持久化与恢复

```
每次下载状态变更
  └── 写 download.josn（Base64 编码 JSON，最多 100 条）
        字段：packageId, state, items[], retryCount

Filmora 启动时
  └── FDownloadPackManager::loadPendingTasks()
        ├── 读取 download.josn
        ├── 过滤 state=Finished → 移入 done.josn
        └── 剩余 Waiting/Failed 任务 → 重新入队（非断点续传，从头重下）
```

### 3.7 错误处理与降级

```
下载失败错误码分层：
  dftNetworkErr_0（无网络）    → 不重试，提示检查网络
  dtfDownloadEffectFailed      → 重试，最多 3 次
  dftDiscSpaceNotEnough        → 不重试，弹出"磁盘空间不足"对话框
  dftNetworkErr_1/2/3/4        → 按严重程度重试，最终失败提示错误码

重试机制：
  FFDownloadCenterResourceItem::retry()
    → 删除本地不完整文件
    → retryCount++
    → 重新调用 IBsCloudResourceLoader::download()
```

---

## 四、核心技术要点

### 4.1 独立子进程架构

主进程与 `FDownloadCenter.exe` 通过 IPC 通信，下载引擎完全隔离在子进程中：
- 子进程崩溃不影响主进程稳定性
- 下载操作不占用主进程 CPU/网络资源
- 子进程可在 Filmora 后台运行时继续下载

### 4.2 颗粒化 vs 包下载

| 维度 | 旧路径（ZIP 包） | 新路径（颗粒化）|
|---|---|---|
| 下载单元 | 整个 ZIP 压缩包 | 每个资源文件独立下载 |
| 失败处理 | 整包重下 | 仅重下失败项 |
| 进度精度 | 包级别（粗）| 文件级别（细）|
| 存储效率 | ZIP 需要解压临时空间 | 直接写入目标路径 |
| 队列控制 | 无优先级 | prepend/append 隐式优先级 |

### 4.3 隐式优先级队列

不使用正式优先级队列，通过 `prepend()` / `append()` 实现语义优先级：
- 缩略图（视觉反馈最优先）→ `prepend()` 插到队首
- 完整资源（实际使用）→ `append()` 追加到队尾
- VBL 层三档 Level（Low/Middle/High）进一步影响网络调度

### 4.4 PIMPL + VBL Observer

`FFDownloadCenterResource` 通过 PIMPL 封装所有 VBL 接口，Public 头文件零 VBL 类型暴露：
```cpp
// Public 头（无 VBL 类型）
class FFDownloadCenterResource : public QObject {
    void startDownload(const FFDownloadCenterPackageInfo&);
Q_SIGNALS:
    void sigProgress(FDownloadPackageResProgress_t);
private:
    QScopedPointer<FFDownloadCenterResourcePrivate> d_ptr;
};

// Private 类实现 VBL Observer
class FFDownloadCenterResourcePrivate : public IBsProgressCallback {
    void onProgress(int64_t bytes, int64_t total) override;
    void onFinish(int result) override;
};
```

### 4.5 红点管理与分类集成

`FFMediaFolderRedDotManager` 按素材分类维护红点状态：
- 每类素材独立的未读计数
- 用户打开对应素材文件夹时清除红点
- 持久化到本地配置（下次启动仍可见）

### 4.6 字体即时加载

字体素材下载完成后无需重启即生效：
```cpp
// IBsProgressCallback::onFinish
if (localPath.contains("/Font/")) {
    int fontId = QFontDatabase::addApplicationFont(localPath);
    // Qt 字体数据库立即生效，当前会话所有 Widget 可使用新字体
    emit sigFontInstalled(QFontDatabase::applicationFontFamilies(fontId));
}
```

---

## 五、设计模式

| 模式 | 应用位置 | 说明 |
|---|---|---|
| 单例 | `FDownloadPackManager` | 全局下载调度入口，防止重复下载 |
| PIMPL | `FFDownloadCenterResource` | 封装 VBL 接口，编译隔离 |
| Observer | `IBsProgressCallback` | VBL 回调进度到上层 |
| 命令模式 | IPC 事件结构体 | `DownloadEffect` 等封装下载命令 |
| 串行队列 | `FFDownloadCenterResourcePackage` | 包内资源顺序下载，控制并发 |
| 策略模式 | 双路径（旧/新）| `FDownloadPackManager` 按类型分流 |

---

## 六、类职责一览

| 类名 | 职责 |
|---|---|
| `FResourceDownloadCenterDlg` | 下载中心主弹窗，双 Tab（Downloading/Downloaded）|
| `FDownloadResourceModel` | QAbstractListModel，下载任务列表数据模型 |
| `FDownloadResourceDelegate` | 自定义 Item 渲染：进度条、缩略图、操作按钮 |
| `FDownloadPackManager` | 单例调度器，路由旧/新路径，管理全局下载队列 |
| `FDownloadPackTask` | 旧下载路径：`FFNetwork` + ZIP 包解压 |
| `FFDownloadCenterResource` | 新下载路径入口，PIMPL 封装 VBL，管理所有包 |
| `FFDownloadCenterResourcePackage` | 单个包的串行下载协调者，维护项队列 |
| `FFDownloadCenterResourceItem` | 单个资源文件下载器，实现 `IBsProgressCallback` |
| `IBsCloudResourceLoader` | VBL OMP 下载接口（纯虚）|
| `IBsLocalResourceLoader` | VBL SQLite 持久化接口（纯虚）|
| `FFMediaFolderRedDotManager` | 按素材分类维护红点未读状态 |
