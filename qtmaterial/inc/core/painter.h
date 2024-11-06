#pragma once
#include "qtmaterial_global.h"
#include <QPainter>
#include <QRect>
#include <QPixmap>

namespace core {
class QTMATERIAL_EXPORT Painter {
public:
    static void paintPixmap(QPainter *painter, QRect rc, const QPixmap &pixmap, float devicPixelRatio, int radius, bool keepRadio);
};
} // namespace core
