#pragma once
#include "qtmaterial_global.h"
#include <QWidget>

namespace widget {
class QTMATERIAL_EXPORT CanMove : public QWidget {
    Q_OBJECT
public:
    CanMove(QWidget *parent);
    ~CanMove();
    void canMoveEnabled(bool enabled) {
        m_canMoveEnabled = enabled;
    }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QWidget *m_parentWidget;
    bool m_canMoveEnabled = true;
    bool m_isMousePressed;
    QPoint m_lastMousePos;
    QRect m_pressRect;
};
} // namespace widget