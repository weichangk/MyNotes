#include "widget/preview.h"
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

namespace widget {
inline QRect previewButtonRect(QRect btnRect, QSize iconSize) {
    auto rc = btnRect.adjusted(1, 1, -1, -1);
    return QRect(rc.x() + (rc.width() - iconSize.width()) / 2, rc.y() + (rc.height() - iconSize.height()) / 2, iconSize.width(), iconSize.height());
}

Preview::Preview(QWidget *parent) :
    QWidget(parent),
    icon_state_(kNormal) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

Preview::~Preview() {
}

QPixmap &Preview::normalIcon() {
    return normal_icon_;
}

void Preview::setNormalIcon(const QPixmap &icon) {
    icon_state_ = kNormal;
    QImage img1 = icon.toImage();
    QImage img2 = normal_icon_.toImage();
    if (img1 != img2) {
        normal_icon_ = icon;
        emit sigNormalIconChanged(normal_icon_);
        update();
    }
}

QPixmap &Preview::hoverIcon() {
    return hover_icon_;
}

void Preview::setHoverIcon(const QPixmap &icon) {
    icon_state_ = kHover;
    QImage img1 = icon.toImage();
    QImage img2 = hover_icon_.toImage();
    if (img1 != img2) {
        hover_icon_ = icon;
        emit sigNormalIconChanged(hover_icon_);
        update();
    }
}

QPixmap &Preview::pressedIcon() {
    return pressed_icon_;
}

void Preview::setPressedIcon(const QPixmap &icon) {
    icon_state_ = kPressed;
    QImage img1 = icon.toImage();
    QImage img2 = pressed_icon_.toImage();
    if (img1 != img2) {
        pressed_icon_ = icon;
        emit sigNormalIconChanged(pressed_icon_);
        update();
    }
}

QSize &Preview::iconSize() {
    return icon_size_;
}

void Preview::setIconSize(const QSize &size) {
    if (icon_size_ != size) {
        icon_size_ = size;
        emit sigIconSizeChanged(icon_size_);
        update();
    }
}

bool Preview::eventFilter(QObject *watched, QEvent *event) {
    return QWidget::eventFilter(watched, event);
}

void Preview::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QPixmap iconPixmap;
    switch (icon_state_) {
    case kNormal:
        iconPixmap = normal_icon_;
        break;
    case kHover:
        iconPixmap = hover_icon_;
        break;
    case kPressed:
        iconPixmap = pressed_icon_;
        break;
    default:
        iconPixmap = normal_icon_;
        break;
    }
    auto iconRc = previewButtonRect(this->rect(), icon_size_);
    painter.drawPixmap(iconRc, iconPixmap);
}

void Preview::changeEvent(QEvent *event) {
    if (event->type() == QEvent::StyleChange) {
        update();
    }
    QWidget::changeEvent(event);
}

void Preview::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit sigClicked();
    }
}
} // namespace widget