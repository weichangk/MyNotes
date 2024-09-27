/*
 * @Author: weick
 * @Date: 2024-03-30 08:40:47
 * @Last Modified by: weick
 * @Last Modified time: 2024-03-30 09:08:35
 */

#pragma once
#include "awidget_global.h"
#include <QTabBar>

class AWIDGET_EXPORT ATabBar : public QTabBar {
    Q_OBJECT
public:
    ATabBar(QWidget *parent = nullptr);
    ~ATabBar();
};