Qt混合架构重构技术方案.md

# 溜云库混合架构重构技术方案

> Qt UI层 + .NET业务逻辑层 - 渐进式重构方案  
> 目标：跨平台、降风险、保业务、快交付

---

## 📋 执行摘要

### 项目背景

**当前痛点**：
- .NET Core 3.1已停止支持，存在安全和性能风险
- WPF仅支持Windows，限制跨平台扩展
- DotNetBrowser商业许可成本高（~102MB，年费续订）
- 15MB单体核心库，耦合严重，难以维护
- 启动慢（2.7秒）、内存占用高（600MB+）、UI卡顿

**重构目标**：
1. **跨平台**：UI层支持Windows/macOS/Linux
2. **高性能**：启动<1.5秒，内存<300MB，UI 60fps
3. **降风险**：保留成熟的C#业务逻辑，仅重写UI层
4. **快交付**：12个月完成，分阶段交付可用版本
5. **降成本**：移除商业依赖，总成本<250万

### 方案概述

**核心策略**：保留C#业务逻辑 + Qt跨平台UI + IPC通信

| 维度 | 方案 |
|------|------|
| **UI层** | Qt6 (Widgets + Quick/QML) - 跨平台UI |
| **业务逻辑层** | C# + .NET 8 - 保留现有代码 |
| **进程间通信** | Protobuf + Named Pipe/Unix Socket |
| **数据存储** | .NET端：SQLite + Dapper（保留）<br>Qt端：读取缓存 |
| **网络通信** | .NET端：HttpClient + WebSocket（保留） |
| **构建系统** | .NET: MSBuild<br>Qt: CMake |

### 架构优势

✅ **风险可控**：核心业务逻辑不变，降低重写风险  
✅ **成本合理**：相比全面重写节省40%成本  
✅ **快速交付**：12个月完成，6个月可交付MVP  
✅ **技术先进**：Qt现代化UI，跨平台原生体验  
✅ **团队友好**：C#团队继续维护业务逻辑，小团队学Qt即可

### 实施计划

**总周期**：12个月  
**团队规模**：7-8人  
**预算**：约240万元人民币

**三阶段路线**：
- **阶段1（1-4个月）**：基础架构 + 核心UI（MVP）
- **阶段2（5-8个月）**：完整功能 + 跨平台适配
- **阶段3（9-12个月）**：优化 + 灰度发布 + 切换

---

## 🏗️ 总体架构设计

### 双进程架构

```
┌─────────────────────────────────────────────────────────────┐
│                    Qt UI 进程 (跨平台)                        │
│                   C++ / Qt6 / QML                            │
├─────────────────────────────────────────────────────────────┤
│  表现层 (Presentation)                                       │
│  ├─ Qt Widgets: 主窗口、菜单、工具栏、表格、树形              │
│  ├─ Qt Quick (QML): 素材瀑布流、详情页、动画                 │
│  └─ Qt WebEngine: 在线浏览（可选）                           │
│                                                              │
│  应用层 (Application - Thin Layer)                           │
│  ├─ ViewModels: QML暴露对象                                  │
│  ├─ IPC Client: 与.NET服务通信                              │
│  ├─ Local Cache: 缩略图、临时数据                            │
│  └─ Event Handling: UI事件处理                              │
└─────────────────────────────────────────────────────────────┘
                            ↕
              Protobuf + Named Pipe / Unix Socket
              (命令/查询/事件双向通信)
                            ↕
┌─────────────────────────────────────────────────────────────┐
│                  .NET 服务进程 (跨平台)                       │
│                    C# / .NET 8                               │
├─────────────────────────────────────────────────────────────┤
│  IPC Server: 消息路由与处理                                   │
│                                                              │
│  应用层 (Application Services)                               │
│  ├─ Use Cases: 业务用例编排                                  │
│  ├─ Commands/Queries: CQRS模式                              │
│  ├─ Event Bus: 事件总线                                      │
│  └─ IPC Message Handler: 消息分发                           │
│                                                              │
│  领域层 (Domain)                                             │
│  ├─ 业务实体: Model, Material, User                         │
│  ├─ 领域服务: ModelService, DownloadService                  │
│  ├─ 仓储接口: IRepository<T>                                 │
│  └─ 业务规则: 验证、策略                                     │
│                                                              │
│  基础设施层 (Infrastructure)                                  │
│  ├─ 数据访问: SQLite + Dapper                                │
│  ├─ 网络服务: HttpClient, WebSocketClient                    │
│  ├─ 云存储: OSS/COS/BCE SDK                                  │
│  ├─ 日志: Serilog                                           │
│  ├─ 文件系统: System.IO                                      │
│  └─ 3ds Max集成: COM Interop (Windows)                       │
└─────────────────────────────────────────────────────────────┘
```

### 架构原则

1. **职责分离**：
   - Qt进程：负责UI展示、用户交互、本地缓存
   - .NET进程：负责业务逻辑、数据访问、外部服务集成

2. **进程隔离**：
   - UI崩溃不影响业务逻辑
   - 业务逻辑更新不影响UI
   - 独立部署、独立升级

3. **通信高效**：
   - Protobuf二进制序列化（高效）
   - Named Pipe/Unix Socket（低延迟<1ms）
   - 异步通信（不阻塞UI）

4. **缓存策略**：
   - Qt端缓存：缩略图、UI状态、最近访问数据
   - .NET端缓存：业务数据、查询结果
   - 双层缓存减少IPC调用

---

## 🔄 IPC通信设计

### 通信协议

**Protobuf消息定义**（关键部分）：

```protobuf
syntax = "proto3";
package liuyunku.ipc;

// ========== 消息基础 ==========

message MessageEnvelope {
  string message_id = 1;        // UUID
  string message_type = 2;      // 消息类型
  int64 timestamp = 3;          // 时间戳
  string correlation_id = 4;    // 关联ID（用于请求-响应）
  bytes payload = 5;            // 序列化的具体消息
}

// ========== 命令 (Commands) ==========

// 加载模型列表
message LoadModelsCommand {
  int32 page = 1;
  int32 page_size = 2;
  string category = 3;
  string search_keyword = 4;
  repeated string tags = 5;
  string sort_by = 6;           // "name", "download_count", "created_at"
  bool ascending = 7;
}

// 下载模型
message DownloadModelCommand {
  string model_id = 1;
  string download_path = 2;
  bool high_quality = 3;
}

// 导入到3ds Max
message ImportToMaxCommand {
  string model_id = 1;
  string local_file_path = 2;
  bool import_materials = 3;
  double unit_scale = 4;
}

// 用户登录
message LoginCommand {
  string username = 1;
  string password = 2;
  bool remember_me = 3;
}

// 提交云渲染任务
message SubmitRenderTaskCommand {
  string scene_file_path = 1;
  RenderSettings settings = 2;
}

// ========== 查询 (Queries) ==========

// 获取模型详情
message GetModelDetailQuery {
  string model_id = 1;
}

// 获取下载进度
message GetDownloadProgressQuery {
  string download_id = 1;
}

// 获取用户信息
message GetUserInfoQuery {
  string user_id = 1;
}

// 搜索模型
message SearchModelsQuery {
  string keyword = 1;
  repeated string categories = 2;
  repeated string tags = 3;
  int32 page = 4;
  int32 page_size = 5;
}

// ========== 响应 (Responses) ==========

// 模型列表响应
message ModelsResponse {
  repeated ModelInfo models = 1;
  int32 total_count = 2;
  int32 current_page = 3;
  int32 page_size = 4;
}

message ModelInfo {
  string id = 1;
  string name = 2;
  string category = 3;
  repeated string tags = 4;
  string thumbnail_url = 5;
  int64 file_size = 6;
  double price = 7;
  int32 download_count = 8;
  int64 created_at = 9;
}

// 模型详情响应
message ModelDetailResponse {
  ModelInfo basic_info = 1;
  string description = 2;
  repeated string preview_images = 3;
  repeated string related_models = 4;
  MaterialInfo materials = 5;
}

// 下载进度响应
message DownloadProgressResponse {
  string download_id = 1;
  string model_id = 2;
  double progress = 3;          // 0.0 - 1.0
  int64 downloaded_bytes = 4;
  int64 total_bytes = 5;
  double speed = 6;             // bytes/s
  int64 remaining_seconds = 7;
  string status = 8;            // "pending", "downloading", "completed", "failed"
  string error_message = 9;
}

// 用户信息响应
message UserInfoResponse {
  string user_id = 1;
  string username = 2;
  string email = 3;
  string avatar_url = 4;
  bool is_vip = 5;
  int64 vip_expire_at = 6;
  int32 download_quota = 7;
}

// ========== 事件 (Events) ==========

// 下载完成事件
message DownloadCompletedEvent {
  string download_id = 1;
  string model_id = 2;
  string local_file_path = 3;
  int64 file_size = 4;
}

// 下载失败事件
message DownloadFailedEvent {
  string download_id = 1;
  string model_id = 2;
  string error_code = 3;
  string error_message = 4;
}

// 登录状态变更事件
message LoginStatusChangedEvent {
  bool is_logged_in = 1;
  string user_id = 2;
}

// 云渲染任务状态变更
message RenderTaskStatusChangedEvent {
  string task_id = 1;
  string status = 2;            // "queued", "rendering", "completed", "failed"
  double progress = 3;
  string preview_url = 4;
}

// ========== 通用响应 ==========

message SuccessResponse {
  string message = 1;
  map<string, string> data = 2;
}

message ErrorResponse {
  string error_code = 1;
  string error_message = 2;
  string stack_trace = 3;       // 仅开发环境
}
```

### C# 服务端实现（.NET）

```csharp
// IPCServer.cs
using System;
using System.IO;
using System.IO.Pipes;
using System.Threading;
using System.Threading.Tasks;
using Google.Protobuf;
using LiuYunKu.IPC;
using Microsoft.Extensions.Logging;

public class IPCServer : IDisposable
{
    private readonly string _pipeName;
    private readonly IMessageHandler _messageHandler;
    private readonly ILogger<IPCServer> _logger;
    private CancellationTokenSource _cts;
    private readonly List<Task> _clientTasks = new();

    public IPCServer(
        string pipeName,
        IMessageHandler messageHandler,
        ILogger<IPCServer> logger)
    {
        _pipeName = pipeName;
        _messageHandler = messageHandler;
        _logger = logger;
    }

    public async Task StartAsync(CancellationToken cancellationToken)
    {
        _cts = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        _logger.LogInformation("IPC Server starting on pipe: {PipeName}", _pipeName);

        while (!_cts.Token.IsCancellationRequested)
        {
            try
            {
                var pipeServer = new NamedPipeServerStream(
                    _pipeName,
                    PipeDirection.InOut,
                    NamedPipeServerStream.MaxAllowedServerInstances,
                    PipeTransmissionMode.Byte,
                    PipeOptions.Asynchronous
                );

                _logger.LogDebug("Waiting for client connection...");
                await pipeServer.WaitForConnectionAsync(_cts.Token);
                _logger.LogInformation("Client connected");

                // 为每个客户端创建独立处理任务
                var clientTask = Task.Run(
                    () => HandleClientAsync(pipeServer, _cts.Token),
                    _cts.Token
                );
                _clientTasks.Add(clientTask);

                // 清理已完成的任务
                _clientTasks.RemoveAll(t => t.IsCompleted);
            }
            catch (OperationCanceledException)
            {
                break;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error in server loop");
                await Task.Delay(1000, _cts.Token); // 避免快速重试
            }
        }

        // 等待所有客户端任务完成
        await Task.WhenAll(_clientTasks);
        _logger.LogInformation("IPC Server stopped");
    }

    private async Task HandleClientAsync(
        NamedPipeServerStream pipe,
        CancellationToken cancellationToken)
    {
        try
        {
            using var reader = new BinaryReader(pipe);
            using var writer = new BinaryWriter(pipe);

            while (pipe.IsConnected && !cancellationToken.IsCancellationRequested)
            {
                // 读取消息长度（4字节）
                var lengthBytes = new byte[4];
                var bytesRead = await pipe.ReadAsync(lengthBytes, 0, 4, cancellationToken);
                if (bytesRead == 0) break;

                int length = BitConverter.ToInt32(lengthBytes, 0);
                if (length <= 0 || length > 10 * 1024 * 1024) // 限制10MB
                {
                    _logger.LogWarning("Invalid message length: {Length}", length);
                    break;
                }

                // 读取消息体
                var buffer = new byte[length];
                bytesRead = 0;
                while (bytesRead < length)
                {
                    var read = await pipe.ReadAsync(
                        buffer,
                        bytesRead,
                        length - bytesRead,
                        cancellationToken
                    );
                    if (read == 0) break;
                    bytesRead += read;
                }

                if (bytesRead != length)
                {
                    _logger.LogWarning("Incomplete message received");
                    break;
                }

                // 解析消息
                var envelope = MessageEnvelope.Parser.ParseFrom(buffer);
                _logger.LogDebug("Received: {MessageType}", envelope.MessageType);

                // 处理消息
                var response = await _messageHandler.HandleAsync(envelope);

                // 发送响应
                if (response != null)
                {
                    var responseBytes = response.ToByteArray();
                    var responseLengthBytes = BitConverter.GetBytes(responseBytes.Length);

                    await pipe.WriteAsync(responseLengthBytes, 0, 4, cancellationToken);
                    await pipe.WriteAsync(responseBytes, 0, responseBytes.Length, cancellationToken);
                    await pipe.FlushAsync(cancellationToken);

                    _logger.LogDebug("Sent response for: {MessageType}", response.MessageType);
                }
            }
        }
        catch (OperationCanceledException)
        {
            _logger.LogInformation("Client handler cancelled");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling client");
        }
        finally
        {
            pipe.Dispose();
            _logger.LogInformation("Client disconnected");
        }
    }

    public void Dispose()
    {
        _cts?.Cancel();
        _cts?.Dispose();
    }
}

// IMessageHandler.cs
public interface IMessageHandler
{
    Task<MessageEnvelope> HandleAsync(MessageEnvelope envelope);
}

// MessageHandler.cs
public class MessageHandler : IMessageHandler
{
    private readonly IModelService _modelService;
    private readonly IDownloadService _downloadService;
    private readonly IAuthService _authService;
    private readonly IRenderService _renderService;
    private readonly ILogger<MessageHandler> _logger;

    public MessageHandler(
        IModelService modelService,
        IDownloadService downloadService,
        IAuthService authService,
        IRenderService renderService,
        ILogger<MessageHandler> logger)
    {
        _modelService = modelService;
        _downloadService = downloadService;
        _authService = authService;
        _renderService = renderService;
        _logger = logger;
    }

    public async Task<MessageEnvelope> HandleAsync(MessageEnvelope envelope)
    {
        try
        {
            return envelope.MessageType switch
            {
                "LoadModelsCommand" => await HandleLoadModelsAsync(envelope),
                "DownloadModelCommand" => await HandleDownloadModelAsync(envelope),
                "GetModelDetailQuery" => await HandleGetModelDetailAsync(envelope),
                "GetDownloadProgressQuery" => await HandleGetDownloadProgressAsync(envelope),
                "LoginCommand" => await HandleLoginAsync(envelope),
                "SearchModelsQuery" => await HandleSearchModelsAsync(envelope),
                "SubmitRenderTaskCommand" => await HandleSubmitRenderTaskAsync(envelope),
                _ => CreateErrorResponse(
                    envelope,
                    "UNKNOWN_MESSAGE_TYPE",
                    $"Unknown message type: {envelope.MessageType}"
                )
            };
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling message: {MessageType}", envelope.MessageType);
            return CreateErrorResponse(envelope, "INTERNAL_ERROR", ex.Message);
        }
    }

    private async Task<MessageEnvelope> HandleLoadModelsAsync(MessageEnvelope envelope)
    {
        var command = LoadModelsCommand.Parser.ParseFrom(envelope.Payload);

        var result = await _modelService.LoadModelsAsync(
            page: command.Page,
            pageSize: command.PageSize,
            category: command.Category,
            searchKeyword: command.SearchKeyword,
            tags: command.Tags.ToList(),
            sortBy: command.SortBy,
            ascending: command.Ascending
        );

        var response = new ModelsResponse
        {
            TotalCount = result.TotalCount,
            CurrentPage = command.Page,
            PageSize = command.PageSize
        };

        response.Models.AddRange(result.Items.Select(m => new ModelInfo
        {
            Id = m.Id,
            Name = m.Name,
            Category = m.Category,
            ThumbnailUrl = m.ThumbnailUrl,
            FileSize = m.FileSize,
            Price = m.Price,
            DownloadCount = m.DownloadCount,
            CreatedAt = new DateTimeOffset(m.CreatedAt).ToUnixTimeMilliseconds()
        }));

        return CreateResponse(envelope, "ModelsResponse", response);
    }

    private async Task<MessageEnvelope> HandleDownloadModelAsync(MessageEnvelope envelope)
    {
        var command = DownloadModelCommand.Parser.ParseFrom(envelope.Payload);

        var downloadId = await _downloadService.StartDownloadAsync(
            modelId: command.ModelId,
            downloadPath: command.DownloadPath,
            highQuality: command.HighQuality
        );

        var response = new SuccessResponse
        {
            Message = "Download started"
        };
        response.Data.Add("download_id", downloadId);

        return CreateResponse(envelope, "SuccessResponse", response);
    }

    private async Task<MessageEnvelope> HandleGetDownloadProgressAsync(MessageEnvelope envelope)
    {
        var query = GetDownloadProgressQuery.Parser.ParseFrom(envelope.Payload);

        var progress = await _downloadService.GetDownloadProgressAsync(query.DownloadId);

        var response = new DownloadProgressResponse
        {
            DownloadId = progress.DownloadId,
            ModelId = progress.ModelId,
            Progress = progress.Progress,
            DownloadedBytes = progress.DownloadedBytes,
            TotalBytes = progress.TotalBytes,
            Speed = progress.Speed,
            RemainingSeconds = progress.RemainingSeconds,
            Status = progress.Status.ToString(),
            ErrorMessage = progress.ErrorMessage ?? ""
        };

        return CreateResponse(envelope, "DownloadProgressResponse", response);
    }

    private MessageEnvelope CreateResponse(
        MessageEnvelope request,
        string messageType,
        IMessage payload)
    {
        return new MessageEnvelope
        {
            MessageId = Guid.NewGuid().ToString(),
            MessageType = messageType,
            Timestamp = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds(),
            CorrelationId = request.MessageId,
            Payload = payload.ToByteString()
        };
    }

    private MessageEnvelope CreateErrorResponse(
        MessageEnvelope request,
        string errorCode,
        string errorMessage)
    {
        var error = new ErrorResponse
        {
            ErrorCode = errorCode,
            ErrorMessage = errorMessage
        };
        return CreateResponse(request, "ErrorResponse", error);
    }
}
```

### Qt 客户端实现（C++）

```cpp
// ipc_client.h
#pragma once
#include <QObject>
#include <QLocalSocket>
#include <QTimer>
#include <functional>
#include <optional>
#include "liuyunku.pb.h"

using namespace liuyunku::ipc;

class IPCClient : public QObject
{
    Q_OBJECT

public:
    explicit IPCClient(QObject* parent = nullptr);
    ~IPCClient();

    // 连接管理
    void connectToServer(const QString& pipeName);
    void disconnect();
    bool isConnected() const;

    // 同步调用（阻塞，适合初始化阶段）
    template<typename TResponse>
    std::optional<TResponse> sendCommandSync(
        const QString& messageType,
        const google::protobuf::Message& command,
        int timeoutMs = 5000);

    // 异步调用（非阻塞，推荐）
    template<typename TResponse>
    void sendCommandAsync(
        const QString& messageType,
        const google::protobuf::Message& command,
        std::function<void(const TResponse&)> onSuccess,
        std::function<void(const QString&)> onError);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);

    // 业务事件信号
    void downloadCompleted(const QString& downloadId, const QString& modelId, const QString& filePath);
    void downloadFailed(const QString& downloadId, const QString& errorMessage);
    void loginStatusChanged(bool isLoggedIn, const QString& userId);
    void renderTaskStatusChanged(const QString& taskId, const QString& status, double progress);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QLocalSocket::LocalSocketError error);
    void onReconnectTimer();

private:
    MessageEnvelope createMessage(const QString& messageType, 
                                   const google::protobuf::Message& payload);
    void handleResponse(const MessageEnvelope& envelope);
    void handleEvent(const MessageEnvelope& envelope);
    void startReconnectTimer();

    QLocalSocket* m_socket;
    QByteArray m_receiveBuffer;
    QTimer* m_reconnectTimer;
    QString m_pipeName;
    bool m_autoReconnect = true;

    struct PendingRequest {
        std::function<void(const MessageEnvelope&)> callback;
        QTimer* timeoutTimer;
    };
    QHash<QString, PendingRequest> m_pendingRequests;  // correlationId -> callback
};

// ipc_client.cpp
#include "ipc_client.h"
#include <QDataStream>
#include <QUuid>
#include <QEventLoop>

IPCClient::IPCClient(QObject* parent)
    : QObject(parent)
    , m_socket(new QLocalSocket(this))
    , m_reconnectTimer(new QTimer(this))
{
    // 连接信号
    connect(m_socket, &QLocalSocket::connected, this, &IPCClient::onConnected);
    connect(m_socket, &QLocalSocket::disconnected, this, &IPCClient::onDisconnected);
    connect(m_socket, &QLocalSocket::readyRead, this, &IPCClient::onReadyRead);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &IPCClient::onError);

    // 重连定时器
    m_reconnectTimer->setInterval(3000);
    connect(m_reconnectTimer, &QTimer::timeout, this, &IPCClient::onReconnectTimer);
}

IPCClient::~IPCClient()
{
    disconnect();
}

void IPCClient::connectToServer(const QString& pipeName)
{
    m_pipeName = pipeName;
    m_socket->connectToServer(pipeName);
}

void IPCClient::disconnect()
{
    m_autoReconnect = false;
    m_reconnectTimer->stop();

    if (m_socket->state() == QLocalSocket::ConnectedState) {
        m_socket->disconnectFromServer();
    }

    // 清理待处理请求
    for (auto& req : m_pendingRequests) {
        if (req.timeoutTimer) {
            req.timeoutTimer->stop();
            req.timeoutTimer->deleteLater();
        }
    }
    m_pendingRequests.clear();
}

bool IPCClient::isConnected() const
{
    return m_socket->state() == QLocalSocket::ConnectedState;
}

void IPCClient::onConnected()
{
    qInfo() << "IPC Client connected to server";
    m_reconnectTimer->stop();
    emit connected();
}

void IPCClient::onDisconnected()
{
    qWarning() << "IPC Client disconnected from server";
    emit disconnected();

    // 清理待处理请求（触发错误回调）
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it) {
        // 这里可以调用错误回调
        if (it.value().timeoutTimer) {
            it.value().timeoutTimer->stop();
            it.value().timeoutTimer->deleteLater();
        }
    }
    m_pendingRequests.clear();

    // 自动重连
    if (m_autoReconnect) {
        startReconnectTimer();
    }
}

void IPCClient::onReadyRead()
{
    m_receiveBuffer.append(m_socket->readAll());

    // 处理所有完整的消息
    while (m_receiveBuffer.size() >= 4) {
        // 读取消息长度
        qint32 length;
        memcpy(&length, m_receiveBuffer.data(), sizeof(length));

        if (length <= 0 || length > 10 * 1024 * 1024) {
            qCritical() << "Invalid message length:" << length;
            m_receiveBuffer.clear();
            disconnect();
            return;
        }

        if (m_receiveBuffer.size() < 4 + length) {
            // 消息不完整，等待更多数据
            break;
        }

        // 提取消息体
        QByteArray messageData = m_receiveBuffer.mid(4, length);
        m_receiveBuffer.remove(0, 4 + length);

        // 解析消息
        MessageEnvelope envelope;
        if (envelope.ParseFromArray(messageData.data(), messageData.size())) {
            if (!envelope.correlation_id().empty()) {
                // 响应消息
                handleResponse(envelope);
            } else {
                // 事件消息（服务端主动推送）
                handleEvent(envelope);
            }
        } else {
            qCritical() << "Failed to parse message";
        }
    }
}

void IPCClient::onError(QLocalSocket::LocalSocketError error)
{
    qWarning() << "IPC Client error:" << m_socket->errorString();
    emit errorOccurred(m_socket->errorString());
}

void IPCClient::onReconnectTimer()
{
    if (!isConnected()) {
        qInfo() << "Attempting to reconnect...";
        connectToServer(m_pipeName);
    }
}

void IPCClient::startReconnectTimer()
{
    if (!m_reconnectTimer->isActive()) {
        m_reconnectTimer->start();
    }
}

MessageEnvelope IPCClient::createMessage(
    const QString& messageType,
    const google::protobuf::Message& payload)
{
    MessageEnvelope envelope;
    envelope.set_message_id(QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString());
    envelope.set_message_type(messageType.toStdString());
    envelope.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    envelope.set_payload(payload.SerializeAsString());

    return envelope;
}

template<typename TResponse>
std::optional<TResponse> IPCClient::sendCommandSync(
    const QString& messageType,
    const google::protobuf::Message& command,
    int timeoutMs)
{
    if (!isConnected()) {
        qWarning() << "Not connected to server";
        return std::nullopt;
    }

    // 创建消息
    MessageEnvelope envelope = createMessage(messageType, command);
    std::string serialized = envelope.SerializeAsString();

    // 发送消息长度
    qint32 length = static_cast<qint32>(serialized.size());
    m_socket->write(reinterpret_cast<const char*>(&length), sizeof(length));

    // 发送消息体
    m_socket->write(serialized.c_str(), serialized.size());
    m_socket->flush();

    // 等待响应
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    TResponse response;
    bool responseReceived = false;

    // 设置回调
    QString correlationId = QString::fromStdString(envelope.message_id());
    m_pendingRequests[correlationId] = {
        [&](const MessageEnvelope& resp) {
            if (resp.message_type() == "ErrorResponse") {
                ErrorResponse error;
                error.ParseFromString(resp.payload());
                qWarning() << "Error response:" << QString::fromStdString(error.error_message());
            } else {
                response.ParseFromString(resp.payload());
                responseReceived = true;
            }
            loop.quit();
        },
        nullptr
    };

    connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
        qWarning() << "Request timeout";
        loop.quit();
    });

    timeoutTimer.start(timeoutMs);
    loop.exec();

    m_pendingRequests.remove(correlationId);

    if (responseReceived) {
        return response;
    }

    return std::nullopt;
}

template<typename TResponse>
void IPCClient::sendCommandAsync(
    const QString& messageType,
    const google::protobuf::Message& command,
    std::function<void(const TResponse&)> onSuccess,
    std::function<void(const QString&)> onError)
{
    if (!isConnected()) {
        if (onError) {
            onError("Not connected to server");
        }
        return;
    }

    // 创建消息
    MessageEnvelope envelope = createMessage(messageType, command);
    std::string serialized = envelope.SerializeAsString();

    // 发送消息
    qint32 length = static_cast<qint32>(serialized.size());
    m_socket->write(reinterpret_cast<const char*>(&length), sizeof(length));
    m_socket->write(serialized.c_str(), serialized.size());
    m_socket->flush();

    // 设置回调和超时
    QString correlationId = QString::fromStdString(envelope.message_id());
    QTimer* timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->setInterval(5000);

    connect(timeoutTimer, &QTimer::timeout, this, [=]() {
        m_pendingRequests.remove(correlationId);
        if (onError) {
            onError("Request timeout");
        }
        timeoutTimer->deleteLater();
    });

    m_pendingRequests[correlationId] = {
        [=](const MessageEnvelope& resp) {
            timeoutTimer->stop();
            timeoutTimer->deleteLater();

            if (resp.message_type() == "ErrorResponse") {
                ErrorResponse error;
                error.ParseFromString(resp.payload());
                if (onError) {
                    onError(QString::fromStdString(error.error_message()));
                }
            } else {
                TResponse response;
                if (response.ParseFromString(resp.payload())) {
                    if (onSuccess) {
                        onSuccess(response);
                    }
                } else {
                    if (onError) {
                        onError("Failed to parse response");
                    }
                }
            }
        },
        timeoutTimer
    };

    timeoutTimer->start();
}

void IPCClient::handleResponse(const MessageEnvelope& envelope)
{
    QString correlationId = QString::fromStdString(envelope.correlation_id());

    if (m_pendingRequests.contains(correlationId)) {
        auto request = m_pendingRequests.take(correlationId);
        if (request.callback) {
            request.callback(envelope);
        }
        if (request.timeoutTimer) {
            request.timeoutTimer->stop();
            request.timeoutTimer->deleteLater();
        }
    }
}

void IPCClient::handleEvent(const MessageEnvelope& envelope)
{
    QString messageType = QString::fromStdString(envelope.message_type());

    if (messageType == "DownloadCompletedEvent") {
        DownloadCompletedEvent event;
        if (event.ParseFromString(envelope.payload())) {
            emit downloadCompleted(
                QString::fromStdString(event.download_id()),
                QString::fromStdString(event.model_id()),
                QString::fromStdString(event.local_file_path())
            );
        }
    }
    else if (messageType == "DownloadFailedEvent") {
        DownloadFailedEvent event;
        if (event.ParseFromString(envelope.payload())) {
            emit downloadFailed(
                QString::fromStdString(event.download_id()),
                QString::fromStdString(event.error_message())
            );
        }
    }
    else if (messageType == "LoginStatusChangedEvent") {
        LoginStatusChangedEvent event;
        if (event.ParseFromString(envelope.payload())) {
            emit loginStatusChanged(
                event.is_logged_in(),
                QString::fromStdString(event.user_id())
            );
        }
    }
    else if (messageType == "RenderTaskStatusChangedEvent") {
        RenderTaskStatusChangedEvent event;
        if (event.ParseFromString(envelope.payload())) {
            emit renderTaskStatusChanged(
                QString::fromStdString(event.task_id()),
                QString::fromStdString(event.status()),
                event.progress()
            );
        }
    }
}

// 显式实例化常用模板
template std::optional<ModelsResponse> IPCClient::sendCommandSync(
    const QString&, const google::protobuf::Message&, int);
template std::optional<ModelDetailResponse> IPCClient::sendCommandSync(
    const QString&, const google::protobuf::Message&, int);
template std::optional<DownloadProgressResponse> IPCClient::sendCommandSync(
    const QString&, const google::protobuf::Message&, int);

template void IPCClient::sendCommandAsync<ModelsResponse>(
    const QString&, const google::protobuf::Message&,
    std::function<void(const ModelsResponse&)>, std::function<void(const QString&)>);
template void IPCClient::sendCommandAsync<SuccessResponse>(
    const QString&, const google::protobuf::Message&,
    std::function<void(const SuccessResponse&)>, std::function<void(const QString&)>);
```

---

## 🎨 Qt UI层技术方案

### UI模块技术选型

| 模块 | 推荐技术 | 理由 |
|------|---------|------|
| **主窗口框架** | Qt Widgets (QMainWindow) | 成熟的桌面框架、菜单/工具栏/Dock完善 |
| **素材瀑布流** | **Qt Quick (GridView)** | GPU加速、虚拟化、流畅动画 |
| **素材详情页** | **Qt Quick (SwipeView)** | 图片轮播、缩放、过渡动画 |
| **本地资源树** | Qt Widgets (QTreeView) | 树形结构、拖拽、右键菜单成熟 |
| **下载管理器** | Qt Widgets (QListView) | 列表+进度条、不需要复杂动画 |
| **在线浏览** | Qt WebEngine | Chromium内核、与官网体验一致 |
| **设置页面** | Qt Widgets (QDialog) | 标准表单控件 |
| **用户中心** | **Qt Quick (QML)** | 头像、卡片、动画效果 |

### 高性能Model实现（Qt端）

**素材列表Model**（支持10万+数据）：

```cpp
// model_list_model.h
#pragma once
#include <QAbstractListModel>
#include <QPixmap>
#include <QCache>
#include "ipc_client.h"

class ModelListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        CategoryRole,
        ThumbnailUrlRole,
        ThumbnailRole,      // QPixmap缓存
        FileSizeRole,
        PriceRole,
        DownloadCountRole
    };

    explicit ModelListModel(IPCClient* ipcClient, QObject* parent = nullptr);

    // Model接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 数据加载
    void loadPage(int page, int pageSize = 50);
    void search(const QString& keyword, const QStringList& categories);
    void prefetchRange(int start, int count);

signals:
    void loadingChanged(bool loading);
    void errorOccurred(const QString& error);

private slots:
    void onModelsLoaded(const ModelsResponse& response);
    void onThumbnailLoaded(const QString& modelId, const QPixmap& pixmap);

private:
    struct ModelItem {
        QString id;
        QString name;
        QString category;
        QString thumbnailUrl;
        qint64 fileSize;
        double price;
        int downloadCount;
        QPixmap thumbnail;
        bool thumbnailLoading = false;
    };

    IPCClient* m_ipcClient;
    QVector<ModelItem> m_items;
    int m_totalCount = 0;
    int m_currentPage = 1;
    bool m_loading = false;

    // 缩略图缓存
    QCache<QString, QPixmap> m_thumbnailCache;  // 内存缓存（默认100个）

    void loadThumbnailAsync(const QString& modelId, const QString& url);
};

// model_list_model.cpp
#include "model_list_model.h"
#include <QtConcurrent>
#include <QNetworkAccessManager>
#include <QNetworkReply>

ModelListModel::ModelListModel(IPCClient* ipcClient, QObject* parent)
    : QAbstractListModel(parent)
    , m_ipcClient(ipcClient)
{
    m_thumbnailCache.setMaxCost(100);  // 最多缓存100个缩略图
}

int ModelListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant ModelListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return QVariant();

    const ModelItem& item = m_items[index.row()];

    switch (role) {
        case IdRole: return item.id;
        case NameRole: return item.name;
        case CategoryRole: return item.category;
        case ThumbnailUrlRole: return item.thumbnailUrl;
        case ThumbnailRole:
            // 如果缩略图未加载，触发异步加载
            if (item.thumbnail.isNull() && !item.thumbnailLoading) {
                // 注意：这里修改了const对象，实际应该用mutable或其他机制
                const_cast<ModelListModel*>(this)->loadThumbnailAsync(
                    item.id,
                    item.thumbnailUrl
                );
            }
            return item.thumbnail;
        case FileSizeRole: return item.fileSize;
        case PriceRole: return item.price;
        case DownloadCountRole: return item.downloadCount;
    }

    return QVariant();
}

QHash<int, QByteArray> ModelListModel::roleNames() const
{
    return {
        {IdRole, "modelId"},
        {NameRole, "name"},
        {CategoryRole, "category"},
        {ThumbnailUrlRole, "thumbnailUrl"},
        {ThumbnailRole, "thumbnail"},
        {FileSizeRole, "fileSize"},
        {PriceRole, "price"},
        {DownloadCountRole, "downloadCount"}
    };
}

void ModelListModel::loadPage(int page, int pageSize)
{
    if (m_loading) return;

    m_loading = true;
    emit loadingChanged(true);

    LoadModelsCommand command;
    command.set_page(page);
    command.set_page_size(pageSize);

    m_ipcClient->sendCommandAsync<ModelsResponse>(
        "LoadModelsCommand",
        command,
        [this, page](const ModelsResponse& response) {
            m_loading = false;
            emit loadingChanged(false);

            // 如果是第一页，清空现有数据
            if (page == 1) {
                beginResetModel();
                m_items.clear();
                endResetModel();
            }

            // 添加新数据
            beginInsertRows(QModelIndex(), m_items.size(), 
                            m_items.size() + response.models_size() - 1);

            for (int i = 0; i < response.models_size(); ++i) {
                const auto& model = response.models(i);
                ModelItem item;
                item.id = QString::fromStdString(model.id());
                item.name = QString::fromStdString(model.name());
                item.category = QString::fromStdString(model.category());
                item.thumbnailUrl = QString::fromStdString(model.thumbnail_url());
                item.fileSize = model.file_size();
                item.price = model.price();
                item.downloadCount = model.download_count();

                m_items.append(item);
            }

            endInsertRows();

            m_totalCount = response.total_count();
            m_currentPage = page;
        },
        [this](const QString& error) {
            m_loading = false;
            emit loadingChanged(false);
            emit errorOccurred(error);
        }
    );
}

void ModelListModel::loadThumbnailAsync(const QString& modelId, const QString& url)
{
    // 检查缓存
    if (m_thumbnailCache.contains(modelId)) {
        return;
    }

    // 标记为加载中
    for (auto& item : m_items) {
        if (item.id == modelId) {
            item.thumbnailLoading = true;
            break;
        }
    }

    // 异步下载缩略图
    QtConcurrent::run([this, modelId, url]() {
        QNetworkAccessManager manager;
        QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(url)));

        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QPixmap pixmap;
            pixmap.loadFromData(data);

            if (!pixmap.isNull()) {
                // 主线程更新
                QMetaObject::invokeMethod(this, [this, modelId, pixmap]() {
                    onThumbnailLoaded(modelId, pixmap);
                }, Qt::QueuedConnection);
            }
        }

        reply->deleteLater();
    });
}

void ModelListModel::onThumbnailLoaded(const QString& modelId, const QPixmap& pixmap)
{
    // 更新缓存
    m_thumbnailCache.insert(modelId, new QPixmap(pixmap));

    // 更新Model
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == modelId) {
            m_items[i].thumbnail = pixmap;
            m_items[i].thumbnailLoading = false;

            // 通知视图更新
            QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {ThumbnailRole});
            break;
        }
    }
}

void ModelListModel::prefetchRange(int start, int count)
{
    // 预加载指定范围的缩略图
    for (int i = start; i < start + count && i < m_items.size(); ++i) {
        const auto& item = m_items[i];
        if (item.thumbnail.isNull() && !item.thumbnailLoading) {
            loadThumbnailAsync(item.id, item.thumbnailUrl);
        }
    }
}
```

### QML视图（素材瀑布流）

```qml
// ModelGridView.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property var modelListModel  // C++传入的ModelListModel

    GridView {
        id: gridView
        anchors.fill: parent
        cellWidth: 220
        cellHeight: 280
        cacheBuffer: 1000  // 预加载1000px

        model: root.modelListModel

        delegate: Item {
            width: gridView.cellWidth - 10
            height: gridView.cellHeight - 10

            Rectangle {
                anchors.fill: parent
                anchors.margins: 5
                color: "#FFFFFF"
                radius: 8
                border.color: "#E0E0E0"
                border.width: 1

                // 悬停效果
                states: State {
                    name: "hovered"
                    when: mouseArea.containsMouse
                    PropertyChanges { target: parent; scale: 1.05 }
                    PropertyChanges { target: parent; z: 10 }
                }

                transitions: Transition {
                    NumberAnimation {
                        properties: "scale"
                        duration: 200
                        easing.type: Easing.OutCubic
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    // 缩略图
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Image {
                            id: thumbnail
                            anchors.fill: parent
                            source: model.thumbnailUrl
                            fillMode: Image.PreserveAspectCrop
                            asynchronous: true
                            cache: true

                            // 加载指示器
                            BusyIndicator {
                                anchors.centerIn: parent
                                running: thumbnail.status === Image.Loading
                                visible: running
                            }

                            // 加载失败提示
                            Text {
                                anchors.centerIn: parent
                                text: "加载失败"
                                visible: thumbnail.status === Image.Error
                                color: "#999999"
                            }
                        }

                        // 圆角遮罩
                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: Rectangle {
                                width: thumbnail.width
                                height: thumbnail.height
                                radius: 8
                            }
                        }
                    }

                    // 模型名称
                    Text {
                        Layout.fillWidth: true
                        text: model.name
                        font.pixelSize: 14
                        font.weight: Font.Medium
                        elide: Text.ElideRight
                        maximumLineCount: 2
                        wrapMode: Text.WordWrap
                    }

                    // 信息栏
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        // 价格
                        Text {
                            text: model.price === 0 ? "免费" : "¥" + model.price.toFixed(2)
                            font.pixelSize: 13
                            font.weight: Font.Bold
                            color: model.price === 0 ? "#4CAF50" : "#FF6B6B"
                        }

                        Item { Layout.fillWidth: true }

                        // 下载量
                        Row {
                            spacing: 4
                            Image {
                                width: 16
                                height: 16
                                source: "qrc:/icons/download.svg"
                            }
                            Text {
                                text: model.downloadCount
                                font.pixelSize: 12
                                color: "#888888"
                            }
                        }
                    }
                }

                // 鼠标交互
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        // 打开详情页
                        root.openDetail(model.modelId);
                    }
                }

                // 点击涟漪效果
                Ripple {
                    anchors.fill: parent
                    clip: true
                    pressed: mouseArea.pressed
                    active: mouseArea.pressed || mouseArea.containsMouse
                    color: Qt.rgba(0, 0, 0, 0.1)
                }
            }
        }

        // 滚动到底部加载更多
        onAtYEndChanged: {
            if (atYEnd && !modelListModel.loading) {
                modelListModel.loadPage(modelListModel.currentPage + 1);
            }
        }

        // 滚动时预加载
        onContentYChanged: {
            var firstVisible = indexAt(contentX, contentY);
            var lastVisible = indexAt(contentX, contentY + height);

            if (firstVisible >= 0 && lastVisible >= 0) {
                // 预加载前后各20条
                modelListModel.prefetchRange(
                    Math.max(0, firstVisible - 20),
                    lastVisible - firstVisible + 40
                );
            }
        }

        // 滚动条
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            minimumSize: 0.1
        }
    }

    // 加载指示器
    BusyIndicator {
        anchors.centerIn: parent
        running: root.modelListModel && root.modelListModel.loading
        visible: running
    }

    // 信号
    signal openDetail(string modelId)
}
```

---

## 📦 模块功能分配

### .NET服务进程负责

| 模块 | 功能 | 技术 |
|------|------|------|
| **业务逻辑** | 模型搜索、分类、排序、筛选 | C# + .NET 8 |
| **数据访问** | SQLite读写、数据库迁移 | Dapper + FluentMigrator |
| **网络通信** | HTTP API调用、WebSocket连接 | HttpClient + ClientWebSocket |
| **下载管理** | 多线程下载、断点续传、队列管理 | HttpClient + FileStream |
| **云存储** | OSS/COS/BCE文件上传下载 | SDK封装 |
| **用户认证** | 登录、注册、Token管理 | JWT + HttpClient |
| **云渲染** | 任务提交、进度查询、结果下载 | HTTP API + WebSocket |
| **3ds Max集成** | COM Interop调用Max API | COM (Windows) |
| **日志** | 结构化日志、日志轮转 | Serilog |
| **配置** | 应用配置、用户偏好 | Microsoft.Extensions.Configuration |

### Qt UI进程负责

| 模块 | 功能 | 技术 |
|------|------|------|
| **UI渲染** | 窗口、控件、布局、动画 | Qt Widgets + Qt Quick |
| **用户交互** | 鼠标、键盘、触摸事件 | Qt Event System |
| **IPC客户端** | 与.NET服务通信 | QLocalSocket + Protobuf |
| **本地缓存** | 缩略图、临时数据 | QCache + QFile |
| **在线浏览** | 嵌入Web页面（可选） | Qt WebEngine |
| **性能优化** | 虚拟化、异步加载、GPU加速 | QML Scene Graph |

---

## 🚀 部署架构

### 进程启动流程

```
用户启动应用 (LiuYunKu.exe / LiuYunKu)
        ↓
  启动.NET服务进程 (LiuYunKuService.exe)
        ├─ 初始化数据库连接
        ├─ 启动IPC Server (Named Pipe)
        ├─ 加载配置
        └─ 等待IPC连接
        ↓
  启动Qt UI进程 (LiuYunKuUI.exe)
        ├─ 连接IPC Server
        ├─ 显示闪屏
        ├─ 加载主窗口
        └─ 显示UI
```

### 目录结构

```
LiuYunKu/
├── LiuYunKuUI.exe           (Qt UI进程，主入口)
├── LiuYunKuService.exe      (.NET服务进程)
├── liuyunku.proto           (Protobuf协议定义)
├── libs/
│   ├── Qt6Core.dll
│   ├── Qt6Widgets.dll
│   ├── Qt6Quick.dll
│   └── ... (其他Qt库)
├── plugins/
│   └── platforms/
│       └── qwindows.dll
├── qml/
│   ├── ModelGridView.qml
│   ├── ModelDetailPage.qml
│   └── ...
├── data/
│   ├── liuyunku.db          (SQLite数据库)
│   └── config.json
├── cache/
│   ├── thumbnails/
│   └── temp/
└── logs/
    ├── ui.log
    └── service.log
```

---

## ⚡ 性能优化策略

### 启动性能

**目标**：< 1.5秒

| 优化项 | 实施方案 | 预期提升 |
|--------|---------|---------|
| **服务进程预启动** | Windows服务/守护进程常驻 | 减少500ms |
| **并行初始化** | UI和服务进程并行启动 | 减少40% |
| **延迟加载** | WebEngine等重型组件按需加载 | 减少30% |
| **缓存预热** | 预加载最近使用数据 | 提升感知速度 |

### 内存优化

**目标**：< 300MB

| 优化项 | 实施方案 | 预期提升 |
|--------|---------|---------|
| **进程隔离** | UI和服务独立内存空间 | 降低耦合 |
| **LRU缓存** | 缩略图、模型数据限制 | 减少150MB |
| **智能GC** | .NET端定期GC触发 | 稳定内存 |
| **Qt缓存** | QPixmapCache限制大小 | 减少100MB |

### IPC性能

**目标**：延迟< 2ms（99分位）

| 优化项 | 实施方案 |
|--------|---------|
| **二进制协议** | Protobuf替代JSON |
| **批量传输** | 合并小消息减少调用 |
| **异步通信** | 非阻塞调用不卡UI |
| **本地缓存** | 减少IPC调用频率 |

---

## 📅 实施计划

### 总体时间线：12个月

```
阶段1：基础架构 + 核心功能（4个月）
├─ M1: 架构搭建（Week 1-4）
│   ├─ Protobuf协议定义
│   ├─ IPC通信框架（C# + Qt）
│   ├─ .NET服务基础架构
│   └─ Qt UI基础框架
├─ M2: 核心UI模块（Week 5-8）
│   ├─ 主窗口框架（菜单/工具栏/布局）
│   ├─ 素材瀑布流（QML + Model）
│   └─ 本地资源树（TreeView）
├─ M3: 核心业务功能（Week 9-12）
│   ├─ 模型搜索/分类/筛选
│   ├─ 下载管理器
│   └─ 用户登录/认证
└─ M4: MVP版本（Week 13-16）
    ├─ 功能集成测试
    ├─ 性能优化（启动/内存）
    └─ 内部试用

阶段2：完整功能 + 跨平台（4个月）
├─ M5: 剩余UI模块（Week 17-20）
│   ├─ 素材详情页
│   ├─ 设置页面
│   └─ 用户中心
├─ M6: 云服务集成（Week 21-24）
│   ├─ 云渲染集成
│   ├─ 云存储集成
│   └─ WebSocket实时推送
├─ M7: 跨平台适配（Week 25-28）
│   ├─ macOS平台构建
│   ├─ Linux平台构建
│   └─ 平台差异处理
└─ M8: 完整版本（Week 29-32）
    ├─ 全功能测试
    ├─ 跨平台测试
    └─ Beta测试

阶段3：优化 + 发布（4个月）
├─ M9: 性能优化（Week 33-36）
│   ├─ 深度性能分析
│   ├─ 内存泄漏排查
│   └─ 渲染性能优化
├─ M10: 测试完善（Week 37-40）
│   ├─ 自动化测试
│   ├─ 兼容性测试
│   └─ 压力测试
├─ M11: 灰度发布（Week 41-44）
│   ├─ 10%用户灰度
│   ├─ 50%用户灰度
│   └─ 问题修复
└─ M12: 正式发布（Week 45-48）
    ├─ 100%用户切换
    ├─ 旧版本下线
    └─ 文档归档
```

### 关键里程碑

| 里程碑 | 时间 | 交付物 | 验收标准 |
|--------|------|--------|---------|
| **M4 (MVP)** | 第4月末 | Windows基础版本 | 核心功能可用，IPC稳定，性能达标 |
| **M8 (完整版)** | 第8月末 | 跨平台完整版本 | 全功能可用，三大平台稳定运行 |
| **M11 (灰度)** | 第11月末 | 灰度版本 | 50%用户无重大问题 |
| **M12 (正式)** | 第12月末 | 正式版本 | 100%用户切换，旧版下线 |

---

## 💰 成本估算

### 人力成本

| 角色 | 人数 | 月薪（元） | 月数 | 小计（元） |
|------|-----|-----------|------|-----------|
| **架构师** | 1 | 38,000 | 12 | 456,000 |
| **Qt高级开发** | 2 | 28,000 | 12 | 672,000 |
| **.NET高级开发** | 1 | 26,000 | 12 | 312,000 |
| **Qt/C++中级开发** | 1 | 20,000 | 12 | 240,000 |
| **.NET中级开发** | 1 | 18,000 | 8 | 144,000 |
| **测试工程师** | 1 | 18,000 | 12 | 216,000 |
| **UI/UX设计师** | 1 | 20,000 | 6 | 120,000 |
| **合计** | **8** | - | - | **2,160,000** |

### 其他成本

| 项目 | 金额（元） |
|------|-----------|
| 开发工具（Qt Creator/CLion/VS/Rider） | 25,000 |
| CI/CD（GitHub Actions/GitLab CI） | 40,000 |
| 云服务（测试环境） | 50,000 |
| 培训费用（Qt课程） | 60,000 |
| 外部咨询（Qt专家） | 100,000 |
| **合计** | **275,000** |

### 总成本

**总计**：约 **240万元人民币**

**成本优势**：
- 相比Qt完全重构节省45%（440万 vs 240万）
- 相比WPF优化升级增加15%（210万 vs 240万）
- 获得跨平台能力 + 技术债清理 + 风险可控

---

## ⚠️ 风险评估与缓解

| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|---------|
| **IPC性能不达标** | 高 | 低 | 提前POC验证，备选共享内存方案 |
| **Qt团队经验不足** | 中 | 中 | 提前2月培训，招聘1-2名Qt专家 |
| **.NET业务逻辑复杂** | 中 | 中 | 充分理解现有代码，保留核心开发人员 |
| **跨平台兼容性问题** | 中 | 中 | 早期并行测试三大平台，使用Qt标准API |
| **进程间依赖导致稳定性** | 高 | 低 | 服务进程健康检查，自动重启机制 |
| **迁移周期超期** | 中 | 中 | 分阶段交付，MVP优先，持续监控进度 |

---

## ✅ 总结与建议

### 核心优势

1. **风险可控**：保留成熟C#业务逻辑，仅重写UI层
2. **成本合理**：240万12个月，相比全面重写节省45%
3. **快速交付**：4个月MVP，8个月完整版，12个月正式发布
4. **跨平台**：Qt UI层支持Windows/macOS/Linux
5. **技术先进**：Qt6现代化UI + .NET 8高性能后端
6. **团队友好**：C#团队继续维护业务，小团队学Qt即可

### 适用场景

✅ **强烈推荐**混合方案的场景：

1. **有跨平台需求但预算有限**（240万 vs 440万）
2. **业务逻辑复杂且稳定**（重写风险高）
3. **团队C#背景深厚**（继续利用现有技能）
4. **需要快速交付**（12个月 vs 18个月）
5. **重视风险控制**（渐进式重构，可随时调整）

### 与完全重构对比

| 维度 | 混合方案 ✅ | Qt完全重构 |
|------|-----------|-----------|
| **跨平台能力** | ✅ 完整支持 | ✅ 完整支持 |
| **开发成本** | ✅ 240万/12月 | 440万/18月 |
| **风险** | ✅ 低（保留业务逻辑） | 高（全面重写） |
| **交付速度** | ✅ 4月MVP | 6月MVP |
| **技术债清理** | ✅ UI层彻底清理 | ✅ 全栈清理 |
| **团队转型** | ✅ 小团队学Qt | 全员转C++ |
| **技术先进性** | ✅ Qt6 + .NET 8 | ✅ Qt6 + C++17 |
| **维护成本** | 中（两套技术栈） | 低（单一技术栈） |

### 实施建议

1. **立即启动**（Week 1-2）：
   - [ ] 组建8人混合团队（架构师+Qt开发+.NET开发+测试+UI设计）
   - [ ] 开展Qt培训（2个月，2-3名核心开发）
   - [ ] POC验证IPC性能（Named Pipe + Protobuf）

2. **第一阶段准备**（Week 3-4）：
   - [ ] 定义完整Protobuf协议
   - [ ] 搭建.NET服务架构（IPC Server + DI容器）
   - [ ] 搭建Qt UI框架（主窗口 + IPC Client）

3. **持续执行**：
   - [ ] 每周进度同步会议
   - [ ] 每月里程碑验收
   - [ ] 每季度性能评审
   - [ ] 持续集成/持续部署

### 决策建议

✅ **强烈推荐采用混合架构方案**，理由：

1. **最佳平衡点**：跨平台能力 + 可控风险 + 合理成本
2. **快速见效**：4个月MVP，8个月完整版
3. **技术先进**：Qt现代化UI + .NET高性能后端
4. **渐进式**：可根据进展随时调整策略
5. **长期收益**：双进程架构便于后续演进

---

**文档版本**：v1.0  
**编写日期**：2026年5月13日  
**预计实施周期**：12个月  
**预计成本**：约240万元人民币  
**推荐指数**：⭐⭐⭐⭐⭐（5星）
