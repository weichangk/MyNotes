#pragma once
#include "inc/qtmaterial_global.h"

class QTMATERIAL_EXPORT AppPathHelper {
public:
    static void setAppName(const QString &name);
    static void setLogName(const QString &name);
    
    static QString appDirPath();
    static QString appLogPath();
    static QString appProgramDataPath();
};
