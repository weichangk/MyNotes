#include "component/icontextwidget.h"
#include "helper/stringhelper.h"
#include <QHBoxLayout>
#include <QMetaEnum>
#include <QMouseEvent>

IconTextWidget::IconTextWidget(QWidget *parent) :
    QWidget(parent) {
    createUi();
}

IconTextWidget::~IconTextWidget() {
}

void IconTextWidget::setState(FourStateImageWidget::StyleStatus state) {
    m_State = state;
    m_Icon->setState(state);
    Q_ASSERT(false);
    // QString stateStr = StringHelper::QtEnumToQString(state).toLower();
    // m_Text->setProperty("style-state", stateStr);
    m_Text->setStyle(m_Text->style());
}

void IconTextWidget::createUi() {
    setObjectName("IconTextWidget");
    auto layout = new QHBoxLayout(this);
    m_Icon = new FourStateImageWidget(this);
    m_Icon->setObjectName("IconTextWidget_Icon");
    m_Icon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_Text = new QLabel(this);
    m_Text->setObjectName("IconTextWidget_Text");
    m_Text->setProperty("style-state", "normal");
    m_Text->setStyle(m_Text->style());
    layout->addWidget(m_Icon);
    layout->addSpacing(m_IconTextSpacing);
    layout->addWidget(m_Text);
    layout->addStretch();
}

void IconTextWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (m_State != FourStateImageWidget::StyleStatus::Checked) {
        m_Icon->setState(FourStateImageWidget::StyleStatus::Pressed);
        m_Text->setProperty("style-state", "pressed");
        m_Text->setStyle(m_Text->style());
    }
    if (event->button() == Qt::LeftButton)
    {
        emit sigClicked();
    }
}

void IconTextWidget::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    if (m_State != FourStateImageWidget::StyleStatus::Checked) {
        m_Icon->setState(FourStateImageWidget::StyleStatus::Hover);
        m_Text->setProperty("style-state", "hover");
        m_Text->setStyle(m_Text->style());
    }
}

void IconTextWidget::enterEvent(QEvent *event) {
    QWidget::enterEvent(event);
    if (m_State != FourStateImageWidget::StyleStatus::Checked) {
        m_Icon->setState(FourStateImageWidget::StyleStatus::Hover);
        m_Text->setProperty("style-state", "hover");
        m_Text->setStyle(m_Text->style());
    }
}

void IconTextWidget::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    if (m_State != FourStateImageWidget::StyleStatus::Checked) {
        m_Icon->setState(FourStateImageWidget::StyleStatus::Normal);
        m_Text->setProperty("style-state", "normal");
        m_Text->setStyle(m_Text->style());
    }
}

void IconTextWidget::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        if (!isEnabled()) {
            m_State = FourStateImageWidget::StyleStatus::Disabled;
            m_Icon->setState(FourStateImageWidget::StyleStatus::Disabled);
            m_Text->setProperty("style-state", "disabled");
            m_Text->setStyle(m_Text->style());
        }
    }
}
