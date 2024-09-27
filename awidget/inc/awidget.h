/*
 * @Author: weick
 * @Date: 2023-12-05 22:58:44
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:58:44
 */

#pragma once
#include "awidget_global.h"

class AWIDGET_EXPORT AWidget : public QWidget {
    Q_OBJECT
public:
    AWidget(QWidget *parent = nullptr);
    ~AWidget();
};
