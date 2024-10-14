#pragma once
#include "qtmaterial_global.h"
#include <QPainter>
#include <QRect>
#include <QPixmap>

class QTMATERIAL_EXPORT PainterHelper
{
public:
    static void paintPixmap(QPainter *painter, QRect rc, const QPixmap &pixmap, float devicPixelRatio, int radius, bool keepRadio);
};

