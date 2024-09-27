/*
 * @Author: weick
 * @Date: 2023-12-05 22:57:45
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:57:45
 */

#pragma once
#include "awidget_global.h"
#include <QPushButton>

class AWIDGET_EXPORT APushButton : public QPushButton {
    Q_OBJECT
public:
    APushButton(QWidget *parent = nullptr);
    ~APushButton();

protected:
    void paintEvent(QPaintEvent *event) override;
};
