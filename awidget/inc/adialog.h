/*
 * @Author: weick 
 * @Date: 2024-07-05 07:36:18 
 * @Last Modified by: weick
 * @Last Modified time: 2024-07-05 07:39:37
 */

#pragma once
#include "awidget_global.h"
#include <QDialog>

class AWIDGET_EXPORT ADialog : public QDialog {
    Q_OBJECT
public:
    ADialog(QDialog* parent = nullptr);
    ~ADialog();
};