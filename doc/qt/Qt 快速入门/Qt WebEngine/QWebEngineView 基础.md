# QWebEngineView 基础

Qt WebEngine 是 Qt 对 Chromium 浏览器引擎的封装，提供在桌面应用中嵌入完整 Web 内容的能力。`QWebEngineView` 是其核心 Widget，相当于一个可嵌入任意 Qt 界面的浏览器控件。

---

## 1. 模块概览与依赖

### 架构层次

```
┌──────────────────────────────────────────┐
│  QWebEngineView（Widget 层，直接嵌入 UI）   │
├──────────────────────────────────────────┤
│  QWebEnginePage（页面逻辑：导航、JS、权限） │
├──────────────────────────────────────────┤
│  QWebEngineProfile（配置：缓存、Cookie、UA）│
├──────────────────────────────────────────┤
│  Chromium 内核（多进程渲染引擎）            │
└──────────────────────────────────────────┘
```

| 类 | 职责 |
|---|---|
| `QWebEngineView` | Widget 容器，显示网页内容，处理用户交互 |
| `QWebEnginePage` | 页面逻辑：导航控制、JavaScript 执行、权限管理、信号通知 |
| `QWebEngineProfile` | 共享配置：缓存策略、Cookie、User-Agent、拦截器 |
| `QWebEngineSettings` | 渲染设置：字体、JavaScript 开关、插件等 |
| `QWebEngineHistory` | 前进/后退历史栈 |
| `QWebEngineScript` / `QWebEngineScriptCollection` | 用户脚本注入 |

### 环境配置

**CMake（Qt 6）：**

```cmake
find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets)
target_link_libraries(myapp PRIVATE Qt6::WebEngineWidgets)
```

**CMake（Qt 5）：**

```cmake
find_package(Qt5 REQUIRED COMPONENTS WebEngineWidgets)
target_link_libraries(myapp PRIVATE Qt5::WebEngineWidgets)
```

**qmake：**

```pro
QT += webenginewidgets
```

**重要初始化（必须在 QApplication 之前）：**

```cpp
#include <QtWebEngineWidgets/QtWebEngineWidgets>

int main(int argc, char *argv[]) {
    // Qt 5 必须在 QApplication 构造前调用
    // Qt 6.4+ 已废弃此调用（自动初始化）
    QtWebEngine::initialize();    // Qt 5

    QApplication app(argc, argv);
    // ...
}
```

---

## 2. 基本用法

### 2.1 最简示例：显示网页

```cpp
#include <QApplication>
#include <QWebEngineView>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWebEngineView view;
    view.setUrl(QUrl("https://www.qt.io"));
    view.resize(1280, 800);
    view.show();

    return app.exec();
}
```

### 2.2 加载不同来源的内容

```cpp
QWebEngineView *view = new QWebEngineView(this);

// 方式 1：加载远程 URL
view->load(QUrl("https://www.example.com"));

// 方式 2：加载本地文件
view->load(QUrl::fromLocalFile("C:/web/index.html"));

// 方式 3：加载 qrc 资源文件
view->load(QUrl("qrc:/html/index.html"));

// 方式 4：直接设置 HTML 字符串
view->setHtml(R"(
    <html>
    <body>
        <h1>Hello from Qt!</h1>
        <p>This is inline HTML content.</p>
    </body>
    </html>
)");

// 方式 5：setHtml + baseUrl（解决相对路径资源加载）
// baseUrl 用于解析 HTML 中的相对路径（CSS、图片等）
view->setHtml(htmlContent, QUrl("qrc:/html/"));

// 方式 6：setContent（指定 MIME 类型和编码）
QByteArray svgData = "<svg>...</svg>";
view->page()->setContent(svgData, "image/svg+xml");
```

### 2.3 嵌入 Qt 窗口布局

```cpp
#include <QMainWindow>
#include <QToolBar>
#include <QLineEdit>
#include <QWebEngineView>

class BrowserWindow : public QMainWindow {
    Q_OBJECT
public:
    BrowserWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        // URL 输入栏
        m_urlBar = new QLineEdit(this);
        connect(m_urlBar, &QLineEdit::returnPressed, this, &BrowserWindow::navigate);

        // 工具栏
        QToolBar *toolbar = addToolBar(tr("Navigation"));
        m_backAction    = toolbar->addAction("←", this, &BrowserWindow::goBack);
        m_forwardAction = toolbar->addAction("→", this, &BrowserWindow::goForward);
        m_reloadAction  = toolbar->addAction("⟳", this, &BrowserWindow::reload);
        toolbar->addWidget(m_urlBar);

        // 核心：WebEngine 视图
        m_view = new QWebEngineView(this);
        setCentralWidget(m_view);

        // 网页标题变化 → 窗口标题
        connect(m_view, &QWebEngineView::titleChanged,
                this, &QWidget::setWindowTitle);

        // URL 变化 → 更新地址栏
        connect(m_view, &QWebEngineView::urlChanged,
                this, [this](const QUrl &url) {
            m_urlBar->setText(url.toString());
        });

        // 加载进度
        connect(m_view, &QWebEngineView::loadProgress,
                this, [](int progress) {
            qDebug() << "Loading:" << progress << "%";
        });

        m_view->load(QUrl("https://www.qt.io"));
        resize(1280, 800);
    }

private slots:
    void navigate() {
        QString text = m_urlBar->text().trimmed();
        QUrl url = QUrl::fromUserInput(text);  // 自动补全 http://
        m_view->load(url);
    }
    void goBack()    { m_view->back(); }
    void goForward() { m_view->forward(); }
    void reload()    { m_view->reload(); }

private:
    QWebEngineView *m_view;
    QLineEdit *m_urlBar;
    QAction *m_backAction, *m_forwardAction, *m_reloadAction;
};
```

---

## 3. QWebEngineView 核心 API

### 3.1 导航方法

```cpp
// 加载 URL
view->load(QUrl("https://example.com"));
view->setUrl(QUrl("https://example.com"));   // 等价

// 加载 HTML
view->setHtml("<h1>Hello</h1>");
view->setHtml(html, QUrl("https://base.url/")); // 带 base URL

// 导航操作
view->back();                    // 后退
view->forward();                 // 前进
view->reload();                  // 刷新
view->stop();                    // 停止加载

// 历史导航
QWebEngineHistory *history = view->history();
history->canGoBack();            // 能否后退
history->canGoForward();         // 能否前进
history->backItems(10);          // 最多10条后退历史
history->forwardItems(10);
history->goToItem(history->itemAt(3));  // 跳转到指定历史项
```

### 3.2 核心信号

```cpp
// 加载状态
connect(view, &QWebEngineView::loadStarted, this, []() {
    qDebug() << "开始加载";
});
connect(view, &QWebEngineView::loadProgress, this, [](int percent) {
    // percent: 0~100，可用于进度条
    progressBar->setValue(percent);
});
connect(view, &QWebEngineView::loadFinished, this, [](bool ok) {
    if (ok) qDebug() << "加载成功";
    else    qDebug() << "加载失败";
});

// 内容变化
connect(view, &QWebEngineView::titleChanged, this, [](const QString &title) {
    setWindowTitle(title);
});
connect(view, &QWebEngineView::iconChanged, this, [](const QIcon &icon) {
    setWindowIcon(icon);  // 网站 favicon
});
connect(view, &QWebEngineView::urlChanged, this, [](const QUrl &url) {
    urlBar->setText(url.toDisplayString());
});

// 选中文本变化（Qt 5.12+）
connect(view, &QWebEngineView::selectionChanged, this, []() {
    qDebug() << view->selectedText();
});

// 渲染进程终止（崩溃恢复）
connect(view, &QWebEngineView::renderProcessTerminated,
        this, [](QWebEnginePage::RenderProcessTerminationStatus status, int code) {
    if (status == QWebEnginePage::CrashedTerminationStatus) {
        qWarning() << "Render process crashed, code:" << code;
        // 可以提示用户并重新加载
    }
});
```

### 3.3 常用属性

```cpp
// 获取页面信息
QString title = view->title();           // 页面标题
QUrl    url   = view->url();             // 当前 URL
QIcon   icon  = view->icon();            // favicon
QUrl    iconUrl = view->iconUrl();       // favicon URL
bool    loading = view->isLoading();     // 是否正在加载（Qt 6.2+）
QString text  = view->selectedText();    // 选中文本

// 缩放
view->setZoomFactor(1.5);  // 150% 缩放
double zoom = view->zoomFactor();

// 查找页面文本
view->findText("search keyword");                              // 查找
view->findText("keyword", QWebEnginePage::FindCaseSensitively); // 区分大小写
view->findText("");                                            // 取消高亮
// 带回调的查找（Qt 5.14+）
view->findText("keyword", {}, [](const QWebEngineFindTextResult &result) {
    qDebug() << "Found" << result.numberOfMatches()
             << "at index" << result.activeMatch();
});
```

---

## 4. QWebEnginePage 深入

`QWebEngineView` 持有一个 `QWebEnginePage`，可通过 `view->page()` 获取。更精细的控制需要操作 Page。

### 4.1 获取页面内容

```cpp
QWebEnginePage *page = view->page();

// 异步获取 HTML（页面加载完成后）
page->toHtml([](const QString &html) {
    qDebug() << "Page HTML length:" << html.length();
    // 注意：回调运行在主线程，可以直接操作 UI
});

// 异步获取纯文本
page->toPlainText([](const QString &text) {
    qDebug() << text.left(200);
});
```

### 4.2 JavaScript 执行

```cpp
// 执行 JS，无需返回值
page->runJavaScript("document.title = 'New Title';");

// 执行 JS，获取返回值
page->runJavaScript("document.title", [](const QVariant &result) {
    qDebug() << "Title:" << result.toString();
});

// 执行复杂 JS
page->runJavaScript(R"(
    (function() {
        var links = document.querySelectorAll('a');
        return links.length;
    })();
)", [](const QVariant &result) {
    qDebug() << "Page has" << result.toInt() << "links";
});

// 指定 world（隔离 JS 执行环境）
page->runJavaScript("myVar = 42;", QWebEngineScript::ApplicationWorld);
```

### 4.3 处理导航请求

自定义 Page 子类可以拦截导航（如阻止跳转、用外部浏览器打开等）：

```cpp
class CustomPage : public QWebEnginePage {
    Q_OBJECT
public:
    using QWebEnginePage::QWebEnginePage;

protected:
    // 在导航发生前调用，返回 true 允许，false 拦截
    bool acceptNavigationRequest(const QUrl &url,
                                 NavigationType type,
                                 bool isMainFrame) override {
        // 外部链接用系统浏览器打开
        if (url.host() != "myapp.local") {
            QDesktopServices::openUrl(url);
            return false;  // 拦截，不在 WebEngine 中加载
        }
        // 拦截下载链接
        if (url.path().endsWith(".zip")) {
            emit downloadRequested(url);
            return false;
        }
        return true;  // 允许导航
    }

    // 处理 JS alert() / confirm() / prompt()
    void javaScriptAlert(const QUrl &origin, const QString &msg) override {
        QMessageBox::information(view(), tr("Page Alert"), msg);
    }
    bool javaScriptConfirm(const QUrl &origin, const QString &msg) override {
        return QMessageBox::question(view(), tr("Confirm"), msg) == QMessageBox::Yes;
    }
    bool javaScriptPrompt(const QUrl &origin, const QString &msg,
                          const QString &defaultValue, QString *result) override {
        bool ok;
        *result = QInputDialog::getText(view(), tr("Input"), msg,
                                        QLineEdit::Normal, defaultValue, &ok);
        return ok;
    }

    // 控制 window.open() 创建新窗口
    QWebEnginePage *createWindow(WebWindowType type) override {
        // type: WebBrowserTab, WebBrowserBackgroundTab, WebBrowserWindow, WebDialog
        auto *newView = new QWebEngineView();
        newView->resize(800, 600);
        newView->show();
        return newView->page();
    }

signals:
    void downloadRequested(const QUrl &url);
};

// 使用
auto *page = new CustomPage(view);
view->setPage(page);
```

### 4.4 NavigationType 枚举

| 值 | 含义 |
|---|---|
| `NavigationTypeLinkClicked` | 用户点击链接 |
| `NavigationTypeTyped` | 用户在地址栏输入 |
| `NavigationTypeFormSubmitted` | 表单提交 |
| `NavigationTypeBackForward` | 前进/后退 |
| `NavigationTypeReload` | 刷新 |
| `NavigationTypeRedirect` | 重定向（Qt 5.14+） |
| `NavigationTypeOther` | 其他 |

---

## 5. QWebEngineProfile：配置与 Cookie

### 5.1 默认 Profile 与自定义 Profile

```cpp
// 全局默认 profile（所有未指定 profile 的 Page 共享）
QWebEngineProfile *defaultProfile = QWebEngineProfile::defaultProfile();

// 自定义持久化 profile（数据保存到磁盘，storageName 为子目录名）
QWebEngineProfile *myProfile = new QWebEngineProfile("myapp", this);

// 隐私模式：off-the-record（不写磁盘）
QWebEngineProfile *incognito = new QWebEngineProfile(this); // storageName 为空 → 隐私模式
qDebug() << incognito->isOffTheRecord();  // true

// 将 profile 绑定到 page
auto *page = new QWebEnginePage(myProfile, this);
view->setPage(page);
```

### 5.2 User-Agent 设置

```cpp
QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();

// 获取默认 UA
QString defaultUA = profile->httpUserAgent();
qDebug() << defaultUA;
// 类似 "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/xxx Chrome/xxx Qt/xxx"

// 设置自定义 UA
profile->setHttpUserAgent(
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
    "AppleWebKit/537.36 (KHTML, like Gecko) "
    "Chrome/120.0.0.0 Safari/537.36 MyApp/1.0"
);
```

### 5.3 缓存控制

```cpp
QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();

// 缓存路径
profile->setCachePath("C:/MyApp/cache");
profile->setPersistentStoragePath("C:/MyApp/storage");  // localStorage, IndexedDB 等

// 缓存策略
profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);   // 磁盘缓存（默认）
profile->setHttpCacheType(QWebEngineProfile::MemoryHttpCache); // 仅内存
profile->setHttpCacheType(QWebEngineProfile::NoCache);         // 无缓存

// 缓存大小限制（字节，0=自动）
profile->setHttpCacheMaximumSize(100 * 1024 * 1024);  // 100MB

// 清除缓存（Qt 5.15+）
profile->clearHttpCache();
```

### 5.4 Cookie 管理

```cpp
#include <QWebEngineCookieStore>

QWebEngineCookieStore *cookieStore = profile->cookieStore();

// 监听 Cookie 变化
connect(cookieStore, &QWebEngineCookieStore::cookieAdded,
        this, [](const QNetworkCookie &cookie) {
    qDebug() << "Cookie added:" << cookie.name() << "=" << cookie.value();
});
connect(cookieStore, &QWebEngineCookieStore::cookieRemoved,
        this, [](const QNetworkCookie &cookie) {
    qDebug() << "Cookie removed:" << cookie.name();
});

// 手动设置 Cookie
QNetworkCookie cookie("session_id", "abc123");
cookie.setDomain(".example.com");
cookie.setPath("/");
cookieStore->setCookie(cookie, QUrl("https://example.com"));

// 删除特定 Cookie
cookieStore->deleteCookie(cookie, QUrl("https://example.com"));

// 删除所有 Cookie
cookieStore->deleteAllCookies();

// 加载所有已存储的 Cookie（触发 cookieAdded 信号）
cookieStore->loadAllCookies();
```

---

## 6. QWebEngineSettings：页面渲染设置

```cpp
QWebEngineSettings *settings = view->settings();  // Page 级别
// 或全局：QWebEngineSettings::globalSettings()（Qt 5）
// 或 profile 级别：profile->settings()

// JavaScript
settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);         // JS 开关（默认开）
settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false); // 禁止 JS 弹窗
settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true); // 允许访问剪贴板

// 插件与媒体
settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);            // PDF 等插件
settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, true); // 视频需要用户交互才自动播放

// 本地内容
settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false); // 安全：禁止本地页面访问远程
settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);    // 允许本地页面访问本地文件
settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);              // localStorage

// 字体
settings->setFontFamily(QWebEngineSettings::StandardFont, "Microsoft YaHei");
settings->setFontFamily(QWebEngineSettings::FixedFont, "Consolas");
settings->setFontSize(QWebEngineSettings::DefaultFontSize, 16);
settings->setFontSize(QWebEngineSettings::MinimumFontSize, 10);

// 开发者工具
settings->setAttribute(QWebEngineSettings::ErrorPageEnabled, true);          // 显示错误页
settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);  // 导航时自动聚焦

// 滚动条（Qt 6.5+）
settings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);
```

---

## 7. 下载管理

### 7.1 处理下载请求

```cpp
#include <QWebEngineDownloadRequest>  // Qt 6
// Qt 5: #include <QWebEngineDownloadItem>

QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();

// Qt 6
connect(profile, &QWebEngineProfile::downloadRequested,
        this, [this](QWebEngineDownloadRequest *download) {

    // 获取下载信息
    qDebug() << "URL:" << download->url();
    qDebug() << "MIME:" << download->mimeType();
    qDebug() << "Size:" << download->totalBytes();
    qDebug() << "Suggested name:" << download->downloadFileName();

    // 设置保存路径
    QString savePath = QFileDialog::getSaveFileName(
        this, tr("Save File"),
        QDir::homePath() + "/" + download->downloadFileName()
    );
    if (savePath.isEmpty()) {
        download->cancel();  // 用户取消
        return;
    }
    download->setDownloadDirectory(QFileInfo(savePath).absolutePath());
    download->setDownloadFileName(QFileInfo(savePath).fileName());

    // 监听进度
    connect(download, &QWebEngineDownloadRequest::receivedBytesChanged,
            this, [download]() {
        qint64 received = download->receivedBytes();
        qint64 total    = download->totalBytes();
        if (total > 0) {
            qDebug() << "Progress:" << (received * 100 / total) << "%";
        }
    });

    // 监听完成
    connect(download, &QWebEngineDownloadRequest::isFinishedChanged,
            this, [download]() {
        if (download->state() == QWebEngineDownloadRequest::DownloadCompleted) {
            qDebug() << "Download completed!";
        }
    });

    download->accept();  // 开始下载
});
```

### 7.2 Qt 5 的下载处理（差异）

```cpp
// Qt 5 使用 QWebEngineDownloadItem
connect(profile, &QWebEngineProfile::downloadRequested,
        this, [](QWebEngineDownloadItem *item) {
    item->setPath(savePath);  // Qt 5 使用 setPath()
    connect(item, &QWebEngineDownloadItem::downloadProgress,
            [](qint64 received, qint64 total) { /* ... */ });
    connect(item, &QWebEngineDownloadItem::finished, []() { /* ... */ });
    item->accept();
});
```

---

## 8. 用户脚本注入（QWebEngineScript）

向页面注入自定义 JavaScript，类似浏览器扩展的 content script。

### 8.1 基本注入

```cpp
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

QWebEngineScript script;
script.setName("my-injected-script");
script.setSourceCode(R"(
    // 移除页面广告元素
    document.addEventListener('DOMContentLoaded', function() {
        var ads = document.querySelectorAll('.ad-banner, .popup-overlay');
        ads.forEach(function(el) { el.remove(); });
        console.log('Ads removed by Qt script injection');
    });
)");

// 注入时机
script.setInjectionPoint(QWebEngineScript::DocumentReady);
// DocumentCreation — DOM 创建前（最早，可拦截脚本）
// DocumentReady   — DOMContentLoaded 时（DOM 就绪，但图片等可能未加载）
// Deferred        — 页面完全加载后

// 作用范围（world）
script.setWorldId(QWebEngineScript::ApplicationWorld);
// MainWorld       — 与页面 JS 共享环境（可访问页面变量，但有冲突风险）
// ApplicationWorld — 隔离环境（安全，推荐）
// UserWorld (0-255) — 自定义隔离 world

script.setRunsOnSubFrames(false);  // 仅主 frame，不在 iframe 中运行

// 注入到当前页面
view->page()->scripts().insert(script);
```

### 8.2 从文件加载脚本

```cpp
QWebEngineScript script;
QFile file(":/scripts/inject.js");
if (file.open(QFile::ReadOnly)) {
    script.setSourceCode(file.readAll());
}
script.setName("inject.js");
script.setInjectionPoint(QWebEngineScript::DocumentCreation);
script.setWorldId(QWebEngineScript::ApplicationWorld);
view->page()->scripts().insert(script);
```

### 8.3 Profile 级脚本（所有页面生效）

```cpp
// 注入到 profile，该 profile 下所有 page 都会执行
QWebEngineProfile::defaultProfile()->scripts()->insert(script);
```

---

## 9. URL 拦截器（QWebEngineUrlRequestInterceptor）

拦截所有网络请求，可用于广告屏蔽、请求修改、流量监控。

```cpp
#include <QWebEngineUrlRequestInterceptor>

class RequestInterceptor : public QWebEngineUrlRequestInterceptor {
    Q_OBJECT
public:
    using QWebEngineUrlRequestInterceptor::QWebEngineUrlRequestInterceptor;

    void interceptRequest(QWebEngineUrlRequestInfo &info) override {
        QUrl url = info.requestUrl();
        QString urlStr = url.toString();

        // 1. 屏蔽广告域名
        static const QStringList blockedDomains = {
            "ads.example.com", "tracker.analytics.com", "popup.adserver.net"
        };
        for (const auto &domain : blockedDomains) {
            if (url.host().contains(domain)) {
                info.block(true);  // 阻止请求
                qDebug() << "Blocked:" << urlStr;
                return;
            }
        }

        // 2. 修改请求头
        info.setHttpHeader("X-Custom-Header", "MyApp/1.0");

        // 3. 重定向请求
        if (url.host() == "old.example.com") {
            QUrl newUrl = url;
            newUrl.setHost("new.example.com");
            info.redirect(newUrl);
            return;
        }

        // 4. 记录请求（调试用）
        qDebug() << info.requestMethod()
                 << info.resourceType()  // MainFrame, SubFrame, Image, Script, ...
                 << urlStr;
    }
};

// 安装到 profile
auto *interceptor = new RequestInterceptor(this);
QWebEngineProfile::defaultProfile()->setUrlRequestInterceptor(interceptor);
```

**QWebEngineUrlRequestInfo::ResourceType 常量：**

| 常量 | 含义 |
|---|---|
| `ResourceTypeMainFrame` | 主页面 |
| `ResourceTypeSubFrame` | iframe |
| `ResourceTypeStylesheet` | CSS |
| `ResourceTypeScript` | JavaScript |
| `ResourceTypeImage` | 图片 |
| `ResourceTypeXhr` | XMLHttpRequest |
| `ResourceTypeFetchRequest` | Fetch API 请求 |

---

## 10. 自定义 URL Scheme（QWebEngineUrlSchemeHandler）

注册自定义协议（如 `myapp://`），将 C++ 数据以 Web 资源形式提供给页面。

```cpp
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlScheme>
#include <QBuffer>

// 步骤 1：注册 scheme（必须在 QApplication 构造前）
void registerScheme() {
    QWebEngineUrlScheme scheme("myapp");
    scheme.setSyntax(QWebEngineUrlScheme::Syntax::HostAndPort);
    scheme.setFlags(QWebEngineUrlScheme::SecureScheme |
                    QWebEngineUrlScheme::CorsEnabled);
    QWebEngineUrlScheme::registerScheme(scheme);
}

// 步骤 2：实现 handler
class MySchemeHandler : public QWebEngineUrlSchemeHandler {
    Q_OBJECT
public:
    using QWebEngineUrlSchemeHandler::QWebEngineUrlSchemeHandler;

    void requestStarted(QWebEngineUrlRequestJob *job) override {
        QUrl url = job->requestUrl();
        QString path = url.path();

        if (path == "/api/data") {
            // 返回 JSON 数据
            QByteArray json = R"({"name": "Qt", "version": "6.7"})";
            QBuffer *buffer = new QBuffer(job);
            buffer->setData(json);
            job->reply("application/json", buffer);
        } else if (path == "/page") {
            // 返回 HTML 页面
            QByteArray html = "<html><body><h1>Custom Page</h1></body></html>";
            QBuffer *buffer = new QBuffer(job);
            buffer->setData(html);
            job->reply("text/html", buffer);
        } else {
            job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        }
    }
};

// 步骤 3：main() 中使用
int main(int argc, char *argv[]) {
    registerScheme();  // 必须在 QApplication 之前！
    QApplication app(argc, argv);

    auto *handler = new MySchemeHandler(&app);
    QWebEngineProfile::defaultProfile()->installUrlSchemeHandler("myapp", handler);

    QWebEngineView view;
    view.load(QUrl("myapp://localhost/page"));  // 加载自定义 scheme
    view.show();
    return app.exec();
}
```

页面中即可用 Fetch 调用自定义 API：

```javascript
fetch('myapp://localhost/api/data')
    .then(r => r.json())
    .then(data => console.log(data.name));  // "Qt"
```

---

## 11. 权限管理

### 11.1 处理权限请求

```cpp
// 处理网页请求的权限（摄像头、麦克风、定位、通知等）
connect(page, &QWebEnginePage::featurePermissionRequested,
        this, [this, page](const QUrl &origin,
                           QWebEnginePage::Feature feature) {
    QString featureName;
    switch (feature) {
        case QWebEnginePage::Geolocation:
            featureName = tr("Location"); break;
        case QWebEnginePage::MediaAudioCapture:
            featureName = tr("Microphone"); break;
        case QWebEnginePage::MediaVideoCapture:
            featureName = tr("Camera"); break;
        case QWebEnginePage::MediaAudioVideoCapture:
            featureName = tr("Camera & Microphone"); break;
        case QWebEnginePage::DesktopVideoCapture:
            featureName = tr("Screen Sharing"); break;
        case QWebEnginePage::Notifications:
            featureName = tr("Notifications"); break;
        default:
            featureName = tr("Unknown"); break;
    }

    auto result = QMessageBox::question(
        this,
        tr("Permission Request"),
        tr("%1 requests access to %2.\nAllow?").arg(origin.host(), featureName)
    );

    page->setFeaturePermission(
        origin, feature,
        result == QMessageBox::Yes
            ? QWebEnginePage::PermissionGrantedByUser
            : QWebEnginePage::PermissionDeniedByUser
    );
});
```

### 11.2 证书错误处理

```cpp
connect(page, &QWebEnginePage::certificateError,
        this, [this](const QWebEngineCertificateError &error) {
    qWarning() << "Certificate error:" << error.description()
               << "URL:" << error.url();

    // 仅在开发模式下忽略证书错误
#ifdef QT_DEBUG
    error.acceptCertificate();    // Qt 6
    // error.ignoreCertificateError();  // Qt 5
#else
    error.rejectCertificate();    // 生产环境严格检查
#endif
});
```

---

## 12. 页面截图与打印

### 12.1 截图

```cpp
// 截取可见区域
view->grab().save("screenshot.png");  // QWidget::grab()，但可能只截到空白

// 正确的异步截图方式（通过 Page）
page->runJavaScript(
    "document.documentElement.scrollHeight",
    [page](const QVariant &height) {
        // 获取页面完整高度后可调整视图大小再截图
        qDebug() << "Page height:" << height.toInt();
    }
);

// 使用 PrintToPdf 也可作为截图的替代方案
```

### 12.2 打印为 PDF

```cpp
// 保存网页为 PDF
page->printToPdf("output.pdf");

// 带回调
page->printToPdf([](const QByteArray &pdfData) {
    if (pdfData.isEmpty()) {
        qWarning() << "PDF generation failed";
        return;
    }
    QFile file("output.pdf");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(pdfData);
    }
});

// 带页面布局设置
QPageLayout layout(QPageSize(QPageSize::A4),
                   QPageLayout::Portrait,
                   QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
page->printToPdf("output.pdf", layout);
```

### 12.3 打印到打印机

```cpp
#include <QPrinter>
#include <QPrintDialog>

void BrowserWindow::printPage() {
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() != QDialog::Accepted) return;

    // Qt 6
    view->print(&printer);

    // Qt 5（异步）
    // view->page()->print(&printer, [](bool success) {
    //     qDebug() << "Print" << (success ? "ok" : "failed");
    // });
}
```

---

## 13. DevTools（开发者工具）

```cpp
// 创建第二个 view 作为 DevTools 容器
QWebEngineView *devToolsView = new QWebEngineView();
devToolsView->setWindowTitle("DevTools");
devToolsView->resize(1000, 600);

// 将被调试页面绑定到 DevTools
view->page()->setDevToolsPage(devToolsView->page());

// 显示 DevTools
devToolsView->show();

// 动态切换（关闭 DevTools）
// view->page()->setDevToolsPage(nullptr);
```

也可以嵌入到同一个窗口中（用 `QSplitter`）：

```cpp
QSplitter *splitter = new QSplitter(Qt::Vertical, this);
splitter->addWidget(view);
splitter->addWidget(devToolsView);
splitter->setSizes({600, 300});
setCentralWidget(splitter);

view->page()->setDevToolsPage(devToolsView->page());
```

---

## 14. 多进程架构与性能

### 14.1 Chromium 多进程模型

```
Qt 应用主进程
    ├── UI 线程（Qt 事件循环）
    ├── Chromium Browser 进程（协调管理）
    ├── Renderer 进程 1（页面渲染，沙箱隔离）
    ├── Renderer 进程 2（另一个页面 / iframe）
    └── GPU 进程（硬件加速合成）
```

每个 `QWebEnginePage` 默认运行在独立的渲染进程中，一个页面崩溃不影响其他页面。

### 14.2 内存优化

```cpp
// 1. 限制缓存大小
profile->setHttpCacheMaximumSize(50 * 1024 * 1024);

// 2. 页面不可见时降低资源消耗（Qt 6.2+）
page->setLifecycleState(QWebEnginePage::LifecycleState::Frozen);   // 冻结（暂停 JS/动画）
page->setLifecycleState(QWebEnginePage::LifecycleState::Discarded); // 丢弃（释放内存，保留历史）
page->setLifecycleState(QWebEnginePage::LifecycleState::Active);    // 恢复

// 3. 可见性提示（影响 requestAnimationFrame、Page Visibility API）
page->setVisible(false);  // 通知 Chromium 页面不可见

// 4. 共享渲染进程（减少内存，但降低隔离性）
// 命令行参数：--single-process 或 --process-per-site
```

### 14.3 命令行参数

通过 `QApplication` 传递给 Chromium 内核：

```cpp
// 在 main() 中，创建 QApplication 前设置
int main(int argc, char *argv[]) {
    // 方法 1：通过环境变量
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
            "--disable-gpu --enable-logging --log-level=0");

    // 方法 2：通过 argv（Chromium args 以 -- 开头）
    QApplication app(argc, argv);
    // ...
}
```

| 常用参数 | 说明 |
|---|---|
| `--disable-gpu` | 禁用 GPU 加速（排查渲染问题） |
| `--single-process` | 单进程模式（调试用，不推荐生产） |
| `--remote-debugging-port=9222` | 远程调试端口（Chrome DevTools 连接） |
| `--enable-logging --log-level=0` | 启用 Chromium 日志 |
| `--disable-web-security` | 禁用同源策略（仅调试！） |
| `--autoplay-policy=no-user-gesture-required` | 允许自动播放媒体 |

---

## 15. 常见问题与最佳实践

### 15.1 常见问题

**问题 1：页面白屏 / 不渲染**

```cpp
// 可能原因：GPU 加速与显卡驱动不兼容
// 解决：禁用 GPU 或切换渲染模式
qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu");

// 或在应用开头设置软件渲染
QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
```

**问题 2：setHtml() 加载后图片/CSS 无法显示**

```cpp
// ❌ 相对路径资源无法解析（无 base URL）
view->setHtml("<img src='logo.png'>");

// ✅ 提供 base URL
view->setHtml("<img src='logo.png'>", QUrl("qrc:/html/"));
// 或
view->setHtml("<img src='logo.png'>", QUrl::fromLocalFile("C:/web/"));
```

**问题 3：页面中文乱码**

```cpp
// setHtml 默认假定 UTF-8，确保 HTML 声明编码
view->setHtml(R"(
    <html>
    <head><meta charset="UTF-8"></head>
    <body>你好世界</body>
    </html>
)");

// 或使用 setContent 显式指定
view->page()->setContent(htmlBytes, "text/html; charset=utf-8");
```

**问题 4：runJavaScript() 无返回值**

```cpp
// ❌ 回调中 result 始终为空
page->runJavaScript("document.title");  // 没有回调，无法获取结果

// ✅ 必须传回调才能获取返回值
page->runJavaScript("document.title", [](const QVariant &v) {
    qDebug() << v.toString();  // 正确获取
});

// ⚠ 注意：页面未加载完就执行 JS 会失败
connect(view, &QWebEngineView::loadFinished, this, [page](bool ok) {
    if (ok) {
        page->runJavaScript("document.title", [](const QVariant &v) {
            qDebug() << v.toString();
        });
    }
});
```

**问题 5：打包部署后找不到 QtWebEngineProcess**

```
// 报错：Could not find QtWebEngineProcess
// 原因：WebEngine 依赖独立的渲染进程可执行文件和资源

// Windows 部署需包含：
// - QtWebEngineProcess.exe
// - resources/ 目录（icudtl.dat, qtwebengine_resources.pak 等）
// - translations/qtwebengine_locales/ 目录

// 使用 windeployqt 时加上 --webenginewidgets
windeployqt --webenginewidgets myapp.exe
```

### 15.2 最佳实践

| 实践 | 说明 |
|---|---|
| 初始化顺序 | Scheme 注册 → `QApplication` → Profile 配置 → 创建 View |
| 安全设置 | 生产环境禁止 `LocalContentCanAccessRemoteUrls`，不要禁用 `WebSecurity` |
| 内存管理 | 不可见标签页用 `setLifecycleState(Frozen)` 冻结 |
| 阻塞避免 | 所有涉及页面内容的操作均为异步（`toHtml`、`printToPdf`、`runJavaScript`） |
| Cookie 持久化 | 生产环境使用命名 Profile，确保登录状态跨会话保留 |
| 错误恢复 | 监听 `renderProcessTerminated` 并自动 `reload()` |
| 下载处理 | 始终连接 `downloadRequested`，否则用户下载无响应 |
| DevTools | 开发阶段绑定 DevTools 页面便于调试，发布版可通过快捷键切换 |
