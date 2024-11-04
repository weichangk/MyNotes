#include "layout.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

//QPushButton如果不设置样式，给按钮设置大小时显示出来的偏小！
//mac下layout里的按钮大小不一样不对齐问题可以通过layout->setContentsMargins(0, 0, 0, 0)解决，还是解决不了可以设置layout所在的widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);

LayoutMacBugWidget::LayoutMacBugWidget(QWidget *parent) :
    QWidget(parent) {
    createUi();
    sigConnect();
}

LayoutMacBugWidget::~LayoutMacBugWidget() {
}

void LayoutMacBugWidget::createUi() {
    // 如果不设置样式，给按钮设置大小时显示出来的偏小！
    QString btnStyle = R"(
        QPushButton { 
            background-color: 
            yellow;border-radius: 10px;
        }
    )";

    setWindowTitle("QLayout Test");
    setFixedSize(800, 600);

    auto layout = new QVBoxLayout(this);

    auto w1 = new QWidget(this);
    w1->setFixedHeight(60);

    auto w2 = new QWidget(this);
    w2->setFixedWidth(60);

    layout->addWidget(w1);
    layout->addWidget(w2);

    btn1 = new QPushButton("1", this);
    btn1->setFixedSize(124, 24);
    btn1->setStyleSheet(btnStyle);
    btn2 = new QPushButton("2", this);
    btn2->setFixedSize(124, 36);
    btn2->setStyleSheet(btnStyle);
    btn3 = new QPushButton("3", this);
    btn3->setFixedSize(124, 48);
    btn3->setStyleSheet(btnStyle);

    btn4 = new QPushButton("4");
    btn4->setFixedSize(24, 124);
    // btn4->setStyleSheet(btnStyle);
    btn5 = new QPushButton("5");
    btn5->setFixedSize(36, 124);
    // btn5->setStyleSheet(btnStyle);
    btn6 = new QPushButton("6");
    btn6->setFixedSize(48, 124);
    // btn6->setStyleSheet(btnStyle);

    auto hlayout = new QHBoxLayout(w1);
    hlayout->setContentsMargins(0, 0, 0, 0);// 解决mac下layout里的按钮大小不一样时不对齐问题，在win下也有问题
    // hlayout->setAlignment(Qt::AlignCenter);
    // hlayout->setSpacing(24);
    hlayout->addWidget(btn1, 0, Qt::AlignCenter);
    hlayout->addWidget(btn2, 0, Qt::AlignCenter);
    hlayout->addWidget(btn3, 0, Qt::AlignCenter);
    hlayout->addStretch();

    auto vlayout = new QVBoxLayout(w2);
    vlayout->setContentsMargins(0, 0, 0, 0);
    //vlayout->setAlignment(Qt::AlignCenter);
    //vlayout->setSpacing(24);
    vlayout->addWidget(btn4, 0, Qt::AlignRight);
    vlayout->addWidget(btn5, 0, Qt::AlignRight);
    vlayout->addWidget(btn6, 0, Qt::AlignRight);
    vlayout->addStretch();


}

void LayoutMacBugWidget::sigConnect() {
}