/*
 * @Author: weick
 * @Date: 2023-12-05 22:48:54
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:48:54
 */

#pragma once
#include "acore_global.h"
#include <QString>
#include <QMetaEnum>

class ACORE_EXPORT AString {
public:
    template <typename T>
    static QString QtEnumToQString(const T value) {
        return QString(QMetaEnum::fromType<T>().valueToKey(value));
    }
};
