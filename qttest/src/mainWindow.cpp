/*
 * @Author: weick
 * @Date: 2024-05-26 08:30:54
 * @Last Modified by: weick
 * @Last Modified time: 2024-05-26 09:39:30
 */

#include "mainwindow.h"
#include <QVBoxLayout>
#include "qtwidgetsbeginners/painter.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent) {
    createUi();
    sigConnect();
}

MainWindow::~MainWindow() {
}

void MainWindow::sigConnect() {
    connect(painter_btn_, &QPushButton::clicked, this, &MainWindow::painterTest);
}

void MainWindow::createUi() {
    // setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setMinimumSize(1096, 680);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    painter_btn_ = new QPushButton(this);
    painter_btn_->setFixedSize(100, 32);
    painter_btn_->setText("painter test");
    layout->addWidget(painter_btn_);

    painter_widget_ = new PainterWidget();
}

void MainWindow::painterTest() {
    painter_widget_->show();
}