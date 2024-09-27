/*
 * @Author: weick
 * @Date: 2023-12-05 22:57:36
 * @Last Modified by: weick
 * @Last Modified time: 2024-01-14 22:08:26
 */

#pragma once
#include "awidget_global.h"
#include <QLabel>

class AWIDGET_EXPORT ALabel : public QLabel {
    Q_OBJECT
public:
    ALabel(QWidget *parent = nullptr);
    ~ALabel();
};
