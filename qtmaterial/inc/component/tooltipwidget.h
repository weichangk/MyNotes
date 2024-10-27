#pragma once
#include "qtmaterial_global.h"
#include <QWidget>

class QFrame;
class QLabel;
class QTMATERIAL_EXPORT ToolTipWidget : public QWidget {
    Q_OBJECT
public:
    static ToolTipWidget *getInstance();
    static void release();
    void showText(const QString &text, QPoint pos = QCursor::pos());
    void hideText();

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    ToolTipWidget();

public:
    QFrame *m_frame = nullptr;
    QLabel *m_text = nullptr;
};
