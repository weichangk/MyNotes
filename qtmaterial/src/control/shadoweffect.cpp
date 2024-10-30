#include "control/shadoweffect.h"
#include <QDialog>
#include <QEvent>
#include <QTimer>
#include <QApplication>
#include <QPainter>

ShadowEffect::ShadowEffect(QWidget *parent) :
    QObject(nullptr),
    m_parentWidget(parent) {
    m_pixmap = QPixmap(":/res/image/shadow.png");
    setParent(m_parentWidget);
    m_parentWidget->installEventFilter(this);
    if (auto dlg = qobject_cast<QDialog *>(m_parentWidget)) {
        connect(dlg, &QDialog::finished, this, [=](int result) {
            Q_UNUSED(result);
            m_shadowWidget->close();
        });
    }
    m_shadowWidget = new QWidget(m_parentWidget);
    m_shadowWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_shadowWidget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    m_shadowWidget->setAttribute(Qt::WA_StyledBackground);
    m_shadowWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_shadowWidget->setAttribute(Qt::WA_InputMethodTransparent);
    m_shadowWidget->installEventFilter(this);
}

ShadowEffect::~ShadowEffect() {
}

bool ShadowEffect::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_parentWidget) {
        auto ResizeEffectFun = [&]() {
            QRect rc = m_parentWidget->geometry();
            rc.adjust(-30, -30, 30, 30);
            m_shadowWidget->setGeometry(rc);
            m_shadowWidget->setFixedSize(rc.size());
            m_maskPixmap = ninePatchPixmap(m_pixmap, 40, 40, rc.width(), rc.height());
        };
        if (event->type() == QEvent::Move) {
            ResizeEffectFun();
            // qt6.5版本，移动到分屏，阴影大小正确更新，需要update！1.15.2就不需要
            // QTimer::singleShot(0, this, [=]() {
            m_shadowWidget->update();
            // });
        } else if (event->type() == QEvent::Show) {
            QTimer::singleShot(50, this, [=]() {
                m_shadowWidget->show();
                if (QApplication::screens().size() > 1) {
                    QTimer::singleShot(50, this, [=]() {
                        ResizeEffectFun();
                    });
                }
            });

        } else if (event->type() == QEvent::Hide) {
            m_shadowWidget->hide();
        } else if (event->type() == QEvent::Close) {
            m_shadowWidget->close();
        } else if (event->type() == QEvent::Resize) {
            ResizeEffectFun();
        } else if (event->type() == QEvent::WindowActivate) {
            QTimer::singleShot(0, this, [=]() {
                m_shadowWidget->update();
            });
        }
    } else if (watched == m_shadowWidget) {
        if (event->type() == QEvent::Paint) {
            if (m_parentWidget->isVisible() && !m_parentWidget->isMinimized()) {
                QPainter painter(m_shadowWidget);
                painter.drawPixmap(m_shadowWidget->rect(), m_maskPixmap);
            }
        }
    }
    return false;
}

QPixmap ShadowEffect::ninePatchPixmap(const QPixmap &srcPixmap, int horzSplit, int vertSplit, int dstWidth, int dstHeight) {
    if (srcPixmap.isNull())
        return QPixmap();

    const QPixmap *pix = &srcPixmap;
    int pixWidth = pix->width();
    int pixHeight = pix->height();

    QPixmap pix_1 = pix->copy(0, 0, horzSplit, vertSplit);
    QPixmap pix_2 = pix->copy(horzSplit, 0, pixWidth - horzSplit * 2, vertSplit);
    QPixmap pix_3 = pix->copy(pixWidth - horzSplit, 0, horzSplit, vertSplit);

    QPixmap pix_4 = pix->copy(0, vertSplit, horzSplit, pixHeight - vertSplit * 2);
    QPixmap pix_5 = pix->copy(horzSplit, vertSplit, pixWidth - horzSplit * 2, pixHeight - vertSplit * 2);
    QPixmap pix_6 = pix->copy(pixWidth - horzSplit, vertSplit, horzSplit, pixHeight - vertSplit * 2);

    QPixmap pix_7 = pix->copy(0, pixHeight - vertSplit, horzSplit, vertSplit);
    QPixmap pix_8 = pix->copy(horzSplit, pixHeight - vertSplit, pixWidth - horzSplit * 2, pixWidth - horzSplit * 2);
    QPixmap pix_9 = pix->copy(pixWidth - horzSplit, pixHeight - vertSplit, horzSplit, vertSplit);

    pix_2 = pix_2.scaled(dstWidth - horzSplit * 2, vertSplit, Qt::IgnoreAspectRatio);
    pix_4 = pix_4.scaled(horzSplit, dstHeight - vertSplit * 2, Qt::IgnoreAspectRatio);
    pix_5 = pix_5.scaled(dstWidth - horzSplit * 2, dstHeight - vertSplit * 2, Qt::IgnoreAspectRatio);
    pix_6 = pix_6.scaled(horzSplit, dstHeight - vertSplit * 2, Qt::IgnoreAspectRatio);
    pix_8 = pix_8.scaled(dstWidth - horzSplit * 2, vertSplit);

    QPixmap resultImg(dstWidth, dstHeight);
    resultImg.fill(Qt::transparent);
    {
        QPainter painter(&resultImg);
        painter.drawPixmap(0, 0, pix_1);
        painter.drawPixmap(horzSplit, 0, pix_2);
        painter.drawPixmap(dstWidth - horzSplit, 0, pix_3);

        painter.drawPixmap(0, vertSplit, pix_4);
        painter.drawPixmap(horzSplit, vertSplit, pix_5);
        painter.drawPixmap(dstWidth - horzSplit, vertSplit, pix_6);

        painter.drawPixmap(0, dstHeight - vertSplit, pix_7);
        painter.drawPixmap(horzSplit, dstHeight - vertSplit, pix_8);
        painter.drawPixmap(dstWidth - horzSplit, dstHeight - vertSplit, pix_9);
    }
    return resultImg;
}
