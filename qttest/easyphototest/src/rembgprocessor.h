/**
 * Qt集成rembg - 使用QProcess调用命令行工具
 *
 * 使用方法:
 * 1. 将此类添加到你的Qt项目
 * 2. 确保Python环境和rembg已正确部署
 * 3. 调用processImage()处理图片
 */

#pragma once
#include <QObject>
#include <QProcess>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QApplication>
#include <QStandardPaths>
#include <QDebug>

class RembgProcessor : public QObject {
    Q_OBJECT
public:
    enum ProcessStatus {
        Idle,         // 空闲
        Initializing, // 初始化中
        Processing,   // 处理中
        Completed,    // 完成
        Error         // 错误
    };

    enum ModelType {
        U2Net,               // 通用模型（默认）
        U2NetP,              // 轻量版（速度快）
        U2NetHumanSeg,       // 人体分割
        U2NetCloth,          // 衣服分割
        SiluetaMobile,       // 移动设备优化
        BiRefNetGeneral,     // BiRefNet通用（高质量）
        BiRefNetGeneralLite, // BiRefNet通用轻量版
        BiRefNetPortrait,    // BiRefNet肖像
        BiRefNetDIS,         // BiRefNet高精度
        BiRefNetHRSOD,       // BiRefNet高分辨率
        BiRefNetCOD,         // BiRefNet伪装物体检测
        BiRefNetDIS5K,       // BiRefNet 5K分辨率
        IsNetGeneral,        // ISNet通用
        IsNetAnime,          // ISNet动漫专用
        Sam                  // SAM (Segment Anything Model)
    };

    explicit RembgProcessor(QObject *parent = nullptr);
    ~RembgProcessor();

    // 配置方法
    void setPythonPath(const QString &path); // 设置Python可执行文件路径
    void setRembgPath(const QString &path);  // 设置rembg命令路径
    void setModel(ModelType model);          // 设置AI模型
    void setAlphaMatting(bool enabled);      // 启用精细抠图
    void setTimeout(int seconds);            // 设置超时时间

    // 主要功能
    bool isAvailable(); // 检查rembg是否可用
    void processImage(const QString &inputPath,
                      const QString &outputPath); // 处理单张图片
    void processBatch(const QStringList &inputPaths,
                      const QString &outputDir); // 批量处理
    void cancel();                               // 取消当前处理

    // 状态查询
    ProcessStatus status() const {
        return m_status;
    }
    QString lastError() const {
        return m_lastError;
    }
    int progress() const {
        return m_progress;
    }

    // 实用工具
    static QString getEmbeddedPythonPath();           // 获取嵌入式Python路径
    static QString getEmbeddedRembgPath();            // 获取嵌入式rembg路径
    static bool checkInstallation(QString &errorMsg); // 检查安装状态

signals:
    void started();                           // 开始处理
    void progressChanged(int percent);        // 进度变化
    void finished(const QString &outputPath); // 处理完成
    void error(const QString &errorMessage);  // 发生错误
    void statusChanged(ProcessStatus status); // 状态改变

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessReadyRead();
    void onTimeout();

private:
    QString modelToString(ModelType model) const;
    void setStatus(ProcessStatus status);
    void processNextBatch();

    QProcess *m_process;
    QTimer *m_timeoutTimer;

    QString m_pythonPath;
    QString m_rembgPath;
    QString m_currentInput;
    QString m_currentOutput;
    QString m_lastError;

    ModelType m_model;
    ProcessStatus m_status;
    int m_progress;
    int m_timeoutSeconds;
    bool m_alphaMatting;

    QStringList m_batchInputs;
    QString m_batchOutputDir;
    int m_batchCurrentIndex;
};