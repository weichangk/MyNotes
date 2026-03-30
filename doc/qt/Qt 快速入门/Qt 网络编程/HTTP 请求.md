# HTTP 请求

Qt 通过 `QNetworkAccessManager` 提供高层次的 HTTP 客户端 API，支持 GET / POST / PUT / DELETE / PATCH 等方法，内建 SSL/TLS、Cookie 管理、重定向、代理和认证机制。所有网络 I/O 均为**异步非阻塞**——发起请求后立即返回 `QNetworkReply *`，通过信号槽接收响应，不会卡住 UI 线程。

> **模块依赖：** 在 `.pro` 中添加 `QT += network`；CMake 中添加 `find_package(Qt5 COMPONENTS Network)` + `target_link_libraries(... Qt5::Network)`。

---

## 1. 核心类总览

| 类 | 职责 |
|---|---|
| `QNetworkAccessManager` | 入口管理器，发起请求、管理连接池/Cookie/代理/SSL 配置，整个应用通常只需一个实例 |
| `QNetworkRequest` | 封装一次请求的 URL、Header、超时、SSL 配置等元信息 |
| `QNetworkReply` | 代表一次请求的响应，继承 `QIODevice`，可读取 body、Header、状态码、错误码 |
| `QSslConfiguration` | SSL/TLS 参数（协议版本、证书、CA 列表） |
| `QHttpMultiPart` | 构造 `multipart/form-data` 请求体（文件上传） |
| `QNetworkCookieJar` | Cookie 存储容器（内存版），可继承实现持久化 |
| `QNetworkProxy` | 代理设置（HTTP / SOCKS5） |
| `QAuthenticator` | HTTP 认证（Basic / Digest / NTLM） |

```
┌──────────────────────────────┐
│   QNetworkAccessManager      │  ← 全局单例 / 每线程一个
│   ├─ get(request)            │
│   ├─ post(request, data)     │
│   ├─ put(request, data)      │
│   ├─ deleteResource(request) │
│   ├─ sendCustomRequest(...)  │
│   └─ cookieJar() / setProxy()│
└──────────┬───────────────────┘
           │  返回
     QNetworkReply*
           │  信号
           ├─ finished()
           ├─ readyRead()
           ├─ downloadProgress(qint64, qint64)
           ├─ uploadProgress(qint64, qint64)
           ├─ errorOccurred(QNetworkReply::NetworkError)
           └─ sslErrors(QList<QSslError>)
```

> **生命周期注意：** `QNetworkReply` 由 `QNetworkAccessManager` 创建，但需要**调用方手动释放**。推荐在 `finished()` 槽中调用 `reply->deleteLater()`。

---

## 2. 发起 GET 请求

### 2.1 最简示例

```cpp
// 头文件
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

// 成员变量（推荐持久持有，复用连接池）
QNetworkAccessManager *m_manager = new QNetworkAccessManager(this);

// 发起 GET
QNetworkRequest request(QUrl("https://httpbin.org/get"));
QNetworkReply *reply = m_manager->get(request);

// 连接信号
connect(reply, &QNetworkReply::finished, this, [reply]() {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "Response:" << data;
    } else {
        qWarning() << "Error:" << reply->errorString();
    }
    reply->deleteLater();  // ← 必须释放
});
```

### 2.2 带查询参数的 GET

```cpp
QUrl url("https://api.example.com/search");
QUrlQuery query;
query.addQueryItem("keyword", "Qt网络编程");
query.addQueryItem("page", "1");
query.addQueryItem("size", "20");
url.setQuery(query);  // → ?keyword=Qt%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B&page=1&size=20

QNetworkRequest request(url);
QNetworkReply *reply = m_manager->get(request);
```

> `QUrlQuery` 会自动进行 percent-encoding，无需手动编码中文或特殊字符。

### 2.3 设置请求 Header

```cpp
QNetworkRequest request(QUrl("https://api.example.com/data"));

// 使用预定义枚举
request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
request.setHeader(QNetworkRequest::UserAgentHeader, "MyApp/1.0");

// 使用自定义 Header
request.setRawHeader("Authorization", "Bearer eyJhbGciOiJIUzI...");
request.setRawHeader("X-Custom-Header", "custom-value");
request.setRawHeader("Accept", "application/json");
```

常用预定义 Header 枚举：

| 枚举值 | 对应 HTTP Header |
|--------|-----------------|
| `ContentTypeHeader` | `Content-Type` |
| `ContentLengthHeader` | `Content-Length` |
| `UserAgentHeader` | `User-Agent` |
| `LocationHeader` | `Location`（重定向） |
| `LastModifiedHeader` | `Last-Modified` |
| `CookieHeader` | `Cookie` |
| `SetCookieHeader` | `Set-Cookie` |

---

## 3. 发起 POST 请求

### 3.1 发送 JSON 数据

```cpp
#include <QJsonDocument>
#include <QJsonObject>

QJsonObject json;
json["username"] = "admin";
json["password"] = "123456";
json["remember"] = true;

QByteArray body = QJsonDocument(json).toJson(QJsonDocument::Compact);

QNetworkRequest request(QUrl("https://api.example.com/login"));
request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

QNetworkReply *reply = m_manager->post(request, body);
connect(reply, &QNetworkReply::finished, this, [reply]() {
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject result = doc.object();
        QString token = result["token"].toString();
        qDebug() << "Login token:" << token;
    } else {
        qWarning() << "Login failed:" << reply->errorString();
    }
    reply->deleteLater();
});
```

### 3.2 发送表单数据（application/x-www-form-urlencoded）

```cpp
QUrlQuery params;
params.addQueryItem("username", "admin");
params.addQueryItem("password", "123456");

QByteArray body = params.toString(QUrl::FullyEncoded).toUtf8();

QNetworkRequest request(QUrl("https://api.example.com/login"));
request.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/x-www-form-urlencoded");

QNetworkReply *reply = m_manager->post(request, body);
```

### 3.3 文件上传（multipart/form-data）

```cpp
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>

QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

// 普通文本字段
QHttpPart textPart;
textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                   QVariant("form-data; name=\"description\""));
textPart.setBody("My photo");
multiPart->append(textPart);

// 文件字段
QHttpPart filePart;
filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                   QVariant("image/jpeg"));
filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                   QVariant("form-data; name=\"file\"; filename=\"photo.jpg\""));

QFile *file = new QFile("C:/photos/photo.jpg");
file->open(QIODevice::ReadOnly);
filePart.setBodyDevice(file);
file->setParent(multiPart);  // multiPart 释放时自动关闭文件
multiPart->append(filePart);

QNetworkRequest request(QUrl("https://api.example.com/upload"));
QNetworkReply *reply = m_manager->post(request, multiPart);
multiPart->setParent(reply);  // reply 释放时自动释放 multiPart

connect(reply, &QNetworkReply::finished, this, [reply]() {
    qDebug() << "Upload status:" << reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater();
});

// 上传进度
connect(reply, &QNetworkReply::uploadProgress,
        this, [](qint64 sent, qint64 total) {
    if (total > 0) {
        qDebug() << "Upload progress:" << sent * 100 / total << "%";
    }
});
```

> **关键：** 使用 `setBodyDevice()` 而非 `setBody()` 来避免将整个文件读入内存，对大文件友好。

---

## 4. PUT / DELETE / PATCH 请求

### 4.1 PUT（更新资源）

```cpp
QJsonObject json;
json["name"] = "New Name";
json["email"] = "new@example.com";

QByteArray body = QJsonDocument(json).toJson(QJsonDocument::Compact);

QNetworkRequest request(QUrl("https://api.example.com/users/42"));
request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

QNetworkReply *reply = m_manager->put(request, body);
```

### 4.2 DELETE（删除资源）

```cpp
QNetworkRequest request(QUrl("https://api.example.com/users/42"));
request.setRawHeader("Authorization", "Bearer eyJhbGci...");

QNetworkReply *reply = m_manager->deleteResource(request);
```

### 4.3 PATCH / 自定义方法（sendCustomRequest）

`QNetworkAccessManager` 没有直接的 `patch()` 方法，使用 `sendCustomRequest()`：

```cpp
QJsonObject json;
json["status"] = "active";
QByteArray body = QJsonDocument(json).toJson(QJsonDocument::Compact);

QNetworkRequest request(QUrl("https://api.example.com/users/42"));
request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

// 自定义 HTTP 方法
QBuffer *buffer = new QBuffer;
buffer->setData(body);
buffer->open(QIODevice::ReadOnly);

QNetworkReply *reply = m_manager->sendCustomRequest(request, "PATCH", buffer);
buffer->setParent(reply);  // 随 reply 释放

connect(reply, &QNetworkReply::finished, this, [reply]() {
    qDebug() << "PATCH status:" << reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater();
});
```

---

## 5. 响应处理

### 5.1 读取状态码与 Header

```cpp
connect(reply, &QNetworkReply::finished, this, [reply]() {
    // HTTP 状态码
    int statusCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString reasonPhrase = reply->attribute(
        QNetworkRequest::HttpReasonPhraseAttribute).toString();
    qDebug() << statusCode << reasonPhrase;  // 200 "OK"

    // 读取所有响应 Header
    const QList<QByteArray> headers = reply->rawHeaderList();
    for (const QByteArray &header : headers) {
        qDebug() << header << ":" << reply->rawHeader(header);
    }

    // 读取特定 Header
    QByteArray contentType = reply->header(
        QNetworkRequest::ContentTypeHeader).toByteArray();

    // 读取 Body
    QByteArray body = reply->readAll();

    reply->deleteLater();
});
```

### 5.2 解析 JSON 响应

```cpp
connect(reply, &QNetworkReply::finished, this, [reply]() {
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        reply->deleteLater();
        return;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        qDebug() << "code:" << obj["code"].toInt();
        qDebug() << "message:" << obj["message"].toString();

        // 嵌套数组
        QJsonArray items = obj["data"].toArray();
        for (const QJsonValue &val : items) {
            QJsonObject item = val.toObject();
            qDebug() << "  id:" << item["id"].toInt()
                     << "name:" << item["name"].toString();
        }
    }

    reply->deleteLater();
});
```

### 5.3 流式读取大响应（readyRead）

对于大文件下载，不应等 `finished()` 后一次性 `readAll()`，而应监听 `readyRead()` 信号分块读取：

```cpp
QFile *outputFile = new QFile("download.bin");
outputFile->open(QIODevice::WriteOnly);

QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl(downloadUrl)));

// 分块写入
connect(reply, &QNetworkReply::readyRead, this, [reply, outputFile]() {
    outputFile->write(reply->readAll());
});

// 完成后关闭文件
connect(reply, &QNetworkReply::finished, this, [reply, outputFile]() {
    outputFile->flush();
    outputFile->close();
    outputFile->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Download error:" << reply->errorString();
    } else {
        qDebug() << "Download complete!";
    }
    reply->deleteLater();
});
```

---

## 6. 错误处理

### 6.1 错误类型

`QNetworkReply::NetworkError` 枚举将错误分为几大类：

| 类别 | 错误值范围 | 常见枚举 | 含义 |
|------|-----------|---------|------|
| 无错误 | 0 | `NoError` | 请求成功 |
| 网络层 | 1–99 | `ConnectionRefusedError`(1)、`RemoteHostClosedError`(2)、`HostNotFoundError`(3)、`TimeoutError`(4)、`OperationCanceledError`(5)、`UnknownNetworkError`(99) | 网络不可达、DNS 失败、超时等 |
| 代理层 | 101–199 | `ProxyConnectionRefusedError`(101)、`ProxyAuthenticationRequiredError`(105) | 代理相关 |
| 内容层 | 201–299 | `ContentAccessDenied`(201)、`ContentNotFoundError`(203)、`ContentConflictError`(206)、`UnknownContentError`(299) | 对应 HTTP 4xx |
| 协议层 | 301–399 | `ProtocolFailure`(399) | 协议层异常 |
| 服务端 | 401–499 | `InternalServerError`(401)、`ServiceUnavailableError`(403)、`UnknownServerError`(499) | 对应 HTTP 5xx |

### 6.2 完善的错误处理模式

```cpp
connect(reply, &QNetworkReply::finished, this, [reply]() {
    int statusCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() == QNetworkReply::NoError) {
        // 成功
        handleSuccess(reply->readAll());
    } else if (reply->error() == QNetworkReply::OperationCanceledError) {
        // 请求被取消（如用户主动 abort）
        qDebug() << "Request canceled";
    } else if (statusCode >= 400 && statusCode < 500) {
        // 客户端错误（4xx）
        QByteArray errorBody = reply->readAll();
        qWarning() << "Client error" << statusCode << errorBody;
    } else if (statusCode >= 500) {
        // 服务端错误（5xx）
        qWarning() << "Server error" << statusCode;
    } else {
        // 网络层错误（无 HTTP 状态码）
        qWarning() << "Network error:" << reply->error()
                   << reply->errorString();
    }

    reply->deleteLater();
});
```

### 6.3 使用 errorOccurred 信号（Qt 5.15+）

```cpp
// Qt 5.15 新增，替代已废弃的 error(QNetworkReply::NetworkError) 信号
connect(reply, &QNetworkReply::errorOccurred,
        this, [](QNetworkReply::NetworkError code) {
    qWarning() << "Error occurred:" << code;
});
```

> **注意：** 在 Qt 5.15 之前使用 `QNetworkReply::error` 信号时，需要用函数指针语法消除与 `error()` 方法的重载歧义：
> ```cpp
> connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
>         this, &MyClass::onError);
> ```

---

## 7. 超时控制

### 7.1 Qt 5.15+ 内建超时

```cpp
QNetworkRequest request(QUrl("https://api.example.com/data"));

// 设置传输超时（毫秒），超时后自动 abort
request.setTransferTimeout(30000);  // 30 秒

QNetworkReply *reply = m_manager->get(request);
```

也可在 `QNetworkAccessManager` 级别设置全局默认超时：

```cpp
m_manager->setTransferTimeout(15000);  // 所有请求默认 15 秒超时
```

### 7.2 兼容旧版 Qt 的手动超时

```cpp
QNetworkReply *reply = m_manager->get(request);

QTimer *timer = new QTimer(this);
timer->setSingleShot(true);
connect(timer, &QTimer::timeout, this, [reply, timer]() {
    if (reply->isRunning()) {
        reply->abort();  // 触发 OperationCanceledError
        qWarning() << "Request timed out!";
    }
    timer->deleteLater();
});
timer->start(30000);  // 30 秒超时

connect(reply, &QNetworkReply::finished, this, [reply, timer]() {
    timer->stop();
    timer->deleteLater();
    // ... 正常处理
    reply->deleteLater();
});
```

---

## 8. SSL/HTTPS

### 8.1 默认行为

Qt 默认使用系统 CA 证书验证服务器证书，对大部分 HTTPS 站点无需额外配置。

### 8.2 处理 SSL 错误

```cpp
connect(reply, &QNetworkReply::sslErrors,
        this, [reply](const QList<QSslError> &errors) {
    for (const QSslError &err : errors) {
        qWarning() << "SSL Error:" << err.errorString();
    }
    // ⚠️ 仅在开发/调试环境使用，生产环境切勿忽略 SSL 错误！
    // reply->ignoreSslErrors();
});
```

### 8.3 自定义 SSL 配置（自签名证书/双向认证）

```cpp
QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();

// 添加自签名 CA 证书
QFile certFile(":/certs/my-ca.pem");
certFile.open(QIODevice::ReadOnly);
QSslCertificate caCert(&certFile, QSsl::Pem);
QList<QSslCertificate> caList = sslConfig.caCertificates();
caList.append(caCert);
sslConfig.setCaCertificates(caList);

// 客户端证书（双向 TLS / mTLS）
QFile clientCertFile(":/certs/client.pem");
clientCertFile.open(QIODevice::ReadOnly);
QSslCertificate clientCert(&clientCertFile, QSsl::Pem);
sslConfig.setLocalCertificate(clientCert);

QFile keyFile(":/certs/client-key.pem");
keyFile.open(QIODevice::ReadOnly);
QSslKey privateKey(&keyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
sslConfig.setPrivateKey(privateKey);

// 指定协议版本
sslConfig.setProtocol(QSsl::TlsV1_2OrLater);

// 应用到请求
QNetworkRequest request(QUrl("https://internal.example.com/api"));
request.setSslConfiguration(sslConfig);
```

---

## 9. 重定向处理

### 9.1 自动跟随重定向（Qt 5.6+）

```cpp
QNetworkRequest request(QUrl("https://example.com/old-page"));

// 自动跟随 3xx 重定向
request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

QNetworkReply *reply = m_manager->get(request);
```

重定向策略：

| 策略 | 说明 |
|------|------|
| `ManualRedirectPolicy` | 手动处理（默认） |
| `NoLessSafeRedirectPolicy` | 自动跟随，但不允许从 HTTPS 降级到 HTTP |
| `SameOriginRedirectPolicy` | 仅跟随同源重定向 |
| `UserVerifiedRedirectPolicy` | 通过 `redirected()` 信号由用户确认 |

也可全局设置：

```cpp
m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
```

### 9.2 手动跟随重定向

```cpp
connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    int statusCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 301 || statusCode == 302 || statusCode == 307) {
        QUrl redirectUrl = reply->header(
            QNetworkRequest::LocationHeader).toUrl();

        // 处理相对 URL
        if (redirectUrl.isRelative()) {
            redirectUrl = reply->url().resolved(redirectUrl);
        }

        qDebug() << "Redirecting to:" << redirectUrl;
        QNetworkReply *newReply = m_manager->get(QNetworkRequest(redirectUrl));
        // ... 重新连接信号
    }

    reply->deleteLater();
});
```

---

## 10. Cookie 管理

### 10.1 自动 Cookie 管理

`QNetworkAccessManager` 内置 `QNetworkCookieJar`，自动处理 `Set-Cookie` 响应和后续请求的 Cookie 附带：

```cpp
// 默认已启用，无需额外配置
// 服务器返回 Set-Cookie 后，后续同域请求自动附带 Cookie
```

### 10.2 手动设置 Cookie

```cpp
#include <QNetworkCookie>
#include <QNetworkCookieJar>

QNetworkCookieJar *jar = m_manager->cookieJar();

QNetworkCookie cookie("session_id", "abc123xyz");
cookie.setDomain(".example.com");
cookie.setPath("/");
cookie.setSecure(true);
cookie.setHttpOnly(true);
cookie.setExpirationDate(QDateTime::currentDateTime().addDays(7));

jar->insertCookie(cookie);
```

### 10.3 持久化 Cookie（自定义 CookieJar）

```cpp
// persistent_cookie_jar.h
class PersistentCookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit PersistentCookieJar(QObject *parent = nullptr);
    ~PersistentCookieJar();

    // 重写以拦截 Cookie 变化
    bool insertCookie(const QNetworkCookie &cookie) override;
    bool deleteCookie(const QNetworkCookie &cookie) override;

private:
    void save();
    void load();
    QString m_filePath;
};

// persistent_cookie_jar.cpp
PersistentCookieJar::PersistentCookieJar(QObject *parent)
    : QNetworkCookieJar(parent)
{
    m_filePath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation) + "/cookies.dat";
    load();
}

void PersistentCookieJar::save()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) return;

    QDataStream stream(&file);
    const QList<QNetworkCookie> cookies = allCookies();
    stream << static_cast<qint32>(cookies.size());
    for (const QNetworkCookie &cookie : cookies) {
        if (!cookie.isSessionCookie()) {  // 只持久化非会话 Cookie
            stream << cookie.toRawForm();
        }
    }
}

void PersistentCookieJar::load()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) return;

    QDataStream stream(&file);
    qint32 count;
    stream >> count;

    QList<QNetworkCookie> cookies;
    for (qint32 i = 0; i < count; ++i) {
        QByteArray raw;
        stream >> raw;
        cookies.append(QNetworkCookie::parseCookies(raw));
    }
    setAllCookies(cookies);
}
```

---

## 11. HTTP 认证

### 11.1 Basic / Digest 认证

```cpp
connect(m_manager, &QNetworkAccessManager::authenticationRequired,
        this, [](QNetworkReply *reply, QAuthenticator *auth) {
    Q_UNUSED(reply)
    auth->setUser("admin");
    auth->setPassword("secret");
});
```

> **注意：** 此信号可能被多次触发（如密码错误时重试），应检查 `auth->user()` 是否已设置过，避免死循环。

### 11.2 Bearer Token 认证

Bearer Token 需手动设置 Header（无内置支持）：

```cpp
QNetworkRequest request(QUrl("https://api.example.com/protected"));
request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

QNetworkReply *reply = m_manager->get(request);
```

### 11.3 封装 Token 自动刷新

```cpp
class ApiClient : public QObject
{
    Q_OBJECT
public:
    void setToken(const QString &token) { m_token = token; }

    QNetworkReply *authenticatedGet(const QUrl &url)
    {
        QNetworkRequest request(url);
        request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        return m_manager->get(request);
    }

    QNetworkReply *authenticatedPost(const QUrl &url, const QByteArray &body)
    {
        QNetworkRequest request(url);
        request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        return m_manager->post(request, body);
    }

private:
    QNetworkAccessManager *m_manager = new QNetworkAccessManager(this);
    QString m_token;
};
```

---

## 12. 代理设置

### 12.1 全局代理

```cpp
QNetworkProxy proxy;
proxy.setType(QNetworkProxy::HttpProxy);
proxy.setHostName("127.0.0.1");
proxy.setPort(8080);
proxy.setUser("proxyUser");       // 可选
proxy.setPassword("proxyPass");   // 可选
QNetworkProxy::setApplicationProxy(proxy);
```

### 12.2 仅对特定 Manager 设置代理

```cpp
m_manager->setProxy(proxy);
```

### 12.3 使用系统代理

```cpp
QNetworkProxyFactory::setUseSystemConfiguration(true);
```

---

## 13. 文件下载（带进度条）

### 13.1 完整下载管理器示例

```cpp
class FileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit FileDownloader(QNetworkAccessManager *manager, QObject *parent = nullptr)
        : QObject(parent), m_manager(manager) {}

    void download(const QUrl &url, const QString &savePath)
    {
        m_file.setFileName(savePath);
        if (!m_file.open(QIODevice::WriteOnly)) {
            emit error("Cannot open file: " + savePath);
            return;
        }

        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                             QNetworkRequest::NoLessSafeRedirectPolicy);

        m_reply = m_manager->get(request);

        connect(m_reply, &QNetworkReply::readyRead, this, [this]() {
            m_file.write(m_reply->readAll());
        });

        connect(m_reply, &QNetworkReply::downloadProgress,
                this, &FileDownloader::progress);

        connect(m_reply, &QNetworkReply::finished, this, [this]() {
            m_file.flush();
            m_file.close();

            if (m_reply->error() != QNetworkReply::NoError) {
                m_file.remove();  // 删除不完整的文件
                emit error(m_reply->errorString());
            } else {
                emit finished(m_file.fileName());
            }
            m_reply->deleteLater();
            m_reply = nullptr;
        });
    }

    void cancel()
    {
        if (m_reply && m_reply->isRunning()) {
            m_reply->abort();
        }
    }

signals:
    void progress(qint64 received, qint64 total);
    void finished(const QString &filePath);
    void error(const QString &message);

private:
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply = nullptr;
    QFile m_file;
};
```

使用方式：

```cpp
auto *downloader = new FileDownloader(m_manager, this);

connect(downloader, &FileDownloader::progress,
        this, [](qint64 received, qint64 total) {
    if (total > 0) {
        int percent = static_cast<int>(received * 100 / total);
        qDebug() << "Download:" << percent << "%"
                 << "(" << received / 1024 << "/" << total / 1024 << "KB)";
    }
});

connect(downloader, &FileDownloader::finished,
        this, [](const QString &path) {
    qDebug() << "Saved to:" << path;
});

connect(downloader, &FileDownloader::error,
        this, [](const QString &msg) {
    qWarning() << "Download failed:" << msg;
});

downloader->download(
    QUrl("https://example.com/large-file.zip"),
    "C:/Downloads/large-file.zip"
);
```

### 13.2 断点续传

```cpp
void resumeDownload(const QUrl &url, const QString &savePath)
{
    QFileInfo fileInfo(savePath);
    qint64 existingSize = fileInfo.exists() ? fileInfo.size() : 0;

    QNetworkRequest request(url);
    if (existingSize > 0) {
        // 设置 Range Header 实现断点续传
        request.setRawHeader("Range",
            QStringLiteral("bytes=%1-").arg(existingSize).toUtf8());
    }

    m_file.setFileName(savePath);
    m_file.open(existingSize > 0 ? QIODevice::Append : QIODevice::WriteOnly);

    QNetworkReply *reply = m_manager->get(request);

    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        m_file.write(reply->readAll());
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int statusCode = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
        // 206 = Partial Content（续传成功）, 200 = 全量重新下载
        qDebug() << "Status:" << statusCode;
        m_file.close();
        reply->deleteLater();
    });
}
```

---

## 14. 并发请求管理

### 14.1 多请求并行 + 汇总结果

```cpp
class BatchRequester : public QObject
{
    Q_OBJECT
public:
    void fetchAll(const QStringList &urls)
    {
        m_pending = urls.size();
        m_results.clear();

        for (int i = 0; i < urls.size(); ++i) {
            QNetworkRequest request(QUrl(urls[i]));
            QNetworkReply *reply = m_manager->get(request);

            connect(reply, &QNetworkReply::finished,
                    this, [this, reply, i]() {
                if (reply->error() == QNetworkReply::NoError) {
                    m_results[i] = reply->readAll();
                } else {
                    m_results[i] = QByteArray();  // 标记失败
                }
                reply->deleteLater();

                if (--m_pending == 0) {
                    emit allFinished(m_results);
                }
            });
        }
    }

signals:
    void allFinished(const QMap<int, QByteArray> &results);

private:
    QNetworkAccessManager *m_manager = new QNetworkAccessManager(this);
    int m_pending = 0;
    QMap<int, QByteArray> m_results;
};
```

### 14.2 限制并发数

```cpp
class ThrottledRequester : public QObject
{
    Q_OBJECT
public:
    explicit ThrottledRequester(int maxConcurrent = 4, QObject *parent = nullptr)
        : QObject(parent), m_maxConcurrent(maxConcurrent) {}

    void enqueue(const QNetworkRequest &request)
    {
        m_queue.enqueue(request);
        processQueue();
    }

private:
    void processQueue()
    {
        while (m_activeCount < m_maxConcurrent && !m_queue.isEmpty()) {
            QNetworkRequest request = m_queue.dequeue();
            QNetworkReply *reply = m_manager->get(request);
            ++m_activeCount;

            connect(reply, &QNetworkReply::finished,
                    this, [this, reply]() {
                --m_activeCount;
                emit replyReceived(reply);
                reply->deleteLater();
                processQueue();  // 继续处理队列
            });
        }
    }

signals:
    void replyReceived(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager = new QNetworkAccessManager(this);
    QQueue<QNetworkRequest> m_queue;
    int m_activeCount = 0;
    int m_maxConcurrent;
};
```

---

## 15. 实战：封装 REST API 客户端

```cpp
// rest_client.h
#pragma once
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <functional>

class RestClient : public QObject
{
    Q_OBJECT
public:
    using Callback = std::function<void(int statusCode, const QJsonDocument &body,
                                         const QString &error)>;

    explicit RestClient(const QString &baseUrl, QObject *parent = nullptr)
        : QObject(parent), m_baseUrl(baseUrl) {}

    void setToken(const QString &token) { m_token = token; }

    void get(const QString &path, Callback callback)
    {
        sendRequest("GET", path, QByteArray(), std::move(callback));
    }

    void post(const QString &path, const QJsonObject &json, Callback callback)
    {
        QByteArray body = QJsonDocument(json).toJson(QJsonDocument::Compact);
        sendRequest("POST", path, body, std::move(callback));
    }

    void put(const QString &path, const QJsonObject &json, Callback callback)
    {
        QByteArray body = QJsonDocument(json).toJson(QJsonDocument::Compact);
        sendRequest("PUT", path, body, std::move(callback));
    }

    void del(const QString &path, Callback callback)
    {
        sendRequest("DELETE", path, QByteArray(), std::move(callback));
    }

private:
    void sendRequest(const QByteArray &method, const QString &path,
                     const QByteArray &body, Callback callback)
    {
        QUrl url(m_baseUrl + path);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setTransferTimeout(30000);

        if (!m_token.isEmpty()) {
            request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
        }

        QNetworkReply *reply = nullptr;
        if (method == "GET") {
            reply = m_manager->get(request);
        } else if (method == "POST") {
            reply = m_manager->post(request, body);
        } else if (method == "PUT") {
            reply = m_manager->put(request, body);
        } else if (method == "DELETE") {
            reply = m_manager->deleteResource(request);
        } else {
            QBuffer *buffer = new QBuffer;
            buffer->setData(body);
            buffer->open(QIODevice::ReadOnly);
            reply = m_manager->sendCustomRequest(request, method, buffer);
            buffer->setParent(reply);
        }

        connect(reply, &QNetworkReply::finished, this,
                [reply, cb = std::move(callback)]() {
            int statusCode = reply->attribute(
                QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QByteArray data = reply->readAll();

            if (reply->error() == QNetworkReply::NoError) {
                cb(statusCode, QJsonDocument::fromJson(data), QString());
            } else {
                cb(statusCode, QJsonDocument::fromJson(data),
                   reply->errorString());
            }
            reply->deleteLater();
        });
    }

    QNetworkAccessManager *m_manager = new QNetworkAccessManager(this);
    QString m_baseUrl;
    QString m_token;
};
```

使用示例：

```cpp
auto *api = new RestClient("https://api.example.com", this);
api->setToken("eyJhbGciOiJIUzI1NiIs...");

// GET 请求
api->get("/users", [](int status, const QJsonDocument &doc, const QString &err) {
    if (err.isEmpty()) {
        qDebug() << "Users:" << doc.array().size();
    } else {
        qWarning() << "Failed:" << status << err;
    }
});

// POST 请求
QJsonObject newUser;
newUser["name"] = "Alice";
newUser["email"] = "alice@example.com";

api->post("/users", newUser, [](int status, const QJsonDocument &doc, const QString &err) {
    if (status == 201) {
        qDebug() << "Created user ID:" << doc.object()["id"].toInt();
    } else {
        qWarning() << "Create failed:" << status << err;
    }
});
```

---

## 16. 最佳实践与常见陷阱

### 16.1 核心原则

| 原则 | 说明 |
|------|------|
| **复用 Manager** | 一个应用只创建 1 个（或少量）`QNetworkAccessManager` 实例，复用 TCP 连接池和 Cookie |
| **异步为主** | 永远不要在主线程用 `QEventLoop` 等待网络响应（阻塞 UI），正确使用信号槽 |
| **及时释放 Reply** | 在 `finished()` 槽中调用 `reply->deleteLater()`，不释放会内存泄漏 |
| **设置超时** | 始终设置 `setTransferTimeout()`，避免请求永久挂起 |
| **处理 SSL 错误** | 生产环境绝不调用 `ignoreSslErrors()`，应正确配置 CA 证书 |
| **检查错误码** | 不要只检查 `error() == NoError`，也要检查 HTTP 状态码 |

### 16.2 线程安全

```
⚠️ QNetworkAccessManager 必须在创建它的线程中使用。
   如果需要在工作线程中发起请求，在该线程中创建独立的 Manager。
```

```cpp
// ❌ 错误：跨线程使用
void WorkerThread::run() {
    // m_manager 在主线程创建，不能在子线程调用
    m_manager->get(request);  // 未定义行为！
}

// ✅ 正确：在工作线程中创建独立的 Manager
void WorkerThread::run() {
    QNetworkAccessManager manager;
    QEventLoop loop;

    QNetworkReply *reply = manager.get(request);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();  // 在工作线程中阻塞等待是可接受的

    QByteArray data = reply->readAll();
    reply->deleteLater();
}
```

### 16.3 常见陷阱速查

| 陷阱 | 症状 | 解决方案 |
|------|------|---------|
| 忘记 `deleteLater()` | 内存持续增长 | 在 `finished()` 槽中始终调用 |
| 主线程中使用 `QEventLoop` | UI 卡死 | 改用异步信号槽 |
| 单次请求创建新 Manager | 连接无法复用、Cookie 丢失 | 持久化一个 Manager 实例 |
| 未设置 `Content-Type` | 服务端解析失败（415） | POST/PUT 时显式设置 |
| 访问已释放的 Reply 指针 | 崩溃 | 使用 `QPointer<QNetworkReply>` 或在单一位置管理生命周期 |
| 大文件用 `readAll()` | 内存暴涨 | 使用 `readyRead()` 信号分块读取 |
| 忽略 SSL 错误 | 中间人攻击风险 | 正确配置 CA 证书链 |
| 未处理重定向 | 收到空响应或 3xx 状态码 | 设置 `RedirectPolicyAttribute` |

---

## 17. API 速查表

### QNetworkAccessManager 主要方法

```cpp
QNetworkReply *get(const QNetworkRequest &request);
QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data);
QNetworkReply *post(const QNetworkRequest &request, QHttpMultiPart *multiPart);
QNetworkReply *put(const QNetworkRequest &request, const QByteArray &data);
QNetworkReply *deleteResource(const QNetworkRequest &request);
QNetworkReply *head(const QNetworkRequest &request);
QNetworkReply *sendCustomRequest(const QNetworkRequest &request,
                                  const QByteArray &verb,
                                  QIODevice *data = nullptr);

void setRedirectPolicy(QNetworkRequest::RedirectPolicy policy);
void setTransferTimeout(int timeout);   // Qt 5.15+
void setProxy(const QNetworkProxy &proxy);
void setCookieJar(QNetworkCookieJar *cookieJar);

// 信号
void finished(QNetworkReply *reply);
void authenticationRequired(QNetworkReply *reply, QAuthenticator *auth);
void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
```

### QNetworkReply 主要方法

```cpp
// 状态
QNetworkReply::NetworkError error() const;
QString errorString() const;
bool isRunning() const;
QUrl url() const;

// 响应数据
QByteArray readAll();
qint64 read(char *data, qint64 maxSize);
QVariant attribute(QNetworkRequest::Attribute code) const;
QVariant header(QNetworkRequest::KnownHeaders header) const;
QByteArray rawHeader(const QByteArray &name) const;
QList<QByteArray> rawHeaderList() const;

// 控制
void abort();
void ignoreSslErrors();

// 信号
void finished();
void readyRead();
void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
void errorOccurred(QNetworkReply::NetworkError code);  // Qt 5.15+
void sslErrors(const QList<QSslError> &errors);
void redirected(const QUrl &url);
```
