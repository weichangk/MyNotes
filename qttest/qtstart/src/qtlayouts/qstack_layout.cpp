#include "qstack_layout.h"

#include <QEventLoop>
#include <QStackedLayout>
#include <QPushButton>
#include <QLabel>

void qstack_layout() {
    QStackLayoutWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &QStackLayoutWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

QStackLayoutWidget::QStackLayoutWidget(QWidget *parent) :
    QWidget(parent) {

    QPushButton *btn1 = new QPushButton("切换到页面1");
    QPushButton *btn2 = new QPushButton("切换到页面2");

    // 页面内容
    QWidget *page1 = new QWidget;
    page1->setStyleSheet("background-color: lightblue");
    page1->setLayout(new QVBoxLayout);
    page1->layout()->addWidget(new QLabel("这是页面1"));

    QWidget *page2 = new QWidget;
    page2->setStyleSheet("background-color: lightgreen");
    page2->setLayout(new QVBoxLayout);
    page2->layout()->addWidget(new QLabel("这是页面2"));

    // 核心：专门创建一个容器 Widget，并设置为 QStackedLayout 的载体
    QWidget *stackedContainer = new QWidget;
    QStackedLayout *stack = new QStackedLayout(stackedContainer);
    stack->addWidget(page1);
    stack->addWidget(page2);

    // 外层主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(btn1);
    mainLayout->addWidget(btn2);
    mainLayout->addWidget(stackedContainer); // ✅ 正确地添加 widget（而不是 layout）

    // 切换页面
    QObject::connect(btn1, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(0);
    });
    QObject::connect(btn2, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(1);
    });
}

QStackLayoutWidget::~QStackLayoutWidget() {
}