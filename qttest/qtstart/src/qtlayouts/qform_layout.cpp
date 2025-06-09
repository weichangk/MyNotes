#include "qform_layout.h"

#include <QEventLoop>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

void qform_layout() {
    QFormLayoutWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &QFormLayoutWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

QFormLayoutWidget::QFormLayoutWidget(QWidget *parent) :
    QWidget(parent) {
    QFormLayout *layout = new QFormLayout;
    layout->addRow("用户名:", new QLineEdit);
    layout->addRow("密码:", new QLineEdit);
    setLayout(layout);
}

QFormLayoutWidget::~QFormLayoutWidget() {
}