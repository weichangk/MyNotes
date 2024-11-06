#pragma once
#include "qtmaterial_global.h"

namespace core {
class QTMATERIAL_EXPORT Folder {
public:
    static bool addFolder(const QString &folderPath);
    static bool removeFolder(const QString &folderPath);
    static bool folderExists(const QString &folderPath);
};
} // namespace core
