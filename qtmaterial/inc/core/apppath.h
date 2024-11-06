#pragma once
#include "qtmaterial_global.h"

namespace core {
class QTMATERIAL_EXPORT AppPath {
public:
    static void setAppName(const QString &name);
    static void setLogName(const QString &name);

    static QString appDirPath();
    static QString appLogPath();
    static QString appProgramDataPath();
};
} // namespace core
