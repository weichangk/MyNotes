#include "helper/folderhelper.h"
#include <QDir>

bool FolderHelper::addFolder(const QString &folderPath) {
    if (!folderExists(folderPath)) {
        QDir dir;
        if (dir.mkdir(folderPath)) {
            return true;
        }
    }
    return false;
}

bool FolderHelper::removeFolder(const QString &folderPath) {
    if (folderExists(folderPath)) {
        QDir dir(folderPath);
        if (dir.rmpath(folderPath)) {
            return true;
        }
    }
    return false;
}

bool FolderHelper::folderExists(const QString &folderPath) {
    QDir dir(folderPath);
    return dir.exists();
}
