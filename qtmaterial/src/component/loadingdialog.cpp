#include "component/loadingdialog.h"
#include <QVBoxLayout>

LoadingWorker::LoadingWorker(QObject *parent) :
    QThread(parent) {
}

LoadingWorker::~LoadingWorker() {
}

void LoadingWorker::run() {
    if (do_work_) {
        do_work_();
    }
}

void LoadingWorker::setDoWork(std::function<void()> work) {
    do_work_ = work;
}

LoadingDialog::LoadingDialog(QDialog *parent) :
    QDialog(parent),
    progress_bar_(new QProgressBar(this)),
    cancel_btn_(new QPushButton(this)),
    loading_worker_(new LoadingWorker(this)) {
    createUi();
    sigConnect();
}

LoadingDialog::~LoadingDialog() {
    // Loading_worker_->quit();
    // Loading_worker_->wait();
}

void LoadingDialog::setDoWork(std::function<void()> work) {
    loading_worker_->setDoWork(work);
}

int LoadingDialog::exec() {
    loading_worker_->start();
    return QDialog::exec();
}

void LoadingDialog::createUi() {
    auto layout = new QVBoxLayout(this);
    progress_bar_->setRange(0, 0);
    layout->addWidget(progress_bar_);
    layout->addWidget(cancel_btn_);
}

void LoadingDialog::sigConnect() {
    connect(loading_worker_, &LoadingWorker::finished, this, &QDialog::accept);
}