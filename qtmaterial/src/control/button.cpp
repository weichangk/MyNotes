#include "control/button.h"
#include <QPainter>
#include <QVariant>

QString buttonStatusToString(button::ButtonStatus state) {
    switch (state) {
    case button::Normal: return "normal";
    case button::Hover: return "hover";
    case button::Pressed: return "pressed";
    case button::Disabled: return "disabled";
    default: return "normal";
    }
}

IconButton::IconButton(QWidget *parent) :
    QPushButton(parent) {
    QPushButton::setObjectName("IconButton");
}

void IconButton::setObjectName(const QString &name) {
    QPushButton::setObjectName(name);
}

void IconButton::setEnabled(bool b) {
    QPushButton::setEnabled(b);
    setButtonState(button::ButtonStatus::Disabled);
}

bool IconButton::isEnabled() const {
    return QPushButton::isEnabled();
}

void IconButton::setIconMargin(int n) {
    m_nIconMargin = n;
}

int IconButton::iconMargin() const {
    return m_nIconMargin;
}

void IconButton::setFourPixmap(const QString &path) {
    m_strFourStatePath = path;
    m_pixmapFourState = QPixmap(path);
    m_pixmapNormal = m_pixmapFourState.copy(0, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
    m_pixmapHover = m_pixmapFourState.copy(m_pixmapFourState.width() / 4, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
    m_pixmapPressed = m_pixmapFourState.copy(m_pixmapFourState.width() / 2, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
    m_pixmapDisabled = m_pixmapFourState.copy(m_pixmapFourState.width() / 4 * 3, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
}

QString IconButton::fourPixmapPath() const {
    return m_strFourStatePath;
}

void IconButton::setNormalPixmap(const QString &path) {
    m_strNormalPath = path;
    m_pixmapNormal = QPixmap(path);
}

QString IconButton::normalPixmapPath() const {
    return m_strNormalPath;
}

void IconButton::setHoverPixmap(const QString &path) {
    m_strHoverPath = path;
    m_pixmapHover = QPixmap(path);
}

QString IconButton::hoverPixmapPath() const {
    return m_strHoverPath;
}

void IconButton::setPressedPixmap(const QString &path) {
    m_strPressedPath = path;
    m_pixmapPressed = QPixmap(path);
}

QString IconButton::pressedPixmapPath() const {
    return m_strPressedPath;
}

void IconButton::setDisablePixmap(const QString &path) {
    m_strDisabledPath = path;
    m_pixmapDisabled = QPixmap(path);
}

QString IconButton::disablePixmapPath() const {
    return m_strDisabledPath;
}

void IconButton::paintEvent(QPaintEvent *event) {
    QPushButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPixmap pixmap = getCurrentPixmap();
    if (!pixmap.isNull()) {
        QSize iconSize(rect().width() - m_nIconMargin, rect().height() - m_nIconMargin);
        QRect iconRect(m_nIconMargin, m_nIconMargin, iconSize.width(), iconSize.height());
        painter.drawPixmap(iconRect, pixmap.scaled(iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
}

void IconButton::mousePressEvent(QMouseEvent *event) {
    QPushButton::mousePressEvent(event);
    setButtonState(button::ButtonStatus::Pressed);
}

void IconButton::mouseReleaseEvent(QMouseEvent *event) {
    QPushButton::mouseReleaseEvent(event);
    setButtonState(button::ButtonStatus::Normal);
}

void IconButton::enterEvent(QEvent *event) {
    QPushButton::enterEvent(event);
    setButtonState(button::ButtonStatus::Hover);
}

void IconButton::leaveEvent(QEvent *event) {
    QPushButton::leaveEvent(event);
    setButtonState(button::ButtonStatus::Normal);
}

void IconButton::setButtonState(button::ButtonStatus state) {
    m_eButtonState = state;
    update();
}

QPixmap IconButton::getCurrentPixmap() const {
    switch (m_eButtonState) {
        case button::Hover: return m_pixmapHover;
        case button::Pressed: return m_pixmapPressed;
        case button::Disabled: return m_pixmapDisabled;
        default: return m_pixmapNormal;
    }
}

HorIconTextButton::HorIconTextButton(QWidget *parent) :
    QPushButton(parent) {
    QPushButton::setObjectName("HorIconTextButton");
    m_pLayout = new QHBoxLayout(this);
    m_pLayout->setContentsMargins(m_nLeftRightSpacing, 0, m_nLeftRightSpacing, 0);

    m_pText = new QLabel(this);
    m_pText->setObjectName("HorIconTextButtonText");
    m_pText->setProperty("ButtonStatus", "normal");

    m_pLayout->addSpacing(m_nIconSize + m_nIconTextSpacing);
    m_pLayout->addWidget(m_pText, 0, Qt::AlignCenter);
    m_pLayout->addStretch();
}

void HorIconTextButton::setObjectName(const QString &name) {
    QPushButton::setObjectName(name);
    if (m_pText) {
        m_pText->setObjectName(name + "Text");
    }
}

void HorIconTextButton::setEnabled(bool b) {
    QPushButton::setEnabled(b);
    setButtonState(button::ButtonStatus::Disabled);
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

void HorIconTextButton::setFourPixmap(const QString &path) {
    m_strFourStatePath = path;
    m_pixmapFourState = QPixmap(path);
    m_pixmapNormal = m_pixmapFourState.copy(0, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
    m_pixmapHover = m_pixmapFourState.copy(m_pixmapFourState.width() / 4, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
    m_pixmapPressed = m_pixmapFourState.copy(m_pixmapFourState.width() / 2, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
    m_pixmapDisabled = m_pixmapFourState.copy(m_pixmapFourState.width() / 4 * 3, 0, m_pixmapFourState.width() / 4, m_pixmapFourState.height());
}

QString HorIconTextButton::fourPixmapPath() const {
    return m_strFourStatePath;
}

void HorIconTextButton::setNormalPixmap(const QString &path) {
    m_strNormalPath = path;
    m_pixmapNormal = QPixmap(path);
}

QString HorIconTextButton::normalPixmapPath() const {
    return m_strNormalPath;
}

void HorIconTextButton::setHoverPixmap(const QString &path) {
    m_strHoverPath = path;
    m_pixmapHover = QPixmap(path);
}

QString HorIconTextButton::hoverPixmapPath() const {
    return m_strHoverPath;
}

void HorIconTextButton::setPressedPixmap(const QString &path) {
    m_strPressedPath = path;
    m_pixmapPressed = QPixmap(path);
}

QString HorIconTextButton::pressedPixmapPath() const {
    return m_strPressedPath;
}

void HorIconTextButton::setDisablePixmap(const QString &path) {
    m_strDisabledPath = path;
    m_pixmapDisabled = QPixmap(path);
}

QString HorIconTextButton::disablePixmapPath() const {
    return m_strDisabledPath;
}

void HorIconTextButton::paintEvent(QPaintEvent *event) {
    QPushButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPixmap pixmap = getCurrentPixmap();
    if (!pixmap.isNull()) {
        QSize iconSize = QSize(m_nIconSize, m_nIconSize);
        QRect iconRect = QRect(m_nLeftRightSpacing, (height() - m_nIconSize) / 2, m_nIconSize, m_nIconSize);
        painter.drawPixmap(iconRect, pixmap.scaled(iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
}

void HorIconTextButton::mousePressEvent(QMouseEvent *event) {
    QPushButton::mousePressEvent(event);
    setButtonState(button::ButtonStatus::Pressed);
}

void HorIconTextButton::mouseReleaseEvent(QMouseEvent *event) {
    QPushButton::mouseReleaseEvent(event);
    setButtonState(button::ButtonStatus::Normal);
}

void HorIconTextButton::enterEvent(QEvent *event) {
    QPushButton::enterEvent(event);
    setButtonState(button::ButtonStatus::Hover);
}

void HorIconTextButton::leaveEvent(QEvent *event) {
    QPushButton::leaveEvent(event);
    setButtonState(button::ButtonStatus::Normal);
}

void HorIconTextButton::setButtonState(button::ButtonStatus state) {
    m_pText->setProperty("ButtonStatus", buttonStatusToString(state));
    m_pText->setStyle(m_pText->style());
    m_eButtonState = state;
    update();
}

QPixmap HorIconTextButton::getCurrentPixmap() const {
    switch (m_eButtonState) {
        case button::Hover: return m_pixmapHover;
        case button::Pressed: return m_pixmapPressed;
        case button::Disabled: return m_pixmapDisabled;
        default: return m_pixmapNormal;
    }
}

IconVectorButton::IconVectorButton(QWidget *parent) :
    QPushButton(parent) {
}