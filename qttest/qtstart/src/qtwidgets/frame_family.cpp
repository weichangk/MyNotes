#include "frame_family.h"

#include <QEventLoop>

void frame_family() {
    FrameFamilyWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &FrameFamilyWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

FrameFamilyWidget::FrameFamilyWidget(QWidget *parent) :
    QWidget(parent) {
}

FrameFamilyWidget::~FrameFamilyWidget() {
}