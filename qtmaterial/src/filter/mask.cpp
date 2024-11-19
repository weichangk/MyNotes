#include "filter/mask.h"
#include "core/define.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

namespace filter {
PopupMask::PopupMask(QWidget *watchedWidget, QWidget *maskWidget) :
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

PopupMask::~PopupMask() {
    SAFE_DELETE(m_pMask);
}

void PopupMask::setMaskColor(const QColor color) {
    m_colorMask = color;
}

QColor PopupMask::maskColor() const {
    return m_colorMask;
}

void PopupMask::setWatchedWidgetRadius(int n) {
    m_nWatchedWidgetRadius = n;
}

int PopupMask::watchedWidgetRadius() const {
    return m_nWatchedWidgetRadius;
}

void PopupMask::setMaskWidgetRadius(int n) {
    m_nMaskWidgetRadius = n;
}

int PopupMask::maskWidgetRadius() const {
    return m_nMaskWidgetRadius;
}

void PopupMask::setFullMask(bool b) {
    m_bFullMask = b;
}

bool PopupMask::fullMask() const {
    return m_bFullMask;
}

bool PopupMask::eventFilter(QObject *watched, QEvent *event) {
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
                painter.setRenderHint(QPainter::HighQualityAntialiasing);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
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

void PopupMask::updateMaskGeometry() {
    QRect rc(0, 0, 0, 0);
    if (m_bFullMask) {
        QScreen *screen = m_pWatchedWidget->screen();
        rc = screen->geometry();
    } else {
        rc = m_pMaskWidget->geometry();
    }
    m_pMask->setGeometry(rc);
}

RadiusMask::RadiusMask(QWidget *parent) :
    QObject(parent), m_pWatchedWidget(parent) {
    m_pWatchedWidget->installEventFilter(this);
}

void RadiusMask::setPramas(RoundType roundType, int maskMargin, int xRadius, int yRadius) {
    if (m_eRoundType == roundType && m_nMaskMargin == maskMargin && 
        m_nXRadius == xRadius && m_nYRadius == yRadius) {
        return;
    }

    m_eRoundType = roundType;
    m_nMaskMargin = maskMargin;
    m_nXRadius = xRadius;
    m_nYRadius = yRadius;
    updateMask();
}


void RadiusMask::updateMask() {
    auto mask = generateMask();
	m_pWatchedWidget->setMask(mask);
}

bool RadiusMask::eventFilter(QObject *watched, QEvent *event) {
    static QSize lastSize;
    if (watched == m_pWatchedWidget) {
        if (event->type() == QEvent::Resize) {
            QSize currentSize = m_pWatchedWidget->size();
            if (lastSize != currentSize) {
                updateMask();
                lastSize = currentSize;
            }
        }
    }
    return false;
}

QBitmap RadiusMask::generateMask() {
    int width = m_pWatchedWidget->width();
    int height = m_pWatchedWidget->height();

    QBitmap bitmap(width, height);
    bitmap.fill(Qt::color0);

    QPainter painter(&bitmap);
    painter.setBrush(Qt::color1); // 使用Qt::color1来绘制可见区域
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.drawRoundedRect(m_nMaskMargin, m_nMaskMargin,
                            width - 2 * m_nMaskMargin, height - 2 * m_nMaskMargin,
                            m_nXRadius, m_nYRadius);

    int temp = 0;
    switch (m_eRoundType) {
        case Round_Left:
            temp = width / 2;
            painter.drawRect(width - temp - m_nMaskMargin, m_nMaskMargin,
                             temp, height - 2 * m_nMaskMargin);
            break;
        case Round_Right:
            painter.drawRect(m_nMaskMargin, m_nMaskMargin,
                             width / 2, height - 2 * m_nMaskMargin);
            break;
        case Round_Top:
            temp = height / 2;
            painter.drawRect(m_nMaskMargin, height - temp - m_nMaskMargin,
                             width - 2 * m_nMaskMargin, temp);
            break;
        case Round_Bottom:
            painter.drawRect(m_nMaskMargin, m_nMaskMargin,
                             width - 2 * m_nMaskMargin, height / 2);
            break;
        case Round_Left_Bottom:
            temp = width / 2;
            painter.drawRect(width - temp - m_nMaskMargin, m_nMaskMargin,
                             temp, height - 2 * m_nMaskMargin);
            painter.drawRect(m_nMaskMargin, m_nMaskMargin,
                             width - 2 * m_nMaskMargin, height / 2);
            break;
        case Round_Right_Bottom:
            painter.drawRect(m_nMaskMargin, m_nMaskMargin,
                             width / 2, height - 2 * m_nMaskMargin);
            painter.drawRect(m_nMaskMargin, m_nMaskMargin,
                             width - 2 * m_nMaskMargin, height / 2);
            break;
        default:
            break;
    }

    return bitmap;
}

} // namespace filter