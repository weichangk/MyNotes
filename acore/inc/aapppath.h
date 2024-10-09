/*
 * @Author: weick 
 * @Date: 2024-05-11 06:54:01 
 * @Last Modified by: weick
 * @Last Modified time: 2024-05-11 07:16:38
 */

#pragma once
#include "acore_global.h"

class ACORE_EXPORT AAppPath {
public:
    static void setAppName(const QString &name);
    static void setLogName(const QString &name);
    
    static QString appDirPath();
    static QString appLogPath();
    static QString appProgramDataPath();
};
