/*
 * @Author: weick
 * @Date: 2024-03-30 22:02:29
 * @Last Modified by: weick
 * @Last Modified time: 2024-03-30 22:15:53
 */

#pragma once
#include "awidget_global.h"
#include <QSlider>

class AWIDGET_EXPORT ASlider : public QSlider {
    Q_OBJECT

public:
    explicit ASlider(QWidget *parent = nullptr);
    explicit ASlider(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~ASlider();
};