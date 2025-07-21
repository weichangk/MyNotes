#pragma once

#include "task/ITaskCallback.h"

#include <QDialog>
#include <QPushButton>

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <string>

using namespace qtmaterialtask;

class ProgressDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    void setMessage(const QString &text);
    void setProgress(int percent);
    void setCancelable(bool cancelable);

signals:
    void canceled();

private:
    QLabel *m_label;
    QProgressBar *m_progressBar;
    QPushButton *m_cancelButton;
};

class MyCallback : public QObject, public ITaskCallback<std::string> {
    Q_OBJECT

signals:
    void sigStarted();
    void sigP(int percent, const std::string &msg);
    void sigProgress(int percent, const QString &msg);
    void sigSucceeded(const QString &result);
    void sigFailed(const QString &error);
    void sigCanceled();
    void sigFinished();

private:
    void onStarted() override;
    void onProgress(int percent, const std::string &msg) override;
    void onSucceeded(const std::string &result) override;
    void onFailed(const std::string &error) override;
    void onCanceled() override;
    void onFinished() override;
};

class TaskTestDialog : public QDialog {
    Q_OBJECT

public:
    TaskTestDialog(QDialog *parent = nullptr);
    ~TaskTestDialog();

private:
    void createUi();
    void sigConnect();

    void slotBtn1();

private slots:
    void slotProgress(int percent, const std::string &msg);
    
private:
    QPushButton *btn1 = nullptr;
};