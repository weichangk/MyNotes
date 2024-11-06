#pragma once
#include "qtmaterial_global.h"
#include <QString>
#include <QMetaEnum>

namespace core {
class QTMATERIAL_EXPORT String {
public:
    template <typename T>
    static QString QtEnumToQString(const T value) {
        return QString(QMetaEnum::fromType<T>().valueToKey(value));
    }
};
} // namespace core
