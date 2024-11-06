#pragma once
#include "qtmaterial_global.h"
#include <QWidget>

namespace widget {
class QTMATERIAL_EXPORT Preview : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QPixmap normalIcon READ normalIcon WRITE setNormalIcon NOTIFY sigNormalIconChanged)
    Q_PROPERTY(QPixmap hoverIcon READ hoverIcon WRITE setHoverIcon NOTIFY sigHoverIconChanged)
    Q_PROPERTY(QPixmap pressedIcon READ pressedIcon WRITE setPressedIcon NOTIFY sigPressedIconChanged)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize NOTIFY sigIconSizeChanged)

public:
    enum IconState {
        kNormal,
        kHover,
        kPressed
    };

    explicit Preview(QWidget *parent = nullptr);
    ~Preview();

    QPixmap &normalIcon();
    void setNormalIcon(const QPixmap &icon);
    QPixmap &hoverIcon();
    void setHoverIcon(const QPixmap &icon);
    QPixmap &pressedIcon();
    void setPressedIcon(const QPixmap &icon);
    QSize &iconSize();
    void setIconSize(const QSize &size);

Q_SIGNALS:
    void sigClicked();
    void sigNormalIconChanged(const QPixmap &icon);
    void sigHoverIconChanged(const QPixmap &icon);
    void sigPressedIconChanged(const QPixmap &icon);
    void sigIconSizeChanged(const QSize &size);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    IconState icon_state_ = kNormal;
    QPixmap normal_icon_;
    QPixmap hover_icon_;
    QPixmap pressed_icon_;
    QSize icon_size_;
};
} // namespace widget