/*
 * @Author: weick
 * @Date: 2024-07-06 09:16:17
 * @Last Modified by: weick
 * @Last Modified time: 2024-07-06 14:47:19
 */

#pragma once
#include "awidget_global.h"
#include <QProgressBar>

class AWIDGET_EXPORT AProgressBar : public QProgressBar {
    Q_OBJECT
public:
    explicit AProgressBar(QWidget *parent = nullptr);
    ~AProgressBar();
};