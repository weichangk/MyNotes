#include "widget/tooltip.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QFontMetrics>
#include <QApplication>
#include <QScreen>

namespace widget {
static ToolTip *m_ins;
ToolTip *ToolTip::getInstance() {
    if (m_ins == 0) {
        m_ins = new ToolTip();
    }
    return m_ins;
}

void ToolTip::release() {
    if (m_ins != 0) {
        delete m_ins;
        m_ins = 0;
    }
}

void ToolTip::showText(const QString &text, QPoint pos) {
    hideText();
    if ("" != text) {
        show();
        m_text->setText(text);
        m_text->adjustSize();
        m_frame->setFixedWidth(m_text->width());
        QPoint p = pos; // QCursor::pos();
        QSize screenSize = QApplication::primaryScreen()->size();
        if (p.rx() + m_frame->width() + 16 > screenSize.width()) {
            p = p - QPoint(m_frame->width() + 16, 0);
        }
        if (p.ry() + m_frame->height() + 16 > screenSize.height()) {
            p = p - QPoint(0, m_frame->height() + 16);
        }
        move(p - QPoint(6, 6));
    }
}

void ToolTip::hideText() {
    hide();
}

void ToolTip::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
}

ToolTip::ToolTip() {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);

    m_frame = new QFrame(this);
    m_frame->setObjectName("ToolTip_frame");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(0);
    layout->addWidget(m_frame);
    layout->addStretch();

    m_text = new QLabel(m_frame);
    m_text->setObjectName("ToolTip_text");

    auto framelayout = new QHBoxLayout(m_frame);
    framelayout->setContentsMargins(0, 0, 0, 0);
    framelayout->setSpacing(0);
    framelayout->addWidget(m_text);
    framelayout->addStretch();

    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setOffset(0, 0);
    effect->setColor(QColor(0, 0, 0, 25));
    effect->setBlurRadius(16);
    m_frame->setGraphicsEffect(effect);
}
} // namespace widget