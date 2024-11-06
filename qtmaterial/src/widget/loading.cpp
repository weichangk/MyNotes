#include "widget/loading.h"
#include <QVBoxLayout>

namespace widget {
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

Loading::Loading(QDialog *parent) :
    QDialog(parent),
    progress_bar_(new QProgressBar(this)),
    cancel_btn_(new QPushButton(this)),
    loading_worker_(new LoadingWorker(this)) {
    createUi();
    sigConnect();
}

Loading::~Loading() {
    // Loading_worker_->quit();
    // Loading_worker_->wait();
}

void Loading::setDoWork(std::function<void()> work) {
    loading_worker_->setDoWork(work);
}

int Loading::exec() {
    loading_worker_->start();
    return QDialog::exec();
}

void Loading::createUi() {
    auto layout = new QVBoxLayout(this);
    progress_bar_->setRange(0, 0);
    layout->addWidget(progress_bar_);
    layout->addWidget(cancel_btn_);
}

void Loading::sigConnect() {
    connect(loading_worker_, &LoadingWorker::finished, this, &QDialog::accept);
}
} // namespace widget