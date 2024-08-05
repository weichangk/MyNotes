/*
 * @Author: weick
 * @Date: 2024-05-26 08:30:54
 * @Last Modified by: weick
 * @Last Modified time: 2024-08-06 00:36:26
 */

#include "mainwindow.h"
#include <QVBoxLayout>
#include "qtwidget/painter.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent) {
    createUi();
    sigConnect();
}

MainWindow::~MainWindow() {
}

void MainWindow::sigConnect() {
    connect(painter_btn_, &QPushButton::clicked, this, &MainWindow::painterShow);
    connect(tab_btn_, &QPushButton::clicked, this, &MainWindow::tabShow);
    connect(translucent_btn_, &QPushButton::clicked, this, &MainWindow::translucentShow);
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


    tab_btn_ = new QPushButton(this);
    tab_btn_->setFixedSize(100, 32);
    tab_btn_->setText("tab test");
    layout->addWidget(tab_btn_);
    tab_widget_ = new TabWidget();

    translucent_btn_ = new QPushButton(this);
    translucent_btn_->setFixedSize(100, 32);
    translucent_btn_->setText("translucent test");
    layout->addWidget(translucent_btn_);
    translucent_widget_ = new TranslucentBackgroundWidget();
}

void MainWindow::painterShow() {
    painter_widget_->show();
}

void MainWindow::tabShow() {
    tab_widget_->show();
}

void MainWindow::translucentShow() {
    translucent_widget_->show();
}