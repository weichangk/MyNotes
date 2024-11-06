#pragma once
#include "qtmaterial_global.h"

namespace core {
inline void blockSignalsFunc(QObject *obj, std::function<void()> func) {
    obj->blockSignals(true);
    func();
    obj->blockSignals(false);
}

class QTMATERIAL_EXPORT Object {
public:
};
} // namespace core
