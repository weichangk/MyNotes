#pragma once
#include "inc/qtmaterial_global.h"
#include <QLineEdit>

class QTMATERIAL_EXPORT LineEdit : public QLineEdit {
    Q_OBJECT
public:
    LineEdit(QWidget *parent = nullptr);
    ~LineEdit();

Q_SIGNALS:
    void sigFocusOut(const QString &);
    void sigEditingConfirm(const QString &);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
};