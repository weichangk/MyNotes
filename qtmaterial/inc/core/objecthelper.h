#pragma once
#include "qtmaterial_global.h"

inline void blockSignalsFunc(QObject *obj, std::function<void()> func) {
    obj->blockSignals(true);
    func();
    obj->blockSignals(false);
}

class QTMATERIAL_EXPORT ObjectHelper
{
public:
};
