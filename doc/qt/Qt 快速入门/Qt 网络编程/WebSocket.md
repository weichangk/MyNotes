# WebSocket

WebSocket 是基于 TCP 的**全双工**通信协议（RFC 6455），通过一次 HTTP 升级握手建立持久连接，之后客户端和服务端可以**随时主动**互发消息，无需轮询。Qt 通过 `QWebSocket`（客户端）和 `QWebSocketServer`（服务端）提供完整的 WebSocket 支持，API 风格与 `QTcpSocket` / `QTcpServer` 高度一致。

> **模块依赖：** 在 `.pro` 中添加 `QT += websockets`；CMake 中添加 `find_package(Qt5 COMPONENTS WebSockets)` + `target_link_libraries(... Qt5::WebSockets)`。
>
> 注意是 `websockets`（复数），不是 `network`。

---

## 1. WebSocket vs HTTP vs TCP

| 特性 | HTTP | WebSocket | 原始 TCP |
|------|------|-----------|----------|
| 连接模式 | 短连接（HTTP/1.0）/ 持久化复用（HTTP/1.1） | 持久长连接 | 持久长连接 |
| 通信方向 | 请求-响应（客户端主动） | 全双工（双方随时发送） | 全双工 |
| 消息边界 | 有（Header + Body） | 有（帧封装） | 无（字节流） |
| 粘包问题 | 无 | 无 | 有 |
| 协议开销 | 大（每次请求带完整 Header） | 小（握手后帧头仅 2–14 字节） | 最小 |
| 浏览器支持 | ✅ | ✅ | ❌ |
| 数据类型 | 文本 / 二进制 | 文本帧 / 二进制帧 | 纯字节流 |
| 典型场景 | REST API、网页 | 实时推送、聊天、游戏、股票行情 | 自定义协议 |

### 1.1 WebSocket 握手流程

```
客户端 ──HTTP GET──→ 服务端
         Upgrade: websocket
         Connection: Upgrade
         Sec-WebSocket-Key: dGhlIHNhbXBsZS...

服务端 ──HTTP 101──→ 客户端
         Switching Protocols
         Upgrade: websocket
         Sec-WebSocket-Accept: s3pPLM...

         ═══ WebSocket 连接已建立 ═══

客户端 ←─────── 全双工消息帧 ───────→ 服务端
```

> 握手完成后，底层 TCP 连接不再使用 HTTP 协议，而是切换为 WebSocket 帧协议。

---

## 2. 核心类总览

| 类 | 职责 |
|---|---|
| `QWebSocket` | WebSocket 客户端，连接服务器、收发消息 |
| `QWebSocketServer` | WebSocket 服务端，监听端口、接受连接 |
| `QWebSocketCorsAuthenticator` | CORS 跨域验证器（服务端使用） |
| `QMaskGenerator` | 掩码生成器（高级，自定义帧掩码） |

```
QWebSocket                          ← 客户端（类似 QTcpSocket）
├─ open(url)                        → 连接服务器
├─ sendTextMessage(text)            → 发送文本帧
├─ sendBinaryMessage(data)          → 发送二进制帧
├─ close()                          → 关闭连接
├─ textMessageReceived(QString)     → 收到文本消息
├─ binaryMessageReceived(QByteArray)→ 收到二进制消息
├─ connected()                      → 连接成功
├─ disconnected()                   → 连接断开
└─ error(QAbstractSocket::SocketError) → 错误

QWebSocketServer                     ← 服务端（类似 QTcpServer）
├─ listen(address, port)             → 开始监听
├─ newConnection()                   → 有新客户端连接
├─ nextPendingConnection()           → 获取新连接的 QWebSocket*
└─ close()                           → 停止监听
```

---

## 3. WebSocket 客户端

### 3.1 最简客户端

```cpp
#include <QWebSocket>

QWebSocket *socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

// 连接成功
connect(socket, &QWebSocket::connected, this, [socket]() {
    qDebug() << "Connected!";
    socket->sendTextMessage("Hello Server!");
});

// 收到文本消息
connect(socket, &QWebSocket::textMessageReceived,
        this, [](const QString &message) {
    qDebug() << "Received:" << message;
});

// 断开连接
connect(socket, &QWebSocket::disconnected, this, []() {
    qDebug() << "Disconnected";
});

// 发起连接
socket->open(QUrl("ws://127.0.0.1:9000"));
```

### 3.2 完整客户端封装

```cpp
// ws_client.h
#pragma once
#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QUrl>

class WsClient : public QObject
{
    Q_OBJECT
public:
    explicit WsClient(QObject *parent = nullptr);

    void connectToServer(const QUrl &url);
    void disconnect();
    void sendText(const QString &message);
    void sendBinary(const QByteArray &data);
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void textReceived(const QString &message);
    void binaryReceived(const QByteArray &data);
    void errorOccurred(const QString &errorString);

private:
    QWebSocket *m_socket = nullptr;
    QUrl m_url;
};

// ws_client.cpp
#include "ws_client.h"

WsClient::WsClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
{
    connect(m_socket, &QWebSocket::connected, this, [this]() {
        qDebug() << "WebSocket connected to" << m_url.toString();
        emit connected();
    });

    connect(m_socket, &QWebSocket::disconnected, this, [this]() {
        qDebug() << "WebSocket disconnected";
        emit disconnected();
    });

    connect(m_socket, &QWebSocket::textMessageReceived,
            this, &WsClient::textReceived);

    connect(m_socket, &QWebSocket::binaryMessageReceived,
            this, &WsClient::binaryReceived);

    // Qt 5 中 error 信号需要用函数指针消岐义
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, [this](QAbstractSocket::SocketError err) {
        Q_UNUSED(err)
        emit errorOccurred(m_socket->errorString());
    });
}

void WsClient::connectToServer(const QUrl &url)
{
    m_url = url;
    m_socket->open(url);
}

void WsClient::disconnect()
{
    m_socket->close();
}

void WsClient::sendText(const QString &message)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->sendTextMessage(message);
    }
}

void WsClient::sendBinary(const QByteArray &data)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->sendBinaryMessage(data);
    }
}

bool WsClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}
```

### 3.3 连接 WSS（加密 WebSocket）

```cpp
// wss:// 即 WebSocket over TLS，类似 HTTPS
socket->open(QUrl("wss://secure.example.com:443/ws"));

// 处理 SSL 错误
connect(socket, &QWebSocket::sslErrors,
        this, [socket](const QList<QSslError> &errors) {
    for (const QSslError &err : errors) {
        qWarning() << "SSL Error:" << err.errorString();
    }
    // ⚠️ 仅在开发调试时使用！
    // socket->ignoreSslErrors();
});
```

### 3.4 自定义 SSL 配置

```cpp
QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
sslConfig.setProtocol(QSsl::TlsV1_2OrLater);

// 添加自签名 CA 证书
QFile caFile(":/certs/my-ca.pem");
caFile.open(QIODevice::ReadOnly);
QSslCertificate caCert(&caFile, QSsl::Pem);
auto certs = sslConfig.caCertificates();
certs.append(caCert);
sslConfig.setCaCertificates(certs);

// 客户端证书（双向 TLS）
QFile certFile(":/certs/client.pem");
certFile.open(QIODevice::ReadOnly);
sslConfig.setLocalCertificate(QSslCertificate(&certFile, QSsl::Pem));

QFile keyFile(":/certs/client-key.pem");
keyFile.open(QIODevice::ReadOnly);
sslConfig.setPrivateKey(QSslKey(&keyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey));

socket->setSslConfiguration(sslConfig);
socket->open(QUrl("wss://internal.example.com/ws"));
```

### 3.5 设置自定义 Header / 子协议

```cpp
// 自定义 HTTP Header（在握手时发送）
QNetworkRequest request(QUrl("ws://api.example.com/ws"));
request.setRawHeader("Authorization", "Bearer eyJhbGci...");
request.setRawHeader("X-Client-Version", "1.0");

socket->open(request);

// 子协议（Sec-WebSocket-Protocol）
// 在构造时传入 origin，通过 open 的 request 设置子协议
request.setRawHeader("Sec-WebSocket-Protocol", "chat, json");
socket->open(request);
```

---

## 4. WebSocket 服务端

### 4.1 最简服务端

```cpp
#include <QWebSocketServer>
#include <QWebSocket>

QWebSocketServer *server = new QWebSocketServer(
    "MyServer",
    QWebSocketServer::NonSecureMode,  // ws:// 模式
    this);

if (!server->listen(QHostAddress::Any, 9000)) {
    qCritical() << "Listen failed:" << server->errorString();
    return;
}
qDebug() << "WebSocket server listening on port 9000";

// 新连接
connect(server, &QWebSocketServer::newConnection, this, [server]() {
    QWebSocket *client = server->nextPendingConnection();
    qDebug() << "New client:" << client->peerAddress().toString();

    // 收到文本消息 → 回显
    connect(client, &QWebSocket::textMessageReceived,
            [client](const QString &msg) {
        qDebug() << "Received:" << msg;
        client->sendTextMessage("Echo: " + msg);
    });

    // 客户端断开
    connect(client, &QWebSocket::disconnected,
            client, &QObject::deleteLater);
});
```

### 4.2 多客户端服务端

```cpp
// ws_server.h
#pragma once
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>

class WsServer : public QObject
{
    Q_OBJECT
public:
    explicit WsServer(QObject *parent = nullptr);

    bool start(quint16 port);
    void stop();
    void broadcast(const QString &message);
    void broadcastBinary(const QByteArray &data);
    int clientCount() const { return m_clients.size(); }

signals:
    void clientConnected(const QString &address);
    void clientDisconnected(const QString &address);
    void textMessageReceived(QWebSocket *client, const QString &message);
    void binaryMessageReceived(QWebSocket *client, const QByteArray &data);

private slots:
    void onNewConnection();

private:
    QWebSocketServer *m_server = nullptr;
    QList<QWebSocket *> m_clients;
};

// ws_server.cpp
#include "ws_server.h"

WsServer::WsServer(QObject *parent)
    : QObject(parent)
    , m_server(new QWebSocketServer("WsServer",
          QWebSocketServer::NonSecureMode, this))
{
    connect(m_server, &QWebSocketServer::newConnection,
            this, &WsServer::onNewConnection);
}

bool WsServer::start(quint16 port)
{
    if (!m_server->listen(QHostAddress::Any, port)) {
        qCritical() << "Listen failed:" << m_server->errorString();
        return false;
    }
    qDebug() << "WebSocket server listening on port" << port;
    return true;
}

void WsServer::stop()
{
    for (QWebSocket *client : qAsConst(m_clients)) {
        client->close();
    }
    m_server->close();
}

void WsServer::broadcast(const QString &message)
{
    for (QWebSocket *client : qAsConst(m_clients)) {
        client->sendTextMessage(message);
    }
}

void WsServer::broadcastBinary(const QByteArray &data)
{
    for (QWebSocket *client : qAsConst(m_clients)) {
        client->sendBinaryMessage(data);
    }
}

void WsServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QWebSocket *client = m_server->nextPendingConnection();
        m_clients.append(client);

        QString addr = QStringLiteral("%1:%2")
            .arg(client->peerAddress().toString())
            .arg(client->peerPort());

        qDebug() << "Client connected:" << addr;
        emit clientConnected(addr);

        connect(client, &QWebSocket::textMessageReceived,
                this, [this, client](const QString &msg) {
            emit textMessageReceived(client, msg);
        });

        connect(client, &QWebSocket::binaryMessageReceived,
                this, [this, client](const QByteArray &data) {
            emit binaryMessageReceived(client, data);
        });

        connect(client, &QWebSocket::disconnected,
                this, [this, client]() {
            QString addr = QStringLiteral("%1:%2")
                .arg(client->peerAddress().toString())
                .arg(client->peerPort());
            m_clients.removeOne(client);
            client->deleteLater();
            qDebug() << "Client disconnected:" << addr;
            emit clientDisconnected(addr);
        });
    }
}
```

### 4.3 WSS 安全服务端

```cpp
QSslConfiguration sslConfig;

// 加载服务端证书
QFile certFile(":/certs/server.pem");
certFile.open(QIODevice::ReadOnly);
sslConfig.setLocalCertificate(QSslCertificate(&certFile, QSsl::Pem));

// 加载私钥
QFile keyFile(":/certs/server-key.pem");
keyFile.open(QIODevice::ReadOnly);
sslConfig.setPrivateKey(QSslKey(&keyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey));

sslConfig.setProtocol(QSsl::TlsV1_2OrLater);

// 使用 SecureMode 创建服务器
QWebSocketServer *server = new QWebSocketServer(
    "SecureServer",
    QWebSocketServer::SecureMode,  // wss:// 模式
    this);

server->setSslConfiguration(sslConfig);

if (server->listen(QHostAddress::Any, 443)) {
    qDebug() << "WSS server listening on port 443";
}

// 处理 SSL 错误
connect(server, &QWebSocketServer::sslErrors,
        this, [](QWebSocket *socket, const QList<QSslError> &errors) {
    Q_UNUSED(socket)
    for (const QSslError &err : errors) {
        qWarning() << "Server SSL error:" << err.errorString();
    }
});
```

### 4.4 CORS 跨域处理

浏览器中的 WebSocket 连接会检查 Origin Header，服务端可通过 `originAuthenticationRequired` 信号控制：

```cpp
connect(server, &QWebSocketServer::originAuthenticationRequired,
        this, [](QWebSocketCorsAuthenticator *auth) {
    QString origin = auth->origin();
    qDebug() << "Origin:" << origin;

    // 白名单检查
    QStringList allowedOrigins = {
        "https://myapp.example.com",
        "https://admin.example.com",
        "http://localhost:3000"  // 开发环境
    };

    auth->setAllowed(allowedOrigins.contains(origin));
});
```

---

## 5. 文本帧 vs 二进制帧

WebSocket 协议区分两种数据帧类型：

| 帧类型 | Qt API（发送） | Qt 信号（接收） | 适用场景 |
|--------|---------------|----------------|---------|
| 文本帧 | `sendTextMessage(QString)` | `textMessageReceived(QString)` | JSON、命令、聊天消息 |
| 二进制帧 | `sendBinaryMessage(QByteArray)` | `binaryMessageReceived(QByteArray)` | 图片、音频、Protobuf、自定义协议 |

> **无粘包问题：** WebSocket 协议自带帧边界，一次 `sendTextMessage()` 对应接收端恰好一次 `textMessageReceived()`。无需像 TCP 那样自行处理消息边界。

### 5.1 发送 JSON（文本帧）

```cpp
QJsonObject msg;
msg["type"] = "chat";
msg["user"] = "Alice";
msg["content"] = "Hello everyone!";
msg["timestamp"] = QDateTime::currentMSecsSinceEpoch();

QString json = QJsonDocument(msg).toJson(QJsonDocument::Compact);
socket->sendTextMessage(json);
```

### 5.2 发送二进制数据

```cpp
// 发送图片
QFile file("photo.jpg");
file.open(QIODevice::ReadOnly);
socket->sendBinaryMessage(file.readAll());

// 发送自定义二进制协议
QByteArray packet;
QDataStream stream(&packet, QIODevice::WriteOnly);
stream.setByteOrder(QDataStream::BigEndian);
stream << quint8(0x01)      // 消息类型
       << quint32(12345)    // 用户 ID
       << qint64(QDateTime::currentMSecsSinceEpoch());
socket->sendBinaryMessage(packet);
```

---

## 6. Ping / Pong 心跳

WebSocket 协议内建 Ping/Pong 控制帧用于保活检测：

```
客户端 ──Ping──→ 服务端
客户端 ←──Pong── 服务端（自动回复）
```

### 6.1 客户端发送心跳

```cpp
class WsHeartbeat : public QObject
{
    Q_OBJECT
public:
    explicit WsHeartbeat(QWebSocket *socket, QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(socket)
        , m_pingTimer(new QTimer(this))
        , m_timeoutTimer(new QTimer(this))
    {
        // 每 30 秒发一次 Ping
        m_pingTimer->setInterval(30000);
        connect(m_pingTimer, &QTimer::timeout, this, [this]() {
            m_socket->ping();  // 发送 Ping 帧
            m_timeoutTimer->start();
        });

        // Pong 超时 10 秒 → 判定断连
        m_timeoutTimer->setInterval(10000);
        m_timeoutTimer->setSingleShot(true);
        connect(m_timeoutTimer, &QTimer::timeout, this, [this]() {
            qWarning() << "Pong timeout, closing connection";
            m_socket->close(QWebSocketProtocol::CloseCodeGoingAway,
                           "Pong timeout");
        });

        // 收到 Pong → 重置超时
        connect(m_socket, &QWebSocket::pong, this,
                [this](quint64 elapsedTime, const QByteArray &payload) {
            Q_UNUSED(payload)
            m_timeoutTimer->stop();
            qDebug() << "Pong received, latency:" << elapsedTime << "ms";
        });

        // 连接建立后启动心跳
        connect(m_socket, &QWebSocket::connected, m_pingTimer,
                QOverload<>::of(&QTimer::start));

        // 断开后停止心跳
        connect(m_socket, &QWebSocket::disconnected, this, [this]() {
            m_pingTimer->stop();
            m_timeoutTimer->stop();
        });
    }

private:
    QWebSocket *m_socket;
    QTimer *m_pingTimer;
    QTimer *m_timeoutTimer;
};
```

> **注意：** Qt 的 `QWebSocket` 收到 Ping 帧后会**自动回复 Pong**，无需手动处理。`pong()` 信号仅在**收到对方的 Pong 回复**时触发。

### 6.2 延迟测量

`pong(quint64 elapsedTime, ...)` 信号的 `elapsedTime` 参数即从发送 Ping 到收到 Pong 的毫秒数，可直接用于网络延迟监控：

```cpp
connect(socket, &QWebSocket::pong,
        this, [this](quint64 elapsedTime, const QByteArray &) {
    m_latencyHistory.append(elapsedTime);
    if (m_latencyHistory.size() > 100) {
        m_latencyHistory.removeFirst();
    }
    double avg = std::accumulate(m_latencyHistory.begin(),
                                 m_latencyHistory.end(), 0.0)
                 / m_latencyHistory.size();
    emit latencyUpdated(static_cast<int>(avg));
});
```

---

## 7. 自动重连

```cpp
class ReconnectableWsClient : public QObject
{
    Q_OBJECT
public:
    explicit ReconnectableWsClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QWebSocket(QString(),
              QWebSocketProtocol::VersionLatest, this))
        , m_reconnectTimer(new QTimer(this))
    {
        m_reconnectTimer->setSingleShot(true);

        connect(m_socket, &QWebSocket::connected, this, [this]() {
            m_reconnectAttempts = 0;
            qDebug() << "Connected to" << m_url.toString();
            emit connected();
        });

        connect(m_socket, &QWebSocket::disconnected, this, [this]() {
            qDebug() << "Disconnected, scheduling reconnect...";
            emit disconnected();
            scheduleReconnect();
        });

        connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(
                    &QWebSocket::error),
                this, [this](QAbstractSocket::SocketError err) {
            Q_UNUSED(err)
            qWarning() << "WebSocket error:" << m_socket->errorString();
        });

        connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
            ++m_reconnectAttempts;
            qDebug() << "Reconnecting... attempt" << m_reconnectAttempts;
            m_socket->open(m_url);
        });
    }

    void connectToServer(const QUrl &url)
    {
        m_url = url;
        m_socket->open(url);
    }

    void disconnect()
    {
        m_reconnectTimer->stop();
        m_reconnectAttempts = m_maxAttempts;  // 防止再次重连
        m_socket->close();
    }

    QWebSocket *socket() const { return m_socket; }

signals:
    void connected();
    void disconnected();

private:
    void scheduleReconnect()
    {
        if (m_reconnectAttempts >= m_maxAttempts) {
            qWarning() << "Max reconnect attempts reached";
            return;
        }
        // 指数退避：1s → 2s → 4s → 8s → ... 最大 30s
        int delay = qMin(1000 * (1 << m_reconnectAttempts), 30000);
        qDebug() << "Reconnecting in" << delay / 1000.0 << "s";
        m_reconnectTimer->setInterval(delay);
        m_reconnectTimer->start();
    }

    QWebSocket *m_socket;
    QTimer *m_reconnectTimer;
    QUrl m_url;
    int m_reconnectAttempts = 0;
    int m_maxAttempts = 15;
};
```

---

## 8. 关闭连接与状态码

### 8.1 正常关闭

```cpp
// 客户端主动关闭
socket->close(QWebSocketProtocol::CloseCodeNormal, "Bye");

// 监听关闭事件
connect(socket, &QWebSocket::disconnected, this, [socket]() {
    qDebug() << "Close code:" << socket->closeCode();
    qDebug() << "Close reason:" << socket->closeReason();
});
```

### 8.2 WebSocket 关闭状态码

| 状态码 | Qt 枚举 | 含义 |
|--------|---------|------|
| 1000 | `CloseCodeNormal` | 正常关闭 |
| 1001 | `CloseCodeGoingAway` | 端点离开（如页面导航、服务器关机） |
| 1002 | `CloseCodeProtocolError` | 协议错误 |
| 1003 | `CloseCodeDatatypeNotSupported` | 数据类型不支持 |
| 1005 | `CloseCodeMissingStatusCode` | 无状态码（预留，不出现在帧中） |
| 1006 | `CloseCodeAbnormalDisconnection` | 异常断开（无 Close 帧） |
| 1007 | `CloseCodeWrongDatatype` | 数据与帧类型不一致 |
| 1008 | `CloseCodePolicyViolated` | 策略违规 |
| 1009 | `CloseCodeTooMuchData` | 消息太大 |
| 1010 | `CloseCodeMissingExtension` | 缺少必需扩展 |
| 1011 | `CloseCodeBadOperation` | 服务端内部错误 |
| 1015 | `CloseCodeTlsHandshakeFailed` | TLS 握手失败 |

### 8.3 区分正常关闭与异常断开

```cpp
connect(socket, &QWebSocket::disconnected, this, [socket]() {
    auto code = socket->closeCode();

    if (code == QWebSocketProtocol::CloseCodeNormal ||
        code == QWebSocketProtocol::CloseCodeGoingAway) {
        qDebug() << "Clean disconnect:" << socket->closeReason();
    } else if (code == QWebSocketProtocol::CloseCodeAbnormalDisconnection) {
        qWarning() << "Abnormal disconnect (network issue?)";
        // 触发重连
    } else {
        qWarning() << "Closed with code:" << code
                    << "reason:" << socket->closeReason();
    }
});
```

---

## 9. 大消息处理

### 9.1 设置最大消息大小

```cpp
// 服务端限制单条消息最大 10MB（防止恶意客户端）
server->setMaxPendingConnections(128);

// QWebSocket 层面没有直接的大小限制 API，
// 需在接收逻辑中检查并截断：
connect(client, &QWebSocket::binaryMessageReceived,
        this, [client](const QByteArray &data) {
    if (data.size() > 10 * 1024 * 1024) {
        qWarning() << "Message too large:" << data.size();
        client->close(QWebSocketProtocol::CloseCodeTooMuchData,
                     "Message exceeds limit");
        return;
    }
    processData(data);
});
```

### 9.2 流式收发（大帧分片）

WebSocket 协议支持帧分片（fragmentation），Qt 提供对应的低级 API：

```cpp
// 发送端：分帧发送大数据
QByteArray hugeData = loadLargeFile();  // 假设 100MB
const int CHUNK = 64 * 1024;  // 每帧 64KB

for (int i = 0; i < hugeData.size(); i += CHUNK) {
    QByteArray chunk = hugeData.mid(i, CHUNK);
    bool isFirst = (i == 0);
    bool isLast = (i + CHUNK >= hugeData.size());

    if (isFirst && isLast) {
        // 单帧完整消息
        socket->sendBinaryMessage(chunk);
    } else if (isFirst) {
        // Qt 不直接暴露分帧 API，使用 sendBinaryMessage 会自动处理
        // 对于超大消息，建议在应用层自行分块
        socket->sendBinaryMessage(chunk);
    }
}

// 更实用的应用层分块方案
void sendChunked(QWebSocket *socket, const QByteArray &data)
{
    const int CHUNK_SIZE = 64 * 1024;
    int totalChunks = (data.size() + CHUNK_SIZE - 1) / CHUNK_SIZE;

    QJsonObject header;
    header["type"] = "chunked_start";
    header["totalSize"] = data.size();
    header["totalChunks"] = totalChunks;
    socket->sendTextMessage(QJsonDocument(header).toJson(QJsonDocument::Compact));

    for (int i = 0; i < totalChunks; ++i) {
        QByteArray chunk = data.mid(i * CHUNK_SIZE, CHUNK_SIZE);
        socket->sendBinaryMessage(chunk);
    }
}
```

---

## 10. 消息协议设计

### 10.1 JSON 消息协议

实际项目中通常定义统一的消息格式：

```cpp
// === 消息协议 ===
// {
//   "type": "chat" | "join" | "leave" | "ping" | "error" | ...,
//   "data": { ... },       // 业务数据
//   "id": "uuid-xxx",      // 消息 ID（用于请求-响应匹配）
//   "timestamp": 1234567890
// }

class WsProtocol
{
public:
    // 构建消息
    static QString createMessage(const QString &type,
                                 const QJsonObject &data = {},
                                 const QString &id = QString())
    {
        QJsonObject msg;
        msg["type"] = type;
        msg["data"] = data;
        msg["id"] = id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : id;
        msg["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        return QJsonDocument(msg).toJson(QJsonDocument::Compact);
    }

    // 解析消息
    struct Message {
        QString type;
        QJsonObject data;
        QString id;
        qint64 timestamp = 0;
        bool valid = false;
    };

    static Message parse(const QString &json)
    {
        Message msg;
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        if (!doc.isObject()) return msg;

        QJsonObject obj = doc.object();
        msg.type = obj["type"].toString();
        msg.data = obj["data"].toObject();
        msg.id = obj["id"].toString();
        msg.timestamp = obj["timestamp"].toVariant().toLongLong();
        msg.valid = !msg.type.isEmpty();
        return msg;
    }
};
```

### 10.2 请求-响应模式（基于消息 ID）

WebSocket 原生是推送模型，但可通过消息 ID 实现请求-响应模式：

```cpp
class WsRpcClient : public QObject
{
    Q_OBJECT
public:
    using Callback = std::function<void(const QJsonObject &data,
                                         const QString &error)>;

    explicit WsRpcClient(QWebSocket *socket, QObject *parent = nullptr)
        : QObject(parent), m_socket(socket)
    {
        connect(m_socket, &QWebSocket::textMessageReceived,
                this, [this](const QString &msg) {
            auto parsed = WsProtocol::parse(msg);
            if (!parsed.valid) return;

            // 检查是否是某个请求的响应
            if (m_pendingCallbacks.contains(parsed.id)) {
                auto cb = m_pendingCallbacks.take(parsed.id);
                m_pendingTimers.take(parsed.id)->deleteLater();

                if (parsed.type == "error") {
                    cb({}, parsed.data["message"].toString());
                } else {
                    cb(parsed.data, QString());
                }
            } else {
                // 服务端主动推送
                emit pushReceived(parsed.type, parsed.data);
            }
        });
    }

    void request(const QString &type, const QJsonObject &data,
                 Callback callback, int timeoutMs = 10000)
    {
        QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        m_pendingCallbacks[id] = std::move(callback);

        // 超时处理
        QTimer *timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, this, [this, id]() {
            if (m_pendingCallbacks.contains(id)) {
                auto cb = m_pendingCallbacks.take(id);
                m_pendingTimers.take(id)->deleteLater();
                cb({}, "Request timed out");
            }
        });
        timer->start(timeoutMs);
        m_pendingTimers[id] = timer;

        QString msg = WsProtocol::createMessage(type, data, id);
        m_socket->sendTextMessage(msg);
    }

signals:
    void pushReceived(const QString &type, const QJsonObject &data);

private:
    QWebSocket *m_socket;
    QMap<QString, Callback> m_pendingCallbacks;
    QMap<QString, QTimer *> m_pendingTimers;
};
```

使用方式：

```cpp
auto *rpc = new WsRpcClient(socket, this);

// 请求-响应
QJsonObject params;
params["userId"] = 42;

rpc->request("getUserInfo", params,
    [](const QJsonObject &data, const QString &err) {
        if (!err.isEmpty()) {
            qWarning() << "RPC error:" << err;
            return;
        }
        qDebug() << "User name:" << data["name"].toString();
        qDebug() << "User email:" << data["email"].toString();
    });

// 服务端主动推送
connect(rpc, &WsRpcClient::pushReceived,
        this, [](const QString &type, const QJsonObject &data) {
    if (type == "notification") {
        qDebug() << "Push notification:" << data["message"].toString();
    }
});
```

---

## 11. 实战：聊天室

### 11.1 服务端

```cpp
// ws_chat_server.h
#pragma once
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QDateTime>

class WsChatServer : public QObject
{
    Q_OBJECT
public:
    explicit WsChatServer(quint16 port, QObject *parent = nullptr)
        : QObject(parent)
        , m_server(new QWebSocketServer("ChatServer",
              QWebSocketServer::NonSecureMode, this))
    {
        if (m_server->listen(QHostAddress::Any, port)) {
            qDebug() << "Chat server started on port" << port;
        }

        connect(m_server, &QWebSocketServer::newConnection,
                this, &WsChatServer::onNewConnection);
    }

    ~WsChatServer()
    {
        m_server->close();
        qDeleteAll(m_clients.keys());
    }

private:
    void onNewConnection()
    {
        QWebSocket *client = m_server->nextPendingConnection();

        connect(client, &QWebSocket::textMessageReceived,
                this, [this, client](const QString &msg) {
            handleMessage(client, msg);
        });

        connect(client, &QWebSocket::disconnected,
                this, [this, client]() {
            QString name = m_clients.value(client, "Unknown");
            m_clients.remove(client);
            client->deleteLater();

            broadcastSystemMessage(name + " left the chat");
            qDebug() << name << "disconnected";
        });
    }

    void handleMessage(QWebSocket *sender, const QString &rawMsg)
    {
        QJsonObject msg = QJsonDocument::fromJson(rawMsg.toUtf8()).object();
        QString type = msg["type"].toString();

        if (type == "join") {
            QString name = msg["name"].toString();
            m_clients[sender] = name;
            broadcastSystemMessage(name + " joined the chat");

            // 发送在线用户列表给新用户
            QJsonArray users;
            for (const QString &user : m_clients.values()) {
                users.append(user);
            }
            QJsonObject welcome;
            welcome["type"] = "userList";
            welcome["users"] = users;
            sender->sendTextMessage(QJsonDocument(welcome).toJson(
                QJsonDocument::Compact));
        }
        else if (type == "chat") {
            QString name = m_clients.value(sender, "Anonymous");
            QString content = msg["content"].toString();
            broadcastChatMessage(name, content);
        }
    }

    void broadcastChatMessage(const QString &sender, const QString &content)
    {
        QJsonObject msg;
        msg["type"] = "chat";
        msg["sender"] = sender;
        msg["content"] = content;
        msg["timestamp"] = QDateTime::currentMSecsSinceEpoch();

        QString json = QJsonDocument(msg).toJson(QJsonDocument::Compact);
        for (QWebSocket *client : m_clients.keys()) {
            client->sendTextMessage(json);
        }
    }

    void broadcastSystemMessage(const QString &message)
    {
        QJsonObject msg;
        msg["type"] = "system";
        msg["content"] = message;
        msg["timestamp"] = QDateTime::currentMSecsSinceEpoch();

        QString json = QJsonDocument(msg).toJson(QJsonDocument::Compact);
        for (QWebSocket *client : m_clients.keys()) {
            client->sendTextMessage(json);
        }
    }

    QWebSocketServer *m_server;
    QMap<QWebSocket *, QString> m_clients;
};
```

### 11.2 客户端

```cpp
// ws_chat_client.h
#pragma once
#include <QObject>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class WsChatClient : public QObject
{
    Q_OBJECT
public:
    explicit WsChatClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QWebSocket(QString(),
              QWebSocketProtocol::VersionLatest, this))
    {
        connect(m_socket, &QWebSocket::connected, this, [this]() {
            // 发送加入消息
            QJsonObject msg;
            msg["type"] = "join";
            msg["name"] = m_userName;
            m_socket->sendTextMessage(
                QJsonDocument(msg).toJson(QJsonDocument::Compact));
            emit connected();
        });

        connect(m_socket, &QWebSocket::textMessageReceived,
                this, [this](const QString &raw) {
            QJsonObject msg = QJsonDocument::fromJson(raw.toUtf8()).object();
            QString type = msg["type"].toString();

            if (type == "chat") {
                emit chatMessage(msg["sender"].toString(),
                                 msg["content"].toString(),
                                 msg["timestamp"].toVariant().toLongLong());
            } else if (type == "system") {
                emit systemMessage(msg["content"].toString());
            } else if (type == "userList") {
                QStringList users;
                for (const auto &val : msg["users"].toArray()) {
                    users << val.toString();
                }
                emit userListUpdated(users);
            }
        });

        connect(m_socket, &QWebSocket::disconnected,
                this, &WsChatClient::disconnected);
    }

    void connectToServer(const QString &host, quint16 port,
                         const QString &userName)
    {
        m_userName = userName;
        m_socket->open(QUrl(QStringLiteral("ws://%1:%2").arg(host).arg(port)));
    }

    void sendChat(const QString &content)
    {
        QJsonObject msg;
        msg["type"] = "chat";
        msg["content"] = content;
        m_socket->sendTextMessage(
            QJsonDocument(msg).toJson(QJsonDocument::Compact));
    }

signals:
    void connected();
    void disconnected();
    void chatMessage(const QString &sender, const QString &content,
                     qint64 timestamp);
    void systemMessage(const QString &content);
    void userListUpdated(const QStringList &users);

private:
    QWebSocket *m_socket;
    QString m_userName;
};
```

### 11.3 使用方式

```cpp
// === 服务端 main.cpp ===
#include <QCoreApplication>
#include "ws_chat_server.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    WsChatServer server(9000);
    return app.exec();
}

// === 客户端 Widget ===
auto *chat = new WsChatClient(this);

connect(chat, &WsChatClient::chatMessage,
        this, [this](const QString &sender, const QString &content, qint64) {
    ui->chatDisplay->appendPlainText(
        QStringLiteral("[%1] %2").arg(sender, content));
});

connect(chat, &WsChatClient::systemMessage,
        this, [this](const QString &msg) {
    ui->chatDisplay->appendHtml(
        QStringLiteral("<i style='color:gray'>%1</i>").arg(msg));
});

connect(chat, &WsChatClient::userListUpdated,
        this, [this](const QStringList &users) {
    ui->userList->clear();
    ui->userList->addItems(users);
});

chat->connectToServer("127.0.0.1", 9000, "Alice");

connect(ui->sendButton, &QPushButton::clicked, this, [this, chat]() {
    QString text = ui->inputEdit->text().trimmed();
    if (!text.isEmpty()) {
        chat->sendChat(text);
        ui->inputEdit->clear();
    }
});
```

---

## 12. 与浏览器互通

Qt WebSocket 服务端天然兼容浏览器的 `WebSocket` API，可直接互通：

```html
<!-- 浏览器客户端 -->
<script>
const ws = new WebSocket('ws://192.168.1.100:9000');

ws.onopen = () => {
    ws.send(JSON.stringify({ type: 'join', name: 'WebUser' }));
};

ws.onmessage = (event) => {
    const msg = JSON.parse(event.data);
    if (msg.type === 'chat') {
        console.log(`[${msg.sender}] ${msg.content}`);
    }
};

// 发送消息
function sendChat(content) {
    ws.send(JSON.stringify({ type: 'chat', content }));
}
</script>
```

> Qt 服务端使用 JSON 文本帧协议，浏览器无需任何特殊适配。

---

## 13. 多线程处理

### 13.1 将连接分配到工作线程

```cpp
class WsWorker : public QObject
{
    Q_OBJECT
public slots:
    void handleConnection(QWebSocket *socket)
    {
        // socket 已在此线程中
        connect(socket, &QWebSocket::textMessageReceived,
                this, [socket](const QString &msg) {
            // 耗时处理（在工作线程中不会阻塞主线程）
            QThread::msleep(100);  // 模拟耗时操作
            socket->sendTextMessage("Processed: " + msg);
        });

        connect(socket, &QWebSocket::disconnected,
                socket, &QObject::deleteLater);
    }
};

// 服务端
class ThreadedWsServer : public QObject
{
    Q_OBJECT
public:
    explicit ThreadedWsServer(QObject *parent = nullptr)
        : QObject(parent)
    {
        m_server = new QWebSocketServer("Threaded",
            QWebSocketServer::NonSecureMode, this);

        // 创建工作线程
        m_workerThread = new QThread(this);
        m_worker = new WsWorker;
        m_worker->moveToThread(m_workerThread);
        connect(m_workerThread, &QThread::finished,
                m_worker, &QObject::deleteLater);
        m_workerThread->start();

        connect(m_server, &QWebSocketServer::newConnection,
                this, [this]() {
            QWebSocket *socket = m_server->nextPendingConnection();
            // 将 socket 移到工作线程
            socket->setParent(nullptr);
            socket->moveToThread(m_workerThread);
            // 在工作线程中处理
            QMetaObject::invokeMethod(m_worker, "handleConnection",
                Qt::QueuedConnection,
                Q_ARG(QWebSocket*, socket));
        });
    }

    bool listen(quint16 port) { return m_server->listen(QHostAddress::Any, port); }

private:
    QWebSocketServer *m_server;
    QThread *m_workerThread;
    WsWorker *m_worker;
};
```

> **注意：** `QWebSocket` 的 `moveToThread()` 必须在 `setParent(nullptr)` 之后调用，否则会拒绝跨线程移动。

---

## 14. 性能调优

### 14.1 读写缓冲区

```cpp
// 设置底层读缓冲区（字节数）
socket->setReadBufferSize(256 * 1024);  // 256KB
```

### 14.2 消息压缩（permessage-deflate）

WebSocket 协议支持 `permessage-deflate` 扩展压缩消息，Qt 5.x 不原生支持该扩展。如需压缩，可在应用层实现：

```cpp
// 发送端：压缩后发送
QByteArray raw = generateLargeJson();
QByteArray compressed = qCompress(raw, 6);  // 压缩级别 1-9

QJsonObject envelope;
envelope["compressed"] = true;
envelope["size"] = raw.size();
socket->sendTextMessage(
    QJsonDocument(envelope).toJson(QJsonDocument::Compact));
socket->sendBinaryMessage(compressed);

// 接收端：解压
connect(socket, &QWebSocket::binaryMessageReceived,
        this, [](const QByteArray &data) {
    QByteArray decompressed = qUncompress(data);
    // 处理...
});
```

### 14.3 高频消息优化

```cpp
// 批量发送：将多个小消息合并为一次发送
void batchSend(QWebSocket *socket, const QList<QJsonObject> &messages)
{
    QJsonArray batch;
    for (const QJsonObject &msg : messages) {
        batch.append(msg);
    }
    socket->sendTextMessage(
        QJsonDocument(batch).toJson(QJsonDocument::Compact));
}

// 节流：限制发送频率
class ThrottledSender : public QObject
{
    Q_OBJECT
public:
    explicit ThrottledSender(QWebSocket *socket, int intervalMs = 50,
                             QObject *parent = nullptr)
        : QObject(parent), m_socket(socket)
    {
        m_timer.setInterval(intervalMs);
        connect(&m_timer, &QTimer::timeout, this, [this]() {
            if (!m_queue.isEmpty()) {
                m_socket->sendTextMessage(m_queue.dequeue());
            } else {
                m_timer.stop();
            }
        });
    }

    void send(const QString &message)
    {
        m_queue.enqueue(message);
        if (!m_timer.isActive()) {
            m_timer.start();
        }
    }

private:
    QWebSocket *m_socket;
    QTimer m_timer;
    QQueue<QString> m_queue;
};
```

---

## 15. 最佳实践与常见陷阱

### 15.1 核心原则

| 原则 | 说明 |
|------|------|
| **定义消息协议** | 使用 JSON 或二进制格式定义统一的 `type + data + id` 消息结构 |
| **实现心跳保活** | 利用内建的 Ping/Pong 或应用层心跳检测连接有效性 |
| **自动重连** | 客户端必须实现指数退避重连机制 |
| **释放资源** | `disconnected` 信号中对 `QWebSocket *` 调用 `deleteLater()` |
| **限制消息大小** | 服务端校验消息长度，超限直接关闭连接 |
| **使用 WSS** | 生产环境务必使用 `wss://` 加密连接 |
| **CORS 检查** | 服务端应验证 Origin Header 防止恶意跨域连接 |

### 15.2 常见陷阱速查

| 陷阱 | 症状 | 解决方案 |
|------|------|---------|
| 模块名写错 | 编译找不到 `QWebSocket` | `QT += websockets`（不是 `network`） |
| 未处理断线 | 网络闪断后无反应 | 实现自动重连 + 心跳检测（§6 + §7） |
| 忘记 `deleteLater()` | 内存泄漏 | 在 `disconnected` 中释放 |
| 跨线程使用 socket | 崩溃、未定义行为 | `moveToThread()` 或在目标线程中创建 |
| `error` 信号歧义 | 编译错误"模糊调用" | 使用 `QOverload<SocketError>::of(&QWebSocket::error)` |
| 未做消息大小限制 | 恶意客户端 OOM | 检查 `data.size()`，超限则 `close()` |
| WSS 证书问题 | 连接失败无错误 | 正确处理 `sslErrors` 信号，检查证书链 |
| 忽略 Origin 检查 | XSS / CSRF 攻击 | 使用 `originAuthenticationRequired` 白名单 |
| JSON 解析失败静默 | 逻辑异常 | 解析后检查 `isObject()` / `isNull()` |
| 单线程处理耗时操作 | 所有客户端卡顿 | 将连接分配到工作线程（§13） |
| 关闭后仍写入 | 错误 / 崩溃 | 发送前检查 `state() == ConnectedState` |

---

## 16. API 速查表

### QWebSocket 主要 API

```cpp
// 连接
void open(const QUrl &url);
void open(const QNetworkRequest &request);  // 带自定义 Header
void close(QWebSocketProtocol::CloseCode closeCode = CloseCodeNormal,
           const QString &reason = QString());
void abort();

// 发送
qint64 sendTextMessage(const QString &message);
qint64 sendBinaryMessage(const QByteArray &data);
void ping(const QByteArray &payload = QByteArray());

// 状态
QAbstractSocket::SocketState state() const;
bool isValid() const;
QHostAddress peerAddress() const;
quint16 peerPort() const;
QHostAddress localAddress() const;
quint16 localPort() const;
QUrl requestUrl() const;
QString origin() const;
QString resourceName() const;

// 关闭信息
QWebSocketProtocol::CloseCode closeCode() const;
QString closeReason() const;

// SSL
void setSslConfiguration(const QSslConfiguration &sslConfiguration);
QSslConfiguration sslConfiguration() const;
void ignoreSslErrors();

// 缓冲区
void setReadBufferSize(qint64 size);
qint64 readBufferSize() const;

// 信号
void connected();
void disconnected();
void textMessageReceived(const QString &message);
void binaryMessageReceived(const QByteArray &message);
void textFrameReceived(const QString &frame, bool isLastFrame);    // 低级分帧
void binaryFrameReceived(const QByteArray &frame, bool isLastFrame);
void pong(quint64 elapsedTime, const QByteArray &payload);
void stateChanged(QAbstractSocket::SocketState state);
void error(QAbstractSocket::SocketError error);
void sslErrors(const QList<QSslError> &errors);
void bytesWritten(qint64 bytes);
```

### QWebSocketServer 主要 API

```cpp
// 监听
bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
void close();
bool isListening() const;
quint16 serverPort() const;
QHostAddress serverAddress() const;
QUrl serverUrl() const;
QString serverName() const;

// 连接管理
bool hasPendingConnections() const;
QWebSocket *nextPendingConnection();
void setMaxPendingConnections(int numConnections);

// SSL
void setSslConfiguration(const QSslConfiguration &sslConfiguration);
QSslConfiguration sslConfiguration() const;
QWebSocketServer::SslMode secureMode() const;

// 信号
void newConnection();
void closed();
void originAuthenticationRequired(QWebSocketCorsAuthenticator *authenticator);
void peerVerifyError(const QSslError &error);
void sslErrors(QWebSocket *socket, const QList<QSslError> &errors);
void serverError(QWebSocketProtocol::CloseCode closeCode);
void acceptError(QAbstractSocket::SocketError socketError);

// 枚举
enum SslMode { SecureMode, NonSecureMode };
```
