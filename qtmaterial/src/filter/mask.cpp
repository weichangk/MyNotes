#include "filter/mask.h"
#include "core/define.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

namespace filter {
Mask::Mask(QWidget *watchedWidget, QWidget *maskWidget) :
    QObject(watchedWidget),
    m_pWatchedWidget(watchedWidget),
    m_pMaskWidget(maskWidget) {

    m_pWatchedWidget->installEventFilter(this);
    m_pMaskWidget->installEventFilter(this);

    // m_pMask = new QWidget(m_pWatchedWidget);
    m_pMask = new QWidget(maskWidget);
    m_pMask->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_pMask->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    m_pMask->setAttribute(Qt::WA_StyledBackground);
    m_pMask->setAttribute(Qt::WA_TranslucentBackground);
    m_pMask->setAttribute(Qt::WA_InputMethodTransparent);
    m_pMask->installEventFilter(this);
}

Mask::~Mask() {
    SAFE_DELETE(m_pMask);
}

void Mask::setMaskColor(const QColor color) {
    m_colorMask = color;
}

QColor Mask::maskColor() const {
    return m_colorMask;
}

void Mask::setWatchedWidgetRadius(int n) {
    m_nWatchedWidgetRadius = n;
}

int Mask::watchedWidgetRadius() const {
    return m_nWatchedWidgetRadius;
}

void Mask::setMaskWidgetRadius(int n) {
    m_nMaskWidgetRadius = n;
}

int Mask::maskWidgetRadius() const {
    return m_nMaskWidgetRadius;
}

void Mask::setFullMask(bool b) {
    m_bFullMask = b;
}

bool Mask::fullMask() const {
    return m_bFullMask;
}

bool Mask::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_pWatchedWidget) {
        if (event->type() == QEvent::Move) {
            updateMaskGeometry();
            m_pMask->update();
        } else if (event->type() == QEvent::Show) {
            m_pMask->show();
        } else if (event->type() == QEvent::Hide) {
            m_pMask->hide();
        } else if (event->type() == QEvent::Close) {
            m_pMask->close();
        } else if (event->type() == QEvent::Resize) {
            updateMaskGeometry();
        }else if (event->type() == QEvent::WindowStateChange) {
            if (m_pWatchedWidget->windowState() == Qt::WindowNoState 
                || m_pWatchedWidget->windowState() == Qt::WindowMaximized
                || m_pWatchedWidget->windowState() == Qt::WindowFullScreen)
            {
                m_pMask->show();
            }
        }
    } else if (watched == m_pMaskWidget) {
        if (event->type() == QEvent::Move) {
            updateMaskGeometry();
            m_pMask->update();
        } else if (event->type() == QEvent::Hide) {
            m_pMask->hide();
        } else if (event->type() == QEvent::Close) {
            m_pMask->close();
        } else if (event->type() == QEvent::Resize) {
            updateMaskGeometry();
        } else if (event->type() == QEvent::WindowStateChange) {
            if (m_pMaskWidget->windowState() == Qt::WindowNoState 
                || m_pMaskWidget->windowState() == Qt::WindowMaximized
                || m_pMaskWidget->windowState() == Qt::WindowFullScreen)
            {
                updateMaskGeometry();
                if (m_pWatchedWidget->isVisible()) {
                    m_pMask->show();
                }
            }
        }
    } else if (watched == m_pMask) {
        if (event->type() == QEvent::Paint) {
            if (m_pWatchedWidget->isVisible()) {
                QPainter painter(m_pMask);
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setPen(Qt::NoPen);
                painter.setBrush(m_colorMask);

                QPainterPath path;
                path.addRoundedRect(m_pMask->rect(), m_nMaskWidgetRadius, m_nMaskWidgetRadius);

                // QPainterPath watchedPath;
                // QPoint offsetPos = m_pWatchedWidget->mapToGlobal(QPoint(0, 0)) - m_pMask->mapToGlobal(QPoint(0, 0));
                // QRect hollowRect = QRect(offsetPos.x(), offsetPos.y(), m_pWatchedWidget->rect().width(), m_pWatchedWidget->rect().height());
                // watchedPath.addRoundedRect(hollowRect, m_nWatchedWidgetRadius, m_nWatchedWidgetRadius);

                // path = path.subtracted(watchedPath);
                painter.drawPath(path);
            }
        }
    }
    return false;
}

void Mask::updateMaskGeometry() {
    QRect rc(0, 0, 0, 0);
    if (m_bFullMask) {
        QScreen *screen = m_pWatchedWidget->screen();
        rc = screen->geometry();
    } else {
        rc = m_pMaskWidget->geometry();
    }
    m_pMask->setGeometry(rc);
}
} // namespace filter