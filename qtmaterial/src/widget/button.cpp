#include "widget/button.h"
#include <QPainter>
#include <QVariant>
#include <QEvent>

namespace widget {
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

VectorButton::VectorButton(QWidget *parent) :
    QPushButton(parent) {
}

HorIconTextVectorButton::HorIconTextVectorButton(QWidget *parent) :
    QPushButton(parent) {
    QPushButton::setObjectName("HorIconTextVectorButton");
    m_pLayout = new QHBoxLayout(this);
    m_pLayout->setContentsMargins(m_nLeftRightSpacing, 0, m_nLeftRightSpacing, 0);
    m_pLayout->setSpacing(m_nIconTextSpacing);

    m_pIcon= new widget::VectorLabel(this);
    m_pIcon->setObjectName("HorIconTextVectorButton_Icon");
    m_pIcon->setFixedSize(m_nIconSize, m_nIconSize);

    m_pLayout->addWidget(m_pIcon, 0, Qt::AlignCenter);

    m_pText = new QLabel(this);
    m_pText->setObjectName("HorIconTextVectorButton_Text");

    m_pLayout->addWidget(m_pText, 0, Qt::AlignCenter);
    m_pLayout->addStretch();

    updateWidgetStatus(style::WidgetStatus::Normal);

    connect(this, &QPushButton::toggled, this, &HorIconTextVectorButton::slotToggled);
}

void HorIconTextVectorButton::setObjectName(const QString &name) {
    QPushButton::setObjectName(name);
    if(m_pIcon) {
        m_pIcon->setObjectName(name + "_Icon");
        m_pIcon->setStyle(m_pIcon->style());
    }
    if (m_pText) {
        m_pText->setObjectName(name + "_Text");
        m_pText->setStyle(m_pText->style());
    }
}

void HorIconTextVectorButton::setIconFont(const QFont &font){
    m_pIcon->setFont(font);
}

void HorIconTextVectorButton::setIcon(const QString &text) {
    m_pIcon->setText(text);
}

void HorIconTextVectorButton::setIconSize(int) {
    m_pIcon->setFixedSize(m_nIconSize, m_nIconSize);
}

void HorIconTextVectorButton::setText(const QString &text) {
    m_pText->setText(text);
}

QString HorIconTextVectorButton::text() const {
    return m_pText->text();
}

void HorIconTextVectorButton::setAdjustWidth(bool b) {
    m_bAdjustWidth = b;
}
bool HorIconTextVectorButton::adjustWidth() const {
    return m_bAdjustWidth;
}

void HorIconTextVectorButton::mousePressEvent(QMouseEvent *event) {
    QPushButton::mousePressEvent(event);
    if (m_eState != style::WidgetStatus::Checked && m_eState != style::WidgetStatus::Disabled) {
        updateWidgetStatus(style::WidgetStatus::Pressed);
    }
}

void HorIconTextVectorButton::mouseReleaseEvent(QMouseEvent *event) {
    QPushButton::mouseReleaseEvent(event);
    if (m_eState != style::WidgetStatus::Checked && m_eState != style::WidgetStatus::Disabled) {
        updateWidgetStatus(style::WidgetStatus::Hover);
    }
}

void HorIconTextVectorButton::enterEvent(QEvent *event) {
    QPushButton::enterEvent(event);
    if (m_eState != style::WidgetStatus::Checked && m_eState != style::WidgetStatus::Disabled) {
        updateWidgetStatus(style::WidgetStatus::Hover);
    }
}

void HorIconTextVectorButton::leaveEvent(QEvent *event) {
    QPushButton::leaveEvent(event);
    if (m_eState != style::WidgetStatus::Checked && m_eState != style::WidgetStatus::Disabled) {
        updateWidgetStatus(style::WidgetStatus::Normal);
    }
}

void HorIconTextVectorButton::changeEvent(QEvent *event) {
    QPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        updateWidgetStatus(isEnabled() ? style::WidgetStatus::Normal : style::WidgetStatus::Disabled);
    }
}

void HorIconTextVectorButton::resizeEvent(QResizeEvent *event) {
    QPushButton::resizeEvent(event);
    if (m_bAdjustWidth) {
        int textWidth = m_pText->fontMetrics().horizontalAdvance(m_pText->text());
        int w = textWidth + m_nLeftRightSpacing * 2 + m_nIconSize + m_nIconTextSpacing;
        int h = height();
        setFixedSize(w, h);
    }
}
// 在QHBoxLayout或QVBoxLayout布局中的所有该自定义控件如果没有设置大小，所有的自定义控件宽或高默认统一和最大的宽或高的那个一样
// 为了在布局内的该自定义控件能准确的设置固定宽度需要resizeEvent和sizeHint都要处理，否则会有点小问题！
// 需要先 setAdjustWidth(false); 再setFixedSize(); 才能设置固定宽度
QSize HorIconTextVectorButton::sizeHint() const {
    if (m_bAdjustWidth) {
        int textWidth = m_pText->fontMetrics().horizontalAdvance(m_pText->text());
        int w = textWidth + m_nLeftRightSpacing * 2 + m_nIconSize + m_nIconTextSpacing;
        int h = height();
        return QSize(w, h);
    }
    return QPushButton::sizeHint();
}

void HorIconTextVectorButton::updateWidgetStatus(style::WidgetStatus state) {
    m_eState = state;
    m_pIcon->setProperty(style::kWidgetStateProperty, style::widgetStatusToString(m_eState));
    m_pText->setProperty(style::kWidgetStateProperty, style::widgetStatusToString(m_eState));
    m_pIcon->setStyle(m_pIcon->style());
    m_pText->setStyle(m_pText->style());
}

void HorIconTextVectorButton::slotToggled(bool checked) {
    if (m_eState != style::WidgetStatus::Disabled) {
        updateWidgetStatus(checked ? style::WidgetStatus::Checked : style::WidgetStatus::Normal);
    }
}
} // namespace widget