/*
 * @Author: weick
 * @Date: 2023-12-05 22:49:04
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:49:04
 */

#pragma once
#include "acore_global.h"

class ACORE_EXPORT AStyleMgr {
public:
    static void setStyleToApp(const QString &qssFolder);
    static void setStyleToWidget(QWidget *widget, const QString &fileName);
};
