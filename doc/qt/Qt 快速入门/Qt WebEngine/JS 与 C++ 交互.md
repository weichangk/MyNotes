# JS 与 C++ 交互

Qt WebEngine 中 JavaScript 与 C++ 的双向通信是混合应用开发的核心。Qt 提供了 **QWebChannel** 机制实现类型安全的双向桥接：C++ 对象暴露给 JS 调用，JS 事件反馈给 C++ 处理。

---

## 1. 交互方式总览

| 方式 | 方向 | 适用场景 | 复杂度 |
|---|---|---|---|
| `runJavaScript()` | C++ → JS | 单次执行 JS 代码，获取返回值 | 低 |
| `QWebChannel` | C++ ↔ JS 双向 | 完整的对象暴露、信号槽绑定 | 中 |
| `QWebEngineScript` 注入 | C++ → JS | 页面加载时自动注入脚本 | 低 |
| 自定义 URL Scheme | JS → C++ | 拦截特定请求做 C++ 处理 | 中 |

```
             QWebChannel（双向桥梁）
    ┌──────────────────────────────────┐
    │  C++ 端                JS 端     │
    │  ┌─────────┐     ┌──────────┐   │
    │  │QObject  │◄───►│JS Proxy  │   │
    │  │属性/方法 │     │对象       │   │
    │  │信号/槽  │     │回调       │   │
    │  └─────────┘     └──────────┘   │
    │       │  WebSocket传输层  │      │
    └──────────────────────────────────┘
```

---

## 2. runJavaScript()：C++ 调用 JS

最直接的方式，适合一次性执行和简单数据获取。

### 2.1 基本用法

```cpp
QWebEnginePage *page = view->page();

// 无返回值：直接执行
page->runJavaScript("document.body.style.backgroundColor = '#f0f0f0';");

// 有返回值：通过回调获取（异步）
page->runJavaScript("document.title", [](const QVariant &result) {
    qDebug() << "Title:" << result.toString();
});

// 执行复杂逻辑并返回结果
page->runJavaScript(R"(
    (function() {
        var items = document.querySelectorAll('.item');
        var data = [];
        items.forEach(function(el) {
            data.push({
                text: el.textContent,
                href: el.getAttribute('href') || ''
            });
        });
        return JSON.stringify(data);
    })();
)", [](const QVariant &result) {
    QJsonDocument doc = QJsonDocument::fromJson(result.toString().toUtf8());
    QJsonArray arr = doc.array();
    for (const auto &item : arr) {
        qDebug() << item.toObject()["text"].toString();
    }
});
```

### 2.2 返回值类型映射

| JS 类型 | QVariant 类型 | 取值方法 |
|---|---|---|
| `string` | `QString` | `result.toString()` |
| `number`（整数） | `int` / `double` | `result.toInt()` / `result.toDouble()` |
| `number`（浮点） | `double` | `result.toDouble()` |
| `boolean` | `bool` | `result.toBool()` |
| `null` / `undefined` | `Invalid` | `result.isNull()` / `!result.isValid()` |
| `Array` | `QVariantList` | `result.toList()` |
| `Object` | `QVariantMap` | `result.toMap()` |

```cpp
// 返回数组
page->runJavaScript("[1, 2, 3]", [](const QVariant &result) {
    QVariantList list = result.toList();
    // list[0].toInt() == 1
});

// 返回对象
page->runJavaScript("({name:'Qt', version:6})", [](const QVariant &result) {
    QVariantMap map = result.toMap();
    // map["name"].toString() == "Qt"
    // map["version"].toInt() == 6
});
```

### 2.3 执行上下文与 World

```cpp
// 默认在 MainWorld（与页面 JS 共享环境）
page->runJavaScript("window.myVar = 42;");

// 在 ApplicationWorld（隔离环境，页面 JS 看不到）
page->runJavaScript("window.myVar = 42;", QWebEngineScript::ApplicationWorld);

// 在 ApplicationWorld 中读取页面 DOM 仍然可以
page->runJavaScript("document.title", QWebEngineScript::ApplicationWorld,
                    [](const QVariant &v) { qDebug() << v; });
```

> **注意**：`runJavaScript()` 必须在页面加载完成后调用，否则结果不确定。配合 `loadFinished` 信号使用。

### 2.4 常见陷阱

```cpp
// ❌ 同步思维：runJavaScript 是异步的，不能立即获取结果
QString title;
page->runJavaScript("document.title", [&title](const QVariant &v) {
    title = v.toString();  // 回调在将来某个时刻执行
});
qDebug() << title;  // 这里 title 还是空的！

// ✅ 正确：后续逻辑放在回调内
page->runJavaScript("document.title", [this](const QVariant &v) {
    QString title = v.toString();
    setWindowTitle(title);  // 在回调中使用结果
});
```

---

## 3. QWebChannel：双向通信核心

### 3.1 原理

`QWebChannel` 通过 WebSocket 或内部 IPC 传输协议，将 C++ 的 `QObject` 暴露给 JS。JS 端通过 `qwebchannel.js` 库建立连接后，可以：

- 读写 C++ 对象的 **Q_PROPERTY**
- 调用 C++ 对象的 **Q_INVOKABLE 方法** 和 **public slots**
- 监听 C++ 对象的 **signals**（在 JS 中用 `.connect()`）

```
C++ 端注册对象 → QWebChannel 序列化元信息 → JS 端生成代理对象 → 双向调用
```

### 3.2 环境准备

**CMake 依赖：**

```cmake
find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets WebChannel)
target_link_libraries(myapp PRIVATE Qt6::WebEngineWidgets Qt6::WebChannel)
```

**JS 端库文件：** Qt 提供 `qwebchannel.js`，位于：
- Qt 安装目录：`<Qt>/qml/QtWebChannel/qwebchannel.js`
- 或从资源中加载：`qrc:///qtwebchannel/qwebchannel.js`

---

## 4. QWebChannel 实战：C++ 暴露对象给 JS

### 4.1 定义 C++ 桥接对象

```cpp
// bridge.h
#pragma once
#include <QObject>
#include <QVariantMap>

class Bridge : public QObject {
    Q_OBJECT

    // 属性：JS 可读写，变化时 JS 收到通知
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit Bridge(QObject *parent = nullptr) : QObject(parent) {}

    QString userName() const { return m_userName; }
    void setUserName(const QString &name) {
        if (m_userName != name) {
            m_userName = name;
            emit userNameChanged(m_userName);
        }
    }
    int count() const { return m_count; }

    // Q_INVOKABLE 方法：JS 可调用
    Q_INVOKABLE QString greet(const QString &name) {
        return QString("Hello, %1! Welcome to Qt.").arg(name);
    }

    Q_INVOKABLE void increment() {
        m_count++;
        emit countChanged(m_count);
    }

    // 接收复杂参数（JS 对象 → QVariantMap）
    Q_INVOKABLE void processData(const QVariantMap &data) {
        QString name = data["name"].toString();
        int age = data["age"].toInt();
        QVariantList tags = data["tags"].toList();
        qDebug() << "Name:" << name << "Age:" << age << "Tags:" << tags;
    }

    // 返回复杂数据（QVariantMap → JS 对象）
    Q_INVOKABLE QVariantMap getUserInfo() {
        return {
            {"name", m_userName},
            {"count", m_count},
            {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    }

    // 异步操作示例：通过信号返回结果
    Q_INVOKABLE void fetchDataAsync(const QString &query) {
        // 模拟异步操作（实际可能是网络请求、数据库查询等）
        QTimer::singleShot(1000, this, [this, query]() {
            QVariantMap result = {
                {"query", query},
                {"results", QVariantList{"item1", "item2", "item3"}}
            };
            emit asyncDataReady(result);
        });
    }

public slots:
    // public slots 也暴露给 JS
    void showMessage(const QString &msg) {
        qDebug() << "[JS → C++]" << msg;
    }

signals:
    // 信号：JS 可用 .connect() 监听
    void userNameChanged(const QString &name);
    void countChanged(int newCount);
    void asyncDataReady(const QVariantMap &data);
    void notifyJS(const QString &message);  // C++ 主动向 JS 推送消息
};
```

### 4.2 C++ 端注册

```cpp
#include <QWebChannel>
#include <QWebEngineView>
#include "bridge.h"

// 在 MainWindow 构造函数中
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_view = new QWebEngineView(this);
    setCentralWidget(m_view);

    // 1. 创建桥接对象
    m_bridge = new Bridge(this);

    // 2. 创建 WebChannel 并注册对象
    QWebChannel *channel = new QWebChannel(this);
    channel->registerObject("bridge", m_bridge);  // "bridge" 是 JS 中的访问名

    // 3. 绑定到页面
    m_view->page()->setWebChannel(channel);

    // 4. 加载包含 JS 交互代码的页面
    m_view->load(QUrl("qrc:/html/index.html"));

    // C++ 端主动发送信号给 JS
    connect(someButton, &QPushButton::clicked, this, [this]() {
        emit m_bridge->notifyJS("Button clicked in C++!");
    });
}
```

### 4.3 JS 端连接

```html
<!-- index.html -->
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <!-- 加载 Qt 提供的 WebChannel JS 库 -->
    <script src="qrc:///qtwebchannel/qwebchannel.js"></script>
</head>
<body>
    <h1 id="title">Qt WebChannel Demo</h1>
    <input id="nameInput" placeholder="Enter your name">
    <button id="greetBtn">Greet</button>
    <button id="incrBtn">Increment</button>
    <p>Count: <span id="count">0</span></p>
    <div id="output"></div>

    <script>
    // 建立 WebChannel 连接
    new QWebChannel(qt.webChannelTransport, function(channel) {
        // 获取 C++ 注册的 bridge 对象
        var bridge = channel.objects.bridge;

        // ========== 调用 C++ 方法 ==========

        // 调用带返回值的方法
        document.getElementById('greetBtn').addEventListener('click', function() {
            var name = document.getElementById('nameInput').value;
            bridge.greet(name, function(result) {
                // 注意：带返回值的调用，回调是最后一个参数
                document.getElementById('output').textContent = result;
            });
        });

        // 调用无返回值的方法
        document.getElementById('incrBtn').addEventListener('click', function() {
            bridge.increment();
        });

        // 传递复杂对象
        bridge.processData({
            name: "Alice",
            age: 30,
            tags: ["developer", "designer"]
        });

        // 获取复杂返回值
        bridge.getUserInfo(function(info) {
            console.log("User info:", JSON.stringify(info));
        });

        // ========== 读写 C++ 属性 ==========

        // 读取属性
        console.log("Current userName:", bridge.userName);

        // 写入属性（触发 C++ 端 setUserName + userNameChanged 信号）
        bridge.userName = "WebUser";

        // ========== 监听 C++ 信号 ==========

        // 监听 countChanged 信号
        bridge.countChanged.connect(function(newCount) {
            document.getElementById('count').textContent = newCount;
        });

        // 监听 C++ 推送的消息
        bridge.notifyJS.connect(function(message) {
            alert("Message from C++: " + message);
        });

        // 监听异步结果
        bridge.asyncDataReady.connect(function(data) {
            console.log("Async result:", JSON.stringify(data));
        });

        // 触发异步操作
        bridge.fetchDataAsync("test query");

        // 调用 public slot
        bridge.showMessage("Hello from JavaScript!");
    });
    </script>
</body>
</html>
```

### 4.4 参数与返回值类型映射

| C++ 类型 | JS 类型 | 说明 |
|---|---|---|
| `QString` | `string` | 字符串双向转换 |
| `int` / `double` | `number` | 数值双向转换 |
| `bool` | `boolean` | 布尔值 |
| `QVariantList` | `Array` | 数组 |
| `QVariantMap` | `Object` | 对象（键值对） |
| `QJsonObject` | `Object` | JSON 对象（Qt 6 改进支持） |
| `QJsonArray` | `Array` | JSON 数组 |
| `QDateTime` | `string`（ISO 格式） | 日期时间序列化为字符串 |
| 注册过的 `QObject*` | proxy object | 嵌套对象引用 |

---

## 5. 高级模式：注册多个对象

```cpp
QWebChannel *channel = new QWebChannel(this);

// 注册多个不同职责的对象
channel->registerObject("auth",     m_authManager);
channel->registerObject("settings", m_settingsManager);
channel->registerObject("file",     m_fileHandler);

view->page()->setWebChannel(channel);
```

```javascript
new QWebChannel(qt.webChannelTransport, function(channel) {
    var auth     = channel.objects.auth;
    var settings = channel.objects.settings;
    var file     = channel.objects.file;

    auth.login("user", "pass", function(token) {
        console.log("Login token:", token);
    });

    settings.theme = "dark";

    file.readFile("/data/config.json", function(content) {
        var config = JSON.parse(content);
        // ...
    });
});
```

---

## 6. 信号槽的 JS 端操作

### 6.1 连接与断开

```javascript
new QWebChannel(qt.webChannelTransport, function(channel) {
    var bridge = channel.objects.bridge;

    // 连接信号
    function onCountChanged(val) {
        console.log("Count changed to:", val);
    }
    bridge.countChanged.connect(onCountChanged);

    // 断开信号
    bridge.countChanged.disconnect(onCountChanged);

    // 多个 handler 可同时连接
    bridge.notifyJS.connect(function(msg) { console.log("Handler 1:", msg); });
    bridge.notifyJS.connect(function(msg) { console.log("Handler 2:", msg); });
});
```

### 6.2 从 JS 触发 C++ 信号

通过调用 C++ 方法间接触发信号（JS 不能直接 emit C++ 信号）：

```cpp
// C++ 端
class Bridge : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE void triggerEvent(const QString &eventName, const QVariantMap &data) {
        if (eventName == "save") {
            emit saveRequested(data);
        } else if (eventName == "cancel") {
            emit cancelRequested();
        }
    }

signals:
    void saveRequested(const QVariantMap &data);
    void cancelRequested();
};
```

```javascript
// JS 端
bridge.triggerEvent("save", { filename: "doc.txt", content: "Hello" });
```

---

## 7. 使用 QWebEngineScript 注入 JS

在页面加载前注入 JS 代码，适合需要早期拦截或通用脚本。

### 7.1 注入 WebChannel 初始化代码

```cpp
// 自动注入 WebChannel 连接代码，页面无需手动写 <script>
void MainWindow::injectWebChannelInit() {
    // 先注入 qwebchannel.js 库
    QFile channelFile(":/qtwebchannel/qwebchannel.js");
    channelFile.open(QFile::ReadOnly);
    QWebEngineScript channelScript;
    channelScript.setName("qwebchannel.js");
    channelScript.setSourceCode(channelFile.readAll());
    channelScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
    channelScript.setWorldId(QWebEngineScript::MainWorld);
    view->page()->scripts().insert(channelScript);

    // 再注入初始化代码
    QWebEngineScript initScript;
    initScript.setName("webchannel-init");
    initScript.setSourceCode(R"(
        document.addEventListener('DOMContentLoaded', function() {
            new QWebChannel(qt.webChannelTransport, function(channel) {
                // 将 C++ 对象挂载到全局，方便页面 JS 使用
                window.bridge = channel.objects.bridge;
                // 派发自定义事件通知页面 channel 已就绪
                document.dispatchEvent(new Event('bridgeReady'));
            });
        });
    )");
    initScript.setInjectionPoint(QWebEngineScript::DocumentReady);
    initScript.setWorldId(QWebEngineScript::MainWorld);
    view->page()->scripts().insert(initScript);
}
```

页面中使用：

```html
<script>
// 等待 bridge 就绪后使用
document.addEventListener('bridgeReady', function() {
    window.bridge.greet("World", function(result) {
        document.title = result;
    });
});
</script>
```

### 7.2 在 ApplicationWorld 中注入（安全隔离）

```cpp
// 在隔离 world 中注入脚本，不与页面 JS 冲突
QWebEngineScript script;
script.setSourceCode(R"(
    // 这里的变量和函数页面 JS 看不到
    function collectPageInfo() {
        return {
            title: document.title,
            url: window.location.href,
            links: document.querySelectorAll('a').length
        };
    }
)");
script.setInjectionPoint(QWebEngineScript::DocumentReady);
script.setWorldId(QWebEngineScript::ApplicationWorld);
view->page()->scripts().insert(script);

// C++ 中调用隔离 world 的函数
view->page()->runJavaScript(
    "JSON.stringify(collectPageInfo())",
    QWebEngineScript::ApplicationWorld,
    [](const QVariant &result) {
        qDebug() << result.toString();
    }
);
```

---

## 8. 外部 WebSocket 方式（非内嵌页面通信）

当页面不是通过 `QWebEngineView` 加载（如外部浏览器访问、Electron 混合等场景），可以用 `QWebSocketServer` + `QWebChannel` 实现通信。

### 8.1 C++ 端：WebSocket Server

```cpp
#include <QWebChannel>
#include <QWebSocketServer>
#include <QWebSocket>

// WebSocket 传输适配器
#include "websockettransport.h"  // 需要自行实现或从 Qt 示例复制

class WebSocketTransport : public QWebChannelAbstractTransport {
    Q_OBJECT
public:
    explicit WebSocketTransport(QWebSocket *socket, QObject *parent = nullptr)
        : QWebChannelAbstractTransport(parent), m_socket(socket) {
        connect(m_socket, &QWebSocket::textMessageReceived,
                this, &WebSocketTransport::onTextMessage);
        connect(m_socket, &QWebSocket::disconnected,
                m_socket, &QWebSocket::deleteLater);
    }

    void sendMessage(const QJsonObject &message) override {
        m_socket->sendTextMessage(
            QString::fromUtf8(QJsonDocument(message).toJson(QJsonDocument::Compact))
        );
    }

private slots:
    void onTextMessage(const QString &message) {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        emit messageReceived(doc.object(), this);
    }

private:
    QWebSocket *m_socket;
};

// 服务端启动
void setupWebSocketChannel() {
    auto *server = new QWebSocketServer("BridgeServer",
                                         QWebSocketServer::NonSecureMode, this);
    if (!server->listen(QHostAddress::LocalHost, 12345)) {
        qWarning() << "WebSocket server failed to start";
        return;
    }

    auto *channel = new QWebChannel(this);
    channel->registerObject("bridge", m_bridge);

    connect(server, &QWebSocketServer::newConnection, this, [channel, server]() {
        QWebSocket *socket = server->nextPendingConnection();
        auto *transport = new WebSocketTransport(socket);
        channel->connectTo(transport);
    });
}
```

### 8.2 JS 端：WebSocket 客户端

```html
<script src="qwebchannel.js"></script>
<script>
var socket = new WebSocket("ws://localhost:12345");

socket.onopen = function() {
    // 用 WebSocket 作为传输层创建 channel
    new QWebChannel(socket, function(channel) {
        var bridge = channel.objects.bridge;
        bridge.greet("External Browser", function(result) {
            console.log(result);
        });
    });
};
</script>
```

---

## 9. 实战案例

### 9.1 文件选择器：C++ 原生对话框 + JS 回调

```cpp
// C++ 端
class FileDialogBridge : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE void openFileDialog(const QString &filter) {
        QString file = QFileDialog::getOpenFileName(
            nullptr, tr("Open File"), QDir::homePath(), filter
        );
        if (!file.isEmpty()) {
            // 读取文件内容
            QFile f(file);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                emit fileSelected(file, QString::fromUtf8(f.readAll()));
            }
        } else {
            emit fileCancelled();
        }
    }

    Q_INVOKABLE void saveFileDialog(const QString &content,
                                     const QString &defaultName) {
        QString file = QFileDialog::getSaveFileName(
            nullptr, tr("Save File"), QDir::homePath() + "/" + defaultName
        );
        if (!file.isEmpty()) {
            QFile f(file);
            if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                f.write(content.toUtf8());
                emit fileSaved(file);
            }
        }
    }

signals:
    void fileSelected(const QString &path, const QString &content);
    void fileCancelled();
    void fileSaved(const QString &path);
};
```

```javascript
// JS 端
bridge.fileSelected.connect(function(path, content) {
    document.getElementById('editor').value = content;
    document.getElementById('filename').textContent = path;
});

document.getElementById('openBtn').addEventListener('click', function() {
    bridge.openFileDialog("Text Files (*.txt);;All Files (*)");
});

document.getElementById('saveBtn').addEventListener('click', function() {
    var content = document.getElementById('editor').value;
    bridge.saveFileDialog(content, "document.txt");
});
```

### 9.2 系统通知桥接

```cpp
// C++ 端
class NotificationBridge : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE void showNotification(const QString &title, const QString &body) {
        auto *tray = qobject_cast<QSystemTrayIcon*>(
            qApp->property("trayIcon").value<QObject*>()
        );
        if (tray) {
            tray->showMessage(title, body, QSystemTrayIcon::Information, 3000);
        }
    }

    Q_INVOKABLE void setClipboard(const QString &text) {
        QGuiApplication::clipboard()->setText(text);
    }

    Q_INVOKABLE QString getClipboard() {
        return QGuiApplication::clipboard()->text();
    }
};
```

### 9.3 实时数据推送（C++ → JS 流式更新）

```cpp
// C++ 端：定时向 JS 推送数据
class DataProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap latestData READ latestData NOTIFY dataUpdated)
public:
    explicit DataProvider(QObject *parent = nullptr) : QObject(parent) {
        // 模拟实时数据源
        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this]() {
            m_data = {
                {"cpu",       QRandomGenerator::global()->bounded(100)},
                {"memory",    QRandomGenerator::global()->bounded(16384)},
                {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
            };
            emit dataUpdated(m_data);
        });
        timer->start(1000);  // 每秒推送
    }

    QVariantMap latestData() const { return m_data; }

signals:
    void dataUpdated(const QVariantMap &data);

private:
    QVariantMap m_data;
};
```

```javascript
// JS 端：实时更新 UI
bridge.dataUpdated.connect(function(data) {
    document.getElementById('cpu').textContent = data.cpu + '%';
    document.getElementById('mem').textContent = (data.memory / 1024).toFixed(1) + ' GB';
    document.getElementById('time').textContent = data.timestamp;

    // 更新图表等
    chart.addDataPoint(data.cpu);
});
```

---

## 10. Promise 封装（现代 JS 风格）

WebChannel 的回调式 API 可以用 Promise 封装，配合 `async/await` 使用。

### 10.1 通用 Promise 包装器

```javascript
// 将 WebChannel 方法包装为 Promise
function callBridge(method, ...args) {
    return new Promise(function(resolve, reject) {
        try {
            method.call(null, ...args, function(result) {
                resolve(result);
            });
        } catch (e) {
            reject(e);
        }
    });
}

// 将信号包装为 Promise（等待一次性信号）
function waitForSignal(signal) {
    return new Promise(function(resolve) {
        var handler = function() {
            signal.disconnect(handler);
            resolve(Array.from(arguments));
        };
        signal.connect(handler);
    });
}
```

### 10.2 使用 async/await

```javascript
new QWebChannel(qt.webChannelTransport, function(channel) {
    var bridge = channel.objects.bridge;

    // 包装具体方法
    function greet(name) {
        return callBridge(bridge.greet, name);
    }
    function getUserInfo() {
        return callBridge(bridge.getUserInfo);
    }

    // 使用 async/await
    async function main() {
        var greeting = await greet("World");
        console.log(greeting);  // "Hello, World! Welcome to Qt."

        var info = await getUserInfo();
        console.log("User:", info.name, "Count:", info.count);

        // 触发异步操作并等待信号
        bridge.fetchDataAsync("search query");
        var [data] = await waitForSignal(bridge.asyncDataReady);
        console.log("Async result:", JSON.stringify(data));
    }

    main().catch(console.error);
});
```

---

## 11. 安全考虑

### 11.1 输入验证

```cpp
// ❌ 危险：JS 传入的数据未校验直接使用
Q_INVOKABLE void runCommand(const QString &cmd) {
    QProcess::execute(cmd);  // 命令注入风险！
}

// ✅ 安全：严格校验输入
Q_INVOKABLE bool openFile(const QString &path) {
    // 白名单校验：只允许特定目录
    QFileInfo info(path);
    QString canonical = info.canonicalFilePath();
    if (!canonical.startsWith(m_allowedDir)) {
        qWarning() << "Access denied:" << path;
        return false;
    }
    // 校验文件类型
    static const QStringList allowedSuffixes = {"txt", "json", "csv"};
    if (!allowedSuffixes.contains(info.suffix().toLower())) {
        return false;
    }
    // ... 安全地读取文件
    return true;
}
```

### 11.2 最小暴露原则

```cpp
// ❌ 把核心业务对象直接暴露
channel->registerObject("app", qApp);  // 极度危险！

// ✅ 创建专用的桥接对象，只暴露必要的方法
class SafeBridge : public QObject {
    Q_OBJECT
public:
    // 只暴露经过安全审查的接口
    Q_INVOKABLE QString getAppVersion() { return "1.0.0"; }
    Q_INVOKABLE bool isFeatureEnabled(const QString &feature) {
        static const QSet<QString> features = {"darkMode", "autoSave"};
        return features.contains(feature);
    }
};
```

### 11.3 Origin 检查

```cpp
// 对于加载外部页面的场景，检查消息来源
void interceptRequest(QWebEngineUrlRequestInfo &info) override {
    // 只允许特定域名访问 WebChannel
    QUrl origin = info.firstPartyUrl();
    if (origin.host() != "trusted.example.com" && origin.scheme() != "qrc") {
        info.block(true);
    }
}
```

---

## 12. 调试技巧

### 12.1 Chrome DevTools 调试 JS

```cpp
// 启动时设置远程调试端口
qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "9222");

// 或通过 Chromium flags
qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--remote-debugging-port=9222");

// 然后在 Chrome 浏览器访问 chrome://inspect 即可调试
```

### 12.2 C++ 端日志

```cpp
// 监听页面 console.log 输出
class LogPage : public QWebEnginePage {
    Q_OBJECT
    using QWebEnginePage::QWebEnginePage;
protected:
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                  const QString &message,
                                  int lineNumber,
                                  const QString &sourceID) override {
        QString levelStr;
        switch (level) {
            case InfoMessageLevel:    levelStr = "INFO";    break;
            case WarningMessageLevel: levelStr = "WARN";    break;
            case ErrorMessageLevel:   levelStr = "ERROR";   break;
        }
        qDebug().noquote()
            << QString("[JS %1] %2 (%3:%4)")
                   .arg(levelStr, message, sourceID)
                   .arg(lineNumber);
    }
};

// 使用
auto *page = new LogPage(profile, view);
view->setPage(page);
```

### 12.3 WebChannel 传输调试

```javascript
// 在 JS 端打印所有 WebChannel 通信内容
new QWebChannel(qt.webChannelTransport, function(channel) {
    // 打印所有注册的对象名
    console.log("Registered objects:", Object.keys(channel.objects));

    var bridge = channel.objects.bridge;

    // 打印对象的所有可用方法和属性
    for (var key in bridge) {
        var type = typeof bridge[key];
        if (type === 'function') {
            console.log("  method:", key);
        } else if (bridge[key] && bridge[key].connect) {
            console.log("  signal:", key);
        } else {
            console.log("  property:", key, "=", bridge[key]);
        }
    }
});
```

---

## 13. Qt 5 与 Qt 6 差异

| 特性 | Qt 5 | Qt 6 |
|---|---|---|
| WebChannel JS 文件 | `qrc:///qtwebchannel/qwebchannel.js` | 同左 |
| 传输层 | `qt.webChannelTransport`（内置） | 同左 |
| `runJavaScript` world 参数 | 第 2 参数（重载） | 第 2 参数（重载） |
| 信号参数序列化 | `QVariant` 为主 | 改进 `QJsonValue` 支持 |
| 下载 API | `QWebEngineDownloadItem` | `QWebEngineDownloadRequest` |
| 初始化 | `QtWebEngine::initialize()` 必须调用 | Qt 6.4+ 自动初始化 |
| `QWebEnginePage::setWebChannel` | 可选 world 参数 | 同左 |

```cpp
// 指定 WebChannel 运行在特定 world（避免与页面 JS 冲突）
// Qt 5.7+ / Qt 6
page->setWebChannel(channel, QWebEngineScript::ApplicationWorld);
// 此时 JS 端脚本也必须注入到相同 world 才能访问 qt.webChannelTransport
```

---

## 14. 常见问题

**问题 1：JS 端 `qt` 未定义**

```javascript
// ❌ Uncaught ReferenceError: qt is not defined
new QWebChannel(qt.webChannelTransport, function(channel) { ... });

// 原因 1：忘记在 C++ 端调用 page->setWebChannel(channel)
// 原因 2：脚本运行在 ApplicationWorld，但 qt.webChannelTransport 在 MainWorld
// 原因 3：页面在 WebChannel 注册前就执行了 JS

// ✅ 修复：确保先设置 channel 再加载页面
QWebChannel *channel = new QWebChannel(this);
channel->registerObject("bridge", m_bridge);
page->setWebChannel(channel);   // 先注册
view->load(QUrl("qrc:/index.html"));  // 后加载
```

**问题 2：方法调用无响应**

```javascript
// ❌ 直接用返回值（WebChannel 调用是异步的）
var result = bridge.greet("World");
console.log(result);  // undefined!

// ✅ 用回调获取返回值
bridge.greet("World", function(result) {
    console.log(result);  // "Hello, World! Welcome to Qt."
});
```

**问题 3：信号参数中的复杂类型丢失**

```cpp
// ❌ 信号参数类型未注册，JS 端收到 undefined
signals:
    void dataReady(MyCustomStruct data);  // 自定义类型无法序列化

// ✅ 使用 QVariantMap / QJsonObject 等可序列化类型
signals:
    void dataReady(const QVariantMap &data);
```

**问题 4：页面刷新后 WebChannel 断开**

```javascript
// 页面刷新或导航后，需要重新建立 channel 连接
// 解决方案：在注入脚本中自动初始化（见第 7 节）
// 或监听 loadFinished 信号重新注入
```

```cpp
connect(view, &QWebEngineView::loadFinished, this, [this](bool ok) {
    if (ok) {
        // WebChannel 已绑定到 page，新页面加载后会自动生效
        // 但页面 JS 需要重新执行 new QWebChannel(...)
        // 使用 QWebEngineScript 注入可以自动完成
    }
});
```

**问题 5：C++ 对象在 JS 中看不到某些方法**

```cpp
// ❌ 普通 public 方法不会暴露给 JS
class Bridge : public QObject {
public:
    void myMethod();  // JS 看不到！
};

// ✅ 必须用 Q_INVOKABLE 或声明为 public slots
class Bridge : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE void myMethod();  // ✅ JS 可调用
public slots:
    void mySlot();                // ✅ JS 可调用
};
```

---

## 15. 最佳实践总结

| 实践 | 说明 |
|---|---|
| 桥接对象单一职责 | 按功能拆分多个桥接对象（文件、认证、设置），不要一个大对象 |
| 异步优先 | 所有跨界调用都是异步的，用信号/回调/Promise 处理 |
| 数据用 QVariantMap | 复杂数据传输统一用 `QVariantMap` / `QVariantList`，JSON 兼容 |
| 输入校验在 C++ | JS 端数据不可信，所有安全检查在 C++ 侧完成 |
| 最小暴露 | 只暴露必要的方法，不要把核心对象直接注册 |
| 错误处理 | C++ 方法返回错误码或通过信号报告错误，JS 端统一处理 |
| 避免阻塞 | C++ 方法中不要做耗时操作（会阻塞 Chromium IPC），用 QThread / QtConcurrent |
| 版本兼容 | 用 `QT_VERSION_CHECK` 条件编译处理 Qt 5/6 API 差异 |
| 调试阶段开 DevTools | 设置 `QTWEBENGINE_REMOTE_DEBUGGING` 便于 Chrome 调试 |
