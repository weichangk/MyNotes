/*
 * @Author: weick
 * @Date: 2024-03-30 21:33:39
 * @Last Modified by: weick
 * @Last Modified time: 2024-03-30 22:16:01
 */

#pragma once
#include "awidget_global.h"
#include <QCheckBox>

class AWIDGET_EXPORT ACheckBox : public QCheckBox {
    Q_OBJECT

public:
    ACheckBox(QWidget *parent = nullptr);
    ~ACheckBox();
};
