#pragma once
#include "qtmaterial_global.h"
#include <QString>
#include <QMetaEnum>

class QTMATERIAL_EXPORT StringHelper {
public:
    template <typename T>
    static QString QtEnumToQString(const T value) {
        return QString(QMetaEnum::fromType<T>().valueToKey(value));
    }
};
