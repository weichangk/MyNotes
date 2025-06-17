#include "masktest.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>

ArrowMaskTestWidget::ArrowMaskTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("ArrowMaskTestWidget");
    setWindowTitle("ArrowMask Test");
    setFixedSize(400, 400);

    auto layout = new QVBoxLayout(this);

    QLabel *label = new QLabel(this);
    label->setText("Shadow Test");

    QWidget *widget = new QWidget(this);
    widget->setFixedSize(100, 100);
    widget->setStyleSheet("background-color:#000000;");
    
    layout->addWidget(label);
    layout->addWidget(widget);
    layout->addStretch();

    auto mask = new qtmaterialfilter::ArrowWindow(this);
}

ArrowMaskTestWidget::~ArrowMaskTestWidget() {
}