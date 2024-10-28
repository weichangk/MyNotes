#include "component/rotatingpixmapwidget.h"

RotatingPixmapItem::RotatingPixmapItem(const QPixmap &pixmap, QGraphicsItem *parent) :
    QGraphicsPixmapItem(pixmap, parent) {
    setTransformOriginPoint(pixmap.width() / 2.0, pixmap.height() / 2.0);

    // 创建旋转动画
    m_RotationAnimation = new QPropertyAnimation(this, "rotation");
    m_RotationAnimation->setDuration(2000); // 旋转动画时长，单位为毫秒
    m_RotationAnimation->setStartValue(0);  // 起始角度
    m_RotationAnimation->setEndValue(360);  // 结束角度
    m_RotationAnimation->setLoopCount(-1);  // 无限循环
}

void RotatingPixmapItem::start() {
    // 启动动画
    m_RotationAnimation->start();
}

void RotatingPixmapItem::stop() {
    // 停止动画
    m_RotationAnimation->stop();
}

WidgetWithRotatingItem::WidgetWithRotatingItem(QPixmap pixmap, QWidget *parent) :
    QWidget(parent) {
    setStyleSheet("background-color:transparent; border:none");
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 创建 QGraphicsView 和 QGraphicsScene
    QGraphicsView *view = new QGraphicsView(this);
    QGraphicsScene *scene = new QGraphicsScene(this);

    m_RotatingPixmapItem = new RotatingPixmapItem(pixmap);

    scene->addItem(m_RotatingPixmapItem);

    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFixedSize(pixmap.width(), pixmap.height());

    // 将 QGraphicsView 添加到布局中
    layout->addWidget(view);

    // 设置场景
    view->setScene(scene);
}

WidgetWithRotatingItem::WidgetWithRotatingItem(QWidget *parent)  :
    WidgetWithRotatingItem(QPixmap(":agui/res/image/loading-48.png"), parent) {
}

void WidgetWithRotatingItem::start() {
    // 启动动画
    m_RotatingPixmapItem->start();
}

void WidgetWithRotatingItem::stop() {
    // 停止动画
    m_RotatingPixmapItem->stop();
}
