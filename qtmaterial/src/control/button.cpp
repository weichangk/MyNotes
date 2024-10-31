#include "control/button.h"
#include <QPainter>
#include <QVariant>

HorIconTextButton::HorIconTextButton(QWidget *parent) :
    QPushButton(parent) {
    QPushButton::setObjectName("HorIconTextButton");
    setAttribute(Qt::WA_StyledBackground, true);
    m_pLayout = new QHBoxLayout(this);
    m_pLayout->setContentsMargins(m_nLeftRightSpacing, 0, m_nLeftRightSpacing, 0);
    m_pLayout->setSpacing(m_nIconTextSpacing);

    m_pIcon = new QLabel(this);
    m_pIcon->setFixedSize(m_nIconSize, m_nIconSize);
    m_pIcon->setObjectName("HorIconTextButtonIcon");

    m_pText = new QLabel(this);
    m_pText->setObjectName("HorIconTextButtonText");
    m_pText->setProperty("ButtonStatus", "normal");

    m_pLayout->addWidget(m_pIcon, 0, Qt::AlignVCenter);
    m_pLayout->addWidget(m_pText, 0, Qt::AlignVCenter);
    m_pLayout->addStretch();
}

void HorIconTextButton::setObjectName(const QString &name) {
    QPushButton::setObjectName(name);
    if (m_pIcon) {
        m_pIcon->setObjectName(name + "Icon");
    }
    if (m_pText) {
        m_pText->setObjectName(name + "Text");
    }
}

void HorIconTextButton::setEnabled(bool b) {
    QPushButton::setEnabled(b);
    setButtonState(ButtonStatus::Disabled);
}

bool HorIconTextButton::isEnabled() const {
    return QPushButton::isEnabled();
}

void HorIconTextButton::setText(const QString &text) {
    m_pText->setText(text);
}

QString HorIconTextButton::text() const {
    return m_pText->text();
}

void HorIconTextButton::setIconSize(int n) {
    m_nIconSize = n;
    m_pIcon->setFixedSize(n, n);
}

int HorIconTextButton::iconSize() const {
    return m_nIconSize;
}

void HorIconTextButton::setLeftRightSpacing(int n) {
    m_nLeftRightSpacing = n;
    m_pLayout->setContentsMargins(m_nLeftRightSpacing, 0, m_nLeftRightSpacing, 0);
}

int HorIconTextButton::leftRightSpacing() const {
    return m_nLeftRightSpacing;
}

void HorIconTextButton::setIconTextSpacing(int n) {
    m_nIconTextSpacing = n;
    m_pLayout->setSpacing(m_nIconTextSpacing);
}

int HorIconTextButton::iconTextSpacing() const {
    return m_nIconTextSpacing;
}

void HorIconTextButton::setFourPixmap(QPixmap p) {
    m_pixmapFourState = p;
    m_pixmapNormal = m_pixmapFourState.copy(0, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
    m_pixmapHover = m_pixmapFourState.copy(m_pixmapFourState.width() / 4, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
    m_pixmapPressed = m_pixmapFourState.copy(m_pixmapFourState.width() / 2, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
    m_pixmapDisabled = m_pixmapFourState.copy(m_pixmapFourState.width() / 4 * 3, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
}

void HorIconTextButton::setNormalPixmap(QPixmap p) {
    m_pixmapNormal = p;
}

void HorIconTextButton::setHoverPixmap(QPixmap p) {
    m_pixmapHover = p;
}

void HorIconTextButton::setPressedPixmap(QPixmap p) {
    m_pixmapPressed = p;
}

void HorIconTextButton::setDisablePixmap(QPixmap p) {
    m_pixmapDisabled = p;
}

void HorIconTextButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPixmap pixmapTemp;
    switch (m_eButtonState) {
        case ButtonStatus::Hover: {
            if (!m_pixmapHover.isNull()) {
                pixmapTemp = m_pixmapNormal.scaled(m_pIcon->rect().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                pixmapTemp.setDevicePixelRatio(1);
                painter.drawPixmap(m_pIcon->rect(), pixmapTemp);
                break;
            }
        }
        case ButtonStatus::Pressed: {
            if (!m_pixmapPressed.isNull()) {
                pixmapTemp = m_pixmapNormal.scaled(m_pIcon->rect().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                pixmapTemp.setDevicePixelRatio(1);
                painter.drawPixmap(m_pIcon->rect(), pixmapTemp);
                break;
            }
        }
        case ButtonStatus::Disabled: {
            if (!m_pixmapDisabled.isNull()) {
                pixmapTemp = m_pixmapNormal.scaled(m_pIcon->rect().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                pixmapTemp.setDevicePixelRatio(1);
                painter.drawPixmap(m_pIcon->rect(), pixmapTemp);
                break;
            }
        }
        default: {
            if (!m_pixmapNormal.isNull()) {
                pixmapTemp = m_pixmapNormal.scaled(m_pIcon->rect().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                pixmapTemp.setDevicePixelRatio(1);
                painter.drawPixmap(m_pIcon->rect(), pixmapTemp);
                break;
            }
        }
        QWidget::paintEvent(event);
    }
}

void HorIconTextButton::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    setButtonState(ButtonStatus::Pressed);
}

void HorIconTextButton::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    setButtonState(ButtonStatus::Normal);
}

void HorIconTextButton::enterEvent(QEvent *event) {
    QWidget::enterEvent(event);
    setButtonState(ButtonStatus::Hover);
}

void HorIconTextButton::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    setButtonState(ButtonStatus::Normal);
}

void HorIconTextButton::setButtonState(ButtonStatus state) {
    m_pText->setProperty("ButtonStatus", buttonStatusToString(state));
    m_pText->setStyle(m_pText->style());
    m_eButtonState = state;
    update();
}