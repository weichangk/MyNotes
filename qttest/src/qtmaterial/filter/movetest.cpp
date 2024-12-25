#include "movetest.h"
#include "filter/move.h"

#include <QVBoxLayout>

MoveTestWidget::MoveTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("MoveTestWidget");
    setWindowTitle("Move Test");
    setFixedSize(800, 600);
    setWindowFlags(Qt::FramelessWindowHint);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto titleBar = new QWidget(this);
    titleBar->setFixedHeight(36);
    titleBar->setStyleSheet("background-color: red;");

    layout->addWidget(titleBar);
    layout->addStretch();

    auto move = new qtmaterialfilter::MoveTitleBar(titleBar, this);
}
