/*
 * @Author: weick
 * @Date: 2024-03-30 21:42:17
 * @Last Modified by: weick
 * @Last Modified time: 2024-03-30 22:15:56
 */

#pragma once
#include "awidget_global.h"
#include <QLineEdit>

class AWIDGET_EXPORT ALineEdit : public QLineEdit {
    Q_OBJECT
public:
    ALineEdit(QWidget *parent = nullptr);
    ~ALineEdit();

Q_SIGNALS:
    void sigFocusOut(const QString &);
    void sigEditingConfirm(const QString &);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
};