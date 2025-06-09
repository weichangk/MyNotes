#include "qgrid_layout.h"

#include <QEventLoop>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

void qgrid_layout() {
    QGridLayoutWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &QGridLayoutWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

QGridLayoutWidget::QGridLayoutWidget(QWidget *parent) :
    QWidget(parent) {
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("姓名:"), 0, 0);
    layout->addWidget(new QLineEdit, 0, 1);
    layout->addWidget(new QLabel("年龄:"), 1, 0);
    layout->addWidget(new QLineEdit, 1, 1);
    setLayout(layout);
}

QGridLayoutWidget::~QGridLayoutWidget() {
}