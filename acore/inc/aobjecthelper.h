/*
 * @Author: weick 
 * @Date: 2024-07-18 07:30:34 
 * @Last Modified by: weick
 * @Last Modified time: 2024-07-18 07:37:51
 */

#pragma once
#include "acore_global.h"

inline void blockSignalsFunc(QObject *obj, std::function<void()> func) {
    obj->blockSignals(true);
    func();
    obj->blockSignals(false);
}

class ACORE_EXPORT AObjectHelper
{
public:
};
