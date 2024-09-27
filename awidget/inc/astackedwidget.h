/*
 * @Author: weick
 * @Date: 2024-04-16 07:44:03
 * @Last Modified by: weick
 * @Last Modified time: 2024-04-16 07:47:06
 */

#pragma once
#include "awidget_global.h"
#include <QStackedWidget>

class AWIDGET_EXPORT AStackedWidget : public QStackedWidget {
    Q_OBJECT
public:
    explicit AStackedWidget(QWidget *parent = nullptr);
    ~AStackedWidget();
};