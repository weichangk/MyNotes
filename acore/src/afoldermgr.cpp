/*
 * @Author: weick
 * @Date: 2024-01-19 23:16:30
 * @Last Modified by: weick
 * @Last Modified time: 2024-01-19 23:18:16
 */

#include "inc/afoldermgr.h"
#include <QDir>

bool AFolderMgr::addFolder(const QString &folderPath) {
    if (!folderExists(folderPath)) {
        QDir dir;
        if (dir.mkdir(folderPath)) {
            return true;
        }
    }
    return false;
}

bool AFolderMgr::removeFolder(const QString &folderPath) {
    if (folderExists(folderPath)) {
        QDir dir(folderPath);
        if (dir.rmpath(folderPath)) {
            return true;
        }
    }
    return false;
}

bool AFolderMgr::folderExists(const QString &folderPath) {
    QDir dir(folderPath);
    return dir.exists();
}
