#pragma once
#include "qtmaterial_global.h"

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QPixmap>

class HorIconTextButton : public QPushButton {
    Q_OBJECT
public:
    enum ButtonStatus {
        Normal = 0,
        Hover,
        Pressed,
        Disabled
    };

    QString buttonStatusToString(ButtonStatus state) {
        switch (state) {
        case Normal: return "normal";
        case Hover: return "hover";
        case Pressed: return "pressed";
        case Disabled: return "disabled";
        default: return "normal";
        }
    }

    HorIconTextButton(QWidget *parent = nullptr);
    ~HorIconTextButton() override {
    }

    void setObjectName(const QString &);

    void setEnabled(bool);
    bool isEnabled() const;

    void setText(const QString &);
    QString text() const;

    void setIconSize(int);
    int iconSize() const;

    void setLeftRightSpacing(int);
    int leftRightSpacing() const;

    void setIconTextSpacing(int);
    int iconTextSpacing() const;

    void setFourPixmap(QPixmap);
    void setNormalPixmap(QPixmap);
    void setHoverPixmap(QPixmap);
    void setPressedPixmap(QPixmap);
    void setDisablePixmap(QPixmap);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;

private:
    void setButtonState(ButtonStatus);

private:
    QHBoxLayout *m_pLayout = nullptr;
    QLabel *m_pIcon = nullptr;
    QLabel *m_pText = nullptr;

    QPixmap m_pixmapFourState;
    QPixmap m_pixmapNormal;
    QPixmap m_pixmapHover;
    QPixmap m_pixmapPressed;
    QPixmap m_pixmapDisabled;

    ButtonStatus m_eButtonState = ButtonStatus::Normal;
    
    int m_nIconSize = 16;
    int m_nLeftRightSpacing = 4;
    int m_nIconTextSpacing = 2;
};
