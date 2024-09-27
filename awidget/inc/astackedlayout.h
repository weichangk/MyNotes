/*
 * @Author: weick
 * @Date: 2024-04-12 07:52:04
 * @Last Modified by: weick
 * @Last Modified time: 2024-04-12 07:55:59
 */

#pragma once
#include "awidget_global.h"
#include <QStackedLayout>

class AWIDGET_EXPORT AStackedLayout : public QStackedLayout {
    Q_OBJECT

public:
    explicit AStackedLayout(QWidget *parent = nullptr);
    explicit AStackedLayout(QLayout *parentLayout);
    ~AStackedLayout();
};