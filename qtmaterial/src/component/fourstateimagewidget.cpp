#include "component/fourstateimagewidget.h"

FourStateImageWidget::FourStateImageWidget(QWidget *parent) :
    QWidget(parent) {
}

FourStateImageWidget::~FourStateImageWidget() {
}

void FourStateImageWidget::setFourPixmap(QPixmap pixmap) {
    m_FourPixmap = pixmap;
    m_NormalPixmap = m_FourPixmap.copy(0, 0, m_FourPixmap.width() / 4, m_FourPixmap.height());
    m_HoverPixmap = m_FourPixmap.copy(m_FourPixmap.width() / 4, 0, m_FourPixmap.width() / 4, m_FourPixmap.height());
    m_PressedPixmap = m_FourPixmap.copy(m_FourPixmap.width() / 2, 0, m_FourPixmap.width() / 4, m_FourPixmap.height());
    m_DisabledPixmap = m_FourPixmap.copy(m_FourPixmap.width() / 4 * 3, 0, m_FourPixmap.width() / 4, m_FourPixmap.height());
}

void FourStateImageWidget::setNormalPixmap(QPixmap pixmap) {
    m_NormalPixmap = pixmap;
}

void FourStateImageWidget::setHoverPixmap(QPixmap pixmap) {
    m_HoverPixmap = pixmap;
}

void FourStateImageWidget::setPressedPixmap(QPixmap pixmap) {
    m_PressedPixmap = pixmap;
}

void FourStateImageWidget::setDisablePixmap(QPixmap pixmap) {
    m_DisabledPixmap = pixmap;
}

void FourStateImageWidget::setState(StyleStatus state) {
    m_State = state;
    update();
}

void FourStateImageWidget::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
}

void FourStateImageWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    if (!m_FourPixmap.isNull()) {
        QPixmap pixmapTemp;
        switch (m_State) {
        case StyleStatus::Hover:
            pixmapTemp = m_HoverPixmap.scaled(this->rect().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            pixmapTemp.setDevicePixelRatio(1);
            painter.drawPixmap(this->rect(), pixmapTemp);
            break;
        case StyleStatus::Pressed:
        case StyleStatus::Checked:
            pixmapTemp = m_PressedPixmap.scaled(this->rect().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            pixmapTemp.setDevicePixelRatio(1);
            painter.drawPixmap(this->rect(), pixmapTemp);
            break;
        case StyleStatus::Disabled:
            pixmapTemp = m_DisabledPixmap.scaled(this->rect().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            pixmapTemp.setDevicePixelRatio(1);
            painter.drawPixmap(this->rect(), pixmapTemp);
            break;
        default:
            pixmapTemp = m_NormalPixmap.scaled(this->rect().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            pixmapTemp.setDevicePixelRatio(1);
            painter.drawPixmap(this->rect(), pixmapTemp);
            break;
        }
    } else {
        switch (m_State) {
        case 1:
            m_HoverPixmap.setDevicePixelRatio(1);
            painter.drawPixmap(this->rect(), m_HoverPixmap);
            break;
        case 2:
            m_PressedPixmap.setDevicePixelRatio(1);
            painter.drawPixmap(this->rect(), m_PressedPixmap);
            break;
        case 3:
            m_DisabledPixmap.setDevicePixelRatio(1);
            painter.drawPixmap(this->rect(), m_DisabledPixmap);
            break;
        default:
            m_NormalPixmap.setDevicePixelRatio(1);
            painter.drawPixmap(this->rect(), m_NormalPixmap);
            break;
        }
    }
    QWidget::paintEvent(event);
}

void FourStateImageWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (m_State != StyleStatus::Checked) {
        m_State = StyleStatus::Pressed;
        update();
    }
}

void FourStateImageWidget::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    if (m_State != StyleStatus::Checked) {
        m_State = StyleStatus::Hover;
        update();
    }
}

void FourStateImageWidget::enterEvent(QEvent *event) {
    QWidget::enterEvent(event);
    if (m_State != StyleStatus::Checked) {
        m_State = StyleStatus::Hover;
        update();
    }
}

void FourStateImageWidget::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    if (m_State != StyleStatus::Checked) {
        m_State = StyleStatus::Normal;
        update();
    }
}

void FourStateImageWidget::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        if (!isEnabled()) {
            m_State = StyleStatus::Disabled;
            update();
        }
    }
}
