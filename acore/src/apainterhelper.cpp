/*
 * @Author: weick
 * @Date: 2023-12-19 23:19:21
 * @Last Modified by: weick
 * @Last Modified time: 2023-12-19 23:32:36
 */

#include "inc/apainterhelper.h"
#include <QPainterPath>

void APainterHelper::paintPixmap(QPainter *painter, QRect rc, const QPixmap &pixmap, float devicPixelRatio, int radius, bool keepRadio) {
    if (pixmap.isNull()) {
        return;
    }
    painter->save();
    QSize pixmapSize = pixmap.size();
    QSize scaledSize = rc.size() * devicPixelRatio;
    QPixmap pixmapTemp;
    QRect drawRc = rc;
    if (keepRadio) {
        pixmapTemp = pixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        drawRc = QRect((rc.width() - pixmapTemp.width()) * 0.5 + rc.x(), (rc.height() - pixmapTemp.height()) * 0.5 + rc.y(), pixmapTemp.width(), pixmapTemp.height());
    } else {
        pixmapTemp = pixmap.scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    pixmapSize = pixmapTemp.size();
    pixmapTemp.setDevicePixelRatio(devicPixelRatio);
    if (radius > 0) {
        QPainterPath path;
        path.addRoundedRect(rc, radius, radius);
        painter->setClipPath(path);
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    }
    painter->drawPixmap(drawRc, pixmapTemp);
    painter->restore();
}
