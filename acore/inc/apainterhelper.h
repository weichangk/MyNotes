/*
 * @Author: weick
 * @Date: 2023-12-19 23:15:08
 * @Last Modified by: weick
 * @Last Modified time: 2023-12-19 23:24:39
 */

#pragma once
#include "acore_global.h"
#include <QPainter>
#include <QRect>
#include <QPixmap>

class ACORE_EXPORT APainterHelper
{
public:
    static void paintPixmap(QPainter *painter, QRect rc, const QPixmap &pixmap, float devicPixelRatio, int radius, bool keepRadio);
};

