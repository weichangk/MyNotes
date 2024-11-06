#pragma once
#include "qtmaterial_global.h"
#include <QWidget>

class QFrame;
class QLabel;

namespace widget {
class QTMATERIAL_EXPORT ToolTip : public QWidget {
    Q_OBJECT
public:
    static ToolTip *getInstance();
    static void release();
    void showText(const QString &text, QPoint pos = QCursor::pos());
    void hideText();

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    ToolTip();

public:
    QFrame *m_frame = nullptr;
    QLabel *m_text = nullptr;
};
} // namespace widget