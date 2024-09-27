/*
 * @Author: weick
 * @Date: 2024-03-30 21:42:37
 * @Last Modified by: weick
 * @Last Modified time: 2024-03-30 21:44:15
 */

#include "inc/alineedit.h"
#include <QKeyEvent>

ALineEdit::ALineEdit(QWidget *parent) :
    QLineEdit(parent) {
}

ALineEdit::~ALineEdit() {
}

void ALineEdit::keyPressEvent(QKeyEvent *event) {
    QLineEdit::keyPressEvent(event); 
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        emit sigEditingConfirm(text());
    }
}

void ALineEdit::focusOutEvent(QFocusEvent *event) {
    QLineEdit::focusOutEvent(event);
    emit sigFocusOut(text());
    emit sigEditingConfirm(text());
}