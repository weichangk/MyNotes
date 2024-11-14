#include "widget/button.h"
#include <QPainter>
#include <QVariant>
#include <QEvent>

namespace widget {
IconButton::IconButton(QWidget *parent) :
    QPushButton(parent) {
    QPushButton::setObjectName("IconButton");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_pIcon = new QLabel(this);
    m_pIcon->setObjectName("IconButton_Icon");
    m_pIcon->setFixedSize(m_nIconSize, m_nIconSize);

    layout->addWidget(m_pIcon, 0, Qt::AlignCenter);
}

void IconButton::setObjectName(const QString &name) {
    QPushButton::setObjectName(name);
    if (m_pIcon) {
        m_pIcon->setObjectName(name + "_Icon");
    }
}

void IconButton::setIconSize(int n) {
    m_nIconSize = n;
    m_pIcon->setFixedSize(m_nIconSize, m_nIconSize);
}

int IconButton::iconSize() const {
    return m_nIconSize;
}

void IconButton::setFivePixmapPath(const QString &path) {
    m_strFivePath = path;
    m_pixmapFive = QPixmap(path);
    if(!m_pixmapFive.isNull()) {
        m_pixmapNormal = m_pixmapFive.copy(0, 0, m_pixmapFive.width() / 5, m_pixmapFive.height());
        m_pixmapHover = m_pixmapFive.copy(m_pixmapFive.width() / 5, 0, m_pixmapFive.width() / 5, m_pixmapFive.height());
        m_pixmapPressed = m_pixmapFive.copy(m_pixmapFive.width() / 5 * 2, 0, m_pixmapFive.width() / 5, m_pixmapFive.height());
        m_pixmapChecked = m_pixmapFive.copy(m_pixmapFive.width() / 5 * 3, 0, m_pixmapFive.width() / 5, m_pixmapFive.height());
        m_pixmapDisabled = m_pixmapFive.copy(m_pixmapFive.width() / 5 * 4, 0, m_pixmapFive.width() / 5, m_pixmapFive.height());
    }
}

QString IconButton::fivePixmapPath() const {
    return m_strFivePath;
}

void IconButton::setNormalPixmapPath(const QString &path) {
    m_strNormalPath = path;
    m_pixmapNormal = QPixmap(path);
}

QString IconButton::normalPixmapPath() const {
    return m_strNormalPath;
}

void IconButton::setHoverPixmapPath(const QString &path) {
    m_strHoverPath = path;
    m_pixmapHover = QPixmap(path);
}

QString IconButton::hoverPixmapPath() const {
    return m_strHoverPath;
}

void IconButton::setPressedPixmapPath(const QString &path) {
    m_strPressedPath = path;
    m_pixmapPressed = QPixmap(path);
}

QString IconButton::pressedPixmapPath() const {
    return m_strPressedPath;
}

void IconButton::setCheckedPixmapPath(const QString &path) {
    m_strCheckedPath = path;
    m_pixmapChecked = QPixmap(path);
}

QString IconButton::checkedPixmapPath() const {
    return m_strCheckedPath;
}

void IconButton::setDisablePixmapPath(const QString &path) {
    m_strDisabledPath = path;
    m_pixmapDisabled = QPixmap(path);
}

QString IconButton::disablePixmapPath() const {
    return m_strDisabledPath;
}

void IconButton::resizeEvent(QResizeEvent *event) {
    QPushButton::resizeEvent(event);
    updateWidgetStatus(isEnabled() ? isChecked() ? style::WidgetStatus::Checked : style::WidgetStatus::Normal : style::WidgetStatus::Disabled);
}

void IconButton::mousePressEvent(QMouseEvent *event) {
    QPushButton::mousePressEvent(event);
    updateWidgetStatus(style::WidgetStatus::Pressed);
}

void IconButton::mouseReleaseEvent(QMouseEvent *event) {
    QPushButton::mouseReleaseEvent(event);
    updateWidgetStatus(style::WidgetStatus::Normal);
}

void IconButton::enterEvent(QEvent *event) {
    QPushButton::enterEvent(event);
    updateWidgetStatus(style::WidgetStatus::Hover);
}

void IconButton::leaveEvent(QEvent *event) {
    QPushButton::leaveEvent(event);
    updateWidgetStatus(style::WidgetStatus::Normal);
}

void IconButton::changeEvent(QEvent *event) {
    QPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        updateWidgetStatus(isEnabled() ? style::WidgetStatus::Normal : style::WidgetStatus::Disabled);
    }
}

void IconButton::updateWidgetStatus(style::WidgetStatus state) {
    m_eState = m_eState == style::WidgetStatus::Disabled ? style::WidgetStatus::Disabled : state;
    QPixmap pixmap = getCurrentPixmap();
    if (!pixmap.isNull()) {
        QSize iconSize = QSize(m_nIconSize, m_nIconSize);
        m_pIcon->setPixmap(pixmap.scaled(iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
}

QPixmap IconButton::getCurrentPixmap() const {
    switch (m_eState) {
    case style::WidgetStatus::Hover: return m_pixmapHover.isNull() ? m_pixmapNormal : m_pixmapHover;
    case style::WidgetStatus::Pressed: return m_pixmapPressed.isNull() ? m_pixmapNormal : m_pixmapPressed;
    case style::WidgetStatus::Checked: return m_pixmapChecked.isNull() ? m_pixmapNormal : m_pixmapChecked;
    case style::WidgetStatus::Disabled: return m_pixmapDisabled.isNull() ? m_pixmapNormal : m_pixmapDisabled;
    default: return m_pixmapNormal;
    }
}

HorIconTextButton::HorIconTextButton(QWidget *parent) :
    QPushButton(parent) {
    QPushButton::setObjectName("HorIconTextButton");
    m_pLayout = new QHBoxLayout(this);
    m_pLayout->setContentsMargins(m_nLeftRightSpacing, 0, m_nLeftRightSpacing, 0);
    m_pLayout->setSpacing(m_nIconTextSpacing);

    m_pIcon = new QLabel(this);
    m_pIcon->setObjectName("HorIconTextButton_Icon");
    m_pIcon->setFixedSize(m_nIconSize, m_nIconSize);

    m_pText = new QLabel(this);
    m_pText->setObjectName("HorIconTextButton_Text");
    m_pText->setProperty("ButtonStatus", "normal");

    m_pLayout->addWidget(m_pIcon, 0, Qt::AlignCenter);
    m_pLayout->addWidget(m_pText, 0, Qt::AlignCenter);
    m_pLayout->addStretch();
}

void HorIconTextButton::setObjectName(const QString &name) {
    QPushButton::setObjectName(name);
    if (m_pIcon) {
        m_pIcon->setObjectName(name + "_Icon");
    }
    if (m_pText) {
        m_pText->setObjectName(name + "_Text");
    }
}

void HorIconTextButton::setEnabled(bool b) {
    QPushButton::setEnabled(b);
    updateWidgetStatus(style::WidgetStatus::Disabled);
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
    m_pIcon->setFixedSize(m_nIconSize, m_nIconSize);
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

void HorIconTextButton::setAdjustWidth(bool b) {
    m_bAdjustWidth = b;
}
bool HorIconTextButton::adjustWidth() const {
    return m_bAdjustWidth;
}

void HorIconTextButton::setFivePixmapPath(const QString &path) {
    m_strFivePath = path;
    m_pixmapFive = QPixmap(path);
    if(!m_pixmapFive.isNull()) {
        m_pixmapNormal = m_pixmapFive.copy(0, 0, m_pixmapFive.width() / 5, m_pixmapFive.height());
        m_pixmapHover = m_pixmapFive.copy(m_pixmapFive.width() / 5, 0, m_pixmapFive.width() / 5, m_pixmapFive.height());
        m_pixmapPressed = m_pixmapFive.copy(m_pixmapFive.width() / 5 * 2, 0, m_pixmapFive.width() / 5, m_pixmapFive.height());
        m_pixmapChecked = m_pixmapFive.copy(m_pixmapFive.width() / 5 * 3, 0, m_pixmapFive.width() / 5, m_pixmapFive.height());
        m_pixmapDisabled = m_pixmapFive.copy(m_pixmapFive.width() / 5 * 4, 0, m_pixmapFive.width() / 5, m_pixmapFive.height());
    }
}

QString HorIconTextButton::fivePixmapPath() const {
    return m_strFivePath;
}

void HorIconTextButton::setNormalPixmapPath(const QString &path) {
    m_strNormalPath = path;
    m_pixmapNormal = QPixmap(path);
}

QString HorIconTextButton::normalPixmapPath() const {
    return m_strNormalPath;
}

void HorIconTextButton::setHoverPixmapPath(const QString &path) {
    m_strHoverPath = path;
    m_pixmapHover = QPixmap(path);
}

QString HorIconTextButton::hoverPixmapPath() const {
    return m_strHoverPath;
}

void HorIconTextButton::setPressedPixmapPath(const QString &path) {
    m_strPressedPath = path;
    m_pixmapPressed = QPixmap(path);
}

QString HorIconTextButton::pressedPixmapPath() const {
    return m_strPressedPath;
}

void HorIconTextButton::setCheckedPixmapPath(const QString &path) {
    m_strCheckedPath = path;
    m_pixmapChecked = QPixmap(path);
}

QString HorIconTextButton::checkedPixmapPath() const {
    return m_strCheckedPath;
}

void HorIconTextButton::setDisablePixmapPath(const QString &path) {
    m_strDisabledPath = path;
    m_pixmapDisabled = QPixmap(path);
}

QString HorIconTextButton::disablePixmapPath() const {
    return m_strDisabledPath;
}

void HorIconTextButton::resizeEvent(QResizeEvent *event) {
    QPushButton::resizeEvent(event);
    if (m_bAdjustWidth) {
        int textWidth = m_pText->fontMetrics().horizontalAdvance(m_pText->text());
        int w = textWidth + m_nLeftRightSpacing * 2 + m_nIconSize + m_nIconTextSpacing;
        int h = height();
        setFixedSize(w, h);
    }
    updateWidgetStatus(isEnabled() ? style::WidgetStatus::Normal : style::WidgetStatus::Disabled);
}

void HorIconTextButton::mousePressEvent(QMouseEvent *event) {
    QPushButton::mousePressEvent(event);
    updateWidgetStatus(style::WidgetStatus::Pressed);
}

void HorIconTextButton::mouseReleaseEvent(QMouseEvent *event) {
    QPushButton::mouseReleaseEvent(event);
    updateWidgetStatus(style::WidgetStatus::Normal);
}

void HorIconTextButton::enterEvent(QEvent *event) {
    QPushButton::enterEvent(event);
    updateWidgetStatus(style::WidgetStatus::Hover);
}

void HorIconTextButton::leaveEvent(QEvent *event) {
    QPushButton::leaveEvent(event);
    updateWidgetStatus(style::WidgetStatus::Normal);
}

void HorIconTextButton::changeEvent(QEvent *event) {
    QPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        updateWidgetStatus(isEnabled() ? style::WidgetStatus::Normal : style::WidgetStatus::Disabled);
    }
}

QSize HorIconTextButton::sizeHint() const {
    if (m_bAdjustWidth) {
        int textWidth = m_pText->fontMetrics().horizontalAdvance(m_pText->text());
        int w = textWidth + m_nLeftRightSpacing * 2 + m_nIconSize + m_nIconTextSpacing;
        int h = height();
        return QSize(w, h);
    }
    return QPushButton::sizeHint();
}

void HorIconTextButton::updateWidgetStatus(style::WidgetStatus state) {
    m_eState = m_eState == style::WidgetStatus::Disabled ? style::WidgetStatus::Disabled : state;
    QPixmap pixmap = getCurrentPixmap();
    if (!pixmap.isNull()) {
        QSize iconSize = QSize(m_nIconSize, m_nIconSize);
        m_pIcon->setPixmap(pixmap.scaled(iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    m_pText->setProperty(style::kWidgetStateProperty, style::widgetStatusToString(m_eState));
    m_pText->setStyle(m_pText->style());
}

QPixmap HorIconTextButton::getCurrentPixmap() const {
    switch (m_eState) {
    case style::WidgetStatus::Hover: return m_pixmapHover.isNull() ? m_pixmapNormal : m_pixmapHover;
    case style::WidgetStatus::Pressed: return m_pixmapPressed.isNull() ? m_pixmapNormal : m_pixmapPressed;
    case style::WidgetStatus::Checked: return m_pixmapChecked.isNull() ? m_pixmapNormal : m_pixmapChecked;
    case style::WidgetStatus::Disabled: return m_pixmapDisabled.isNull() ? m_pixmapNormal : m_pixmapDisabled;
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
    updateWidgetStatus(isEnabled() ? isChecked() ? style::WidgetStatus::Checked : style::WidgetStatus::Normal : style::WidgetStatus::Disabled);
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