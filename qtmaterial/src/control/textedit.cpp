#include "control/textedit.h"
#include <QPainter>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QDebug>

TextEdit::TextEdit(QWidget *parent) :
    QTextEdit(parent) {
    installEventFilter(this);
}

TextEdit::~TextEdit() {
}

void TextEdit::timerEvent(QTimerEvent *e) {
    if (m_useCustomTextCursor && e->timerId() == m_customTextCursorTimerId) {
        if (m_customTextCursorOriginalColor != m_customTextCursorFlickerColor) {
            m_customTextCursorOriginalColor = m_customTextCursorFlickerColor;
        } else {
            m_customTextCursorOriginalColor = Qt::transparent;
        }
        update();
    } else {
        QTextEdit::timerEvent(e);
    }
}

void TextEdit::paintEvent(QPaintEvent *e) {
    QTextEdit::paintEvent(e);

    if (m_useCustomTextCursor) {
        setCursorWidth(0);
        QPainter painter(viewport());
        painter.setPen(m_customTextCursorOriginalColor);
        painter.drawRect(cursorRect());
    }
}

void TextEdit::focusInEvent(QFocusEvent *event) {
    QTextEdit::focusInEvent(event);

    if (m_useCustomTextCursor) {
        m_customTextCursorOriginalColor = m_customTextCursorFlickerColor;
        update();
        m_customTextCursorTimerId = startTimer(500);
    }
}

void TextEdit::focusOutEvent(QFocusEvent *e) {
    QTextEdit::focusOutEvent(e);

    if (m_useCustomTextCursor) {
        killTimer(m_customTextCursorTimerId);
        m_customTextCursorOriginalColor = Qt::transparent;
        update();
    }
}

void TextEdit::keyPressEvent(QKeyEvent *e) {
    QKeyEvent *key_event = static_cast<QKeyEvent *>(e);
    QString q_str = key_event->text();
    QTextEdit::keyPressEvent(e);
}

bool TextEdit::eventFilter(QObject *watched, QEvent *event) {
    if (QEvent::InputMethod == event->type()) {
        QInputMethodEvent *keyEvent = dynamic_cast<QInputMethodEvent *>(event);
        QString preeditString = keyEvent->preeditString();
        QString commitString = keyEvent->commitString();
        qDebug() << "preeditString:" << preeditString << " commitString:" << commitString;
    }
    return QTextEdit::eventFilter(watched, event);
}

void TextEdit::setUseCustomTextCursor(bool b) {
    m_useCustomTextCursor = b;
}

void TextEdit::setCustomTextCursorColor(QColor color) {
    m_customTextCursorFlickerColor = color;
}
