/*
 * @Author: weick
 * @Date: 2023-12-05 22:57:29
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:57:29
 */

#pragma once
#include "awidget_global.h"
#include <QHBoxLayout>

class AWIDGET_EXPORT AHBoxLayout : public QHBoxLayout {
    Q_OBJECT
public:
    AHBoxLayout(QWidget *parent = nullptr);
    ~AHBoxLayout();
};
