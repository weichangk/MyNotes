/**
 * setupwindow.cpp
 * 启动设置窗口实现
 */

#include "setupwindow.h"
#include <QApplication>
#include <QTime>
#include <QTimer>

SetupWindow::SetupWindow(QWidget *parent)
    : QWidget(parent)
    , m_envManager(new EnvironmentManager(this))
    , m_setupComplete(false)
{
    setupUI();
    
    // 连接信号
    connect(m_envManager, &EnvironmentManager::statusChanged,
            this, &SetupWindow::onStatusChanged);
    connect(m_envManager, &EnvironmentManager::progressChanged,
            this, &SetupWindow::onProgressChanged);
    connect(m_envManager, &EnvironmentManager::finished,
            this, &SetupWindow::onSetupFinished);
    
    // 自动开始设置
    QTimer::singleShot(500, m_envManager, &EnvironmentManager::startSetup);
}

void SetupWindow::setupUI()
{
    setWindowTitle("EasyPhoto - 环境配置");
    setMinimumSize(600, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);
    
    // 标题
    m_titleLabel = new QLabel("🚀 EasyPhoto 环境配置");
    m_titleLabel->setStyleSheet(
        "font-size: 24px; "
        "font-weight: bold; "
        "color: #2c3e50; "
        "padding: 10px;"
    );
    m_titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_titleLabel);
    
    // 状态图标
    m_iconLabel = new QLabel("⏳");
    m_iconLabel->setStyleSheet(
        "font-size: 48px; "
        "padding: 10px;"
    );
    m_iconLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_iconLabel);
    
    // 状态文本
    m_statusLabel = new QLabel("正在初始化...");
    m_statusLabel->setStyleSheet(
        "font-size: 16px; "
        "color: #34495e; "
        "padding: 5px;"
    );
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);
    
    // 进度条
    m_progressBar = new QProgressBar();
    m_progressBar->setMinimumHeight(30);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    text-align: center;"
        "    background-color: #ecf0f1;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #3498db;"
        "    border-radius: 3px;"
        "}"
    );
    mainLayout->addWidget(m_progressBar);
    
    // 详细信息
    m_detailLabel = new QLabel("等待开始...");
    m_detailLabel->setStyleSheet(
        "font-size: 12px; "
        "color: #7f8c8d; "
        "padding: 5px;"
    );
    m_detailLabel->setAlignment(Qt::AlignCenter);
    m_detailLabel->setWordWrap(true);
    mainLayout->addWidget(m_detailLabel);
    
    // 日志区域
    QLabel *logTitle = new QLabel("安装日志:");
    logTitle->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    mainLayout->addWidget(logTitle);
    
    m_logEdit = new QTextEdit();
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(150);
    m_logEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: #2c3e50;"
        "    color: #ecf0f1;"
        "    border: 1px solid #34495e;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "    font-family: 'Consolas', 'Courier New', monospace;"
        "    font-size: 10px;"
        "}"
    );
    mainLayout->addWidget(m_logEdit);
    
    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_retryButton = new QPushButton("重试");
    m_retryButton->setVisible(false);
    m_retryButton->setMinimumSize(100, 35);
    m_retryButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 8px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
    );
    connect(m_retryButton, &QPushButton::clicked, this, &SetupWindow::onRetryClicked);
    buttonLayout->addWidget(m_retryButton);
    
    m_skipButton = new QPushButton("跳过（不推荐）");
    m_skipButton->setVisible(false);
    m_skipButton->setMinimumSize(120, 35);
    m_skipButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #95a5a6;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 8px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #7f8c8d;"
        "}"
    );
    connect(m_skipButton, &QPushButton::clicked, this, [this]() {
        m_setupComplete = false;
        emit setupCompleted(false);
        close();
    });
    buttonLayout->addWidget(m_skipButton);
    
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    mainLayout->addStretch();
}

void SetupWindow::onStatusChanged(EnvironmentManager::Status status, const QString &message)
{
    m_statusLabel->setText(message);
    updateStatusIcon(status);
    
    // 添加到日志
    QString logEntry = QString("[%1] %2")
        .arg(QTime::currentTime().toString("HH:mm:ss"))
        .arg(message);
    m_logEdit->append(logEntry);
}

void SetupWindow::onProgressChanged(int percent, const QString &detail)
{
    m_progressBar->setValue(percent);
    m_detailLabel->setText(detail);
    
    // 添加到日志
    if (!detail.isEmpty()) {
        QString logEntry = QString("[%1] %2 (%3%)")
            .arg(QTime::currentTime().toString("HH:mm:ss"))
            .arg(detail)
            .arg(percent);
        m_logEdit->append(logEntry);
    }
}

void SetupWindow::onSetupFinished(bool success)
{
    if (success) {
        m_setupComplete = true;
        m_pythonPath = m_envManager->pythonPath();
        
        m_iconLabel->setText("✅");
        m_statusLabel->setText("环境配置完成！");
        m_statusLabel->setStyleSheet(
            "font-size: 16px; "
            "color: #27ae60; "
            "font-weight: bold; "
            "padding: 5px;"
        );
        m_detailLabel->setText("即将启动应用程序...");
        m_progressBar->setValue(100);
        
        // 延迟关闭窗口
        QTimer::singleShot(1500, this, [this]() {
            emit setupCompleted(true);
            close();
        });
    } else {
        m_iconLabel->setText("❌");
        m_statusLabel->setText("环境配置失败");
        m_statusLabel->setStyleSheet(
            "font-size: 16px; "
            "color: #e74c3c; "
            "font-weight: bold; "
            "padding: 5px;"
        );
        m_detailLabel->setText(m_envManager->lastError());
        
        // 显示重试和跳过按钮
        m_retryButton->setVisible(true);
        m_skipButton->setVisible(true);
    }
}

void SetupWindow::onRetryClicked()
{
    m_retryButton->setVisible(false);
    m_skipButton->setVisible(false);
    m_logEdit->clear();
    m_progressBar->setValue(0);
    
    m_iconLabel->setText("⏳");
    m_statusLabel->setText("正在重试...");
    m_statusLabel->setStyleSheet(
        "font-size: 16px; "
        "color: #34495e; "
        "padding: 5px;"
    );
    
    QTimer::singleShot(500, m_envManager, &EnvironmentManager::startSetup);
}

void SetupWindow::updateStatusIcon(EnvironmentManager::Status status)
{
    switch (status) {
        case EnvironmentManager::NotChecked:
        case EnvironmentManager::Checking:
            m_iconLabel->setText("🔍");
            break;
        case EnvironmentManager::PythonMissing:
        case EnvironmentManager::DownloadingPython:
            m_iconLabel->setText("⬇️");
            break;
        case EnvironmentManager::ExtractingPython:
            m_iconLabel->setText("📦");
            break;
        case EnvironmentManager::ConfiguringPip:
            m_iconLabel->setText("⚙️");
            break;
        case EnvironmentManager::RembgMissing:
        case EnvironmentManager::InstallingRembg:
            m_iconLabel->setText("🔧");
            break;
        case EnvironmentManager::Ready:
            m_iconLabel->setText("✅");
            break;
        case EnvironmentManager::Error:
            m_iconLabel->setText("❌");
            break;
    }
}
