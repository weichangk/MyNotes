#pragma once
#include "qtmaterial_global.h"

class QTMATERIAL_EXPORT FolderHelper {
public:
    static bool addFolder(const QString &folderPath);
    static bool removeFolder(const QString &folderPath);
    static bool folderExists(const QString &folderPath);
};
