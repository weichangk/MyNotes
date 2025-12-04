/**
 * environmentmanager.cpp
 * 环境管理器实现
 */

#include "environmentmanager.h"
#include <QApplication>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QTimer>
#include <JlCompress.h>  // quazip 解压库

// 定义 Python 配置
const QString EnvironmentManager::PYTHON_VERSION = "3.11.9";
// 官方源（优先使用）
const QString EnvironmentManager::PYTHON_EMBED_URL = 
    "https://www.python.org/ftp/python/3.11.9/python-3.11.9-embed-amd64.zip";
// 备用镜像源（国内加速）
const QString EnvironmentManager::PYTHON_EMBED_URL_MIRROR = 
    "https://registry.npmmirror.com/-/binary/python/3.11.9/python-3.11.9-embed-amd64.zip";
const QString EnvironmentManager::PYTHON_DIR_NAME = "python";

EnvironmentManager::EnvironmentManager(QObject *parent)
    : QObject(parent)
    , m_status(NotChecked)
    , m_process(new QProcess(this))
    , m_currentProgress(0)
    , m_totalBytes(0)
    , m_downloadRetryCount(0)
{
    m_pythonDir = QApplication::applicationDirPath() + "/" + PYTHON_DIR_NAME;
    m_pythonPath = getPythonExe();
}

EnvironmentManager::~EnvironmentManager()
{
    cancel();
    cleanupTempFiles();
}

void EnvironmentManager::startSetup()
{
    setStatus(Checking, "正在检查环境...");
    checkPython();
}

void EnvironmentManager::cancel()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

QString EnvironmentManager::statusText() const
{
    switch (m_status) {
        case NotChecked: return "未检查";
        case Checking: return "检查环境中...";
        case PythonMissing: return "Python 未安装";
        case DownloadingPython: return "正在下载 Python...";
        case ExtractingPython: return "正在解压 Python...";
        case ConfiguringPip: return "正在配置 pip...";
        case RembgMissing: return "rembg 未安装";
        case InstallingRembg: return "正在安装 rembg...";
        case Ready: return "环境就绪";
        case Error: return "错误";
        default: return "未知状态";
    }
}

void EnvironmentManager::setStatus(Status status, const QString &message)
{
    m_status = status;
    QString msg = message.isEmpty() ? statusText() : message;
    emit statusChanged(status, msg);
    qDebug() << "状态:" << statusText() << msg;
}

QString EnvironmentManager::getPythonDir() const
{
    return m_pythonDir;
}

QString EnvironmentManager::getPythonExe() const
{
    return m_pythonDir + "/python.exe";
}

QString EnvironmentManager::getPipExe() const
{
    return m_pythonDir + "/Scripts/pip.exe";
}

void EnvironmentManager::checkPython()
{
    setStatus(Checking, "检查 Python 是否存在...");
    
    QFileInfo pythonFile(getPythonExe());
    
    if (!pythonFile.exists()) {
        qDebug() << "Python 不存在:" << getPythonExe();
        setStatus(PythonMissing, "未找到 Python，准备下载...");
        emit progressChanged(0, "Python 未安装");
        
        // 延迟一点再开始下载，让用户看到提示
        QTimer::singleShot(500, this, &EnvironmentManager::downloadPython);
        return;
    }
    
    // 验证 Python 可执行
    qDebug() << "找到 Python，验证中...";
    emit progressChanged(10, "验证 Python 安装...");
    
    m_process->setProgram(getPythonExe());
    m_process->setArguments(QStringList() << "--version");
    
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &EnvironmentManager::onCheckPythonFinished, Qt::UniqueConnection);
    
    m_process->start();
}

void EnvironmentManager::onCheckPythonFinished()
{
    disconnect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
               this, &EnvironmentManager::onCheckPythonFinished);
    
    if (m_process->exitCode() == 0) {
        QString version = QString::fromLocal8Bit(m_process->readAllStandardOutput()).trimmed();
        qDebug() << "Python 版本:" << version;
        emit progressChanged(20, "Python 已安装: " + version);
        
        // Python 存在，检查 rembg
        QTimer::singleShot(300, this, &EnvironmentManager::checkRembg);
    } else {
        setStatus(PythonMissing, "Python 无法运行，需要重新安装");
        QTimer::singleShot(500, this, &EnvironmentManager::downloadPython);
    }
}

void EnvironmentManager::downloadPython()
{
    setStatus(DownloadingPython, "开始下载 Python 嵌入式版本...");
    
    m_downloadedFile = QApplication::applicationDirPath() + "/python_temp.zip";
    
    // 选择下载源：首次使用官方源，失败后使用国内镜像
    QString downloadUrl;
    if (m_downloadRetryCount == 0) {
        downloadUrl = PYTHON_EMBED_URL;
        emit progressChanged(0, "连接 Python 官方服务器...");
        qDebug() << "使用官方源下载:", downloadUrl;
    } else {
        downloadUrl = PYTHON_EMBED_URL_MIRROR;
        emit progressChanged(0, "连接国内镜像服务器...");
        qDebug() << "切换到备用源:", downloadUrl;
    }
    
    // 使用 curl 下载 (Windows 10+ 自带，无需 OpenSSL 库)
    // -L: 跟随重定向, -o: 输出文件, --progress-bar: 显示进度, -f: 失败时返回错误码
    QStringList arguments;
    arguments << "-L"           // 跟随重定向
              << "-f"           // HTTP 错误时失败
              << "--ssl-no-revoke"  // 跳过 SSL 证书吐销检查
              << "-o" << m_downloadedFile
              << downloadUrl;
    
    m_process->setProgram("curl");
    m_process->setArguments(arguments);
    
    // 连接输出信号用于进度提示
    connect(m_process, &QProcess::readyReadStandardError,
            this, &EnvironmentManager::onCurlProgress, Qt::UniqueConnection);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &EnvironmentManager::onDownloadFinished, Qt::UniqueConnection);
    
    qDebug() << "curl 命令:" << "curl" << arguments.join(" ");
    m_process->start();
}

void EnvironmentManager::onCurlProgress()
{
    // curl 输出进度到 stderr，格式如: % Total    % Received % Xferd  Average Speed   Time
    QString output = QString::fromLocal8Bit(m_process->readAllStandardError());
    
    // 简单解析百分比，查找数字后面的 %
    QRegExp rx("(\\d+)%");
    if (rx.indexIn(output) != -1) {
        int percent = rx.cap(1).toInt();
        if (percent != m_currentProgress && percent <= 100) {
            m_currentProgress = percent;
            emit progressChanged(percent, QString("正在下载 Python: %1%").arg(percent));
        }
    }
}

void EnvironmentManager::onDownloadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    disconnect(m_process, &QProcess::readyReadStandardError,
               this, &EnvironmentManager::onCurlProgress);
    disconnect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
               this, &EnvironmentManager::onDownloadFinished);
    
    // curl 退出码 0 表示成功
    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        QString errorMsg = QString::fromLocal8Bit(m_process->readAllStandardError());
        if (errorMsg.isEmpty()) {
            errorMsg = QString("退出码: %1").arg(exitCode);
        }
        
        qWarning() << "curl 下载失败:" << exitCode << errorMsg;
        
        // 尝试使用备用源重试
        if (m_downloadRetryCount == 0) {
            m_downloadRetryCount++;
            QString retryMsg = QString("官方源下载失败\n切换到国内镜像重试...");
            setStatus(DownloadingPython, retryMsg);
            emit progressChanged(0, "切换到备用下载源...");
            
            qDebug() << "切换到备用源重试...";
            QTimer::singleShot(1000, this, &EnvironmentManager::downloadPython);
            return;
        }
        
        // 两次都失败
        m_lastError = QString("下载失败\n\n可能原因：\n"
                             "1. 网络连接问题\n"
                             "2. 防火墙拦截\n"
                             "3. curl 命令不可用\n\n"
                             "建议：\n"
                             "- 检查网络连接\n"
                             "- 临时关闭防火墙\n"
                             "- 点击重试按钮\n"
                             "- 或手动下载 Python");
        
        setStatus(Error, m_lastError);
        emit error(m_lastError);
        emit finished(false);
        return;
    }
    
    // 验证下载的文件是否存在
    QFileInfo fileInfo(m_downloadedFile);
    if (!fileInfo.exists() || fileInfo.size() == 0) {
        m_lastError = "下载的文件不存在或为空";
        setStatus(Error, m_lastError);
        emit error(m_lastError);
        emit finished(false);
        return;
    }
    
    qDebug() << "下载成功，文件大小:" << fileInfo.size() << "bytes";
    emit progressChanged(100, "下载完成，准备解压...");
    
    // 短暂延迟后开始解压
    QTimer::singleShot(500, this, &EnvironmentManager::extractPython);
}

void EnvironmentManager::extractPython()
{
    setStatus(ExtractingPython, "正在解压 Python...");
    emit progressChanged(0, "解压 Python 文件...");
    
    // 确保目标目录不存在
    QDir dir(m_pythonDir);
    if (dir.exists()) {
        dir.removeRecursively();
    }
    dir.mkpath(".");
    
    // 使用 quazip 的 JlCompress 解压
    QStringList extractedFiles = JlCompress::extractDir(m_downloadedFile, m_pythonDir);
    
    if (extractedFiles.isEmpty()) {
        m_lastError = "解压失败：ZIP 文件损坏或为空";
        setStatus(Error, m_lastError);
        emit error(m_lastError);
        emit finished(false);
        return;
    }
    
    emit progressChanged(100, QString("解压完成，共 %1 个文件").arg(extractedFiles.size()));
    
    // 验证 Python 是否存在
    if (!QFileInfo::exists(getPythonExe())) {
        m_lastError = "解压后未找到 python.exe";
        setStatus(Error, m_lastError);
        emit error(m_lastError);
        emit finished(false);
        return;
    }
    
    qDebug() << "Python 解压成功，文件数:" << extractedFiles.size();
    
    // 清理下载的 zip 文件
    QFile::remove(m_downloadedFile);
    
    // 配置 pip
    QTimer::singleShot(500, this, &EnvironmentManager::configurePip);
}

bool EnvironmentManager::modifyPthFile()
{
    // 查找 ._pth 文件
    QDir pythonDir(m_pythonDir);
    QStringList pthFiles = pythonDir.entryList(QStringList() << "python*._pth", QDir::Files);
    
    if (pthFiles.isEmpty()) {
        qWarning() << "未找到 ._pth 文件";
        return false;
    }
    
    QString pthFile = m_pythonDir + "/" + pthFiles.first();
    qDebug() << "修改 PTH 文件:" << pthFile;
    
    // 读取文件
    QFile file(pthFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << file.errorString();
        return false;
    }
    
    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        // 取消注释 import site
        if (line.trimmed().startsWith("#import site")) {
            lines.append("import site");
        } else {
            lines.append(line);
        }
    }
    file.close();
    
    // 写回文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法写入文件:" << file.errorString();
        return false;
    }
    
    QTextStream out(&file);
    for (const QString &line : lines) {
        out << line << "\n";
    }
    file.close();
    
    return true;
}

void EnvironmentManager::configurePip()
{
    setStatus(ConfiguringPip, "正在配置 pip...");
    emit progressChanged(0, "启用 pip 支持...");
    
    // 修改 ._pth 文件
    if (!modifyPthFile()) {
        m_lastError = "修改 PTH 文件失败";
        setStatus(Error, m_lastError);
        emit error(m_lastError);
        emit finished(false);
        return;
    }
    
    emit progressChanged(30, "下载 get-pip.py...");
    
    // 使用 curl 下载 get-pip.py
    QString getPipUrl = "https://bootstrap.pypa.io/get-pip.py";
    QString getPipFile = m_pythonDir + "/get-pip.py";
    
    QStringList arguments;
    arguments << "-L" << "-f" << "--ssl-no-revoke"
              << "-o" << getPipFile
              << getPipUrl;
    
    m_process->setProgram("curl");
    m_process->setArguments(arguments);
    
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, getPipFile](int exitCode, QProcess::ExitStatus exitStatus) {
        
        disconnect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                   this, nullptr);
        
        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            QString errorMsg = QString::fromLocal8Bit(m_process->readAllStandardError());
            m_lastError = "下载 get-pip.py 失败: " + errorMsg;
            setStatus(Error, m_lastError);
            emit error(m_lastError);
            emit finished(false);
            return;
        }
        
        // 验证文件
        QFileInfo fileInfo(getPipFile);
        if (!fileInfo.exists() || fileInfo.size() == 0) {
            m_lastError = "下载的 get-pip.py 不存在或为空";
            setStatus(Error, m_lastError);
            emit error(m_lastError);
            emit finished(false);
            return;
        }
        
        emit progressChanged(60, "安装 pip...");
        
        // 运行 get-pip.py
        m_process->setProgram(getPythonExe());
        m_process->setArguments(QStringList() << getPipFile);
        
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &EnvironmentManager::onPipConfigFinished, Qt::UniqueConnection);
        
        m_process->start();
    }, Qt::UniqueConnection);
    
    qDebug() << "curl 下载 get-pip.py:" << arguments.join(" ");
    m_process->start();
}

void EnvironmentManager::onPipConfigFinished()
{
    disconnect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
               this, &EnvironmentManager::onPipConfigFinished);
    
    if (m_process->exitCode() != 0) {
        QString errorOutput = QString::fromLocal8Bit(m_process->readAllStandardError());
        m_lastError = "pip 安装失败: " + errorOutput;
        setStatus(Error, m_lastError);
        emit error(m_lastError);
        emit finished(false);
        return;
    }
    
    emit progressChanged(100, "pip 配置完成");
    
    // 删除 get-pip.py
    QFile::remove(m_pythonDir + "/get-pip.py");
    
    // 检查 rembg
    QTimer::singleShot(500, this, &EnvironmentManager::checkRembg);
}

void EnvironmentManager::checkRembg()
{
    setStatus(Checking, "检查 rembg 是否已安装...");
    emit progressChanged(30, "验证 rembg 安装...");
    
    m_process->setProgram(getPythonExe());
    m_process->setArguments(QStringList() 
        << "-c" << "import rembg; print('ok')");
    
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &EnvironmentManager::onCheckRembgFinished, Qt::UniqueConnection);
    
    m_process->start();
}

void EnvironmentManager::onCheckRembgFinished()
{
    disconnect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
               this, &EnvironmentManager::onCheckRembgFinished);
    
    QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput()).trimmed();
    
    if (m_process->exitCode() == 0 && output == "ok") {
        qDebug() << "rembg 已安装";
        emit progressChanged(100, "环境已就绪");
        setStatus(Ready, "所有环境已准备就绪！");
        emit finished(true);
    } else {
        qDebug() << "rembg 未安装";
        setStatus(RembgMissing, "rembg 未安装，准备安装...");
        emit progressChanged(40, "rembg 未安装");
        QTimer::singleShot(500, this, &EnvironmentManager::installRembg);
    }
}

void EnvironmentManager::installRembg()
{
    setStatus(InstallingRembg, "正在安装 rembg（这可能需要几分钟）...");
    emit progressChanged(0, "开始安装 rembg 和依赖...");
    
    m_process->setProgram(getPythonExe());
    m_process->setArguments(QStringList() 
        << "-m" << "pip" << "install" 
        << "rembg[cpu,cli]"
        << "--no-warn-script-location");
    
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &EnvironmentManager::onInstallRembgOutput, Qt::UniqueConnection);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &EnvironmentManager::onInstallRembgFinished, Qt::UniqueConnection);
    
    m_process->start();
}

void EnvironmentManager::onInstallRembgOutput()
{
    QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    
    // 简单的进度估算
    if (output.contains("Collecting")) {
        emit progressChanged(20, "正在收集依赖包...");
    } else if (output.contains("Downloading")) {
        emit progressChanged(40, "正在下载依赖包...");
    } else if (output.contains("Installing")) {
        emit progressChanged(70, "正在安装依赖包...");
    } else if (output.contains("Successfully installed")) {
        emit progressChanged(95, "安装完成...");
    }
    
    qDebug() << "安装输出:" << output;
}

void EnvironmentManager::onInstallRembgFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    disconnect(m_process, &QProcess::readyReadStandardOutput,
               this, &EnvironmentManager::onInstallRembgOutput);
    disconnect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
               this, &EnvironmentManager::onInstallRembgFinished);
    
    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        QString errorOutput = QString::fromLocal8Bit(m_process->readAllStandardError());
        m_lastError = "rembg 安装失败: " + errorOutput;
        setStatus(Error, m_lastError);
        emit error(m_lastError);
        emit finished(false);
        return;
    }
    
    emit progressChanged(100, "rembg 安装成功！");
    setStatus(Ready, "环境配置完成！");
    emit finished(true);
}

void EnvironmentManager::cleanupTempFiles()
{
    // 清理临时文件
    if (!m_downloadedFile.isEmpty()) {
        QFile::remove(m_downloadedFile);
    }
}
