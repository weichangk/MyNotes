# UDP 通信

Qt 通过 `QUdpSocket` 提供 UDP（User Datagram Protocol）通信能力。UDP 是**无连接**的轻量级传输协议——不需要建立连接、没有重传机制、没有顺序保证，但延迟极低、支持广播和组播，适合实时音视频、游戏同步、设备发现、日志推送等场景。

> **模块依赖：** 在 `.pro` 中添加 `QT += network`；CMake 中添加 `find_package(Qt5 COMPONENTS Network)` + `target_link_libraries(... Qt5::Network)`。

---

## 1. TCP vs UDP 对比

| 特性 | TCP | UDP |
|------|-----|-----|
| 连接模式 | 面向连接（三次握手） | 无连接 |
| 可靠性 | 保证到达、保证顺序、自动重传 | 不保证到达、不保证顺序、无重传 |
| 数据边界 | 字节流（无边界） | 数据报（有边界，一次发送 = 一次接收） |
| 延迟 | 较高（握手 + 拥塞控制） | 极低 |
| 广播/组播 | 不支持 | 支持 |
| 头部开销 | 20 字节 | 8 字节 |
| 粘包问题 | 有（需要自定义消息协议） | 无（每个数据报独立） |
| 典型场景 | HTTP、文件传输、数据库连接 | DNS、DHCP、实时音视频、游戏、设备发现 |

> **UDP 无粘包问题：** 每次 `writeDatagram()` 发送的数据报在接收端会作为完整的一条 `readDatagram()` 接收，不会合并也不会拆分。但单个数据报的大小受 MTU 限制（通常 ~1472 字节，超过会被 IP 层分片）。

---

## 2. 核心类

| 类 | 职责 |
|---|---|
| `QUdpSocket` | UDP 套接字，继承 `QAbstractSocket` → `QIODevice`，收发数据报 |
| `QHostAddress` | IP 地址封装（IPv4/IPv6），含 `Broadcast`、`Any`、`LocalHost` 等特殊地址 |
| `QNetworkDatagram` | Qt 5.8+ 引入，封装一个完整的 UDP 数据报（数据 + 源地址/端口 + 目标地址/端口 + TTL 等） |
| `QNetworkInterface` | 查询本机网卡信息，用于组播时选择网卡 |

```
QIODevice
└── QAbstractSocket
    ├── QTcpSocket       ← 面向连接
    └── QUdpSocket       ← 无连接（数据报）
            ├─ writeDatagram()     → 发送数据报
            ├─ readDatagram()      → 接收数据报
            ├─ joinMulticastGroup() → 加入组播组
            └─ readyRead 信号      → 有数据报到达
```

---

## 3. 单播（Unicast）

### 3.1 发送端

发送 UDP 数据报不需要绑定端口，也不需要建立连接：

```cpp
#include <QUdpSocket>

QUdpSocket *socket = new QUdpSocket(this);

// 向指定地址和端口发送数据
QByteArray data = "Hello UDP!";
socket->writeDatagram(data, QHostAddress("192.168.1.100"), 9000);
```

### 3.2 接收端

接收端必须先 `bind()` 绑定一个端口：

```cpp
QUdpSocket *socket = new QUdpSocket(this);

// 绑定端口 9000，监听所有网卡
if (!socket->bind(QHostAddress::Any, 9000)) {
    qCritical() << "Bind failed:" << socket->errorString();
    return;
}
qDebug() << "Listening on port 9000";

// 有数据到达时
connect(socket, &QUdpSocket::readyRead, this, [socket]() {
    while (socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        socket->readDatagram(data.data(), data.size(), &sender, &senderPort);
        qDebug() << "From" << sender.toString() << ":" << senderPort
                 << "Data:" << data;
    }
});
```

### 3.3 使用 QNetworkDatagram（Qt 5.8+，推荐）

`QNetworkDatagram` 比原始的 `readDatagram()` / `writeDatagram()` 更易用，携带完整的元信息：

```cpp
// 发送
QNetworkDatagram datagram(QByteArray("Hello!"),
                          QHostAddress("192.168.1.100"), 9000);
socket->writeDatagram(datagram);

// 接收
connect(socket, &QUdpSocket::readyRead, this, [socket]() {
    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        qDebug() << "From:" << datagram.senderAddress().toString()
                 << ":" << datagram.senderPort();
        qDebug() << "To:" << datagram.destinationAddress().toString()
                 << ":" << datagram.destinationPort();
        qDebug() << "Data:" << datagram.data();

        // 快捷回复（自动交换源/目标地址）
        socket->writeDatagram(datagram.makeReply("ACK"));
    }
});
```

> **`makeReply()` 的便利：** 自动将原数据报的发送方地址/端口设为回复的目标，减少手动组装。

### 3.4 双向通信完整示例

```cpp
// udp_peer.h — 既能发也能收的 UDP 对等端
#pragma once
#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>

class UdpPeer : public QObject
{
    Q_OBJECT
public:
    explicit UdpPeer(quint16 localPort, QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QUdpSocket(this))
    {
        if (!m_socket->bind(QHostAddress::Any, localPort)) {
            qCritical() << "Bind failed:" << m_socket->errorString();
            return;
        }
        qDebug() << "Bound to port" << localPort;

        connect(m_socket, &QUdpSocket::readyRead, this, [this]() {
            while (m_socket->hasPendingDatagrams()) {
                QNetworkDatagram dg = m_socket->receiveDatagram();
                emit messageReceived(dg.senderAddress().toString(),
                                     dg.senderPort(),
                                     dg.data());
            }
        });
    }

    void sendTo(const QString &host, quint16 port, const QByteArray &data)
    {
        m_socket->writeDatagram(data, QHostAddress(host), port);
    }

signals:
    void messageReceived(const QString &senderAddr,
                         quint16 senderPort,
                         const QByteArray &data);

private:
    QUdpSocket *m_socket;
};
```

使用：

```cpp
// 节点 A 监听 9001
auto *peerA = new UdpPeer(9001, this);

// 节点 B 监听 9002
auto *peerB = new UdpPeer(9002, this);

// A → B
peerA->sendTo("127.0.0.1", 9002, "Hello from A");

// B 收到后回复
connect(peerB, &UdpPeer::messageReceived,
        this, [peerB](const QString &addr, quint16 port, const QByteArray &data) {
    qDebug() << "B received:" << data;
    peerB->sendTo(addr, port, "Hello from B");
});
```

---

## 4. 广播（Broadcast）

### 4.1 概念

广播将数据报发送到**同一子网内**的所有主机。目标地址使用 `QHostAddress::Broadcast`（即 `255.255.255.255`）或子网广播地址（如 `192.168.1.255`）。

```
发送端 ──广播──→ 子网内所有主机（绑定了相应端口的都能收到）
```

### 4.2 广播发送端

```cpp
QUdpSocket *socket = new QUdpSocket(this);

QByteArray data = "Broadcast message!";
socket->writeDatagram(data, QHostAddress::Broadcast, 9000);

// 或使用子网广播地址（更精确）
socket->writeDatagram(data, QHostAddress("192.168.1.255"), 9000);
```

### 4.3 广播接收端

接收广播时，绑定方式需注意：

```cpp
QUdpSocket *socket = new QUdpSocket(this);

// 方式 1：绑定到 Any（推荐，同时收单播和广播）
socket->bind(QHostAddress::Any, 9000);

// 方式 2：仅绑定到 0.0.0.0（效果同 Any）
socket->bind(QHostAddress(QHostAddress::AnyIPv4), 9000);

connect(socket, &QUdpSocket::readyRead, this, [socket]() {
    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram dg = socket->receiveDatagram();
        qDebug() << "Broadcast from:" << dg.senderAddress().toString()
                 << "Data:" << dg.data();
    }
});
```

### 4.4 设备发现示例

局域网设备发现（类似 SSDP / mDNS 的简化版）：

```cpp
// === 发现端（客户端）：发送广播查询 ===
class DeviceDiscovery : public QObject
{
    Q_OBJECT
public:
    explicit DeviceDiscovery(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QUdpSocket(this))
    {
        // 绑定随机端口以接收回复
        m_socket->bind(QHostAddress::Any, 0);

        connect(m_socket, &QUdpSocket::readyRead, this, [this]() {
            while (m_socket->hasPendingDatagrams()) {
                QNetworkDatagram dg = m_socket->receiveDatagram();
                QJsonDocument doc = QJsonDocument::fromJson(dg.data());
                if (doc.isObject()) {
                    QJsonObject info = doc.object();
                    emit deviceFound(dg.senderAddress().toString(),
                                     info["name"].toString(),
                                     info["port"].toInt());
                }
            }
        });
    }

    void discover()
    {
        QByteArray query = R"({"type":"discover"})";
        m_socket->writeDatagram(query, QHostAddress::Broadcast, 8888);
        qDebug() << "Discovery broadcast sent";
    }

signals:
    void deviceFound(const QString &address,
                     const QString &name, int port);

private:
    QUdpSocket *m_socket;
};

// === 被发现端（服务端）：监听并回应 ===
class DeviceResponder : public QObject
{
    Q_OBJECT
public:
    explicit DeviceResponder(const QString &deviceName, int servicePort,
                             QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QUdpSocket(this))
    {
        m_socket->bind(QHostAddress::Any, 8888,
                       QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

        connect(m_socket, &QUdpSocket::readyRead, this, [=]() {
            while (m_socket->hasPendingDatagrams()) {
                QNetworkDatagram dg = m_socket->receiveDatagram();
                QJsonDocument doc = QJsonDocument::fromJson(dg.data());
                if (doc.object()["type"].toString() == "discover") {
                    // 回复设备信息
                    QJsonObject reply;
                    reply["name"] = deviceName;
                    reply["port"] = servicePort;
                    QByteArray data = QJsonDocument(reply).toJson(
                        QJsonDocument::Compact);
                    m_socket->writeDatagram(
                        dg.makeReply(data));
                }
            }
        });

        qDebug() << "Device responder ready:" << deviceName;
    }

private:
    QUdpSocket *m_socket;
};
```

使用：

```cpp
// 服务端：声明自己
auto *responder = new DeviceResponder("MyPrinter", 9100, this);

// 客户端：发现设备
auto *discovery = new DeviceDiscovery(this);
connect(discovery, &DeviceDiscovery::deviceFound,
        this, [](const QString &addr, const QString &name, int port) {
    qDebug() << "Found device:" << name << "at" << addr << ":" << port;
});
discovery->discover();
```

---

## 5. 组播（Multicast）

### 5.1 概念

组播（多播）将数据报发送到一个**组播组**（Multicast Group），只有加入该组的主机才能收到。相比广播，组播可以**跨子网**（需路由器支持 IGMP）、**不会骚扰无关主机**。

```
发送端 ──→ 组播组 224.1.1.100:9000
              ├─ 接收端 A（已加入组）  ✅ 收到
              ├─ 接收端 B（已加入组）  ✅ 收到
              └─ 接收端 C（未加入组）  ❌ 收不到
```

**组播地址范围（IPv4）：**

| 地址范围 | 用途 |
|---------|------|
| `224.0.0.0` – `224.0.0.255` | 本地链路（局域网），TTL=1，不被路由器转发 |
| `224.0.1.0` – `238.255.255.255` | 可路由的组播地址 |
| `239.0.0.0` – `239.255.255.255` | 管理域范围（组织内部使用，推荐） |

### 5.2 组播接收端

```cpp
QUdpSocket *socket = new QUdpSocket(this);

// 1. 绑定端口（必须使用 ShareAddress 允许多进程共享端口）
socket->bind(QHostAddress::AnyIPv4, 9000,
             QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

// 2. 加入组播组
QHostAddress groupAddr("239.255.43.21");
bool ok = socket->joinMulticastGroup(groupAddr);
if (!ok) {
    qWarning() << "Failed to join multicast group:" << socket->errorString();
}

qDebug() << "Joined multicast group" << groupAddr.toString() << "on port 9000";

// 3. 接收数据
connect(socket, &QUdpSocket::readyRead, this, [socket]() {
    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram dg = socket->receiveDatagram();
        qDebug() << "Multicast from:" << dg.senderAddress().toString()
                 << "Data:" << dg.data();
    }
});
```

### 5.3 组播发送端

发送端不需要加入组播组，直接向组播地址发送即可：

```cpp
QUdpSocket *socket = new QUdpSocket(this);

QHostAddress groupAddr("239.255.43.21");

// 设置 TTL（Time To Live）
// TTL=1 → 仅限本地子网；TTL>1 → 可跨路由器（需路由器支持 IGMP）
socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);

// 可选：禁止回环（发送端自己不收到）
socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);

QByteArray data = "Multicast message!";
socket->writeDatagram(data, groupAddr, 9000);
```

### 5.4 指定组播网卡

多网卡主机上，需要指定通过哪个网卡收发组播数据：

```cpp
// 方式 1：加入组播组时指定网卡
QNetworkInterface iface = QNetworkInterface::interfaceFromName("eth0");
socket->joinMulticastGroup(groupAddr, iface);

// 方式 2：设置发送组播的出接口
socket->setMulticastInterface(iface);

// 查找合适的网卡
QNetworkInterface findMulticastInterface()
{
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            iface.flags().testFlag(QNetworkInterface::IsRunning) &&
            iface.flags().testFlag(QNetworkInterface::CanMulticast) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            return iface;
        }
    }
    return QNetworkInterface();  // 使用系统默认
}
```

### 5.5 离开组播组

```cpp
socket->leaveMulticastGroup(groupAddr);
// 或指定网卡离开
socket->leaveMulticastGroup(groupAddr, iface);
```

### 5.6 完整组播示例：实时股票行情推送

```cpp
// === 行情推送端 ===
class StockPublisher : public QObject
{
    Q_OBJECT
public:
    explicit StockPublisher(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QUdpSocket(this))
        , m_timer(new QTimer(this))
    {
        m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);

        connect(m_timer, &QTimer::timeout, this, &StockPublisher::publish);
        m_timer->start(1000);  // 每秒推送一次
    }

private:
    void publish()
    {
        QJsonObject quote;
        quote["symbol"] = "AAPL";
        quote["price"] = 150.25 + (QRandomGenerator::global()->bounded(200) - 100) / 100.0;
        quote["volume"] = QRandomGenerator::global()->bounded(1000000);
        quote["timestamp"] = QDateTime::currentMSecsSinceEpoch();

        QByteArray data = QJsonDocument(quote).toJson(QJsonDocument::Compact);
        m_socket->writeDatagram(data, QHostAddress("239.255.43.21"), 9000);
    }

    QUdpSocket *m_socket;
    QTimer *m_timer;
};

// === 行情订阅端 ===
class StockSubscriber : public QObject
{
    Q_OBJECT
public:
    explicit StockSubscriber(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QUdpSocket(this))
    {
        m_socket->bind(QHostAddress::AnyIPv4, 9000,
                       QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        m_socket->joinMulticastGroup(QHostAddress("239.255.43.21"));

        connect(m_socket, &QUdpSocket::readyRead, this, [this]() {
            while (m_socket->hasPendingDatagrams()) {
                QNetworkDatagram dg = m_socket->receiveDatagram();
                QJsonObject quote = QJsonDocument::fromJson(dg.data()).object();
                emit quoteReceived(
                    quote["symbol"].toString(),
                    quote["price"].toDouble(),
                    quote["volume"].toInt());
            }
        });
    }

signals:
    void quoteReceived(const QString &symbol, double price, int volume);

private:
    QUdpSocket *m_socket;
};
```

---

## 6. 广播 vs 组播对比

| 特性 | 广播（Broadcast） | 组播（Multicast） |
|------|-------------------|-------------------|
| 目标 | 子网内所有主机 | 仅加入组的主机 |
| 地址 | `255.255.255.255` 或子网广播 | `224.0.0.0` – `239.255.255.255` |
| 跨子网 | ❌ 不能跨路由器 | ✅ 可跨路由器（需 IGMP） |
| 网络负担 | 高（所有主机都要处理） | 低（只有组成员处理） |
| 使用场景 | 局域网设备发现 | 实时数据分发、流媒体 |
| 绑定标志 | 正常绑定即可 | 需要 `ShareAddress` |

---

## 7. 数据报大小限制与分片

### 7.1 MTU 与分片

```
以太网 MTU = 1500 字节
  - IP 头：20 字节
  - UDP 头：8 字节
  ────────────────
  最大 payload：1472 字节（不分片）
```

| 大小 | 行为 |
|------|------|
| ≤ 1472 字节 | 单个 IP 包传输，最可靠 |
| 1473 – 65507 字节 | IP 层分片，任一片丢失则整个数据报丢失 |
| > 65507 字节 | `writeDatagram()` 失败 |

> **最佳实践：** 保持 UDP 数据报 payload ≤ 1400 字节（留出 VPN/隧道等额外头部空间），避免 IP 分片。

### 7.2 大数据传输策略

如果需要通过 UDP 传输大于 MTU 的数据，应在应用层自行分片：

```cpp
// 应用层分片发送
void sendLargeData(QUdpSocket *socket, const QHostAddress &host,
                   quint16 port, const QByteArray &data)
{
    const int MAX_CHUNK = 1400;
    quint32 totalChunks = (data.size() + MAX_CHUNK - 1) / MAX_CHUNK;
    quint32 msgId = QRandomGenerator::global()->generate();

    for (quint32 i = 0; i < totalChunks; ++i) {
        QByteArray chunk;
        QDataStream stream(&chunk, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);

        // 头部：消息ID(4) + 分片序号(4) + 总分片数(4) = 12 字节
        stream << msgId << i << totalChunks;

        // 数据体
        int offset = i * MAX_CHUNK;
        int len = qMin(MAX_CHUNK, data.size() - offset);
        chunk.append(data.mid(offset, len));

        socket->writeDatagram(chunk, host, port);
    }
}

// 应用层分片重组
class DatagramAssembler : public QObject
{
    Q_OBJECT
public:
    void addChunk(const QByteArray &raw)
    {
        if (raw.size() < 12) return;

        QDataStream stream(raw.left(12));
        stream.setByteOrder(QDataStream::BigEndian);
        quint32 msgId, index, total;
        stream >> msgId >> index >> total;

        if (total == 0 || index >= total) return;

        auto &entry = m_pending[msgId];
        entry.total = total;
        entry.chunks[index] = raw.mid(12);

        if (entry.chunks.size() == static_cast<int>(total)) {
            // 所有分片已到达，重组
            QByteArray assembled;
            for (quint32 i = 0; i < total; ++i) {
                assembled.append(entry.chunks[i]);
            }
            m_pending.remove(msgId);
            emit assembled(assembled);
        }
    }

signals:
    void assembled(const QByteArray &data);

private:
    struct PendingMessage {
        quint32 total = 0;
        QMap<quint32, QByteArray> chunks;
    };
    QMap<quint32, PendingMessage> m_pending;
};
```

---

## 8. 可靠 UDP 模式

原生 UDP 不保证到达和顺序，但某些场景需要一定可靠性（如游戏中的关键操作）。可在应用层实现简单的确认重传：

### 8.1 ACK + 超时重传

```cpp
class ReliableUdpSender : public QObject
{
    Q_OBJECT
public:
    explicit ReliableUdpSender(QUdpSocket *socket, QObject *parent = nullptr)
        : QObject(parent), m_socket(socket)
    {
        connect(&m_retryTimer, &QTimer::timeout,
                this, &ReliableUdpSender::retransmit);
        m_retryTimer.setInterval(500);  // 500ms 重传间隔
    }

    void sendReliable(const QHostAddress &host, quint16 port,
                      const QByteArray &data)
    {
        quint32 seqNum = m_nextSeq++;

        QByteArray packet;
        QDataStream stream(&packet, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << quint8(0x01);   // 类型：DATA
        stream << seqNum;
        packet.append(data);

        PendingPacket pkt;
        pkt.host = host;
        pkt.port = port;
        pkt.data = packet;
        pkt.retries = 0;
        m_pending[seqNum] = pkt;

        m_socket->writeDatagram(packet, host, port);

        if (!m_retryTimer.isActive()) {
            m_retryTimer.start();
        }
    }

    void handleAck(quint32 seqNum)
    {
        m_pending.remove(seqNum);
        if (m_pending.isEmpty()) {
            m_retryTimer.stop();
        }
    }

private:
    void retransmit()
    {
        auto it = m_pending.begin();
        while (it != m_pending.end()) {
            if (it->retries >= 5) {
                qWarning() << "Packet" << it.key() << "lost after 5 retries";
                emit packetLost(it.key());
                it = m_pending.erase(it);
            } else {
                m_socket->writeDatagram(it->data, it->host, it->port);
                ++it->retries;
                ++it;
            }
        }
    }

signals:
    void packetLost(quint32 seqNum);

private:
    struct PendingPacket {
        QHostAddress host;
        quint16 port;
        QByteArray data;
        int retries;
    };

    QUdpSocket *m_socket;
    QTimer m_retryTimer;
    QMap<quint32, PendingPacket> m_pending;
    quint32 m_nextSeq = 0;
};
```

**接收端回复 ACK：**

```cpp
void handleDatagram(QUdpSocket *socket, const QNetworkDatagram &dg)
{
    const QByteArray &raw = dg.data();
    if (raw.size() < 5) return;

    QDataStream stream(raw.left(5));
    stream.setByteOrder(QDataStream::BigEndian);
    quint8 type;
    quint32 seqNum;
    stream >> type >> seqNum;

    if (type == 0x01) {  // DATA
        // 处理数据
        QByteArray payload = raw.mid(5);
        processPayload(payload);

        // 回复 ACK
        QByteArray ack;
        QDataStream ackStream(&ack, QIODevice::WriteOnly);
        ackStream.setByteOrder(QDataStream::BigEndian);
        ackStream << quint8(0x02);  // 类型：ACK
        ackStream << seqNum;
        socket->writeDatagram(dg.makeReply(ack));
    }
}
```

---

## 9. 错误处理

### 9.1 常见错误

| 错误 | 含义 | 解决方案 |
|------|------|---------|
| `AddressInUseError` | 端口被占用 | 使用 `ShareAddress` 标志 / 换端口 |
| `SocketAccessError` | 权限不足 | Windows 防火墙 / Linux 需 root |
| `NetworkError` | 网络不可达 | 检查网卡状态 |
| `DatagramTooLargeError` | 数据报超过 65507 字节 | 应用层分片 |
| `SocketResourceError` | 文件描述符耗尽 | 检查资源泄漏 |

### 9.2 绑定标志

```cpp
// 默认绑定（独占端口）
socket->bind(QHostAddress::Any, 9000);

// 允许多进程/多 socket 共享同一端口（组播/广播必须）
socket->bind(QHostAddress::Any, 9000,
             QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

// 仅绑定 IPv4
socket->bind(QHostAddress::AnyIPv4, 9000);

// 仅绑定 IPv6
socket->bind(QHostAddress::AnyIPv6, 9000);
```

| 标志 | 说明 |
|------|------|
| `QUdpSocket::ShareAddress` | 允许其他进程/socket 绑定同一端口（对应 `SO_REUSEADDR`） |
| `QUdpSocket::ReuseAddressHint` | 提示操作系统可复用 TIME_WAIT 状态的端口 |
| `QUdpSocket::DontShareAddress` | 独占端口（默认） |

### 9.3 错误处理模式

```cpp
connect(socket, &QAbstractSocket::errorOccurred,
        this, [](QAbstractSocket::SocketError err) {
    switch (err) {
    case QAbstractSocket::AddressInUseError:
        qWarning() << "端口被占用，尝试其他端口";
        break;
    case QAbstractSocket::SocketAccessError:
        qWarning() << "权限不足，请检查防火墙设置";
        break;
    case QAbstractSocket::NetworkError:
        qWarning() << "网络不可达，请检查网络连接";
        break;
    case QAbstractSocket::DatagramTooLargeError:
        qWarning() << "数据报过大，请分片发送";
        break;
    default:
        qWarning() << "未知错误:" << err;
        break;
    }
});
```

---

## 10. 性能调优

### 10.1 收发缓冲区

```cpp
// 增大系统级收发缓冲区（减少高频率数据丢包）
socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption,
                        512 * 1024);  // 512KB
socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption,
                        512 * 1024);
```

### 10.2 高频数据优化

```cpp
// 减少信号开销：一次 readyRead 中读完所有数据报
connect(socket, &QUdpSocket::readyRead, this, [socket]() {
    // 循环读取所有待处理数据报
    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram dg = socket->receiveDatagram();
        // 批量处理...
    }
});

// 提示：QUdpSocket::readyRead 不保证每个数据报触发一次！
// 可能多个数据报到达但只触发一次 readyRead，
// 所以必须用 while + hasPendingDatagrams() 循环读取。
```

### 10.3 批量发送

```cpp
// 多个小数据报 → 合并成一个（减少系统调用次数）
QJsonArray batch;
for (const auto &item : items) {
    QJsonObject obj;
    obj["id"] = item.id;
    obj["value"] = item.value;
    batch.append(obj);
}
QByteArray data = QJsonDocument(batch).toJson(QJsonDocument::Compact);
socket->writeDatagram(data, targetHost, targetPort);
```

---

## 11. 实战：局域网屏幕共享（帧推送）

一个简化的屏幕截图组播推送示例：

```cpp
// === 推送端（截图 + 压缩 + 组播） ===
class ScreenCaster : public QObject
{
    Q_OBJECT
public:
    explicit ScreenCaster(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QUdpSocket(this))
        , m_timer(new QTimer(this))
    {
        m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);

        connect(m_timer, &QTimer::timeout, this, &ScreenCaster::captureAndSend);
    }

    void start(int fps = 10)
    {
        m_timer->start(1000 / fps);
        qDebug() << "Screen casting at" << fps << "FPS";
    }

    void stop() { m_timer->stop(); }

private:
    void captureAndSend()
    {
        // 截图并缩放
        QScreen *screen = QGuiApplication::primaryScreen();
        if (!screen) return;
        QPixmap pixmap = screen->grabWindow(0);
        QPixmap scaled = pixmap.scaled(640, 360, Qt::KeepAspectRatio);

        // 压缩为 JPEG
        QByteArray jpegData;
        QBuffer buffer(&jpegData);
        buffer.open(QIODevice::WriteOnly);
        scaled.save(&buffer, "JPEG", 50);  // 质量 50%

        // 若超过 MTU 则应用层分片发送
        const QHostAddress group("239.255.43.22");
        const quint16 port = 9100;

        if (jpegData.size() <= 1400) {
            QByteArray packet;
            QDataStream s(&packet, QIODevice::WriteOnly);
            s.setByteOrder(QDataStream::BigEndian);
            s << quint32(m_frameId) << quint16(1) << quint16(0);  // id, total, index
            packet.append(jpegData);
            m_socket->writeDatagram(packet, group, port);
        } else {
            // 分片
            const int CHUNK = 1392;  // 1400 - 8 字节头
            int total = (jpegData.size() + CHUNK - 1) / CHUNK;
            for (int i = 0; i < total; ++i) {
                QByteArray packet;
                QDataStream s(&packet, QIODevice::WriteOnly);
                s.setByteOrder(QDataStream::BigEndian);
                s << quint32(m_frameId) << quint16(total) << quint16(i);
                packet.append(jpegData.mid(i * CHUNK, CHUNK));
                m_socket->writeDatagram(packet, group, port);
            }
        }
        ++m_frameId;
    }

    QUdpSocket *m_socket;
    QTimer *m_timer;
    quint32 m_frameId = 0;
};

// === 接收端 ===
class ScreenViewer : public QObject
{
    Q_OBJECT
public:
    explicit ScreenViewer(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(new QUdpSocket(this))
    {
        m_socket->bind(QHostAddress::AnyIPv4, 9100,
                       QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        m_socket->joinMulticastGroup(QHostAddress("239.255.43.22"));

        connect(m_socket, &QUdpSocket::readyRead, this, [this]() {
            while (m_socket->hasPendingDatagrams()) {
                QNetworkDatagram dg = m_socket->receiveDatagram();
                processPacket(dg.data());
            }
        });
    }

signals:
    void frameReady(const QPixmap &pixmap);

private:
    void processPacket(const QByteArray &raw)
    {
        if (raw.size() < 8) return;
        QDataStream s(raw.left(8));
        s.setByteOrder(QDataStream::BigEndian);
        quint32 frameId;
        quint16 total, index;
        s >> frameId >> total >> index;

        QByteArray payload = raw.mid(8);

        if (total == 1) {
            // 单包帧
            QPixmap pm;
            pm.loadFromData(payload, "JPEG");
            emit frameReady(pm);
        } else {
            // 多包帧重组
            auto &frame = m_pending[frameId];
            frame.total = total;
            frame.chunks[index] = payload;

            if (frame.chunks.size() == total) {
                QByteArray assembled;
                for (quint16 i = 0; i < total; ++i) {
                    assembled.append(frame.chunks[i]);
                }
                QPixmap pm;
                pm.loadFromData(assembled, "JPEG");
                emit frameReady(pm);
                m_pending.remove(frameId);
            }
        }

        // 清理过时的不完整帧
        auto it = m_pending.begin();
        while (it != m_pending.end()) {
            if (it.key() + 30 < frameId) {
                it = m_pending.erase(it);
            } else {
                ++it;
            }
        }
    }

    struct PendingFrame {
        quint16 total = 0;
        QMap<quint16, QByteArray> chunks;
    };

    QUdpSocket *m_socket;
    QMap<quint32, PendingFrame> m_pending;
};
```

---

## 12. 最佳实践与常见陷阱

### 12.1 核心原则

| 原则 | 说明 |
|------|------|
| **控制数据报大小** | payload ≤ 1400 字节，避免 IP 分片 |
| **循环读取** | `readyRead` 中必须用 `while (hasPendingDatagrams)` 循环读取所有待处理数据报 |
| **组播需 ShareAddress** | `bind()` 时传入 `ShareAddress | ReuseAddressHint`，否则同机多进程冲突 |
| **区分场景** | 局域网发现用广播，数据分发用组播，点对点用单播 |
| **不依赖到达顺序** | UDP 不保证顺序，如需有序则在应用层加序号排序 |
| **不假设可靠送达** | 关键数据需加 ACK 确认 + 重传机制 |
| **设置合理的 TTL** | 组播 TTL=1 限制在本地子网，避免意外泄漏 |

### 12.2 常见陷阱速查

| 陷阱 | 症状 | 解决方案 |
|------|------|---------|
| 未循环读取 | 丢失数据报 | `while (hasPendingDatagrams)` 循环 |
| 组播未用 `ShareAddress` | 第二个进程 bind 失败 | 加 `ShareAddress \| ReuseAddressHint` 标志 |
| 单个数据报过大 | IP 分片导致高丢包率 | 应用层分片，保持 ≤ 1400 字节 |
| 假设 readyRead 与 writeDatagram 一一对应 | 逻辑错误 | 每次 readyRead 可能对应多个数据报 |
| 忘记 `joinMulticastGroup` | 收不到组播数据 | 接收端必须加入组，发送端不需要 |
| 多网卡环境组播失败 | 数据走了错误的网卡 | 显式指定 `setMulticastInterface()` |
| Windows 防火墙阻止 | 本机测试正常，跨机器收不到 | 添加防火墙入站规则 |
| 广播跨子网 | 数据到不了其他网段 | 改用组播 |
| `readDatagram` 缓冲区过小 | 数据截断 | 用 `pendingDatagramSize()` 预分配 / 用 `receiveDatagram()` |

---

## 13. API 速查表

### QUdpSocket 主要 API

```cpp
// 绑定
bool bind(const QHostAddress &address, quint16 port,
          BindMode mode = DefaultForPlatform);
bool bind(quint16 port = 0, BindMode mode = DefaultForPlatform);

// 发送数据报
qint64 writeDatagram(const QByteArray &data,
                     const QHostAddress &host, quint16 port);
qint64 writeDatagram(const QNetworkDatagram &datagram);

// 接收数据报
bool hasPendingDatagrams() const;
qint64 pendingDatagramSize() const;
qint64 readDatagram(char *data, qint64 maxSize,
                    QHostAddress *address = nullptr,
                    quint16 *port = nullptr);
QNetworkDatagram receiveDatagram(qint64 maxSize = -1);  // Qt 5.8+

// 组播
bool joinMulticastGroup(const QHostAddress &groupAddress);
bool joinMulticastGroup(const QHostAddress &groupAddress,
                        const QNetworkInterface &iface);
bool leaveMulticastGroup(const QHostAddress &groupAddress);
bool leaveMulticastGroup(const QHostAddress &groupAddress,
                         const QNetworkInterface &iface);
void setMulticastInterface(const QNetworkInterface &iface);
QNetworkInterface multicastInterface() const;

// Socket 选项
void setSocketOption(QAbstractSocket::SocketOption option,
                     const QVariant &value);
//   MulticastTtlOption       → 组播 TTL
//   MulticastLoopbackOption  → 组播回环開關
//   ReceiveBufferSizeSocketOption → 接收缓冲区
//   SendBufferSizeSocketOption    → 发送缓冲区

// 状态（继承自 QAbstractSocket）
SocketState state() const;
QHostAddress localAddress() const;
quint16 localPort() const;

// 信号（继承自 QAbstractSocket / QIODevice）
void readyRead();
void stateChanged(QAbstractSocket::SocketState state);
void errorOccurred(QAbstractSocket::SocketError error);  // Qt 5.15+
```

### QNetworkDatagram 主要 API（Qt 5.8+）

```cpp
// 构造
QNetworkDatagram();
QNetworkDatagram(const QByteArray &data,
                 const QHostAddress &destinationAddress = QHostAddress(),
                 quint16 port = 0);

// 数据
QByteArray data() const;
void setData(const QByteArray &data);

// 地址信息
QHostAddress senderAddress() const;
quint16 senderPort() const;
QHostAddress destinationAddress() const;
quint16 destinationPort() const;
void setDestination(const QHostAddress &address, quint16 port);
void setSender(const QHostAddress &address, quint16 port = 0);

// 跳数
int hopLimit() const;
void setHopLimit(int count);

// 接口
uint interfaceIndex() const;
void setInterfaceIndex(uint index);

// 快捷回复
QNetworkDatagram makeReply(const QByteArray &payload) const;

// 有效性
bool isValid() const;
bool isNull() const;
```
