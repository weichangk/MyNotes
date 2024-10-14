#pragma once
#include "qtmaterial_global.h"
#include <QTextEdit>

class QTMATERIAL_EXPORT TextEdit : public QTextEdit {
    Q_OBJECT
public:
    TextEdit(QWidget *parent = nullptr);
    ~TextEdit();
    void setUseCustomTextCursor(bool b);
    void setCustomTextCursorColor(QColor color);

protected:
    virtual void timerEvent(QTimerEvent *e) override;
    virtual void paintEvent(QPaintEvent *e) override;
    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *e) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool m_useCustomTextCursor = false;
    QColor m_customTextCursorOriginalColor = Qt::transparent;
    QColor m_customTextCursorFlickerColor = Qt::black;
    int m_customTextCursorTimerId;
};
