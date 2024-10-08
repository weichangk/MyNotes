/*
 * @Author: weick
 * @Date: 2023-12-05 22:59:48
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:59:48
 */

#include "inc/atextedit.h"
#include <QPainter>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QDebug>

ATextEdit::ATextEdit(QWidget *parent) :
    QTextEdit(parent) {
    installEventFilter(this);
}

ATextEdit::~ATextEdit() {
}

void ATextEdit::timerEvent(QTimerEvent *e) {
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

void ATextEdit::paintEvent(QPaintEvent *e) {
    QTextEdit::paintEvent(e);

    if (m_useCustomTextCursor) {
        setCursorWidth(0);
        QPainter painter(viewport());
        painter.setPen(m_customTextCursorOriginalColor);
        painter.drawRect(cursorRect());
    }
}

void ATextEdit::focusInEvent(QFocusEvent *event) {
    QTextEdit::focusInEvent(event);

    if (m_useCustomTextCursor) {
        m_customTextCursorOriginalColor = m_customTextCursorFlickerColor;
        update();
        m_customTextCursorTimerId = startTimer(500);
    }
}

void ATextEdit::focusOutEvent(QFocusEvent *e) {
    QTextEdit::focusOutEvent(e);

    if (m_useCustomTextCursor) {
        killTimer(m_customTextCursorTimerId);
        m_customTextCursorOriginalColor = Qt::transparent;
        update();
    }
}

void ATextEdit::keyPressEvent(QKeyEvent *e) {
    QKeyEvent *key_event = static_cast<QKeyEvent *>(e);
    QString q_str = key_event->text();
    QTextEdit::keyPressEvent(e);
}

bool ATextEdit::eventFilter(QObject *watched, QEvent *event) {
    if (QEvent::InputMethod == event->type()) {
        QInputMethodEvent *keyEvent = dynamic_cast<QInputMethodEvent *>(event);
        QString preeditString = keyEvent->preeditString();
        QString commitString = keyEvent->commitString();
        qDebug() << "preeditString:" << preeditString << " commitString:" << commitString;
    }
    return QTextEdit::eventFilter(watched, event);
}

void ATextEdit::setUseCustomTextCursor(bool b) {
    m_useCustomTextCursor = b;
}

void ATextEdit::setCustomTextCursorColor(QColor color) {
    m_customTextCursorFlickerColor = color;
}
