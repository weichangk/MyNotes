/*
 * @Author: weick
 * @Date: 2024-03-31 08:09:00
 * @Last Modified by: weick
 * @Last Modified time: 2024-03-31 08:10:40
 */

#pragma once
#include "awidget_global.h"
#include <QRadioButton>

class AWIDGET_EXPORT ARadioButton : public QRadioButton {
    Q_OBJECT
public:
    ARadioButton(QWidget *parent = nullptr);
    ~ARadioButton();
};