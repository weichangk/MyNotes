#pragma once
#include "inc/qtmaterial_global.h"

inline void blockSignalsFunc(QObject *obj, std::function<void()> func) {
    obj->blockSignals(true);
    func();
    obj->blockSignals(false);
}

class QTMATERIAL_EXPORT ObjectHelper
{
public:
};
