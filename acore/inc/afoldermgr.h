/*
 * @Author: weick
 * @Date: 2024-01-19 23:12:36
 * @Last Modified by: weick
 * @Last Modified time: 2024-01-19 23:15:58
 */

#pragma once
#include "acore_global.h"

class ACORE_EXPORT AFolderMgr {
public:
    static bool addFolder(const QString &folderPath);
    static bool removeFolder(const QString &folderPath);
    static bool folderExists(const QString &folderPath);
};
