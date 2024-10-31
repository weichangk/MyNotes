#include "extension/toolext.h"
#include <QApplication>
#include <QMouseEvent>

ToolExt::ToolExt(QWidget *parent) :
    QObject(parent), m_pWatchedObj(parent) {
    setParent(m_pWatchedObj);
    m_pWatchedObj->setWindowFlags(m_pWatchedObj->windowFlags() | Qt::Tool);
    qApp->installEventFilter(this);
}

bool ToolExt::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e = (QMouseEvent *)event;
        auto pos = m_pWatchedObj->mapFromGlobal(e->globalPos());
        if (!m_pWatchedObj->rect().contains(pos)) {
            m_pWatchedObj->hide();
        }
    }
    return QObject::eventFilter(watched, event);
}