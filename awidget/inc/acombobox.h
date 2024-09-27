/*
 * @Author: weick
 * @Date: 2023-12-05 22:57:22
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:57:22
 */

#pragma once
#include "awidget_global.h"
#include <QComboBox>

class AWIDGET_EXPORT AComboBox : public QComboBox {
    Q_OBJECT
public:
    AComboBox(QWidget *parent = nullptr);
    ~AComboBox();
};
