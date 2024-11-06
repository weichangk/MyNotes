#pragma once
#include "qtmaterial_global.h"
#include <QWidget>
#include <QPainter>
#include <QEvent>
#include <QPixmap>

namespace widget {
class QTMATERIAL_EXPORT FourStateImage : public QWidget {
    Q_OBJECT
public:
    enum StyleStatus {
        Normal = 0,
        Hover,
        Pressed,
        Checked,
        Disabled
    };
    FourStateImage(QWidget *parent = 0);
    ~FourStateImage();
    void setFourPixmap(QPixmap pixmap);
    void setNormalPixmap(QPixmap pixmap);
    void setHoverPixmap(QPixmap pixmap);
    void setPressedPixmap(QPixmap pixmap);
    void setDisablePixmap(QPixmap pixmap);
    void setState(StyleStatus state);

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    StyleStatus m_State = StyleStatus::Normal;
    QPixmap m_FourPixmap;
    QPixmap m_NormalPixmap;
    QPixmap m_HoverPixmap;
    QPixmap m_PressedPixmap;
    QPixmap m_DisabledPixmap;
};
} // namespace widget