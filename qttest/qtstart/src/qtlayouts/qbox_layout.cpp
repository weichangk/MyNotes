#include "qbox_layout.h"

#include <QEventLoop>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

void qbox_layout() {
    QBoxLayoutWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &QBoxLayoutWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

QBoxLayoutWidget::QBoxLayoutWidget(QWidget *parent) :
    QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QPushButton("按钮1"));
    layout->addWidget(new QPushButton("按钮2"));
    layout->addWidget(new QPushButton("按钮3"));

    auto hlayout = new QHBoxLayout;
    hlayout->addWidget(new QPushButton("按钮4"));
    hlayout->addWidget(new QPushButton("按钮5"));
    hlayout->addWidget(new QPushButton("按钮6"));

    layout->addLayout(hlayout);
    
    setLayout(layout);
}

QBoxLayoutWidget::~QBoxLayoutWidget() {
}