Qt完全重构技术方案.md
# 溜云库Qt完全重构技术方案

> 基于Qt框架的全栈重构方案 - 纯Qt技术栈  
> 目标：跨平台、高性能、现代化架构

---

## 📋 执行摘要

### 项目背景

**当前痛点**：
- .NET Core 3.1已停止支持，存在安全风险
- WPF仅支持Windows，无法跨平台
- DotNetBrowser商业许可成本高（~102MB，每年续费）
- 15MB单体核心库，耦合严重
- 启动慢（2.7秒）、内存占用高（600MB+）、UI卡顿

**重构目标**：
1. **跨平台**：Windows/macOS/Linux三大平台原生支持
2. **高性能**：启动<1秒，内存<250MB，UI 60fps
3. **现代化**：Qt6 + C++17，插件化架构，易扩展
4. **降成本**：移除商业依赖，使用开源组件
5. **提体验**：流畅动画，响应式设计，现代UI

### 方案概述

**技术选型**：Qt6.5+ LTS + C++17 + QML + CMake

**架构策略**：全面采用Qt技术栈，完全重写

| 维度 | 方案 |
|------|------|
| **UI框架** | Qt Widgets（桌面控件） + Qt Quick/QML（现代UI） |
| **编程语言** | C++17（业务逻辑） + QML（UI声明） |
| **数据库** | Qt SQL（SQLite封装） |
| **网络** | Qt Network（HTTP/WebSocket） |
| **浏览器** | Qt WebEngine（Chromium内核） |
| **构建系统** | CMake 3.21+ |
| **测试框架** | Qt Test + GTest |

### 实施计划

**总周期**：18个月  
**团队规模**：8-10人  
**预算**：约280万元人民币

**三阶段路线**：
- **阶段1（6个月）**：核心框架 + 基础功能
- **阶段2（6个月）**：完整功能 + 跨平台适配
- **阶段3（6个月）**：优化 + 灰度发布 + 切换

---

## 🏗️ 总体架构设计

### 分层架构

```
┌─────────────────────────────────────────────────────────────┐
│                    表现层 (Presentation Layer)                │
├─────────────────────┬───────────────────────────────────────┤
│   Qt Quick (QML)    │        Qt Widgets                      │
│   ----------------  │        -------------                   │
│   • 素材卡片瀑布流   │   • 主窗口框架（MainWindow）            │
│   • 详情页富交互     │   • 菜单/工具栏（QMenuBar/QToolBar）   │
│   • 仪表盘/Dashboard │   • 表格（QTableView）                 │
│   • 动画效果        │   • 树形结构（QTreeView）               │
│   • 自定义控件      │   • 设置对话框（QDialog）               │
└─────────────────────┴───────────────────────────────────────┘
                            ↓ Signal/Slot + 属性绑定
┌─────────────────────────────────────────────────────────────┐
│                  应用层 (Application Layer)                   │
│                         C++17                                │
├─────────────────────────────────────────────────────────────┤
│  • ViewModels / Presenters（QML暴露对象）                    │
│  • Application Services（用例协调）                          │
│  • Commands / Queries（CQRS模式）                            │
│  • Event Bus（事件总线，QObject信号槽）                      │
│  • State Management（状态管理）                              │
└─────────────────────────────────────────────────────────────┘
                            ↓ 依赖接口
┌─────────────────────────────────────────────────────────────┐
│                   领域层 (Domain Layer)                       │
│                         C++17                                │
├─────────────────────────────────────────────────────────────┤
│  • 业务实体（Model/Material/Asset/User）                     │
│  • 领域服务（ModelService/DownloadService）                  │
│  • 仓储接口（IRepository<T>）                                │
│  • 业务规则（验证、策略模式）                                 │
│  • 领域事件（DomainEvents）                                  │
└─────────────────────────────────────────────────────────────┘
                            ↓ 实现接口
┌─────────────────────────────────────────────────────────────┐
│                 基础设施层 (Infrastructure Layer)              │
│                         C++17 + Qt                           │
├─────────────────────────────────────────────────────────────┤
│  • 数据访问（Qt SQL + SQLite）                               │
│  • 文件系统（QFile/QDir/QStandardPaths）                     │
│  • 网络服务（QNetworkAccessManager/QWebSocket）              │
│  • 云存储（OSS/COS SDK封装）                                 │
│  • 日志系统（QLoggingCategory + 文件输出）                   │
│  • 缓存管理（QCache + LRU）                                  │
│  • 配置管理（QSettings）                                     │
└─────────────────────────────────────────────────────────────┘
```

### 架构原则

1. **分层依赖单向**：上层依赖下层，下层不依赖上层
2. **依赖倒置**：应用层和领域层依赖接口，基础设施层实现接口
3. **关注点分离**：UI与业务逻辑分离，业务逻辑与数据访问分离
4. **可测试性**：每层独立测试，接口易Mock
5. **插件化**：核心功能模块化，支持动态加载

---

## 🎨 UI层技术选型

### Qt Widgets vs Qt Quick 决策矩阵

| 模块 | 推荐技术 | 理由 | 主要控件/技术 |
|------|---------|------|--------------|
| **主窗口框架** | Qt Widgets | 成熟的桌面应用框架，菜单/工具栏/Dock支持完善 | QMainWindow, QMenuBar, QToolBar, QStatusBar, QDockWidget |
| **素材瀑布流** | **Qt Quick (QML)** | 需要高性能虚拟化、流畅动画、GPU加速渲染 | GridView + Loader + Image (async) |
| **素材详情页** | **Qt Quick (QML)** | 富交互、图片轮播、缩放、动画过渡 | SwipeView, PathView, Behavior, Transition |
| **素材列表（表格）** | Qt Widgets | 大量数据展示，成熟的Model/View框架 | QTableView + QAbstractTableModel |
| **本地资源树** | Qt Widgets | 树形结构、拖拽、右键菜单，Widget更成熟 | QTreeView + QFileSystemModel, QMenu |
| **下载管理器** | Qt Widgets | 列表+进度条，不需要复杂动画 | QListView + QProgressBar delegate |
| **设置页面** | Qt Widgets | 标准表单控件（CheckBox/ComboBox/SpinBox） | QGroupBox, QFormLayout, QPushButton |
| **云渲染面板** | Qt Widgets | 表单提交、状态展示，业务逻辑重 | QFormLayout, QLineEdit, QTextEdit |
| **用户中心** | **Qt Quick (QML)** | 头像、卡片、动画效果，视觉体验优先 | Rectangle, Image, OpacityAnimator |
| **Dashboard** | **Qt Quick (QML)** | 仪表盘、图表、数据可视化 | Canvas, Charts (Qt Charts模块) |
| **通知中心** | **Qt Quick (QML)** | 弹窗、Toast、滑入滑出动画 | Popup, SequentialAnimation |

### 混合架构策略

**主容器**：QMainWindow（Qt Widgets）  
**嵌入QML**：QQuickWidget（将QML嵌入Widget窗口）

```
QMainWindow
├── QMenuBar (菜单)
├── QToolBar (工具栏)
├── Central Widget
│   ├── QSplitter (分割器)
│   │   ├── QTreeView (左侧：本地资源树) [Widget]
│   │   └── QStackedWidget (右侧内容区)
│   │       ├── QQuickWidget (素材瀑布流) [QML]
│   │       ├── QQuickWidget (素材详情) [QML]
│   │       ├── QTableView (下载管理器) [Widget]
│   │       └── ...
└── QStatusBar (状态栏)
```

**优势**：
- ✅ 利用Qt Widgets的成熟桌面框架
- ✅ 利用QML的现代UI和高性能渲染
- ✅ 逐步迁移，降低风险
- ✅ 各取所长，灵活组合

---

## 🧩 核心模块技术方案

### 1. 素材浏览模块

**功能需求**：
- 显示10,000+素材卡片（缩略图+名称+价格+下载量）
- 流畅滚动（60fps）
- 搜索、筛选、排序
- 分类导航

**技术选型**：

| 组件 | 技术 | 说明 |
|------|------|------|
| **UI** | Qt Quick (GridView) | GPU加速，虚拟化渲染 |
| **数据模型** | QAbstractListModel (C++) | 支持10万+数据，LRU缓存 |
| **图片加载** | QML Image (async) | 异步加载，自动缓存 |
| **缩略图缓存** | QPixmapCache + 磁盘缓存 | 内存+磁盘二级缓存 |
| **网络请求** | QNetworkAccessManager | 异步HTTP请求 |
| **搜索** | Qt SQL (FTS5) | SQLite全文搜索 |

**核心实现要点**：

1. **高性能Model**：
   - 使用`QAbstractListModel`，支持延迟加载（Lazy Loading）
   - LRU缓存最近访问的1000条数据
   - `data()`方法中触发异步加载，加载完成后`emit dataChanged()`
   
2. **虚拟化渲染**：
   - `GridView`的`cacheBuffer: 1000`，预加载屏幕外1000px内容
   - `delegate`简单化，减少复杂计算和绑定
   - 使用`Loader`按需加载复杂组件

3. **图片优化**：
   - `Image { asynchronous: true; cache: true }`
   - 缩略图服务端生成（200x150），减少传输
   - 磁盘缓存：`~/.cache/liuyunku/thumbnails/`

4. **分页加载**：
   - 滚动到底部时自动加载下一页（20-50条/页）
   - `GridView.onAtYEnd`触发加载

**架构层次**：
```
QML GridView
    ↓ (roleNames: modelId, name, thumbnailUrl, ...)
ModelListModel (QAbstractListModel, C++)
    ↓ (异步加载)
ModelService (C++, 业务逻辑)
    ↓
ModelRepository (C++, 数据访问)
    ↓
Qt SQL (SQLite)
```

---

### 2. 素材下载模块

**功能需求**：
- 多线程下载（支持5-10个并发）
- 断点续传
- 下载队列管理
- 进度显示（速度、剩余时间、百分比）

**技术选型**：

| 组件 | 技术 | 说明 |
|------|------|------|
| **下载引擎** | QNetworkAccessManager + QNetworkReply | Qt原生网络库 |
| **多线程** | QThreadPool + QRunnable | 线程池管理 |
| **断点续传** | HTTP Range请求 | Range: bytes=已下载字节- |
| **下载队列** | QQueue<DownloadTask> | FIFO队列 |
| **进度通知** | Qt Signals/Slots | 跨线程信号槽 |
| **持久化** | SQLite (下载记录表) | 保存未完成任务 |
| **文件校验** | QCryptographicHash (MD5/SHA256) | 完整性验证 |

**核心实现要点**：

1. **DownloadManager（单例）**：
   - 管理下载队列和工作线程
   - 限制并发数（`QThreadPool::setMaxThreadCount(5)`）
   - 提供API：`addTask()`, `pauseTask()`, `resumeTask()`, `cancelTask()`

2. **DownloadTask（QRunnable）**：
   - 每个任务独立线程执行
   - 使用`QNetworkAccessManager`下载
   - 支持`Range`请求实现断点续传
   - 下载完成后emit `downloadCompleted(taskId, filePath)`

3. **断点续传实现**：
   ```cpp
   // 检查本地已下载部分
   QFile file(localPath);
   qint64 bytesDownloaded = file.size();
   
   // 设置Range请求头
   QNetworkRequest request(url);
   if (bytesDownloaded > 0) {
       QString rangeHeader = QString("bytes=%1-").arg(bytesDownloaded);
       request.setRawHeader("Range", rangeHeader.toUtf8());
   }
   
   // 追加模式打开文件
   file.open(QIODevice::Append);
   ```

4. **进度计算**：
   - `downloadProgress(qint64 bytesReceived, qint64 bytesTotal)`信号
   - 计算速度：`(当前字节 - 上次字节) / 时间差`
   - 计算剩余时间：`剩余字节 / 当前速度`

**架构层次**：
```
UI (QTableView / QML ListView)
    ↓ (显示进度)
DownloadManager (单例, C++)
    ↓ (管理任务队列)
DownloadTask (QRunnable)
    ↓ (执行下载)
QNetworkAccessManager
    ↓
HTTP/HTTPS
```

---

### 3. 本地资源管理模块

**功能需求**：
- 树形目录结构展示
- 文件/文件夹操作（新建、重命名、删除、移动）
- 拖拽支持（从外部拖入，内部拖拽排序）
- 右键菜单
- 文件预览（缩略图、属性）

**技术选型**：

| 组件 | 技术 | 说明 |
|------|------|------|
| **UI** | QTreeView + QFileSystemModel | Qt标准树形控件 |
| **数据模型** | 自定义QAbstractItemModel | 支持虚拟目录、标签分类 |
| **文件操作** | QFile, QDir | Qt文件API |
| **拖拽** | Qt Drag & Drop框架 | 支持内部/外部拖拽 |
| **右键菜单** | QMenu, QAction | 上下文菜单 |
| **数据库** | SQLite (文件元数据表) | 存储标签、分类、自定义属性 |
| **缩略图** | QImage + QPixmap | 生成和缓存 |

**核心实现要点**：

1. **自定义Model**：
   - 继承`QAbstractItemModel`
   - 支持虚拟目录（"我的收藏"、"最近使用"）
   - 支持标签分类（"室内"、"家具"、"灯光"）
   - 数据源：文件系统 + SQLite元数据

2. **拖拽实现**：
   ```cpp
   // 启用拖放
   treeView->setDragEnabled(true);
   treeView->setAcceptDrops(true);
   treeView->setDropIndicatorShown(true);
   
   // Model中实现
   Qt::ItemFlags flags() const override {
       return Qt::ItemIsSelectable | Qt::ItemIsEnabled | 
              Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
   }
   
   QMimeData* mimeData(const QModelIndexList &indexes) const override;
   bool dropMimeData(...) override;
   ```

3. **右键菜单**：
   ```cpp
   void onCustomContextMenu(const QPoint &pos) {
       QModelIndex index = treeView->indexAt(pos);
       QMenu menu;
       menu.addAction("打开", this, &Widget::onOpen);
       menu.addAction("重命名", this, &Widget::onRename);
       menu.addAction("删除", this, &Widget::onDelete);
       menu.exec(treeView->mapToGlobal(pos));
   }
   ```

4. **性能优化**：
   - 延迟加载子节点（`canFetchMore()` / `fetchMore()`）
   - 缩略图异步生成（`QtConcurrent::run()`）
   - 大目录分页加载

**架构层次**：
```
QTreeView
    ↓ (Model/View)
LocalResourceModel (QAbstractItemModel)
    ↓ (数据源)
LocalResourceRepository
    ↓ (文件系统 + 数据库)
QFileSystemWatcher (监控文件变化)
```

---

### 4. 在线素材浏览模块

**功能需求**：
- 嵌入Web页面（3D66官网）
- 与原生交互（登录状态同步、下载触发）
- Cookie管理
- JavaScript交互

**技术选型**：

| 组件 | 技术 | 说明 |
|------|------|------|
| **浏览器引擎** | Qt WebEngine | Chromium内核，完整Web支持 |
| **UI容器** | QWebEngineView | Qt Widgets版本 |
| **JS交互** | QWebChannel | 原生C++ ↔ JavaScript双向通信 |
| **Cookie管理** | QWebEngineCookieStore | 持久化Cookie |
| **下载拦截** | QWebEngineProfile::downloadRequested | 捕获下载请求 |

**核心实现要点**：

1. **嵌入WebEngine**：
   ```cpp
   QWebEngineView *webView = new QWebEngineView(this);
   QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
   
   // 设置持久化路径
   profile->setPersistentStoragePath(QStandardPaths::writableLocation(
       QStandardPaths::AppDataLocation) + "/webengine");
   
   webView->load(QUrl("https://www.3d66.com/"));
   ```

2. **C++ ↔ JavaScript交互**：
   ```cpp
   // C++端：暴露对象给JS
   class BridgeObject : public QObject {
       Q_OBJECT
   public slots:
       void downloadModel(const QString &modelId) {
           // 原生下载逻辑
       }
   signals:
       void loginStatusChanged(bool isLoggedIn);
   };
   
   QWebChannel *channel = new QWebChannel(this);
   BridgeObject *bridge = new BridgeObject(this);
   channel->registerObject("bridge", bridge);
   webView->page()->setWebChannel(channel);
   
   // JavaScript端：
   // <script src="qrc:///qtwebchannel/qwebchannel.js"></script>
   // new QWebChannel(qt.webChannelTransport, function(channel) {
   //     var bridge = channel.objects.bridge;
   //     bridge.downloadModel("model-12345");
   // });
   ```

3. **下载拦截**：
   ```cpp
   connect(profile, &QWebEngineProfile::downloadRequested,
           this, [](QWebEngineDownloadItem *download) {
       // 拦截下载，使用原生下载管理器
       QString url = download->url().toString();
       QString fileName = download->suggestedFileName();
       download->cancel();
       
       // 触发原生下载
       DownloadManager::instance().addTask(url, fileName);
   });
   ```

4. **Cookie同步**：
   ```cpp
   // 从WebEngine获取Cookie同步到原生
   QWebEngineCookieStore *cookieStore = profile->cookieStore();
   connect(cookieStore, &QWebEngineCookieStore::cookieAdded,
           this, [](const QNetworkCookie &cookie) {
       // 保存到原生Cookie存储
       AuthService::instance().saveCookie(cookie);
   });
   ```

**替代方案**：
- 如果不需要完整Web页面，可使用`QNetworkAccessManager` + 自定义UI
- 优点：体积小、性能好
- 缺点：需要重新实现Web UI

---

### 5. 云渲染集成模块

**功能需求**：
- 提交渲染任务（场景文件上传）
- 渲染参数配置（分辨率、质量、渲染器）
- 任务进度查询（轮询或WebSocket）
- 结果下载

**技术选型**：

| 组件 | 技术 | 说明 |
|------|------|------|
| **UI** | Qt Widgets (QFormLayout) | 表单提交界面 |
| **HTTP客户端** | QNetworkAccessManager | RESTful API调用 |
| **实时通信** | QWebSocket | 接收实时进度推送 |
| **文件上传** | QHttpMultiPart | 大文件分片上传 |
| **JSON解析** | QJsonDocument | API响应解析 |

**核心实现要点**：

1. **文件上传（分片）**：
   ```cpp
   void uploadLargeFile(const QString &filePath) {
       QFile *file = new QFile(filePath);
       file->open(QIODevice::ReadOnly);
       
       qint64 chunkSize = 5 * 1024 * 1024; // 5MB分片
       qint64 totalSize = file->size();
       int chunkCount = (totalSize + chunkSize - 1) / chunkSize;
       
       for (int i = 0; i < chunkCount; ++i) {
           QByteArray chunk = file->read(chunkSize);
           uploadChunk(chunk, i, chunkCount);
       }
   }
   ```

2. **WebSocket实时推送**：
   ```cpp
   QWebSocket *socket = new QWebSocket();
   connect(socket, &QWebSocket::textMessageReceived,
           this, [](const QString &message) {
       QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
       QJsonObject obj = doc.object();
       
       if (obj["type"] == "progress") {
           double progress = obj["progress"].toDouble();
           emit renderProgressUpdated(progress);
       }
   });
   
   socket->open(QUrl("wss://render.3d66.com/ws"));
   ```

3. **任务状态机**：
   ```cpp
   enum class RenderTaskState {
       Pending,      // 排队中
       Uploading,    // 上传中
       Rendering,    // 渲染中
       Completed,    // 完成
       Failed        // 失败
   };
   
   class RenderTask {
       RenderTaskState state;
       double progress;
       QString errorMessage;
   };
   ```

---

### 6. 3ds Max集成模块（Windows专有）

**功能需求**：
- 一键导入模型到Max
- 材质自动映射
- 单位和坐标系转换
- 批量导入

**技术选型**：

| 组件 | 技术 | 说明 |
|------|------|------|
| **进程间通信** | Named Pipe / QLocalSocket | 与Max插件通信 |
| **Max插件** | MaxScript / MAXScript SDK | Max端脚本或C++插件 |
| **文件格式** | FBX / OBJ / 3DS | 标准交换格式 |
| **协议** | JSON命令协议 | 结构化命令传输 |

**核心实现要点**：

1. **架构**：
   ```
   溜云库客户端 (Qt)
       ↓ (命名管道)
   Max插件 (MaxScript/C++)
       ↓ (MAXScript API)
   3ds Max
   ```

2. **命令协议**：
   ```json
   {
     "command": "importModel",
     "params": {
       "filePath": "C:/models/chair.fbx",
       "importMaterials": true,
       "unitScale": 1.0,
       "position": [0, 0, 0]
     }
   }
   ```

3. **Qt端实现**：
   ```cpp
   QLocalSocket socket;
   socket.connectToServer("LiuYunKu_Max_Bridge");
   
   QJsonObject command;
   command["command"] = "importModel";
   command["params"] = QJsonObject{
       {"filePath", modelPath},
       {"importMaterials", true}
   };
   
   QJsonDocument doc(command);
   socket.write(doc.toJson());
   socket.waitForBytesWritten();
   
   // 等待响应
   socket.waitForReadyRead();
   QByteArray response = socket.readAll();
   ```

4. **Max端（MaxScript示例）**：
   ```maxscript
   -- 监听命名管道
   fn importModelFromBridge filePath importMaterials:true = (
       importFile filePath #noPrompt using:FBXIMP
       
       if importMaterials then (
           -- 材质处理逻辑
       )
       
       return true
   )
   ```

---

### 7. 用户系统模块

**功能需求**：
- 登录/注册/登出
- 用户信息管理（头像、昵称、VIP状态）
- Token管理（JWT）
- 单点登录（SSO）

**技术选型**：

| 组件 | 技术 | 说明 |
|------|------|------|
| **UI** | Qt Quick (QML) | 登录页、用户中心 |
| **HTTP客户端** | QNetworkAccessManager | RESTful API |
| **加密** | QCryptographicHash | 密码哈希（SHA256） |
| **Token存储** | QSettings (加密存储) | 持久化Token |
| **头像缓存** | QPixmapCache | 内存缓存 |

**核心实现要点**：

1. **登录流程**：
   ```cpp
   void AuthService::login(const QString &username, const QString &password) {
       QNetworkRequest request(QUrl("https://api.3d66.com/auth/login"));
       request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
       
       QJsonObject json;
       json["username"] = username;
       json["password"] = hashPassword(password); // SHA256
       
       QNetworkReply *reply = m_networkManager->post(request, 
           QJsonDocument(json).toJson());
       
       connect(reply, &QNetworkReply::finished, this, [=]() {
           QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
           QString token = doc["token"].toString();
           
           // 保存Token
           saveToken(token);
           emit loginSucceeded();
       });
   }
   ```

2. **Token自动续期**：
   ```cpp
   QTimer *refreshTimer = new QTimer(this);
   connect(refreshTimer, &QTimer::timeout, this, [=]() {
       // Token过期前5分钟刷新
       if (tokenExpiresIn() < 300) {
           refreshToken();
       }
   });
   refreshTimer->start(60000); // 每分钟检查
   ```

3. **全局请求拦截（添加Token）**：
   ```cpp
   class AuthInterceptor : public QNetworkAccessManager {
   protected:
       QNetworkReply* createRequest(Operation op, 
                                     const QNetworkRequest &req,
                                     QIODevice *outgoingData) override {
           QNetworkRequest authReq(req);
           QString token = AuthService::instance().getToken();
           authReq.setRawHeader("Authorization", 
                                "Bearer " + token.toUtf8());
           return QNetworkAccessManager::createRequest(op, authReq, outgoingData);
       }
   };
   ```

---

### 8. 配置与设置模块

**功能需求**：
- 应用设置（主题、语言、下载路径）
- 用户偏好（启动行为、缓存策略）
- 导入/导出配置

**技术选型**：

| 组件 | 技术 | 说明 |
|------|------|------|
| **持久化** | QSettings | 跨平台配置存储 |
| **UI** | Qt Widgets (QDialog) | 设置对话框 |
| **主题** | QSS (Qt Style Sheets) | 样式表 |
| **多语言** | Qt Linguist (.ts文件) | 国际化 |

**核心实现要点**：

1. **QSettings用法**：
   ```cpp
   // 写入
   QSettings settings("3D66", "LiuYunKu");
   settings.setValue("download/path", "/path/to/downloads");
   settings.setValue("ui/theme", "dark");
   
   // 读取
   QString downloadPath = settings.value("download/path", 
       QStandardPaths::writableLocation(QStandardPaths::DownloadLocation))
       .toString();
   ```

2. **主题切换**：
   ```cpp
   void ThemeManager::applyTheme(const QString &themeName) {
       QFile styleFile(":/styles/" + themeName + ".qss");
       styleFile.open(QFile::ReadOnly);
       QString styleSheet = QLatin1String(styleFile.readAll());
       qApp->setStyleSheet(styleSheet);
   }
   ```

3. **多语言支持**：
   ```cpp
   QTranslator translator;
   QString locale = QLocale::system().name(); // "zh_CN"
   translator.load(":/i18n/liuyunku_" + locale + ".qm");
   qApp->installTranslator(&translator);
   
   // 在代码中使用
   QString text = tr("Download"); // 运行时翻译
   
   // 在QML中使用
   Text { text: qsTr("Download") }
   ```

---

### 9. 数据存储模块

**技术选型**：

| 数据类型 | 技术 | 说明 |
|---------|------|------|
| **结构化数据** | SQLite (Qt SQL) | 用户数据、素材元数据、下载记录 |
| **配置数据** | QSettings | 应用配置、用户偏好 |
| **缓存数据** | QCache + 磁盘缓存 | 缩略图、临时数据 |
| **日志** | 文本文件 (QFile) | 应用日志、错误日志 |

**数据库设计（主要表）**：

```sql
-- 素材表
CREATE TABLE models (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    category TEXT,
    tags TEXT, -- JSON数组
    thumbnail_url TEXT,
    file_size INTEGER,
    price REAL,
    download_count INTEGER,
    created_at INTEGER,
    updated_at INTEGER
);

CREATE INDEX idx_models_category ON models(category);
CREATE INDEX idx_models_name ON models(name);

-- 全文搜索
CREATE VIRTUAL TABLE models_fts USING fts5(
    name, tags, content=models, content_rowid=id
);

-- 下载任务表
CREATE TABLE download_tasks (
    id TEXT PRIMARY KEY,
    model_id TEXT,
    url TEXT,
    local_path TEXT,
    status TEXT, -- pending, downloading, completed, failed
    progress REAL,
    downloaded_bytes INTEGER,
    total_bytes INTEGER,
    created_at INTEGER,
    updated_at INTEGER
);

-- 用户收藏表
CREATE TABLE favorites (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    model_id TEXT,
    user_id TEXT,
    created_at INTEGER,
    UNIQUE(model_id, user_id)
);

-- 本地文件元数据表
CREATE TABLE local_files (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_path TEXT UNIQUE,
    model_id TEXT,
    tags TEXT, -- JSON数组
    custom_name TEXT,
    thumbnail_path TEXT,
    file_size INTEGER,
    created_at INTEGER,
    updated_at INTEGER
);
```

**仓储模式实现**：

```cpp
// 接口定义
template<typename T>
class IRepository {
public:
    virtual ~IRepository() = default;
    
    virtual QList<T> findAll() = 0;
    virtual std::optional<T> findById(const QString &id) = 0;
    virtual bool save(const T &entity) = 0;
    virtual bool remove(const QString &id) = 0;
};

// 实现（以ModelRepository为例）
class ModelRepository : public IRepository<Model> {
public:
    QList<Model> findAll() override {
        QList<Model> models;
        QSqlQuery query("SELECT * FROM models");
        while (query.next()) {
            models.append(mapToModel(query));
        }
        return models;
    }
    
    std::optional<Model> findById(const QString &id) override {
        QSqlQuery query;
        query.prepare("SELECT * FROM models WHERE id = ?");
        query.addBindValue(id);
        
        if (query.exec() && query.next()) {
            return mapToModel(query);
        }
        return std::nullopt;
    }
    
private:
    Model mapToModel(const QSqlQuery &query) {
        Model model;
        model.id = query.value("id").toString();
        model.name = query.value("name").toString();
        // ... 其他字段映射
        return model;
    }
};
```

---

### 10. 网络通信模块

**技术选型**：

| 功能 | 技术 | 说明 |
|------|------|------|
| **HTTP客户端** | QNetworkAccessManager | RESTful API调用 |
| **WebSocket** | QWebSocket | 实时双向通信 |
| **文件上传** | QHttpMultiPart | 多部分表单上传 |
| **SSL/TLS** | QSslSocket | HTTPS支持 |
| **请求重试** | 自定义RetryPolicy | 网络容错 |
| **请求缓存** | QNetworkDiskCache | HTTP缓存 |

**核心实现要点**：

1. **单例NetworkManager**：
   ```cpp
   class NetworkManager : public QObject {
       Q_OBJECT
   public:
       static NetworkManager& instance() {
           static NetworkManager instance;
           return instance;
       }
       
       QNetworkReply* get(const QUrl &url);
       QNetworkReply* post(const QUrl &url, const QByteArray &data);
       
   private:
       NetworkManager();
       QNetworkAccessManager *m_manager;
       QNetworkDiskCache *m_cache;
   };
   ```

2. **请求重试**：
   ```cpp
   QNetworkReply* NetworkManager::getWithRetry(const QUrl &url, int maxRetries) {
       int retries = 0;
       QNetworkReply *reply = nullptr;
       
       while (retries < maxRetries) {
           reply = m_manager->get(QNetworkRequest(url));
           
           QEventLoop loop;
           connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
           loop.exec();
           
           if (reply->error() == QNetworkReply::NoError) {
               return reply;
           }
           
           retries++;
           QThread::msleep(1000 * retries); // 指数退避
       }
       
       return reply;
   }
   ```

3. **启用HTTP缓存**：
   ```cpp
   QNetworkDiskCache *cache = new QNetworkDiskCache(this);
   cache->setCacheDirectory(QStandardPaths::writableLocation(
       QStandardPaths::CacheLocation) + "/http");
   cache->setMaximumCacheSize(100 * 1024 * 1024); // 100MB
   m_manager->setCache(cache);
   ```

---

### 11. 日志系统模块

**技术选型**：

| 组件 | 技术 | 说明 |
|------|------|------|
| **日志框架** | QLoggingCategory + 自定义Handler | Qt原生日志系统 |
| **日志输出** | QFile (文件) + qDebug (控制台) | 双重输出 |
| **日志轮转** | 按日期/大小分割 | 防止单个文件过大 |
| **日志级别** | Debug / Info / Warning / Critical | 分级过滤 |

**核心实现要点**：

```cpp
// 自定义日志处理器
void customMessageHandler(QtMsgType type, 
                          const QMessageLogContext &context, 
                          const QString &msg) {
    QString levelStr;
    switch (type) {
        case QtDebugMsg:    levelStr = "DEBUG"; break;
        case QtInfoMsg:     levelStr = "INFO"; break;
        case QtWarningMsg:  levelStr = "WARN"; break;
        case QtCriticalMsg: levelStr = "ERROR"; break;
        case QtFatalMsg:    levelStr = "FATAL"; break;
    }
    
    QString timestamp = QDateTime::currentDateTime()
        .toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString category = context.category ? context.category : "default";
    QString file = context.file ? QFileInfo(context.file).fileName() : "";
    
    QString logMessage = QString("[%1] [%2] [%3:%4] %5")
        .arg(timestamp)
        .arg(levelStr)
        .arg(file)
        .arg(context.line)
        .arg(msg);
    
    // 输出到文件
    QFile logFile(getLogFilePath());
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&logFile);
        stream << logMessage << "\n";
        logFile.close();
    }
    
    // 输出到控制台
    fprintf(stderr, "%s\n", logMessage.toUtf8().constData());
    
    // 日志轮转检查
    checkLogRotation();
}

// 在main函数中安装
int main(int argc, char *argv[]) {
    qInstallMessageHandler(customMessageHandler);
    
    // 使用
    qDebug() << "Application started";
    qInfo() << "User logged in:" << username;
    qWarning() << "Network timeout";
    qCritical() << "Database connection failed";
}
```

---

### 12. 插件系统模块

**功能需求**：
- 动态加载/卸载插件
- 插件版本管理
- 插件依赖解析
- 插件热更新

**技术选型**：

| 组件 | 技术 | 说明 |
|------|------|------|
| **插件加载** | QPluginLoader | Qt插件机制 |
| **接口定义** | Q_DECLARE_INTERFACE | 插件接口宏 |
| **插件元数据** | Q_PLUGIN_METADATA (JSON) | 插件描述 |
| **依赖管理** | 拓扑排序 | 按依赖顺序加载 |

**核心实现要点**：

1. **插件接口定义**：
   ```cpp
   // iplugin.h
   class IPlugin {
   public:
       virtual ~IPlugin() = default;
       
       virtual QString name() const = 0;
       virtual QString version() const = 0;
       virtual QStringList dependencies() const = 0;
       
       virtual bool initialize() = 0;
       virtual void shutdown() = 0;
   };
   
   Q_DECLARE_INTERFACE(IPlugin, "com.3d66.LiuYunKu.IPlugin/1.0")
   ```

2. **插件实现示例**：
   ```cpp
   // sample_plugin.h
   class SamplePlugin : public QObject, public IPlugin {
       Q_OBJECT
       Q_PLUGIN_METADATA(IID "com.3d66.LiuYunKu.IPlugin/1.0" 
                         FILE "sample_plugin.json")
       Q_INTERFACES(IPlugin)
       
   public:
       QString name() const override { return "SamplePlugin"; }
       QString version() const override { return "1.0.0"; }
       QStringList dependencies() const override { return {}; }
       
       bool initialize() override {
           qInfo() << "SamplePlugin initialized";
           return true;
       }
       
       void shutdown() override {
           qInfo() << "SamplePlugin shutdown";
       }
   };
   ```

3. **插件元数据（sample_plugin.json）**：
   ```json
   {
       "name": "SamplePlugin",
       "version": "1.0.0",
       "description": "示例插件",
       "author": "3D66",
       "dependencies": []
   }
   ```

4. **插件管理器**：
   ```cpp
   class PluginManager : public QObject {
   public:
       void loadPlugins(const QString &pluginDir) {
           QDir dir(pluginDir);
           
           for (const QString &fileName : dir.entryList(QDir::Files)) {
               QString filePath = dir.absoluteFilePath(fileName);
               QPluginLoader loader(filePath);
               
               QObject *pluginObj = loader.instance();
               if (pluginObj) {
                   IPlugin *plugin = qobject_cast<IPlugin*>(pluginObj);
                   if (plugin) {
                       m_plugins.append(plugin);
                   }
               }
           }
           
           // 依赖排序
           sortByDependencies();
           
           // 初始化
           for (IPlugin *plugin : m_plugins) {
               plugin->initialize();
           }
       }
       
   private:
       QList<IPlugin*> m_plugins;
       
       void sortByDependencies() {
           // 拓扑排序实现
       }
   };
   ```

---

## 🚀 性能优化策略

### 启动性能

**目标**：< 1秒（当前2.7秒）

| 优化项 | 技术方案 | 预期提升 |
|--------|---------|---------|
| **延迟加载** | 非核心模块延迟初始化 | 减少40%启动时间 |
| **并行初始化** | QtConcurrent并行加载数据库、配置、网络 | 减少30%启动时间 |
| **轻量级闪屏** | QSplashScreen最小化加载 | 减少100ms |
| **减少依赖** | 移除Qt WebEngine预加载（按需加载） | 减少500ms |
| **预编译** | MOC/UIC预编译，减少运行时开销 | 减少10% |

**实现**：
```cpp
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 轻量级闪屏
    QSplashScreen splash(QPixmap(":/splash.png"));
    splash.show();
    
    // 并行初始化
    auto dbFuture = QtConcurrent::run([] { DatabaseManager::instance().init(); });
    auto configFuture = QtConcurrent::run([] { ConfigManager::instance().load(); });
    auto cacheFuture = QtConcurrent::run([] { CacheManager::instance().init(); });
    
    dbFuture.waitForFinished();
    configFuture.waitForFinished();
    
    MainWindow mainWindow;
    splash.finish(&mainWindow);
    mainWindow.show();
    
    // 后台继续加载非核心模块
    QtConcurrent::run([] { PluginManager::instance().loadPlugins(); });
    
    return app.exec();
}
```

---

### 内存优化

**目标**：< 250MB（当前600MB+）

| 优化项 | 技术方案 | 预期提升 |
|--------|---------|---------|
| **LRU缓存** | 缩略图、模型数据限制缓存大小 | 减少200MB |
| **智能指针** | 使用QSharedPointer/QWeakPointer | 防止内存泄漏 |
| **图片压缩** | 缩略图JPEG压缩（质量80%） | 减少50MB |
| **延迟加载** | QML Loader按需加载组件 | 减少100MB |
| **定期清理** | QTimer定期清理过期缓存 | 稳定内存占用 |

**LRU缓存实现**：
```cpp
template<typename K, typename V>
class LRUCache {
public:
    LRUCache(int capacity) : m_capacity(capacity) {}
    
    std::optional<V> get(const K &key) {
        auto it = m_cache.find(key);
        if (it == m_cache.end()) return std::nullopt;
        
        // 移到最前面
        m_list.splice(m_list.begin(), m_list, it->second);
        return it->second->second;
    }
    
    void put(const K &key, const V &value) {
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            m_list.erase(it->second);
            m_cache.erase(it);
        }
        
        // 插入到最前面
        m_list.push_front({key, value});
        m_cache[key] = m_list.begin();
        
        // 淘汰最旧的
        if (m_cache.size() > m_capacity) {
            auto last = m_list.back();
            m_cache.erase(last.first);
            m_list.pop_back();
        }
    }
    
private:
    int m_capacity;
    std::list<std::pair<K, V>> m_list;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> m_cache;
};
```

---

### 渲染性能

**目标**：素材列表60fps

| 优化项 | 技术方案 |
|--------|---------|
| **虚拟化** | QML GridView cacheBuffer |
| **异步加载** | Image { asynchronous: true } |
| **GPU加速** | Qt Quick Scene Graph |
| **简化Delegate** | 减少复杂计算和绑定 |
| **批量更新** | dataChanged批量通知 |

---

## 🧪 测试策略

### 单元测试

**框架**：Qt Test

**覆盖目标**：70%+

**测试示例**：
```cpp
class TestModelRepository : public QObject {
    Q_OBJECT
    
private slots:
    void testFindAll() {
        ModelRepository repo;
        QList<Model> models = repo.findAll();
        QVERIFY(models.size() > 0);
    }
    
    void testFindById() {
        ModelRepository repo;
        auto model = repo.findById("test-id-123");
        QVERIFY(model.has_value());
        QCOMPARE(model->name, "Test Model");
    }
};

QTEST_MAIN(TestModelRepository)
```

---

### 集成测试

**框架**：Qt Test + 模拟服务器

**测试场景**：
- 登录流程（Mock HTTP Server）
- 下载流程（Mock文件下载）
- 数据库迁移（SQLite内存数据库）

---

### UI测试

**框架**：Qt Test (QTest::mouseClick / keyClick)

**测试示例**：
```cpp
void testButtonClick() {
    MainWindow window;
    QPushButton *button = window.findChild<QPushButton*>("downloadButton");
    
    QTest::mouseClick(button, Qt::LeftButton);
    
    QVERIFY(DownloadManager::instance().hasActiveTasks());
}
```

---

### 性能测试

**工具**：
- Qt Quick Profiler（QML性能）
- Valgrind / Dr. Memory（内存泄漏）
- GProf / Perf（CPU性能）

**基准测试**：
```cpp
void benchmarkModelLoading() {
    ModelRepository repo;
    
    QBENCHMARK {
        repo.findAll(); // 测试查询性能
    }
}
```

---

## 📦 构建与部署

### CMake配置（完整）

```cmake
cmake_minimum_required(VERSION 3.21)
project(LiuYunKu VERSION 5.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt模块
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Quick
    Qml
    QuickWidgets
    Network
    WebEngineWidgets
    WebSockets
    Sql
    Test
)

# 源文件
set(SOURCES
    src/main.cpp
    src/ui/mainwindow.cpp
    src/models/model.cpp
    src/models/model_list_model.cpp
    src/services/model_service.cpp
    src/services/download_service.cpp
    src/services/auth_service.cpp
    src/repositories/model_repository.cpp
    src/repositories/download_repository.cpp
    src/network/network_manager.cpp
    src/utils/lru_cache.cpp
)

# 头文件
set(HEADERS
    src/ui/mainwindow.h
    src/models/model.h
    src/models/model_list_model.h
    # ... 其他头文件
)

# QML文件
set(QML_FILES
    qml/main.qml
    qml/ModelGridView.qml
    qml/ModelDetailPage.qml
)

# 资源文件
qt6_add_resources(RESOURCES
    resources/resources.qrc
)

# 可执行文件
add_executable(LiuYunKu
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# 链接Qt库
target_link_libraries(LiuYunKu PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickWidgets
    Qt6::Network
    Qt6::WebEngineWidgets
    Qt6::WebSockets
    Qt6::Sql
)

# 包含目录
target_include_directories(LiuYunKu PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# 安装规则
install(TARGETS LiuYunKu
    RUNTIME DESTINATION bin
)

# Windows部署
if(WIN32)
    find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
    add_custom_command(TARGET LiuYunKu POST_BUILD
        COMMAND ${WINDEPLOYQT} 
            --qmldir ${CMAKE_SOURCE_DIR}/qml
            $<TARGET_FILE:LiuYunKu>
    )
endif()

# macOS部署
if(APPLE)
    set_target_properties(LiuYunKu PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
    )
    
    find_program(MACDEPLOYQT macdeployqt HINTS "${Qt6_DIR}/../../../bin")
    add_custom_command(TARGET LiuYunKu POST_BUILD
        COMMAND ${MACDEPLOYQT} 
            $<TARGET_FILE_DIR:LiuYunKu>/../..
            -qmldir=${CMAKE_SOURCE_DIR}/qml
    )
endif()

# Linux部署
if(UNIX AND NOT APPLE)
    install(FILES ${CMAKE_SOURCE_DIR}/liuyunku.desktop
        DESTINATION share/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/icons/liuyunku.png
        DESTINATION share/icons/hicolor/256x256/apps)
endif()
```

---

### 目录结构

```
LiuYunKu/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── src/
│   ├── main.cpp
│   ├── ui/
│   │   ├── mainwindow.h/cpp
│   │   └── dialogs/
│   ├── models/
│   │   ├── model.h/cpp
│   │   └── model_list_model.h/cpp
│   ├── services/
│   │   ├── model_service.h/cpp
│   │   ├── download_service.h/cpp
│   │   └── auth_service.h/cpp
│   ├── repositories/
│   │   ├── model_repository.h/cpp
│   │   └── download_repository.h/cpp
│   ├── network/
│   │   └── network_manager.h/cpp
│   └── utils/
│       ├── lru_cache.h/cpp
│       └── logger.h/cpp
├── qml/
│   ├── main.qml
│   ├── ModelGridView.qml
│   ├── ModelDetailPage.qml
│   └── components/
│       ├── ModelCard.qml
│       └── CustomButton.qml
├── resources/
│   ├── resources.qrc
│   ├── images/
│   ├── styles/
│   │   ├── light.qss
│   │   └── dark.qss
│   └── i18n/
│       ├── liuyunku_zh_CN.ts
│       └── liuyunku_en_US.ts
├── tests/
│   ├── test_model_repository.cpp
│   ├── test_download_service.cpp
│   └── test_lru_cache.cpp
├── docs/
│   ├── architecture.md
│   ├── api.md
│   └── build.md
└── scripts/
    ├── build.sh
    ├── deploy.sh
    └── package.sh
```

---

## 📅 实施计划

### 总体时间线：18个月

```
阶段1：核心框架 + 基础功能（6个月）
├─ M1: 项目搭建 + 架构设计（Week 1-4）
├─ M2: 主窗口 + 基础UI（Week 5-8）
├─ M3: 素材浏览模块（Week 9-12）
├─ M4: 下载管理模块（Week 13-16）
├─ M5: 本地资源管理（Week 17-20）
└─ M6: 用户系统 + 设置（Week 21-26）

阶段2：完整功能 + 跨平台（6个月）
├─ M7: 在线浏览 + WebEngine（Week 27-30）
├─ M8: 云渲染集成（Week 31-34）
├─ M9: 3ds Max集成（Week 35-38）
├─ M10: macOS平台适配（Week 39-42）
├─ M11: Linux平台适配（Week 43-46）
└─ M12: 插件系统（Week 47-52）

阶段3：优化 + 发布（6个月）
├─ M13: 性能优化（Week 53-56）
├─ M14: 测试完善（Week 57-60）
├─ M15: 文档编写（Week 61-64）
├─ M16: 灰度发布（Week 65-68）
├─ M17: 全量发布（Week 69-72）
└─ M18: 旧版下线（Week 73-78）
```

---

### 关键里程碑

| 里程碑 | 时间 | 交付物 | 验收标准 |
|--------|------|--------|---------|
| **M6** | 第6月末 | Windows基础版本 | 核心功能可用，性能达标 |
| **M12** | 第12月末 | 跨平台完整版 | 三大平台功能一致 |
| **M15** | 第15月末 | 优化完善版 | 性能、稳定性达标 |
| **M17** | 第17月末 | 正式发布 | 全量用户切换 |
| **M18** | 第18月末 | 项目完成 | 旧版下线，代码归档 |

---

## 💰 成本估算

### 人力成本

| 角色 | 人数 | 月薪（元） | 月数 | 小计（元） |
|------|-----|-----------|------|-----------|
| **Qt架构师** | 1 | 40,000 | 18 | 720,000 |
| **Qt高级开发** | 3 | 30,000 | 18 | 1,620,000 |
| **Qt中级开发** | 2 | 22,000 | 18 | 792,000 |
| **UI/UX设计师** | 1 | 20,000 | 12 | 240,000 |
| **测试工程师** | 2 | 18,000 | 15 | 540,000 |
| **技术文档** | 1 | 16,000 | 6 | 96,000 |
| **合计** | **10** | - | - | **4,008,000** |

### 其他成本

| 项目 | 金额（元） |
|------|-----------|
| 开发工具（Qt Creator/CLion/VS） | 30,000 |
| CI/CD（GitHub Actions/Jenkins） | 50,000 |
| 云服务（测试服务器） | 60,000 |
| 培训费用（Qt课程、书籍） | 80,000 |
| 外部咨询（Qt专家） | 150,000 |
| 差旅费用 | 40,000 |
| **合计** | **410,000** |

### 总成本

**总计**：约 **440万元人民币**

---

## ⚠️ 风险评估

| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|---------|
| **团队C++/Qt经验不足** | 极高 | 高 | 提前3-6月培训，招聘Qt专家，外部咨询支持 |
| **重写工作量超预期** | 高 | 中 | 分阶段交付，MVP优先，功能裁剪备选 |
| **性能不达标** | 高 | 中 | 提前性能测试，持续优化，设置性能基准 |
| **跨平台兼容性问题** | 中 | 中 | 早期三大平台并行测试，使用Qt标准API |
| **用户接受度低** | 中 | 低 | 内测收集反馈，UI/UX持续打磨，灰度发布 |
| **业务中断** | 极高 | 低 | 新旧版本并行运行6个月，充分测试 |

---

## ✅ 总结

### 核心优势

1. **跨平台**：一次开发，Windows/macOS/Linux全支持
2. **高性能**：Qt原生渲染，GPU加速，60fps流畅体验
3. **现代化**：Qt6 + C++17 + QML，技术栈先进
4. **开源免费**：无商业许可成本，LGPL授权
5. **社区活跃**：Qt生态成熟，问题解决快

### 挑战与应对

1. **挑战**：团队技能转型（C# → C++）
   - **应对**：提前培训、招聘Qt专家、分阶段过渡

2. **挑战**：重写工作量大（18个月）
   - **应对**：MVP优先、分阶段交付、功能裁剪备选

3. **挑战**：业务连续性风险
   - **应对**：新旧版本并行、充分测试、灰度发布

### 建议决策

✅ **推荐采用Qt完全重构方案，前提是：**
1. 公司有跨平台的明确需求（macOS/Linux市场）
2. 愿意投入18个月 + 440万元成本
3. 团队愿意学习C++/Qt技术栈
4. 接受新旧版本并行6-12个月的过渡期

❌ **不推荐的场景**：
1. 仅Windows用户，无跨平台需求
2. 预算或时间有限（<12个月 / <300万）
3. 团队C#背景深厚，不愿转型
4. 业务快速变化，无法承受长周期重写

---

**文档版本**：v2.0  
**编写日期**：2026年5月13日  
**预计实施周期**：18个月  
**预计成本**：约440万元人民币
