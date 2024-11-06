#pragma once
#include "qtmaterial_global.h"
#include <QDialog>
#include <QPushButton>
#include <QProgressBar>
#include <QThread>

namespace widget {
class LoadingWorker : public QThread {
    Q_OBJECT
public:
    explicit LoadingWorker(QObject *parent = nullptr);
    ~LoadingWorker();
    void run() override;
    void setDoWork(std::function<void()> work);

private:
    std::function<void()> do_work_ = nullptr;
};

class QTMATERIAL_EXPORT Loading : public QDialog {
    Q_OBJECT

public:
    explicit Loading(QDialog *parent = nullptr);
    ~Loading();
    void setDoWork(std::function<void()> work);

public:
    int exec() override;

protected:
    void createUi();
    void sigConnect();

private:
    QProgressBar *progress_bar_ = nullptr;
    QPushButton *cancel_btn_ = nullptr;
    LoadingWorker *loading_worker_ = nullptr;
};
} // namespace widget