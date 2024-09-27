/*
 * @Author: weick
 * @Date: 2023-12-05 22:58:26
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:58:26
 */

#pragma once
#include "awidget_global.h"
#include <QVBoxLayout>

class AWIDGET_EXPORT AVBoxLayout : public QVBoxLayout {
    Q_OBJECT
public:
    AVBoxLayout(QWidget *parent = nullptr);
    ~AVBoxLayout();
};
