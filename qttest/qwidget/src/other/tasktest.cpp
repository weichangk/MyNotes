#include "tasktest.h"
#include "task/TaskFactory.h"
#include "task/AsyncTask.h"
#include "task/TaskData.h"
#include "task/TaskResult.h"

#include <QDebug>
#include <QThread>
#include <QTimer>

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    m_label(new QLabel(this)),
    m_progressBar(new QProgressBar(this)),
    m_cancelButton(new QPushButton("取消", this)) {
    setWindowTitle("进度");
    setModal(true);
    setMinimumWidth(300);

    m_progressBar->setRange(0, 100);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_label);
    layout->addWidget(m_progressBar);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    layout->addLayout(buttonLayout);

    connect(m_cancelButton, &QPushButton::clicked, this, &ProgressDialog::canceled);
}

void ProgressDialog::setMessage(const QString &text) {
    m_label->setText(text);
}

void ProgressDialog::setProgress(int percent) {
    m_progressBar->setValue(percent);
}

void ProgressDialog::setCancelable(bool cancelable) {
    m_cancelButton->setVisible(cancelable);
}

void MyCallback::onStarted() {
    qDebug() << "任务开始";
    emit sigStarted();
}

void MyCallback::onProgress(int percent, const std::string &msg) {
    QString msgStr = QString::fromStdString(msg);
    qDebug() << "进度:" << percent << msgStr;
    emit sigProgress(percent, msgStr);
    emit sigP(percent, msg);
}

void MyCallback::onSucceeded(const std::string &result) {
    QString resultStr = QString::fromStdString(result);
    qDebug() << "任务成功:" << resultStr;
    emit sigSucceeded(resultStr);
}

void MyCallback::onFailed(const std::string &error) {
    QString errorStr = QString::fromStdString(error);
    qDebug() << "任务失败:" << errorStr;
    emit sigFailed(errorStr);
}

void MyCallback::onCanceled() {
    qDebug() << "任务被取消";
    emit sigCanceled();
}

void MyCallback::onFinished() {
    qDebug() << "任务完成";
    emit sigFinished();
}


//////////////////////////////

TaskTestDialog::TaskTestDialog(QDialog *parent) :
    QDialog(parent) {
    createUi();
    sigConnect();
}

TaskTestDialog::~TaskTestDialog() {
}

void TaskTestDialog::createUi() {
    setMinimumSize(1096, 680);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    btn1 = new QPushButton(this);
    btn1->setFixedSize(100, 32);
    btn1->setText("");
    layout->addWidget(btn1);
}

void TaskTestDialog::sigConnect() {
    connect(btn1, &QPushButton::clicked, this, &TaskTestDialog::slotBtn1);
}

void TaskTestDialog::slotBtn1() {

    auto callback = std::make_shared<MyCallback>();
    auto task = TaskFactory::instance()->createTask<int, std::string>(
        [](AsyncTask<int,std::string> *self) {
            qDebug() << "getData:" << self->getData().value;
            for (int i = 0; i <= 100; i += 10) {
                if (self->isCanceled()) {
                    return TaskResult<std::string>::Failure("任务完成啦！");
                }
                self->progress(i, "处理中...");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            return TaskResult<std::string>::Success("任务完成啦！");
        },
        callback, TaskData<int>(20));

    auto dialog = new ProgressDialog(this);
    dialog->setCancelable(true);

    connect(dialog, &ProgressDialog::canceled, this, [task](){
        task->cancel();
    });

    connect(callback.get(), &MyCallback::sigFinished, dialog, &QDialog::accept);

    connect(callback.get(), &MyCallback::sigProgress, this, [dialog](int percent, const QString &msg){
        dialog->setProgress(percent);
        dialog->setMessage(msg);
    });

    QObject::connect(callback.get(), SIGNAL(sigP(int, const QString &)), this, SLOT(slotProgress(int, const QString &)));

    task->start();
    dialog->exec();
    if(task->isRunning()) {
        task->cancel();
    }
}

void TaskTestDialog::slotProgress(int percent, const std::string &msg) {
    qDebug() << "TaskTestDialog::slotProgress" << percent << QString::fromStdString(msg);
}
