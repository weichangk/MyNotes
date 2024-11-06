#include "widget/icontext.h"
#include "core/string.h"
#include <QHBoxLayout>
#include <QMetaEnum>
#include <QMouseEvent>

namespace widget {
IconText::IconText(QWidget *parent) :
    QWidget(parent) {
    createUi();
}

IconText::~IconText() {
}

void IconText::setState(FourStateImage::StyleStatus state) {
    m_State = state;
    m_Icon->setState(state);
    Q_ASSERT(false);
    // QString stateStr = StringHelper::QtEnumToQString(state).toLower();
    // m_Text->setProperty("style-state", stateStr);
    m_Text->setStyle(m_Text->style());
}

void IconText::createUi() {
    setObjectName("IconText");
    auto layout = new QHBoxLayout(this);
    m_Icon = new FourStateImage(this);
    m_Icon->setObjectName("IconText_Icon");
    m_Icon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_Text = new QLabel(this);
    m_Text->setObjectName("IconText_Text");
    m_Text->setProperty("style-state", "normal");
    m_Text->setStyle(m_Text->style());
    layout->addWidget(m_Icon);
    layout->addSpacing(m_IconTextSpacing);
    layout->addWidget(m_Text);
    layout->addStretch();
}

void IconText::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (m_State != FourStateImage::StyleStatus::Checked) {
        m_Icon->setState(FourStateImage::StyleStatus::Pressed);
        m_Text->setProperty("style-state", "pressed");
        m_Text->setStyle(m_Text->style());
    }
    if (event->button() == Qt::LeftButton) {
        emit sigClicked();
    }
}

void IconText::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    if (m_State != FourStateImage::StyleStatus::Checked) {
        m_Icon->setState(FourStateImage::StyleStatus::Hover);
        m_Text->setProperty("style-state", "hover");
        m_Text->setStyle(m_Text->style());
    }
}

void IconText::enterEvent(QEvent *event) {
    QWidget::enterEvent(event);
    if (m_State != FourStateImage::StyleStatus::Checked) {
        m_Icon->setState(FourStateImage::StyleStatus::Hover);
        m_Text->setProperty("style-state", "hover");
        m_Text->setStyle(m_Text->style());
    }
}

void IconText::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    if (m_State != FourStateImage::StyleStatus::Checked) {
        m_Icon->setState(FourStateImage::StyleStatus::Normal);
        m_Text->setProperty("style-state", "normal");
        m_Text->setStyle(m_Text->style());
    }
}

void IconText::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        if (!isEnabled()) {
            m_State = FourStateImage::StyleStatus::Disabled;
            m_Icon->setState(FourStateImage::StyleStatus::Disabled);
            m_Text->setProperty("style-state", "disabled");
            m_Text->setStyle(m_Text->style());
        }
    }
}
} // namespace widget