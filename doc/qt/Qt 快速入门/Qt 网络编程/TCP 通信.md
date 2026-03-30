# TCP 通信

Qt 通过 `QTcpServer` 和 `QTcpSocket` 提供面向连接的 TCP 通信能力。`QTcpSocket` 继承自 `QAbstractSocket` → `QIODevice`，天然支持 Qt 的**异步信号槽**机制——数据到达时触发 `readyRead()`，连接状态变化时触发 `stateChanged()`，无需手动轮询，不阻塞 UI。

> **模块依赖：** 在 `.pro` 中添加 `QT += network`；CMake 中添加 `find_package(Qt5 COMPONENTS Network)` + `target_link_libraries(... Qt5::Network)`。

---

## 1. 核心类总览

| 类 | 职责 |
|---|---|
| `QTcpSocket` | TCP 客户端套接字，连接远程主机，收发数据 |
| `QTcpServer` | TCP 服务端，监听端口，接受传入连接 |
| `QAbstractSocket` | `QTcpSocket` / `QUdpSocket` 的公共基类，定义状态机和通用 API |
| `QHostAddress` | 封装 IPv4 / IPv6 地址（`QHostAddress::Any`、`QHostAddress::LocalHost` 等） |
| `QNetworkInterface` | 查询本机网卡信息（IP、MAC、子网掩码） |
| `QSslSocket` | `QTcpSocket` 的 SSL/TLS 扩展，用于加密 TCP 通信 |

```
QIODevice                          ← 统一 I/O 抽象（read / write / readyRead）
└── QAbstractSocket                ← 套接字基类（状态机 / 错误码）
    ├── QTcpSocket                 ← TCP 客户端
    │   └── QSslSocket             ← TLS 加密层
    └── QUdpSocket                 ← UDP（无连接）

QTcpServer                         ← TCP 服务端（独立类，不继承 QIODevice）
    └─ newConnection() 信号 → nextPendingConnection() 返回 QTcpSocket*
```

### 1.1 QTcpSocket 状态机

```
UnconnectedState → HostLookupState → ConnectingState → ConnectedState
                                                            │
                                                      ClosingState
                                                            │
                                                    UnconnectedState
```

| 状态枚举 | 含义 |
|----------|------|
| `UnconnectedState` | 未连接 |
| `HostLookupState` | 正在 DNS 解析 |
| `ConnectingState` | 正在建立 TCP 连接（三次握手中） |
| `ConnectedState` | 已连接，可收发数据 |
| `BoundState` | 已绑定本地地址（用于 UDP） |
| `ClosingState` | 正在关闭（缓冲区数据尚未发完） |
| `ListeningState` | 服务端监听中（仅 `QTcpServer` 内部） |

---

## 2. TCP 客户端

### 2.1 最简客户端

```cpp
#include <QTcpSocket>

QTcpSocket *socket = new QTcpSocket(this);

// 连接到服务器
socket->connectToHost("192.168.1.100", 9000);

// 连接成功
connect(socket, &QTcpSocket::connected, this, [socket]() {
    qDebug() << "Connected!";
    socket->write("Hello Server\n");
});

// 收到数据
connect(socket, &QTcpSocket::readyRead, this, [socket]() {
    QByteArray data = socket->readAll();
    qDebug() << "Received:" << data;
});

// 断开连接
connect(socket, &QTcpSocket::disconnected, this, []() {
    qDebug() << "Disconnected";
});
```

### 2.2 完整客户端类

```cpp
// tcp_client.h
#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);

    void connectToServer(const QString &host, quint16 port);
    void disconnect();
    void send(const QByteArray &data);
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &errorString);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_socket = nullptr;
};

// tcp_client.cpp
#include "tcp_client.h"

TcpClient::TcpClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected,
            this, &TcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &TcpClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead,
            this, &TcpClient::onReadyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred,
            this, &TcpClient::onErrorOccurred);
}

void TcpClient::connectToServer(const QString &host, quint16 port)
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();  // 强制断开旧连接
    }
    m_socket->connectToHost(host, port);
}

void TcpClient::disconnect()
{
    m_socket->disconnectFromHost();
}

void TcpClient::send(const QByteArray &data)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(data);
    }
}

bool TcpClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void TcpClient::onConnected()
{
    qDebug() << "Connected to" << m_socket->peerAddress().toString()
             << ":" << m_socket->peerPort();
    emit connected();
}

void TcpClient::onDisconnected()
{
    qDebug() << "Disconnected";
    emit disconnected();
}

void TcpClient::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    emit dataReceived(data);
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    qWarning() << "Socket error:" << m_socket->errorString();
    emit errorOccurred(m_socket->errorString());
}
```

### 2.3 同步（阻塞）模式

`QTcpSocket` 提供 `waitFor*()` 系列阻塞方法，**仅适用于非 GUI 线程**（如工作线程、控制台程序）：

```cpp
QTcpSocket socket;
socket.connectToHost("192.168.1.100", 9000);

if (!socket.waitForConnected(5000)) {  // 等待连接，超时 5 秒
    qWarning() << "Connection failed:" << socket.errorString();
    return;
}

socket.write("Hello\n");
if (!socket.waitForBytesWritten(3000)) {  // 等待数据发出
    qWarning() << "Write failed";
    return;
}

if (socket.waitForReadyRead(5000)) {  // 等待响应
    QByteArray response = socket.readAll();
    qDebug() << "Response:" << response;
}

socket.disconnectFromHost();
if (socket.state() != QAbstractSocket::UnconnectedState) {
    socket.waitForDisconnected(3000);
}
```

> **⚠️ 警告：** 在主线程中调用 `waitFor*()` 会冻结 UI。主线程务必使用异步信号槽模式。

**阻塞 vs 异步对比：**

| 方面 | 异步（信号槽） | 同步（waitFor*） |
|------|---------------|-----------------|
| 适用场景 | GUI 程序主线程 | 工作线程 / 控制台程序 |
| 代码风格 | 事件驱动，回调 | 顺序执行，直观 |
| UI 影响 | 不阻塞 | 会冻结 UI |
| 并发 | 天然支持多连接 | 需手动管理线程 |
| 错误处理 | 信号中处理 | 返回值 + errorString() |

---

## 3. TCP 服务端

### 3.1 最简服务端

```cpp
#include <QTcpServer>
#include <QTcpSocket>

QTcpServer *server = new QTcpServer(this);

// 监听所有网卡的 9000 端口
if (!server->listen(QHostAddress::Any, 9000)) {
    qCritical() << "Listen failed:" << server->errorString();
    return;
}
qDebug() << "Listening on port 9000";

// 有新连接时
connect(server, &QTcpServer::newConnection, this, [server]() {
    QTcpSocket *client = server->nextPendingConnection();
    qDebug() << "New client:" << client->peerAddress().toString()
             << ":" << client->peerPort();

    // 收到数据 → 回显
    connect(client, &QTcpSocket::readyRead, [client]() {
        QByteArray data = client->readAll();
        qDebug() << "Received:" << data;
        client->write("Echo: " + data);  // 回显
    });

    // 客户端断开 → 清理
    connect(client, &QTcpSocket::disconnected, client, &QObject::deleteLater);
});
```

### 3.2 多客户端服务端

```cpp
// tcp_server.h
#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);

    bool start(quint16 port);
    void stop();
    void broadcast(const QByteArray &data);
    int clientCount() const { return m_clients.size(); }

signals:
    void clientConnected(const QString &address);
    void clientDisconnected(const QString &address);
    void messageReceived(QTcpSocket *client, const QByteArray &data);

private slots:
    void onNewConnection();

private:
    QTcpServer *m_server = nullptr;
    QList<QTcpSocket *> m_clients;
};

// tcp_server.cpp
#include "tcp_server.h"
#include <QHostAddress>

TcpServer::TcpServer(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection,
            this, &TcpServer::onNewConnection);
}

bool TcpServer::start(quint16 port)
{
    if (m_server->isListening()) {
        m_server->close();
    }
    if (!m_server->listen(QHostAddress::Any, port)) {
        qCritical() << "Listen failed:" << m_server->errorString();
        return false;
    }
    qDebug() << "Server listening on port" << port;
    return true;
}

void TcpServer::stop()
{
    // 断开所有客户端
    for (QTcpSocket *client : qAsConst(m_clients)) {
        client->disconnectFromHost();
    }
    m_server->close();
}

void TcpServer::broadcast(const QByteArray &data)
{
    for (QTcpSocket *client : qAsConst(m_clients)) {
        client->write(data);
    }
}

void TcpServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *client = m_server->nextPendingConnection();
        m_clients.append(client);

        QString addr = QStringLiteral("%1:%2")
            .arg(client->peerAddress().toString())
            .arg(client->peerPort());
        qDebug() << "Client connected:" << addr;
        emit clientConnected(addr);

        // 收到数据
        connect(client, &QTcpSocket::readyRead, this, [this, client]() {
            QByteArray data = client->readAll();
            emit messageReceived(client, data);
        });

        // 客户端断开
        connect(client, &QTcpSocket::disconnected, this, [this, client]() {
            QString addr = QStringLiteral("%1:%2")
                .arg(client->peerAddress().toString())
                .arg(client->peerPort());
            m_clients.removeOne(client);
            client->deleteLater();
            qDebug() << "Client disconnected:" << addr;
            emit clientDisconnected(addr);
        });

        // 错误处理
        connect(client, &QAbstractSocket::errorOccurred,
                this, [client](QAbstractSocket::SocketError err) {
            Q_UNUSED(err)
            qWarning() << "Client error:" << client->errorString();
        });
    }
}
```

使用方式：

```cpp
auto *server = new TcpServer(this);
server->start(9000);

connect(server, &TcpServer::messageReceived,
        this, [server](QTcpSocket *client, const QByteArray &data) {
    qDebug() << "From" << client->peerAddress().toString()
             << ":" << data;

    // 回复发送者
    client->write("Got it!\n");

    // 广播给所有客户端
    server->broadcast("[Broadcast] " + data);
});
```

---

## 4. TCP 粘包问题与消息协议

### 4.1 什么是粘包

TCP 是**字节流**协议，没有消息边界。发送端调用两次 `write()`，接收端可能在一次 `readyRead()` 中收到合并的数据：

```
发送端：                     接收端：
write("Hello")              readyRead → "HelloWorld"  ← 粘包
write("World")

write("AABBCCDD")           readyRead → "AABB"        ← 拆包
                             readyRead → "CCDD"
```

> **核心结论：** 不能假设一次 `readyRead()` 对应一次 `write()`，必须自定义消息边界协议。

### 4.2 方案一：固定长度头 + 变长体（推荐）

最常用的二进制协议格式——用固定 4 字节头部存储消息体长度：

```
┌──────────┬───────────────────┐
│ 4 bytes  │  N bytes          │
│ 消息长度  │  消息体（payload）│
│ (uint32) │                   │
└──────────┴───────────────────┘
```

**发送端：打包**

```cpp
void sendMessage(QTcpSocket *socket, const QByteArray &message)
{
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    // 先写占位长度（4 字节），再写消息体
    stream << static_cast<quint32>(0);  // 占位
    stream.writeRawData(message.constData(), message.size());

    // 回填实际长度
    stream.device()->seek(0);
    stream << static_cast<quint32>(message.size());

    socket->write(packet);
}
```

**接收端：拆包**

```cpp
// 成员变量
QByteArray m_buffer;

// 在 readyRead 槽中调用
void onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    while (true) {
        // 至少需要 4 字节头
        if (m_buffer.size() < 4) break;

        // 读取消息长度
        QDataStream headerStream(m_buffer.left(4));
        headerStream.setByteOrder(QDataStream::BigEndian);
        quint32 msgLen = 0;
        headerStream >> msgLen;

        // 防止恶意超大包（安全）
        if (msgLen > 10 * 1024 * 1024) {  // 最大 10MB
            qWarning() << "Message too large:" << msgLen;
            m_socket->abort();
            return;
        }

        // 数据尚未完整到达
        if (m_buffer.size() < static_cast<int>(4 + msgLen)) break;

        // 提取完整消息
        QByteArray message = m_buffer.mid(4, msgLen);
        m_buffer.remove(0, 4 + msgLen);

        // 处理消息
        processMessage(message);
    }
}
```

### 4.3 方案二：使用 QDataStream 序列化

Qt 的 `QDataStream` 提供了方便的事务机制（Qt 5.7+），简化拆包逻辑：

**发送端：**

```cpp
void sendData(QTcpSocket *socket, const QString &text, int id)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << quint32(0);   // 预留长度位
    out << id;           // 写入业务数据
    out << text;

    out.device()->seek(0);
    out << static_cast<quint32>(block.size() - sizeof(quint32));  // 回填

    socket->write(block);
}
```

**接收端（事务模式）：**

```cpp
// 成员变量
QDataStream m_stream;
quint32 m_nextBlockSize = 0;

// 构造时绑定
m_stream.setDevice(m_socket);
m_stream.setVersion(QDataStream::Qt_5_15);

// readyRead 槽
void onReadyRead()
{
    while (true) {
        // 开始事务——如果数据不足会自动回滚
        m_stream.startTransaction();

        quint32 blockSize;
        m_stream >> blockSize;

        // 读取业务数据
        int id;
        QString text;
        m_stream >> id >> text;

        if (!m_stream.commitTransaction()) {
            break;  // 数据不完整，等待更多数据
        }

        // 成功提取一条完整消息
        qDebug() << "Message id:" << id << "text:" << text;
    }
}
```

> **`startTransaction()` / `commitTransaction()` 的原理：** 开始事务后，如果中途读取的数据不够（`QDataStream` 检测到 underflow），`commitTransaction()` 返回 `false` 并自动回滚读取位置。下次 `readyRead()` 触发时重新尝试。

### 4.4 方案三：分隔符协议

适用于纯文本协议（如行协议）：

```cpp
// 成员变量
QByteArray m_buffer;

void onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    int idx;
    while ((idx = m_buffer.indexOf('\n')) != -1) {
        QByteArray line = m_buffer.left(idx);    // 不含 '\n'
        m_buffer.remove(0, idx + 1);             // 移除含 '\n'
        processLine(line.trimmed());
    }

    // 防止无分隔符的恶意数据撑爆缓冲区
    if (m_buffer.size() > 64 * 1024) {
        qWarning() << "Buffer overflow, disconnecting";
        m_socket->abort();
    }
}
```

### 4.5 三种方案对比

| 方案 | 优点 | 缺点 | 适用场景 |
|------|------|------|---------|
| 固定长度头 + 变长体 | 高效、支持二进制、长度可预知 | 需手动管理缓冲区 | 通用二进制协议 |
| QDataStream 事务 | Qt 原生、代码简洁、自动回滚 | 两端必须用相同 Qt 版本 | Qt-to-Qt 通信 |
| 分隔符 | 简单、人类可读 | 需转义分隔符、不适合二进制 | 文本行协议、调试 |

---

## 5. 服务端多线程处理

### 5.1 为什么需要多线程

默认情况下，`QTcpServer` 和所有 `QTcpSocket` 运行在同一线程的事件循环中。如果某个客户端的消息处理耗时（如数据库操作、文件 I/O），会阻塞其他客户端的数据收发。

### 5.2 方案一：每连接一线程

重写 `QTcpServer::incomingConnection()` 将新连接分配到独立线程：

```cpp
// threaded_server.h
class ThreadedTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ThreadedTcpServer(QObject *parent = nullptr)
        : QTcpServer(parent) {}

protected:
    // 重写——在新线程中处理连接
    void incomingConnection(qintptr socketDescriptor) override;
};

// client_thread.h
class ClientThread : public QThread
{
    Q_OBJECT
public:
    explicit ClientThread(qintptr socketDescriptor, QObject *parent = nullptr)
        : QThread(parent), m_socketDescriptor(socketDescriptor) {}

signals:
    void error(QTcpSocket::SocketError err);

protected:
    void run() override
    {
        // 在子线程中创建 socket（不能跨线程传递 QTcpSocket 指针）
        QTcpSocket socket;
        if (!socket.setSocketDescriptor(m_socketDescriptor)) {
            emit error(socket.error());
            return;
        }

        qDebug() << "Thread" << QThread::currentThreadId()
                 << "handling client:" << socket.peerAddress().toString();

        // 阻塞模式处理（子线程中安全）
        while (socket.state() == QAbstractSocket::ConnectedState) {
            if (socket.waitForReadyRead(3000)) {
                QByteArray data = socket.readAll();
                // 耗时处理...
                socket.write("Processed: " + data);
                socket.waitForBytesWritten();
            }
        }

        socket.disconnectFromHost();
        if (socket.state() != QAbstractSocket::UnconnectedState) {
            socket.waitForDisconnected();
        }
        qDebug() << "Client handler thread finished";
    }

private:
    qintptr m_socketDescriptor;
};

// threaded_server.cpp
void ThreadedTcpServer::incomingConnection(qintptr socketDescriptor)
{
    auto *thread = new ClientThread(socketDescriptor, this);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}
```

### 5.3 方案二：线程池 + Worker

比"每连接一线程"更高效——使用固定大小的线程池：

```cpp
// socket_worker.h
class SocketWorker : public QObject
{
    Q_OBJECT
public:
    explicit SocketWorker(qintptr descriptor, QObject *parent = nullptr)
        : QObject(parent), m_descriptor(descriptor) {}

public slots:
    void process()
    {
        m_socket = new QTcpSocket(this);
        m_socket->setSocketDescriptor(m_descriptor);

        connect(m_socket, &QTcpSocket::readyRead, this, &SocketWorker::onReadyRead);
        connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
            emit finished();
        });
    }

private slots:
    void onReadyRead()
    {
        QByteArray data = m_socket->readAll();
        // 业务处理...
        m_socket->write("OK: " + data);
    }

signals:
    void finished();

private:
    qintptr m_descriptor;
    QTcpSocket *m_socket = nullptr;
};

// 在 incomingConnection 中使用
void MyServer::incomingConnection(qintptr socketDescriptor)
{
    QThread *thread = new QThread(this);
    SocketWorker *worker = new SocketWorker(socketDescriptor);
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &SocketWorker::process);
    connect(worker, &SocketWorker::finished, thread, &QThread::quit);
    connect(worker, &SocketWorker::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
}
```

> **关键规则：** `QTcpSocket` 不能跨线程使用。要么在目标线程中通过 `setSocketDescriptor()` 创建新 socket，要么用 `moveToThread()` 转移（需小心）。

### 5.4 多线程方案对比

| 方案 | 优点 | 缺点 | 适用场景 |
|------|------|------|---------|
| 单线程（默认） | 简单、无竞态 | 处理慢会阻塞其他客户端 | 轻量级、低并发 |
| 每连接一线程 | 实现简单 | 连接多时线程开销大 | 中等连接数、长连接 |
| 线程池 + Worker | 资源可控 | 实现较复杂 | 高并发 |
| `moveToThread` | 可复用线程 | 对象归属管理复杂 | 需异步事件循环的 Worker |

---

## 6. 自动重连机制

网络不稳定时，客户端需要自动重连：

```cpp
class ReconnectClient : public QObject
{
    Q_OBJECT
public:
    explicit ReconnectClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QTcpSocket(this))
        , m_reconnectTimer(new QTimer(this))
    {
        m_reconnectTimer->setInterval(3000);  // 3 秒后重连
        m_reconnectTimer->setSingleShot(true);

        connect(m_socket, &QTcpSocket::connected, this, [this]() {
            m_reconnectAttempts = 0;
            m_reconnectTimer->stop();
            qDebug() << "Connected!";
        });

        connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
            qDebug() << "Disconnected, will reconnect...";
            scheduleReconnect();
        });

        connect(m_socket, &QAbstractSocket::errorOccurred,
                this, [this](QAbstractSocket::SocketError err) {
            Q_UNUSED(err)
            if (m_socket->state() == QAbstractSocket::UnconnectedState) {
                scheduleReconnect();
            }
        });

        connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
            attemptConnect();
        });
    }

    void connectToServer(const QString &host, quint16 port)
    {
        m_host = host;
        m_port = port;
        attemptConnect();
    }

private:
    void attemptConnect()
    {
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->abort();
        }
        ++m_reconnectAttempts;
        qDebug() << "Connecting... attempt" << m_reconnectAttempts;
        m_socket->connectToHost(m_host, m_port);
    }

    void scheduleReconnect()
    {
        if (m_reconnectAttempts >= m_maxAttempts) {
            qWarning() << "Max reconnect attempts reached, giving up";
            return;
        }
        // 指数退避：3s → 6s → 12s → 24s → ... 最大 60s
        int delay = qMin(3000 * (1 << m_reconnectAttempts), 60000);
        qDebug() << "Reconnecting in" << delay / 1000 << "seconds...";
        m_reconnectTimer->setInterval(delay);
        m_reconnectTimer->start();
    }

    QTcpSocket *m_socket;
    QTimer *m_reconnectTimer;
    QString m_host;
    quint16 m_port = 0;
    int m_reconnectAttempts = 0;
    int m_maxAttempts = 10;
};
```

> **指数退避（Exponential Backoff）** 避免网络恢复瞬间所有客户端同时涌入服务端。

---

## 7. 心跳保活

长连接场景中，需要定期发送心跳包检测连接是否存活：

```cpp
class HeartbeatClient : public QObject
{
    Q_OBJECT
public:
    explicit HeartbeatClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QTcpSocket(this))
        , m_heartbeatTimer(new QTimer(this))
        , m_timeoutTimer(new QTimer(this))
    {
        // 每 10 秒发一次心跳
        m_heartbeatTimer->setInterval(10000);
        connect(m_heartbeatTimer, &QTimer::timeout, this, [this]() {
            if (m_socket->state() == QAbstractSocket::ConnectedState) {
                QByteArray ping = QByteArrayLiteral("\x00PING");
                m_socket->write(ping);
                m_timeoutTimer->start();  // 启动超时计时
            }
        });

        // 心跳超时 5 秒无响应 → 判定断线
        m_timeoutTimer->setInterval(5000);
        m_timeoutTimer->setSingleShot(true);
        connect(m_timeoutTimer, &QTimer::timeout, this, [this]() {
            qWarning() << "Heartbeat timeout, connection lost";
            m_socket->abort();
        });

        connect(m_socket, &QTcpSocket::connected, this, [this]() {
            m_heartbeatTimer->start();
        });

        connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
            m_heartbeatTimer->stop();
            m_timeoutTimer->stop();
        });

        connect(m_socket, &QTcpSocket::readyRead, this, [this]() {
            QByteArray data = m_socket->readAll();
            if (data.contains("PONG")) {
                m_timeoutTimer->stop();  // 收到心跳回复，重置超时
            } else {
                // 处理业务数据...
            }
        });
    }

private:
    QTcpSocket *m_socket;
    QTimer *m_heartbeatTimer;
    QTimer *m_timeoutTimer;
};
```

### 7.1 TCP KeepAlive（系统级）

除应用层心跳外，也可启用操作系统的 TCP KeepAlive：

```cpp
// 启用系统级 TCP KeepAlive
m_socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

// 更细粒度的参数需平台 API（Windows 示例）
#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
struct tcp_keepalive ka;
ka.onoff = 1;
ka.keepalivetime = 30000;     // 30 秒无数据后开始探测
ka.keepaliveinterval = 5000;  // 每 5 秒发一次探测
DWORD bytesReturned;
WSAIoctl(m_socket->socketDescriptor(), SIO_KEEPALIVE_VALS,
         &ka, sizeof(ka), nullptr, 0, &bytesReturned, nullptr, nullptr);
#endif
```

> **应用层心跳 vs 系统 TCP KeepAlive：** 系统级 KeepAlive 只能检测网络层断开（如拔网线），无法检测应用层假死。实际项目中建议**两者结合**使用。

---

## 8. 错误处理

### 8.1 常见错误码

| 枚举值 | 含义 |
|--------|------|
| `ConnectionRefusedError` | 目标端口未监听或防火墙拒绝 |
| `RemoteHostClosedError` | 远端正常关闭连接 |
| `HostNotFoundError` | DNS 解析失败 |
| `SocketAccessError` | 权限不足（如绑定 < 1024 端口） |
| `SocketTimeoutError` | 操作超时 |
| `NetworkError` | 网络不可达（如 Wi-Fi 断开） |
| `AddressInUseError` | 端口被占用（服务端 listen 时） |
| `SocketResourceError` | 系统 socket 资源耗尽（文件描述符上限） |

### 8.2 完善的错误处理

```cpp
connect(m_socket, &QAbstractSocket::errorOccurred,
        this, [this](QAbstractSocket::SocketError err) {
    switch (err) {
    case QAbstractSocket::ConnectionRefusedError:
        handleError("连接被拒绝，请确认服务器已启动");
        break;
    case QAbstractSocket::HostNotFoundError:
        handleError("找不到主机，请检查地址");
        break;
    case QAbstractSocket::RemoteHostClosedError:
        handleError("服务器断开了连接");
        scheduleReconnect();
        break;
    case QAbstractSocket::SocketTimeoutError:
        handleError("连接超时");
        scheduleReconnect();
        break;
    case QAbstractSocket::NetworkError:
        handleError("网络不可达");
        scheduleReconnect();
        break;
    default:
        handleError(QStringLiteral("未知错误 [%1]: %2")
            .arg(err).arg(m_socket->errorString()));
        break;
    }
});
```

### 8.3 服务端错误：端口被占用

```cpp
if (!server->listen(QHostAddress::Any, 9000)) {
    if (server->serverError() == QAbstractSocket::AddressInUseError) {
        qCritical() << "Port 9000 is already in use!";
        // 策略1：换端口
        server->listen(QHostAddress::Any, 0);  // 系统自动分配
        qDebug() << "Using port:" << server->serverPort();
        // 策略2：启用 SO_REUSEADDR
    }
}
```

---

## 9. 连接信息查询

### 9.1 QTcpSocket 连接信息

```cpp
// 远端信息
qDebug() << "Peer address:" << socket->peerAddress().toString();
qDebug() << "Peer port:" << socket->peerPort();
qDebug() << "Peer name:" << socket->peerName();

// 本地信息
qDebug() << "Local address:" << socket->localAddress().toString();
qDebug() << "Local port:" << socket->localPort();

// 底层描述符
qDebug() << "Socket descriptor:" << socket->socketDescriptor();

// 缓冲区状态
qDebug() << "Bytes available:" << socket->bytesAvailable();
qDebug() << "Bytes to write:" << socket->bytesToWrite();
```

### 9.2 查询本机网络信息

```cpp
#include <QNetworkInterface>

const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
for (const QNetworkInterface &iface : interfaces) {
    if (iface.flags() & QNetworkInterface::IsUp &&
        !(iface.flags() & QNetworkInterface::IsLoopBack)) {

        qDebug() << "Interface:" << iface.humanReadableName();
        qDebug() << "  MAC:" << iface.hardwareAddress();

        const QList<QNetworkAddressEntry> entries = iface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                qDebug() << "  IPv4:" << entry.ip().toString();
                qDebug() << "  Netmask:" << entry.netmask().toString();
            }
        }
    }
}
```

---

## 10. 实战：聊天室

### 10.1 服务端

```cpp
// chat_server.h
#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QDataStream>

class ChatServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatServer(QObject *parent = nullptr) : QTcpServer(parent) {}

    bool startServer(quint16 port)
    {
        if (!listen(QHostAddress::Any, port)) {
            qCritical() << "Cannot start server:" << errorString();
            return false;
        }
        qDebug() << "Chat server started on port" << port;
        return true;
    }

protected:
    void incomingConnection(qintptr descriptor) override
    {
        QTcpSocket *client = new QTcpSocket(this);
        client->setSocketDescriptor(descriptor);
        m_clients[client] = QStringLiteral("User_%1").arg(descriptor);

        broadcastMessage("System",
            m_clients[client] + " joined the chat");

        connect(client, &QTcpSocket::readyRead, this, [this, client]() {
            m_buffers[client].append(client->readAll());
            processBuffer(client);
        });

        connect(client, &QTcpSocket::disconnected, this, [this, client]() {
            QString name = m_clients.value(client, "Unknown");
            m_clients.remove(client);
            m_buffers.remove(client);
            client->deleteLater();
            broadcastMessage("System", name + " left the chat");
        });
    }

private:
    // 使用长度前缀协议
    void processBuffer(QTcpSocket *client)
    {
        QByteArray &buf = m_buffers[client];
        while (buf.size() >= 4) {
            QDataStream stream(buf.left(4));
            stream.setByteOrder(QDataStream::BigEndian);
            quint32 len;
            stream >> len;

            if (len > 1024 * 1024) {  // 最大 1MB
                client->abort();
                return;
            }
            if (buf.size() < static_cast<int>(4 + len)) break;

            QByteArray message = buf.mid(4, len);
            buf.remove(0, 4 + len);

            QString sender = m_clients.value(client, "Unknown");
            broadcastMessage(sender, QString::fromUtf8(message));
        }
    }

    void broadcastMessage(const QString &sender, const QString &message)
    {
        QString formatted = QStringLiteral("[%1] %2").arg(sender, message);
        QByteArray payload = formatted.toUtf8();

        QByteArray packet;
        QDataStream stream(&packet, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << static_cast<quint32>(payload.size());
        packet.append(payload);

        for (QTcpSocket *client : m_clients.keys()) {
            client->write(packet);
        }

        qDebug() << formatted;
    }

    QMap<QTcpSocket *, QString> m_clients;
    QMap<QTcpSocket *, QByteArray> m_buffers;
};
```

### 10.2 客户端

```cpp
// chat_client.h
#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QDataStream>

class ChatClient : public QObject
{
    Q_OBJECT
public:
    explicit ChatClient(QObject *parent = nullptr)
        : QObject(parent), m_socket(new QTcpSocket(this))
    {
        connect(m_socket, &QTcpSocket::connected, this, [this]() {
            qDebug() << "Connected to chat server";
            emit connected();
        });

        connect(m_socket, &QTcpSocket::readyRead, this, [this]() {
            m_buffer.append(m_socket->readAll());
            while (m_buffer.size() >= 4) {
                QDataStream stream(m_buffer.left(4));
                stream.setByteOrder(QDataStream::BigEndian);
                quint32 len;
                stream >> len;

                if (m_buffer.size() < static_cast<int>(4 + len)) break;

                QByteArray msg = m_buffer.mid(4, len);
                m_buffer.remove(0, 4 + len);
                emit messageReceived(QString::fromUtf8(msg));
            }
        });

        connect(m_socket, &QTcpSocket::disconnected,
                this, &ChatClient::disconnected);
    }

    void connectToServer(const QString &host, quint16 port)
    {
        m_socket->connectToHost(host, port);
    }

    void sendMessage(const QString &text)
    {
        QByteArray payload = text.toUtf8();
        QByteArray packet;
        QDataStream stream(&packet, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << static_cast<quint32>(payload.size());
        packet.append(payload);
        m_socket->write(packet);
    }

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &message);

private:
    QTcpSocket *m_socket;
    QByteArray m_buffer;
};
```

### 10.3 使用方式

```cpp
// === 服务端 main.cpp ===
#include <QCoreApplication>
#include "chat_server.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    ChatServer server;
    server.startServer(9000);
    return app.exec();
}

// === 客户端 Widget ===
auto *client = new ChatClient(this);
client->connectToServer("127.0.0.1", 9000);

connect(client, &ChatClient::messageReceived,
        this, [this](const QString &msg) {
    ui->chatDisplay->appendPlainText(msg);
});

// 发送按钮
connect(ui->sendButton, &QPushButton::clicked, this, [this, client]() {
    QString text = ui->inputEdit->text().trimmed();
    if (!text.isEmpty()) {
        client->sendMessage(text);
        ui->inputEdit->clear();
    }
});
```

---

## 11. SSL/TLS 加密 TCP（QSslSocket）

### 11.1 客户端加密连接

```cpp
#include <QSslSocket>

QSslSocket *sslSocket = new QSslSocket(this);

connect(sslSocket, &QSslSocket::encrypted, this, []() {
    qDebug() << "SSL handshake complete, connection encrypted!";
});

connect(sslSocket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
        this, [sslSocket](const QList<QSslError> &errors) {
    for (const QSslError &err : errors) {
        qWarning() << "SSL Error:" << err.errorString();
    }
    // ⚠️ 仅开发环境！
    // sslSocket->ignoreSslErrors();
});

sslSocket->connectToHostEncrypted("secure.example.com", 443);
```

### 11.2 服务端 SSL

```cpp
// 重写 incomingConnection
void SslServer::incomingConnection(qintptr descriptor)
{
    QSslSocket *sslSocket = new QSslSocket(this);
    sslSocket->setSocketDescriptor(descriptor);

    // 加载服务端证书和私钥
    sslSocket->setLocalCertificate(":/certs/server.pem");
    sslSocket->setPrivateKey(":/certs/server-key.pem", QSsl::Rsa);
    sslSocket->setProtocol(QSsl::TlsV1_2OrLater);

    connect(sslSocket, &QSslSocket::encrypted, this, [sslSocket]() {
        qDebug() << "Client encrypted:" << sslSocket->peerAddress().toString();
    });

    sslSocket->startServerEncryption();
}
```

---

## 12. 性能调优

### 12.1 Socket 选项

```cpp
// 禁用 Nagle 算法（减少小包延迟，适合实时通信）
socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

// 启用 TCP KeepAlive
socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

// 设置收发缓冲区大小（字节）
socket->setReadBufferSize(256 * 1024);   // 256KB 读缓冲
socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 256 * 1024);
socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 256 * 1024);
```

### 12.2 Nagle 算法 vs 低延迟

| 选项 | 行为 | 适用场景 |
|------|------|---------|
| Nagle 开启（默认） | 小数据包会等待一段时间再发送，合并成大包 | 批量数据传输（文件传输） |
| `LowDelayOption = 1`（Nagle 关闭） | 数据立即发送，不等待合并 | 实时通信（游戏、聊天、远程桌面） |

### 12.3 服务端最大连接数

```cpp
// 设置最大待处理连接队列（listen backlog）
server->setMaxPendingConnections(128);  // 默认 30

// 系统级文件描述符限制（Linux）
// ulimit -n 65535
// 或在代码中：
#ifdef Q_OS_LINUX
#include <sys/resource.h>
struct rlimit rl;
rl.rlim_cur = 65535;
rl.rlim_max = 65535;
setrlimit(RLIMIT_NOFILE, &rl);
#endif
```

---

## 13. 最佳实践与常见陷阱

### 13.1 核心原则

| 原则 | 说明 |
|------|------|
| **异步优先** | 主线程中永远使用信号槽，`waitFor*()` 仅在工作线程中使用 |
| **处理粘包** | 必须实现消息协议（长度前缀 / 分隔符 / QDataStream 事务） |
| **释放资源** | 断开连接后用 `deleteLater()` 释放 `QTcpSocket` |
| **不跨线程** | `QTcpSocket` 必须在创建它的线程中使用 |
| **设置超时** | 连接和读取都应有超时机制，避免永久挂起 |
| **限制缓冲区** | 防止恶意客户端发送超大数据耗尽内存 |
| **心跳保活** | 长连接务必实现应用层心跳，检测连接有效性 |

### 13.2 常见陷阱速查

| 陷阱 | 症状 | 解决方案 |
|------|------|---------|
| 未处理粘包 | 收到的数据不完整或多条消息粘在一起 | 实现消息协议（§4） |
| 主线程 `waitFor*()` | UI 冻结卡死 | 改用异步信号槽 |
| 跨线程使用 Socket | 崩溃或未定义行为 | 在目标线程中通过 `setSocketDescriptor()` 创建 |
| 忘记 `deleteLater()` | 内存泄漏 | 在 `disconnected` 信号中释放 |
| `readAll()` 假设完整 | 数据截断 | 使用缓冲区累积，按协议解析 |
| `write()` 后立即 `close()` | 数据丢失 | 用 `disconnectFromHost()`（等缓冲区清空）或监听 `bytesWritten` |
| 服务端未清理断开的 socket | 资源泄漏、列表膨胀 | 在 `disconnected` 中从列表移除并 `deleteLater()` |
| 忽略 `errorOccurred` 信号 | 连接失败时无反馈 | 始终连接错误信号并处理 |
| 连接池未复用 | 频繁创建/销毁 socket | 维护长连接 + 心跳 |
| 未设置读缓冲区上限 | 恶意客户端 DoS | `setReadBufferSize()` + 应用层长度校验 |

---

## 14. API 速查表

### QTcpSocket 主要 API

```cpp
// 连接
void connectToHost(const QString &hostName, quint16 port,
                   OpenMode mode = ReadWrite);
void disconnectFromHost();
void abort();  // 立即断开，丢弃缓冲区

// 状态
SocketState state() const;
bool isValid() const;
QHostAddress peerAddress() const;
quint16 peerPort() const;
QHostAddress localAddress() const;
quint16 localPort() const;
qintptr socketDescriptor() const;

// 读写（继承自 QIODevice）
qint64 write(const QByteArray &data);
QByteArray readAll();
qint64 read(char *data, qint64 maxSize);
qint64 bytesAvailable() const;
qint64 bytesToWrite() const;
bool canReadLine() const;
QByteArray readLine(qint64 maxSize = 0);

// 选项
void setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value);
void setReadBufferSize(qint64 size);

// 阻塞模式（仅用于非 GUI 线程）
bool waitForConnected(int msecs = 30000);
bool waitForReadyRead(int msecs = 30000);
bool waitForBytesWritten(int msecs = 30000);
bool waitForDisconnected(int msecs = 30000);

// 信号
void connected();
void disconnected();
void readyRead();
void bytesWritten(qint64 bytes);
void stateChanged(QAbstractSocket::SocketState state);
void errorOccurred(QAbstractSocket::SocketError error);  // Qt 5.15+
```

### QTcpServer 主要 API

```cpp
// 监听
bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
void close();
bool isListening() const;
quint16 serverPort() const;
QHostAddress serverAddress() const;

// 连接管理
bool hasPendingConnections() const;
QTcpSocket *nextPendingConnection();
void setMaxPendingConnections(int numConnections);

// 可重写
virtual void incomingConnection(qintptr socketDescriptor);

// 信号
void newConnection();
void acceptError(QAbstractSocket::SocketError socketError);
```
