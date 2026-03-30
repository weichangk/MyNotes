# QProcess

## 概述

`QProcess` 是 Qt 提供的跨平台进程管理类，用于**启动外部程序、与其交互（读写标准输入/输出/错误流）并获取退出状态**。它继承自 `QIODevice`，因此可以像操作文件一样读写子进程的输入输出。

典型应用场景：

- 调用系统命令（`ping`、`git`、`ffmpeg`、`cmake` 等）
- 运行脚本（Python、Shell、PowerShell）
- 启动子进程并通过管道通信
- 进程间通信（IPC）的简单方案
- 构建工具链中的编译/打包步骤

---

## 1. 基本用法

### 1.1 启动进程

```cpp
#include <QProcess>

QProcess process;

// 方式一：program + arguments 分开传递（推荐）
process.start("git", {"log", "--oneline", "-5"});

// 方式二：单字符串（Qt 会自动分割，但路径含空格时容易出问题）
process.start("git log --oneline -5");

// 方式三：先设置参数再启动
process.setProgram("ffmpeg");
process.setArguments({"-i", "input.mp4", "-c:v", "libx264", "output.mp4"});
process.start();
```

### 1.2 等待完成并读取输出

```cpp
QProcess process;
process.start("git", {"status"});

// 阻塞等待进程结束
if (!process.waitForFinished(5000)) { // 最多等 5 秒
    qWarning() << "进程超时:" << process.errorString();
    return;
}

// 读取标准输出和错误输出
QString stdOut = QString::fromUtf8(process.readAllStandardOutput());
QString stdErr = QString::fromUtf8(process.readAllStandardError());

qDebug() << "Exit code:" << process.exitCode();
qDebug() << "stdout:" << stdOut;
qDebug() << "stderr:" << stdErr;
```

### 1.3 一行搞定的静态方法（Qt 5.15 阻塞式）

```cpp
// QProcess::execute() —— 阻塞执行，返回退出码
int exitCode = QProcess::execute("ping", {"-c", "4", "google.com"});

// QProcess::startDetached() —— 启动后与父进程脱离
bool ok = QProcess::startDetached("notepad.exe", {"readme.txt"});
```

> **注意**：`execute()` 会阻塞当前线程直到子进程结束，不适合在 GUI 线程中使用。

---

## 2. 进程的启动模式

### 2.1 start() vs startDetached()

| 特性 | `start()` | `startDetached()` |
|---|---|---|
| 父子关系 | 子进程与 QProcess 对象关联 | 完全独立，无父子关系 |
| 输入输出 | 可读写 stdin/stdout/stderr | 不能通信 |
| 生命周期 | QProcess 销毁时子进程可能被影响 | 独立存活 |
| 退出状态 | 可获取 exitCode/exitStatus | 不可获取 |
| 典型用途 | 执行命令并获取结果 | 启动独立应用程序 |

```cpp
// 启动独立进程（不等待、不通信）
qint64 pid;
QProcess::startDetached("explorer.exe", {"/select,", filePath}, QString(), &pid);
qDebug() << "启动进程 PID:" << pid;
```

### 2.2 open() —— 指定 I/O 模式

```cpp
QProcess process;
process.setProgram("python");
process.setArguments({"-i"}); // Python 交互模式

// 只开写通道（只向子进程发送输入）
process.open(QIODevice::WriteOnly);

// 只开读通道
process.open(QIODevice::ReadOnly);

// 读写（默认）
process.open(QIODevice::ReadWrite);
```

---

## 3. 输入输出交互

### 3.1 读取标准输出和标准错误

```cpp
QProcess process;
process.start("cmake", {"--version"});
process.waitForFinished();

// 方式一：分开读取
QByteArray stdOut = process.readAllStandardOutput();
QByteArray stdErr = process.readAllStandardError();

// 方式二：合并到标准输出
process.setProcessChannelMode(QProcess::MergedChannels);
process.start("make", {"-j8"});
process.waitForFinished();
QByteArray allOutput = process.readAll(); // stdout + stderr 混合
```

### 3.2 通道模式

```cpp
// 标准输出和错误分开（默认）
process.setProcessChannelMode(QProcess::SeparateChannels);

// 标准错误合并到标准输出
process.setProcessChannelMode(QProcess::MergedChannels);

// 完全转发到父进程的对应通道（用于命令行工具）
process.setProcessChannelMode(QProcess::ForwardedChannels);

// 只转发错误输出
process.setProcessChannelMode(QProcess::ForwardedErrorChannel);

// 只转发标准输出
process.setProcessChannelMode(QProcess::ForwardedOutputChannel);
```

### 3.3 写入标准输入

```cpp
QProcess process;
process.start("python3", {"-c", "import sys; print('你输入了:', sys.stdin.readline())"});

// 向子进程写入数据
process.write("Hello from Qt!\n");
process.closeWriteChannel(); // 关闭写通道，子进程收到 EOF

process.waitForFinished();
qDebug() << process.readAllStandardOutput();
// 输出：你输入了: Hello from Qt!
```

### 3.4 交互式通信

持续与子进程交互（如调用 Python 解释器、数据库 CLI 等）：

```cpp
class PythonRunner : public QObject
{
    Q_OBJECT
public:
    PythonRunner(QObject *parent = nullptr) : QObject(parent) {
        m_process = new QProcess(this);
        m_process->setProcessChannelMode(QProcess::MergedChannels);

        connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
            QByteArray data = m_process->readAll();
            emit outputReceived(QString::fromUtf8(data));
        });

        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this](int exitCode, QProcess::ExitStatus status) {
            emit processFinished(exitCode);
        });
    }

    void start() {
        m_process->start("python3", {"-u", "-i"}); // -u 无缓冲，-i 交互模式
    }

    void sendCommand(const QString &cmd) {
        m_process->write((cmd + "\n").toUtf8());
    }

    void stop() {
        m_process->write("exit()\n");
        m_process->waitForFinished(3000);
    }

signals:
    void outputReceived(const QString &output);
    void processFinished(int exitCode);

private:
    QProcess *m_process;
};

// 使用
PythonRunner runner;
runner.start();
runner.sendCommand("print(1 + 2)");
runner.sendCommand("import os; print(os.getcwd())");
// outputReceived 信号接收输出
```

---

## 4. 信号驱动的异步模式

### 4.1 核心信号

| 信号 | 触发时机 |
|---|---|
| `started()` | 子进程成功启动 |
| `finished(int exitCode, QProcess::ExitStatus)` | 子进程结束 |
| `errorOccurred(QProcess::ProcessError)` | 发生错误 |
| `readyReadStandardOutput()` | 标准输出有新数据可读 |
| `readyReadStandardError()` | 标准错误有新数据可读 |
| `stateChanged(QProcess::ProcessState)` | 进程状态变化 |
| `readyRead()` | 当前读通道有新数据（继承自 QIODevice） |

### 4.2 完整的异步执行示例

```cpp
class CommandRunner : public QObject
{
    Q_OBJECT
public:
    void run(const QString &program, const QStringList &args)
    {
        auto *process = new QProcess(this);

        // 收集输出
        QByteArray *stdOutBuffer = new QByteArray;
        QByteArray *stdErrBuffer = new QByteArray;

        connect(process, &QProcess::readyReadStandardOutput, this,
                [process, stdOutBuffer]() {
            stdOutBuffer->append(process->readAllStandardOutput());
        });

        connect(process, &QProcess::readyReadStandardError, this,
                [process, stdErrBuffer]() {
            stdErrBuffer->append(process->readAllStandardError());
        });

        connect(process, &QProcess::errorOccurred, this,
                [this, process](QProcess::ProcessError error) {
            emit commandError(errorToString(error));
            process->deleteLater();
        });

        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, process, stdOutBuffer, stdErrBuffer]
                      (int exitCode, QProcess::ExitStatus status) {
            emit commandFinished(exitCode,
                                 QString::fromUtf8(*stdOutBuffer),
                                 QString::fromUtf8(*stdErrBuffer));
            delete stdOutBuffer;
            delete stdErrBuffer;
            process->deleteLater();
        });

        process->start(program, args);
    }

signals:
    void commandFinished(int exitCode, const QString &stdOut, const QString &stdErr);
    void commandError(const QString &error);

private:
    static QString errorToString(QProcess::ProcessError error) {
        switch (error) {
        case QProcess::FailedToStart: return "启动失败（程序不存在或无权限）";
        case QProcess::Crashed:       return "进程崩溃";
        case QProcess::Timedout:      return "操作超时";
        case QProcess::WriteError:    return "写入错误";
        case QProcess::ReadError:     return "读取错误";
        default:                      return "未知错误";
        }
    }
};

// 使用：不阻塞 UI
CommandRunner runner;
connect(&runner, &CommandRunner::commandFinished, this,
        [this](int code, const QString &out, const QString &err) {
    m_outputEdit->setPlainText(out);
    if (code != 0)
        m_statusBar->showMessage("命令失败: " + err);
});
runner.run("git", {"diff", "--stat"});
```

---

## 5. 进程状态与错误处理

### 5.1 进程状态

```cpp
QProcess::ProcessState state = process.state();
```

| 状态 | 枚举值 | 说明 |
|---|---|---|
| 未启动 | `QProcess::NotRunning` | 进程未运行 |
| 正在启动 | `QProcess::Starting` | 正在启动子进程 |
| 运行中 | `QProcess::Running` | 子进程正在运行 |

### 5.2 错误类型

| 错误 | 枚举值 | 常见原因 |
|---|---|---|
| 启动失败 | `QProcess::FailedToStart` | 程序不存在、无执行权限、路径错误 |
| 崩溃 | `QProcess::Crashed` | 子进程异常终止（段错误等） |
| 超时 | `QProcess::Timedout` | `waitFor*()` 超时 |
| 写入错误 | `QProcess::WriteError` | 写管道失败 |
| 读取错误 | `QProcess::ReadError` | 读管道失败 |
| 未知错误 | `QProcess::UnknownError` | 其他错误 |

### 5.3 退出状态

```cpp
connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, [](int exitCode, QProcess::ExitStatus status) {
    if (status == QProcess::NormalExit) {
        qDebug() << "正常退出，退出码:" << exitCode;
        // exitCode 由子进程的 return/exit() 决定
    } else {
        qDebug() << "异常退出（崩溃）";
    }
});
```

### 5.4 健壮的错误处理模板

```cpp
void executeCommand(const QString &cmd, const QStringList &args)
{
    QProcess process;
    process.start(cmd, args);

    if (!process.waitForStarted(3000)) {
        qCritical() << "启动失败:" << process.errorString();
        return;
    }

    if (!process.waitForFinished(30000)) {
        qWarning() << "进程超时，强制终止";
        process.kill();              // 发送 SIGKILL (Unix) / TerminateProcess (Win)
        process.waitForFinished(3000);
        return;
    }

    if (process.exitStatus() == QProcess::CrashExit) {
        qCritical() << "进程崩溃";
        return;
    }

    if (process.exitCode() != 0) {
        qWarning() << "命令失败，退出码:" << process.exitCode();
        qWarning() << "错误输出:" << process.readAllStandardError();
        return;
    }

    qDebug() << "成功:" << process.readAllStandardOutput();
}
```

---

## 6. 环境变量与工作目录

### 6.1 设置工作目录

```cpp
QProcess process;
process.setWorkingDirectory("/path/to/project");
process.start("cmake", {"--build", ".", "--config", "Release"});
```

### 6.2 设置环境变量

```cpp
QProcess process;

// 方式一：继承当前环境 + 修改
QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
env.insert("MY_VAR", "hello");
env.insert("PATH", "/custom/bin:" + env.value("PATH"));
env.remove("DEBUG_MODE");
process.setProcessEnvironment(env);

// 方式二：全新环境（不继承父进程的环境变量）
QProcessEnvironment cleanEnv;
cleanEnv.insert("PATH", "/usr/bin:/bin");
cleanEnv.insert("HOME", "/tmp");
process.setProcessEnvironment(cleanEnv);

process.start("my_program");
```

### 6.3 查询环境变量

```cpp
QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

// 读取
QString path = env.value("PATH");
QString home = env.value("HOME", "/default/home"); // 带默认值

// 检查
if (env.contains("JAVA_HOME")) {
    qDebug() << "Java:" << env.value("JAVA_HOME");
}

// 遍历
QStringList keys = env.keys();
for (const QString &key : keys) {
    qDebug() << key << "=" << env.value(key);
}
```

---

## 7. 终止进程

### 7.1 三种终止方式

```cpp
// 1. 优雅终止：发送关闭请求
process.terminate();
// Unix: 发送 SIGTERM（进程可以捕获并清理）
// Windows: 发送 WM_CLOSE 到顶层窗口

// 2. 强制终止：立即杀死进程
process.kill();
// Unix: 发送 SIGKILL（无法被捕获或忽略）
// Windows: 调用 TerminateProcess

// 3. 关闭写通道
process.closeWriteChannel(); // 子进程收到 EOF，可能自行退出
```

### 7.2 推荐的终止流程

```cpp
void gracefulStop(QProcess *process, int timeoutMs = 5000)
{
    if (process->state() == QProcess::NotRunning)
        return;

    // 第一步：关闭写通道（某些程序收到 EOF 会自动退出）
    process->closeWriteChannel();

    if (process->waitForFinished(1000))
        return;

    // 第二步：优雅终止
    process->terminate();

    if (process->waitForFinished(timeoutMs))
        return;

    // 第三步：强制终止
    qWarning() << "进程未响应 terminate，强制 kill";
    process->kill();
    process->waitForFinished(3000);
}
```

---

## 8. 管道与进程链

### 8.1 管道串联（A | B）

模拟 Shell 管道：将进程 A 的标准输出连接到进程 B 的标准输入。

```cpp
// 模拟： cat file.txt | grep "error" | wc -l
QProcess catProcess;
QProcess grepProcess;
QProcess wcProcess;

// 设置管道链
catProcess.setStandardOutputProcess(&grepProcess);
grepProcess.setStandardOutputProcess(&wcProcess);

// 按逆序启动（接收端先准备好）
wcProcess.start("wc", {"-l"});
grepProcess.start("grep", {"error"});
catProcess.start("cat", {"server.log"});

// 等待最终结果
wcProcess.waitForFinished();
QString count = QString::fromUtf8(wcProcess.readAllStandardOutput()).trimmed();
qDebug() << "包含 error 的行数:" << count;
```

### 8.2 重定向到文件

```cpp
QProcess process;

// 标准输出写入文件
process.setStandardOutputFile("output.log");

// 标准错误追加到文件
process.setStandardErrorFile("error.log", QIODevice::Append);

// 标准输入从文件读取
process.setStandardInputFile("input.txt");

process.start("my_program");
process.waitForFinished();
```

### 8.3 输出重定向到 /dev/null（丢弃）

```cpp
process.setStandardOutputFile(QProcess::nullDevice());
process.setStandardErrorFile(QProcess::nullDevice());
```

---

## 9. 实战示例

### 9.1 FFmpeg 视频转码器

```cpp
class VideoConverter : public QObject
{
    Q_OBJECT
public:
    void convert(const QString &input, const QString &output)
    {
        m_process = new QProcess(this);
        m_process->setProcessChannelMode(QProcess::MergedChannels);

        connect(m_process, &QProcess::readyRead, this, [this]() {
            QString line = QString::fromUtf8(m_process->readAll());
            // FFmpeg 进度输出格式：frame= 1234 fps= 60 ... time=00:01:23.45
            QRegularExpression rx(R"(time=(\d+):(\d+):(\d+)\.(\d+))");
            auto match = rx.match(line);
            if (match.hasMatch()) {
                int hours = match.captured(1).toInt();
                int mins = match.captured(2).toInt();
                int secs = match.captured(3).toInt();
                int totalSecs = hours * 3600 + mins * 60 + secs;
                emit progressChanged(totalSecs);
            }
        });

        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this](int code, QProcess::ExitStatus status) {
            if (status == QProcess::NormalExit && code == 0)
                emit finished(true, "转码成功");
            else
                emit finished(false, "转码失败，退出码: " + QString::number(code));
            m_process->deleteLater();
            m_process = nullptr;
        });

        m_process->start("ffmpeg", {
            "-i", input,
            "-c:v", "libx264",
            "-preset", "medium",
            "-crf", "23",
            "-c:a", "aac",
            "-y",       // 覆盖输出文件
            output
        });
    }

    void cancel() {
        if (m_process && m_process->state() == QProcess::Running) {
            // FFmpeg 用 'q' 键优雅退出
            m_process->write("q");
            if (!m_process->waitForFinished(5000))
                m_process->kill();
        }
    }

signals:
    void progressChanged(int seconds);
    void finished(bool success, const QString &message);

private:
    QProcess *m_process = nullptr;
};
```

### 9.2 Git 操作封装

```cpp
class GitHelper : public QObject
{
    Q_OBJECT
public:
    explicit GitHelper(const QString &repoPath, QObject *parent = nullptr)
        : QObject(parent), m_repoPath(repoPath) {}

    // 同步方法（适用于快速命令）
    struct GitResult {
        int exitCode;
        QString output;
        QString error;
        bool success() const { return exitCode == 0; }
    };

    GitResult runGit(const QStringList &args, int timeoutMs = 10000)
    {
        QProcess process;
        process.setWorkingDirectory(m_repoPath);
        process.start("git", args);

        GitResult result;
        if (!process.waitForFinished(timeoutMs)) {
            process.kill();
            process.waitForFinished(3000);
            result.exitCode = -1;
            result.error = "超时";
            return result;
        }

        result.exitCode = process.exitCode();
        result.output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        result.error = QString::fromUtf8(process.readAllStandardError()).trimmed();
        return result;
    }

    // 便捷方法
    QString currentBranch() {
        auto r = runGit({"rev-parse", "--abbrev-ref", "HEAD"});
        return r.success() ? r.output : "unknown";
    }

    QStringList modifiedFiles() {
        auto r = runGit({"diff", "--name-only"});
        return r.success() ? r.output.split('\n', Qt::SkipEmptyParts) : QStringList();
    }

    bool commit(const QString &message) {
        return runGit({"commit", "-m", message}).success();
    }

    // 异步方法（适用于耗时命令如 clone、pull）
    void asyncClone(const QUrl &url, const QString &targetDir)
    {
        auto *process = new QProcess(this);

        connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
            QString line = QString::fromUtf8(process->readAllStandardError());
            // Git clone 进度输出在 stderr
            emit cloneProgress(line.trimmed());
        });

        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, process](int code, QProcess::ExitStatus) {
            emit cloneDone(code == 0);
            process->deleteLater();
        });

        process->start("git", {"clone", "--progress", url.toString(), targetDir});
    }

signals:
    void cloneProgress(const QString &info);
    void cloneDone(bool success);

private:
    QString m_repoPath;
};

// 使用
GitHelper git("/path/to/repo");
qDebug() << "当前分支:" << git.currentBranch();
qDebug() << "修改的文件:" << git.modifiedFiles();

auto result = git.runGit({"log", "--oneline", "-5"});
if (result.success())
    qDebug() << result.output;
```

### 9.3 系统信息收集器

```cpp
class SystemInfo
{
public:
    static QString cpuInfo() {
#ifdef Q_OS_WIN
        return runCommand("wmic", {"cpu", "get", "Name", "/value"});
#elif defined(Q_OS_LINUX)
        return runCommand("sh", {"-c", "cat /proc/cpuinfo | grep 'model name' | head -1"});
#elif defined(Q_OS_MAC)
        return runCommand("sysctl", {"-n", "machdep.cpu.brand_string"});
#endif
    }

    static QString diskUsage(const QString &path = "/") {
#ifdef Q_OS_WIN
        return runCommand("cmd", {"/c", "wmic logicaldisk get size,freespace,caption"});
#else
        return runCommand("df", {"-h", path});
#endif
    }

    static QStringList runningProcesses() {
#ifdef Q_OS_WIN
        QString output = runCommand("tasklist", {"/FO", "CSV", "/NH"});
#else
        QString output = runCommand("ps", {"aux", "--no-headers"});
#endif
        return output.split('\n', Qt::SkipEmptyParts);
    }

    static bool isPortInUse(int port) {
#ifdef Q_OS_WIN
        QString output = runCommand("netstat", {"-an"});
#else
        QString output = runCommand("ss", {"-tln"});
#endif
        return output.contains(QString::number(port));
    }

private:
    static QString runCommand(const QString &cmd, const QStringList &args) {
        QProcess process;
        process.start(cmd, args);
        if (!process.waitForFinished(5000))
            return {};
        return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    }
};
```

### 9.4 带实时输出的编译器集成

```cpp
class BuildRunner : public QObject
{
    Q_OBJECT
public:
    void build(const QString &buildDir)
    {
        auto *process = new QProcess(this);
        process->setWorkingDirectory(buildDir);
        process->setProcessChannelMode(QProcess::MergedChannels);

        connect(process, &QProcess::readyRead, this, [this, process]() {
            // 逐行解析输出
            while (process->canReadLine()) {
                QString line = QString::fromUtf8(process->readLine()).trimmed();
                parseBuildOutput(line);
            }
        });

        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, process](int code, QProcess::ExitStatus) {
            // 读取剩余数据
            QString remaining = QString::fromUtf8(process->readAll()).trimmed();
            if (!remaining.isEmpty())
                parseBuildOutput(remaining);

            emit buildFinished(code == 0);
            process->deleteLater();
        });

        process->start("cmake", {"--build", ".", "--config", "Release", "-j"});
    }

private:
    void parseBuildOutput(const QString &line) {
        if (line.contains("error", Qt::CaseInsensitive)) {
            emit buildOutput(line, "error");
        } else if (line.contains("warning", Qt::CaseInsensitive)) {
            emit buildOutput(line, "warning");
        } else if (line.contains("Building") || line.contains("Linking")) {
            emit buildOutput(line, "info");
        } else {
            emit buildOutput(line, "normal");
        }
    }

signals:
    void buildOutput(const QString &line, const QString &level);
    void buildFinished(bool success);
};
```

---

## 10. 跨平台注意事项

### 10.1 程序查找

```cpp
// Qt 会自动在 PATH 中查找程序
process.start("python3", args); // Qt 在 PATH 中搜索 python3

// Windows 的特殊情况
process.start("notepad");       // 可以省略 .exe
process.start("cmd", {"/c", "dir"}); // 运行内部命令需要 cmd /c

// 绝对路径（避免 PATH 问题）
process.start("C:/Program Files/Git/bin/git.exe", {"status"});
```

### 10.2 Shell 命令

```cpp
// ❌ 不能直接运行 Shell 语法
process.start("ls *.txt");           // 通配符由 shell 展开，QProcess 不理解
process.start("echo $HOME");         // 环境变量由 shell 展开

// ✅ 通过 Shell 执行
#ifdef Q_OS_WIN
    process.start("cmd", {"/c", "dir /b *.txt"});
    // 或 PowerShell
    process.start("powershell", {"-Command", "Get-ChildItem *.txt"});
#else
    process.start("sh", {"-c", "ls *.txt"});
    // 或
    process.start("bash", {"-c", "echo $HOME && ls -la"});
#endif
```

### 10.3 路径中的空格

```cpp
// ❌ 问题：路径含空格
process.start("C:/Program Files/My App/app.exe arg1 arg2");
// Qt 可能将路径分割错误

// ✅ 使用分离的 program 和 arguments
process.start("C:/Program Files/My App/app.exe", {"arg1", "arg2"});

// ✅ 或使用引号（单字符串模式）
process.start("\"C:/Program Files/My App/app.exe\" arg1 arg2");
```

### 10.4 编码问题

```cpp
// Windows 默认使用系统编码（如 GBK），非 UTF-8
QProcess process;
process.start("cmd", {"/c", "chcp 65001 && dir"}); // 切换到 UTF-8 代码页

// 或者使用正确的编码转换
process.waitForFinished();
QByteArray raw = process.readAllStandardOutput();

// Windows 使用本地编码
QString output = QString::fromLocal8Bit(raw);

// Unix 通常是 UTF-8
QString output = QString::fromUtf8(raw);
```

---

## 11. 高级用法

### 11.1 Native 进程参数（Qt 5.15+ / 6.0+）

```cpp
QProcess process;

#ifdef Q_OS_WIN
    // Windows：设置创建标志
    process.setCreateProcessArgumentsModifier(
        [](QProcess::CreateProcessArguments *args) {
            args->flags |= CREATE_NO_WINDOW;      // 不创建控制台窗口
            // args->flags |= CREATE_NEW_CONSOLE; // 创建新的控制台窗口
        });
#endif

process.start("my_background_tool", args);
```

### 11.2 监控多个并发进程

```cpp
class BatchExecutor : public QObject
{
    Q_OBJECT
public:
    void runBatch(const QList<QPair<QString, QStringList>> &commands)
    {
        m_total = commands.size();
        m_completed = 0;
        m_errors = 0;

        for (const auto &[cmd, args] : commands) {
            auto *process = new QProcess(this);

            connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    this, [this, process, cmd](int exitCode, QProcess::ExitStatus) {
                m_completed++;
                if (exitCode != 0) m_errors++;

                emit taskCompleted(cmd, exitCode,
                    QString::fromUtf8(process->readAllStandardOutput()),
                    QString::fromUtf8(process->readAllStandardError()));

                process->deleteLater();

                if (m_completed == m_total) {
                    emit allDone(m_total - m_errors, m_errors);
                }
            });

            process->start(cmd, args);
        }
    }

signals:
    void taskCompleted(const QString &cmd, int exitCode,
                       const QString &out, const QString &err);
    void allDone(int successCount, int errorCount);

private:
    int m_total = 0;
    int m_completed = 0;
    int m_errors = 0;
};

// 并行执行多个命令
BatchExecutor executor;
executor.runBatch({
    {"ping", {"-c", "1", "google.com"}},
    {"ping", {"-c", "1", "github.com"}},
    {"ping", {"-c", "1", "qt.io"}},
});
```

### 11.3 waitFor 系列方法

```cpp
process.start(cmd, args);

process.waitForStarted(3000);          // 等待进程启动
process.waitForReadyRead(5000);        // 等待有数据可读
process.waitForBytesWritten(3000);     // 等待数据写入完成
process.waitForFinished(30000);        // 等待进程结束
```

> **注意**：所有 `waitFor*()` 方法都会**阻塞当前线程**。在 GUI 线程中应使用信号槽异步模式。

---

## 12. QProcess 与 QThread 的配合

当需要在后台执行多个进程且保持主线程响应时：

```cpp
class ProcessWorker : public QObject
{
    Q_OBJECT
public slots:
    void runSequence(const QStringList &commands) {
        for (const QString &cmd : commands) {
            QProcess process;
            process.setProcessChannelMode(QProcess::MergedChannels);
            process.start("sh", {"-c", cmd});

            if (!process.waitForFinished(60000)) {
                emit error("超时: " + cmd);
                process.kill();
                continue;
            }

            QString output = QString::fromUtf8(process.readAll());
            emit stepDone(cmd, process.exitCode(), output);

            if (process.exitCode() != 0) {
                emit error("失败: " + cmd);
                return; // 失败则中止后续步骤
            }
        }
        emit allDone();
    }

signals:
    void stepDone(const QString &cmd, int exitCode, const QString &output);
    void error(const QString &msg);
    void allDone();
};

// 使用 Worker + moveToThread 模式
QThread *thread = new QThread;
ProcessWorker *worker = new ProcessWorker;
worker->moveToThread(thread);

connect(thread, &QThread::started, worker, [worker]() {
    worker->runSequence({"cmake -S . -B build",
                         "cmake --build build --config Release",
                         "ctest --test-dir build -C Release"});
});

connect(worker, &ProcessWorker::stepDone, this, [this](const QString &cmd, int code, const QString &out) {
    m_logView->appendPlainText(QString("✅ [%1] exit=%2\n%3").arg(cmd).arg(code).arg(out));
});

connect(worker, &ProcessWorker::allDone, thread, &QThread::quit);
connect(thread, &QThread::finished, worker, &QObject::deleteLater);
connect(thread, &QThread::finished, thread, &QObject::deleteLater);

thread->start();
```

---

## 13. API 速查

### 13.1 启动与控制

| 方法 | 说明 |
|---|---|
| `start(program, args)` | 启动进程 |
| `open(mode)` | 以指定 I/O 模式启动 |
| `terminate()` | 优雅终止（SIGTERM / WM_CLOSE） |
| `kill()` | 强制终止（SIGKILL / TerminateProcess） |
| `close()` | 关闭所有通道并终止 |
| `waitForStarted(ms)` | 阻塞等待启动 |
| `waitForFinished(ms)` | 阻塞等待结束 |

### 13.2 I/O

| 方法 | 说明 |
|---|---|
| `write(data)` | 向标准输入写数据 |
| `readAllStandardOutput()` | 读取全部标准输出 |
| `readAllStandardError()` | 读取全部标准错误 |
| `readAll()` | 读取当前通道全部数据 |
| `closeWriteChannel()` | 关闭写通道（发送 EOF） |
| `setStandardOutputProcess(proc)` | 管道串联 |
| `setStandardOutputFile(path)` | 输出重定向到文件 |

### 13.3 配置

| 方法 | 说明 |
|---|---|
| `setWorkingDirectory(dir)` | 设置工作目录 |
| `setProcessEnvironment(env)` | 设置环境变量 |
| `setProcessChannelMode(mode)` | 设置通道模式 |
| `setProgram(program)` | 设置程序路径 |
| `setArguments(args)` | 设置参数列表 |

### 13.4 状态查询

| 方法 | 说明 |
|---|---|
| `state()` | 当前进程状态 |
| `exitCode()` | 退出码 |
| `exitStatus()` | 退出状态（正常/崩溃） |
| `error()` | 最后发生的错误 |
| `errorString()` | 错误的可读描述 |
| `processId()` | 子进程 PID |

### 13.5 静态方法

| 方法 | 说明 |
|---|---|
| `QProcess::execute(cmd, args)` | 阻塞执行并返回退出码 |
| `QProcess::startDetached(cmd, args)` | 启动独立进程 |
| `QProcess::systemEnvironment()` | 获取系统环境变量列表 |
| `QProcess::nullDevice()` | 返回空设备路径（/dev/null 或 NUL） |

---

## 14. 总结

| 要点 | 说明 |
|---|---|
| **推荐模式** | 异步信号槽模式（不阻塞 UI），避免在 GUI 线程中使用 `waitFor*()` |
| **参数传递** | 始终将 program 和 arguments 分开传递，避免路径空格问题 |
| **错误处理** | 检查 `waitForStarted`→`waitForFinished`→`exitStatus`→`exitCode` |
| **编码问题** | Windows 用 `fromLocal8Bit()`，Unix 用 `fromUtf8()` |
| **管道** | `setStandardOutputProcess()` 实现进程链 |
| **终止** | 优先 `terminate()` + 等待，最后手段 `kill()` |
| **Shell 命令** | 涉及通配符/管道/环境变量展开时，通过 `sh -c` 或 `cmd /c` |
| **独立进程** | 不需要通信时使用 `startDetached()` |
| **跨平台** | 注意程序名、路径分隔符、编码、创建窗口标志的平台差异 |
