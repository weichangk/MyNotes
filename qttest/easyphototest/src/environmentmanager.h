/**
 * environmentmanager.h
 * 环境管理器 - 自动下载、安装和配置 Python 及 rembg
 * 
 * 功能:
 * 1. 检测项目目录下是否存在 Python
 * 2. 如果不存在则自动下载并解压 Python 嵌入式版本
 * 3. 配置 pip 并安装 rembg
 * 4. 提供进度反馈
 */

#ifndef ENVIRONMENTMANAGER_H
#define ENVIRONMENTMANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QRegExp>

class EnvironmentManager : public QObject
{
    Q_OBJECT
public:
    enum Status {
        NotChecked,         // 未检查
        Checking,           // 检查中
        PythonMissing,      // Python 缺失
        DownloadingPython,  // 下载 Python 中
        ExtractingPython,   // 解压 Python 中
        ConfiguringPip,     // 配置 pip 中
        RembgMissing,       // rembg 缺失
        InstallingRembg,    // 安装 rembg 中
        Ready,              // 就绪
        Error               // 错误
    };

    explicit EnvironmentManager(QObject *parent = nullptr);
    ~EnvironmentManager();

    // 开始环境检查和配置
    void startSetup();
    
    // 取消安装
    void cancel();
    
    // 获取状态
    Status status() const { return m_status; }
    QString statusText() const;
    QString pythonPath() const { return m_pythonPath; }
    QString lastError() const { return m_lastError; }
    bool isReady() const { return m_status == Ready; }

signals:
    void statusChanged(Status status, const QString &message);
    void progressChanged(int percent, const QString &detail);
    void finished(bool success);
    void error(const QString &errorMessage);

private slots:
    void onCheckPythonFinished();
    void onCurlProgress();  // curl 进度输出
    void onDownloadFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onPipConfigFinished();
    void onCheckRembgFinished();
    void onInstallRembgOutput();
    void onInstallRembgFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void setStatus(Status status, const QString &message = QString());
    void checkPython();
    void downloadPython();
    void extractPython();
    void configurePip();
    void checkRembg();
    void installRembg();
    void cleanupTempFiles();
    
    QString getPythonDir() const;
    QString getPythonExe() const;
    QString getPipExe() const;
    bool modifyPthFile();

    Status m_status;
    QString m_lastError;
    QString m_pythonPath;
    QString m_pythonDir;
    QString m_downloadedFile;
    
    QProcess *m_process;
    
    int m_currentProgress;
    qint64 m_totalBytes;
    
    // Python 配置
    static const QString PYTHON_VERSION;
    static const QString PYTHON_EMBED_URL;          // 官方源
    static const QString PYTHON_EMBED_URL_MIRROR;   // 备用镜像源
    static const QString PYTHON_DIR_NAME;
    
    int m_downloadRetryCount;  // 下载重试次数
};

#endif // ENVIRONMENTMANAGER_H

