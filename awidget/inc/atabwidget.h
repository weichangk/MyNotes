/*
 * @Author: weick
 * @Date: 2024-03-30 08:41:42
 * @Last Modified by: weick
 * @Last Modified time: 2024-03-30 09:08:38
 */

#pragma once
#include "awidget_global.h"
#include <QTabWidget>

class AWIDGET_EXPORT ATabWidget : public QTabWidget {
    Q_OBJECT
public:
    ATabWidget(QWidget *parent = nullptr);
    ~ATabWidget();
};