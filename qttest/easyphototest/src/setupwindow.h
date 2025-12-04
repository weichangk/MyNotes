/**
 * setupwindow.h
 * 启动设置窗口 - 显示环境检测和安装进度
 */

#ifndef SETUPWINDOW_H
#define SETUPWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include "environmentmanager.h"

class SetupWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SetupWindow(QWidget *parent = nullptr);
    
    bool isSetupComplete() const { return m_setupComplete; }
    QString pythonPath() const { return m_pythonPath; }

signals:
    void setupCompleted(bool success);

private slots:
    void onStatusChanged(EnvironmentManager::Status status, const QString &message);
    void onProgressChanged(int percent, const QString &detail);
    void onSetupFinished(bool success);
    void onRetryClicked();

private:
    void setupUI();
    void updateStatusIcon(EnvironmentManager::Status status);

    EnvironmentManager *m_envManager;
    
    QLabel *m_titleLabel;
    QLabel *m_statusLabel;
    QLabel *m_iconLabel;
    QProgressBar *m_progressBar;
    QLabel *m_detailLabel;
    QTextEdit *m_logEdit;
    QPushButton *m_retryButton;
    QPushButton *m_skipButton;
    
    bool m_setupComplete;
    QString m_pythonPath;
};

#endif // SETUPWINDOW_H
