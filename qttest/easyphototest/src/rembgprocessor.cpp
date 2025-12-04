#include "rembgprocessor.h"

RembgProcessor::RembgProcessor(QObject *parent) : QObject(parent), m_process(new QProcess(this)), m_timeoutTimer(new QTimer(this)), m_model(U2Net), m_status(Idle), m_progress(0), m_timeoutSeconds(300) // 默认5分钟超时
                                                  ,
                                                  m_alphaMatting(false), m_batchCurrentIndex(0) {
    // 连接信号
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RembgProcessor::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &RembgProcessor::onProcessError);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &RembgProcessor::onProcessReadyRead);

    connect(m_timeoutTimer, &QTimer::timeout,
            this, &RembgProcessor::onTimeout);

    // 自动检测Python和rembg路径
    m_pythonPath = getEmbeddedPythonPath();
    m_rembgPath = getEmbeddedRembgPath();
}

RembgProcessor::~RembgProcessor() {
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

void RembgProcessor::setPythonPath(const QString &path) {
    m_pythonPath = path;
}

void RembgProcessor::setRembgPath(const QString &path) {
    m_rembgPath = path;
}

void RembgProcessor::setModel(ModelType model) {
    m_model = model;
}

void RembgProcessor::setAlphaMatting(bool enabled) {
    m_alphaMatting = enabled;
}

void RembgProcessor::setTimeout(int seconds) {
    m_timeoutSeconds = seconds;
}

bool RembgProcessor::isAvailable() {
    QString errorMsg;
    return checkInstallation(errorMsg);
}

void RembgProcessor::processImage(const QString &inputPath, const QString &outputPath) {
    if (m_status == Processing) {
        emit error("正在处理中，请等待或取消当前任务");
        return;
    }

    // 验证输入文件
    QFileInfo inputFile(inputPath);
    if (!inputFile.exists() || !inputFile.isFile()) {
        emit error(QString("输入文件不存在: %1").arg(inputPath));
        return;
    }

    // 确保输出目录存在
    QFileInfo outputFile(outputPath);
    QDir outputDir = outputFile.dir();
    if (!outputDir.exists()) {
        outputDir.mkpath(".");
    }

    m_currentInput = inputPath;
    m_currentOutput = outputPath;
    m_progress = 0;

    setStatus(Processing);
    emit started();

    // 构建命令
    QStringList arguments;

    // 方式1: 使用rembg命令（如果在PATH中）
    if (!m_rembgPath.isEmpty() && QFileInfo::exists(m_rembgPath)) {
        m_process->setProgram(m_rembgPath);
    } else {
        // 方式2: 使用python -m rembg
        m_process->setProgram(m_pythonPath);
        arguments << "-m" << "rembg";
    }

    // 添加rembg参数
    arguments << "i";                            // 单文件模式
    arguments << "-m" << modelToString(m_model); // 指定模型

    if (m_alphaMatting) {
        arguments << "-a"; // 启用alpha matting
    }

    arguments << inputPath << outputPath;

    m_process->setArguments(arguments);

    qDebug() << "执行命令:" << m_process->program() << arguments.join(" ");

    // 启动进程
    m_process->start();

    // 启动超时定时器
    if (m_timeoutSeconds > 0) {
        m_timeoutTimer->start(m_timeoutSeconds * 1000);
    }
}

void RembgProcessor::processBatch(const QStringList &inputPaths, const QString &outputDir) {
    if (inputPaths.isEmpty()) {
        emit error("没有要处理的文件");
        return;
    }

    // 确保输出目录存在
    QDir dir(outputDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_batchInputs = inputPaths;
    m_batchOutputDir = outputDir;
    m_batchCurrentIndex = 0;

    // 处理第一个文件
    processNextBatch();
}

void RembgProcessor::cancel() {
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_timeoutTimer->stop();
        setStatus(Idle);
        emit error("用户取消操作");
    }
}

QString RembgProcessor::getEmbeddedPythonPath() {
    // 优先级1: 应用程序目录下的嵌入式Python
    QString appDir = QApplication::applicationDirPath();
    QString embeddedPython = appDir + "/python/python.exe";

    if (QFileInfo::exists(embeddedPython)) {
        return embeddedPython;
    }

    // 优先级2: 环境变量中的Python
    QProcess process;
    process.start("where", QStringList() << "python");
    process.waitForFinished(3000);
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();

    if (!output.isEmpty()) {
        return output.split('\n').first(); // 返回第一个找到的python
    }

    // 优先级3: 常见安装路径
    QStringList commonPaths = {
        "C:/Python313/python.exe",
        "C:/Python312/python.exe",
        "C:/Python311/python.exe",
        "C:/Python310/python.exe",
        QDir::homePath() + "/AppData/Local/Programs/Python/Python313/python.exe",
        QDir::homePath() + "/AppData/Local/Programs/Python/Python312/python.exe"};

    for (const QString &path : commonPaths) {
        if (QFileInfo::exists(path)) {
            return path;
        }
    }

    return "python"; // 最后尝试使用系统PATH
}

QString RembgProcessor::getEmbeddedRembgPath() {
    // 优先级1: 应用程序目录下的rembg
    QString appDir = QApplication::applicationDirPath();
    QString embeddedRembg = appDir + "/python/Scripts/rembg.exe";

    if (QFileInfo::exists(embeddedRembg)) {
        return embeddedRembg;
    }

    // 优先级2: 环境变量中的rembg
    QProcess process;
    process.start("where", QStringList() << "rembg");
    process.waitForFinished(3000);
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();

    if (!output.isEmpty()) {
        return output.split('\n').first();
    }

    return ""; // 未找到，将使用python -m rembg
}

bool RembgProcessor::checkInstallation(QString &errorMsg) {
    // 检查Python
    QProcess process;
    process.start(getEmbeddedPythonPath(), QStringList() << "--version");
    process.waitForFinished(3000);

    if (process.exitCode() != 0) {
        errorMsg = "未找到Python环境";
        return false;
    }

    // 检查rembg模块
    process.start(getEmbeddedPythonPath(),
                  QStringList() << "-c" << "import rembg; print('ok')");
    process.waitForFinished(3000);
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();

    if (output != "ok") {
        errorMsg = "rembg模块未安装";
        return false;
    }

    return true;
}

void RembgProcessor::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_timeoutTimer->stop();

    if (exitStatus == QProcess::CrashExit) {
        m_lastError = "进程崩溃";
        setStatus(Error);
        emit error(m_lastError);
        return;
    }

    if (exitCode != 0) {
        QString errorOutput = QString::fromLocal8Bit(m_process->readAllStandardError());
        m_lastError = QString("处理失败 (退出码: %1): %2").arg(exitCode).arg(errorOutput);
        setStatus(Error);
        emit error(m_lastError);
        return;
    }

    // 检查输出文件是否创建
    if (!QFileInfo::exists(m_currentOutput)) {
        m_lastError = "输出文件未创建";
        setStatus(Error);
        emit error(m_lastError);
        return;
    }

    // 批量处理
    if (!m_batchInputs.isEmpty()) {
        m_batchCurrentIndex++;
        if (m_batchCurrentIndex < m_batchInputs.size()) {
            processNextBatch();
            return;
        }
    }

    m_progress = 100;
    emit progressChanged(m_progress);

    setStatus(Completed);
    emit finished(m_currentOutput);
}

void RembgProcessor::onProcessError(QProcess::ProcessError error) {
    m_timeoutTimer->stop();

    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "启动失败，请检查Python和rembg是否正确安装";
        break;
    case QProcess::Crashed:
        errorMsg = "进程崩溃";
        break;
    case QProcess::Timedout:
        errorMsg = "进程超时";
        break;
    default:
        errorMsg = "未知错误";
        break;
    }

    m_lastError = errorMsg;
    setStatus(Error);
    emit this->error(errorMsg);
}

void RembgProcessor::onProcessReadyRead() {
    // 读取标准输出，可以解析进度信息
    QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());

    // 简单的进度估算（实际rembg不输出进度）
    if (m_progress < 90) {
        m_progress += 10;
        emit progressChanged(m_progress);
    }

    qDebug() << "rembg输出:" << output;
}

void RembgProcessor::onTimeout() {
    m_lastError = QString("处理超时（超过%1秒）").arg(m_timeoutSeconds);
    cancel();
}

QString RembgProcessor::modelToString(ModelType model) const {
    switch (model) {
    case U2Net: return "u2net";
    case U2NetP: return "u2netp";
    case U2NetHumanSeg: return "u2net_human_seg";
    case U2NetCloth: return "u2net_cloth_seg";
    case SiluetaMobile: return "silueta";
    case BiRefNetGeneral: return "birefnet-general";
    case BiRefNetGeneralLite: return "birefnet-general-lite";
    case BiRefNetPortrait: return "birefnet-portrait";
    case BiRefNetDIS: return "birefnet-dis";
    case BiRefNetHRSOD: return "birefnet-hrsod";
    case BiRefNetCOD: return "birefnet-cod";
    case BiRefNetDIS5K: return "birefnet-dis-5k";
    case IsNetGeneral: return "isnet-general-use";
    case IsNetAnime: return "isnet-anime";
    case Sam: return "sam";
    default: return "u2net";
    }
}

void RembgProcessor::setStatus(ProcessStatus status) {
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void RembgProcessor::processNextBatch() {
    if (m_batchCurrentIndex >= m_batchInputs.size()) {
        return;
    }

    QString inputPath = m_batchInputs[m_batchCurrentIndex];
    QFileInfo fileInfo(inputPath);
    QString outputPath = m_batchOutputDir + "/" + fileInfo.completeBaseName() + "_nobg.png";

    int percent = (m_batchCurrentIndex * 100) / m_batchInputs.size();
    emit progressChanged(percent);

    processImage(inputPath, outputPath);
}