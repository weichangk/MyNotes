#include "translucentbackground.h"
#include <QPainterPath>
#include <QMouseEvent>

TranslucentBackgroundWidget::TranslucentBackgroundWidget(QWidget *parent) :
    QWidget(parent) {
    createUi();
    sigConnect();
}

TranslucentBackgroundWidget::~TranslucentBackgroundWidget() {
}

void TranslucentBackgroundWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    Q_UNUSED(event);
    QPainter painter(this);

    QRect rect = this->rect();
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

#ifdef Q_OS_WIN
    // win 透明窗体要设透明度才会有鼠标事件
    painter.fillRect(rect, QColor(0, 0, 0, 1));
#endif

    QPen pen;
    pen.setWidth(1);
    pen.setColor(Qt::red);
    painter.setPen(pen);
    painter.drawRect(rect);
}

void TranslucentBackgroundWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
    }
}

void TranslucentBackgroundWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
    }
}

void TranslucentBackgroundWidget::createUi() {
    setWindowTitle("Translucent Background Test Widget");
    setFixedSize(800, 600);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // 接收鼠标移动事件
    // setMouseTracking(true): 在鼠标移动时，无论是否按下鼠标按钮，都会接收到mouseMoveEvent事件
    // setMouseTracking(false): 只有在按下鼠标按钮并移动时，才会接收到mouseMoveEvent事件
    setMouseTracking(true);
}

void TranslucentBackgroundWidget::sigConnect() {
}
