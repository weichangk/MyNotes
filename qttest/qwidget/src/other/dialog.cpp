#include "dialog.h"
#include <QVBoxLayout>

Dialog::Dialog(QDialog *parent) :
    QDialog(parent) {
    createUi();
    sigConnect();
}

Dialog::~Dialog() {
}

void Dialog::createUi() {
    setMinimumSize(1096, 680);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    translucent_btn_ = new QPushButton(this);
    translucent_btn_->setFixedSize(100, 32);
    translucent_btn_->setText("translucent test");
    layout->addWidget(translucent_btn_);
    translucent_widget_ = new TranslucentBackgroundWidget();
    // mac下，在dilog窗体下打开透明窗体，透明窗体置顶无效，且无法获取鼠标事件
    translucent_widget_->setWindowFlags(translucent_widget_->windowFlags() | Qt::WindowStaysOnTopHint);
}

void Dialog::sigConnect() {
    connect(translucent_btn_, &QPushButton::clicked, this, &Dialog::translucentShow);
}

void Dialog::translucentShow() {
    translucent_widget_->show();
}
