#include "core/folder.h"
#include <QDir>

namespace core {
bool Folder::addFolder(const QString &folderPath) {
    if (!folderExists(folderPath)) {
        QDir dir;
        if (dir.mkdir(folderPath)) {
            return true;
        }
    }
    return false;
}

bool Folder::removeFolder(const QString &folderPath) {
    if (folderExists(folderPath)) {
        QDir dir(folderPath);
        if (dir.rmpath(folderPath)) {
            return true;
        }
    }
    return false;
}

bool Folder::folderExists(const QString &folderPath) {
    QDir dir(folderPath);
    return dir.exists();
}
} // namespace core
