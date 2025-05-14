#include "painter.h"
#include <QPainterPath>

PainterWidget::PainterWidget(QWidget *parent) :
    QWidget(parent) {
    createUi();
    sigConnect();
}

PainterWidget::~PainterWidget() {
}

void PainterWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    QColor borderColor = QColor("#FFFFFF");
    QColor contentColor = QColor("#AF5FFF");
    Painter *painterTest = new Painter();
    painterTest->draw2Diamond(&painter, QPoint(100, 100), QPoint(300, 100), borderColor, contentColor);
}

void PainterWidget::createUi() {
    setWindowTitle("Painter Test Widget");
    setFixedSize(800, 600);
}

void PainterWidget::sigConnect() {
}

void Painter::draw2Diamond(QPainter *painter, QPoint startPoint, QPoint endPoint, const QColor &borderColor, const QColor &contentColor) {
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath path1;
    path1.moveTo(startPoint.x() - 6, startPoint.y());
    path1.lineTo(startPoint.x(), startPoint.y() - 6);
    path1.lineTo(startPoint.x() + 5, startPoint.y() - 2);
    path1.lineTo(endPoint.x() - 8, endPoint.y() - 5);
    path1.lineTo(endPoint.x(), endPoint.y() - 12);
    path1.lineTo(endPoint.x() + 12, endPoint.y());
    path1.lineTo(endPoint.x(), endPoint.y() + 12);
    path1.lineTo(endPoint.x() - 8, endPoint.y() + 5);
    path1.lineTo(startPoint.x() + 5, startPoint.y() + 2);
    path1.lineTo(startPoint.x(), startPoint.y() + 6);
    path1.closeSubpath();

    // 如果启用了QPainter::Antialiasing抗锯齿渲染，但仍然出现了不希望的阴影效果，可能是因为在某些情况下，抗锯齿渲染可能会导致颜色混合，从而产生阴影效果。要消除这种情况下的阴影效果，可以尝试以下方法：
    // 1. 设置画笔的颜色为透明：将画笔的颜色设置为透明，这样可以确保绘制的对象不会在边缘产生额外的颜色混合。
    // 2.使用更加平滑的渐变颜色：如果您需要在绘制的对象上应用颜色，可以尝试使用更加平滑的渐变颜色，以减少颜色混合带来的阴影效果。

    // QLinearGradient gradient(0, 0, 0, 0);
    // gradient.setColorAt(0, borderColor);
    // gradient.setColorAt(1, borderColor);
    // painter->setPen(QPen(gradient, 2));
    // painter->drawPath(path1);

    painter->setBrush(borderColor);
    painter->setPen(QPen(Qt::transparent, 2));
    painter->drawPath(path1);

    QPainterPath path2;
    path2.moveTo(startPoint.x() - 4, startPoint.y());
    path2.lineTo(startPoint.x(), startPoint.y() - 4);
    path2.lineTo(startPoint.x() + 3, startPoint.y() - 1);
    path2.lineTo(endPoint.x() - 7, endPoint.y() - 3);
    path2.lineTo(endPoint.x(), endPoint.y() - 10);
    path2.lineTo(endPoint.x() + 10, endPoint.y());
    path2.lineTo(endPoint.x(), endPoint.y() + 10);
    path2.lineTo(endPoint.x() - 7, endPoint.y() + 3);
    path2.lineTo(startPoint.x() + 3, startPoint.y() + 1);
    path2.lineTo(startPoint.x(), startPoint.y() + 4);
    path2.closeSubpath();

    painter->setBrush(contentColor);
    painter->setPen(QPen(Qt::transparent, 2));
    painter->drawPath(path2);
}