#include "component/canmovewidget.h"
#include <QMouseEvent>

CanMoveWidget::CanMoveWidget(QWidget *parent) :
    QWidget(parent), m_parentWidget(parent) {
}

CanMoveWidget::~CanMoveWidget() {
}

void CanMoveWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (contentsRect().contains(event->pos().rx(), event->pos().ry())) {
            if (m_parentWidget) {
                m_lastMousePos = event->globalPos() - m_parentWidget->geometry().topLeft();
                m_isMousePressed = true;
                m_pressRect = m_parentWidget->rect();
            }
        }
    }
    QWidget::mousePressEvent(event);
}

void CanMoveWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton))
        return;
    if (m_isMousePressed == true && m_canMoveEnabled) {
        if (m_parentWidget) {
            auto pos = event->globalPos() - m_lastMousePos;
            m_parentWidget->setGeometry(QRect(pos.x(), pos.y(), m_pressRect.width(), m_pressRect.height()));
        }
    }
    QWidget::mouseMoveEvent(event);
}

void CanMoveWidget::mouseReleaseEvent(QMouseEvent *event) {
    m_isMousePressed = false;
    QWidget::mouseReleaseEvent(event);
}
