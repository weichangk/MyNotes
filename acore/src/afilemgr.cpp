/*
 * @Author: weick
 * @Date: 2024-01-20 00:12:15
 * @Last Modified by: weick
 * @Last Modified time: 2024-01-20 00:13:42
 */

#include "inc/afilemgr.h"
#include <QFile>
#include <QDir>

bool AFileMgr::fileExists(const QString filePath) {
    QFile file(filePath);
    return file.exists();
}

QFileInfo AFileMgr::fileInfo(const QString filePath) {
    QFile file(filePath);
    QFileInfo fileInfo(file);
    return fileInfo;
}

void AFileMgr::renameIfExists(QString &filePath) {
    QFile file(filePath);
    QFileInfo fileInfo(file);
    if (file.exists()) {
        QString baseName = fileInfo.baseName();
        QString suffix = fileInfo.completeSuffix();
        QString newPath = filePath;
        int count = 1;
        while (file.exists()) {
            newPath = QString("%1 (%2).%3").arg(baseName).arg(count).arg(suffix);
            file.setFileName(newPath);
            count++;
        }
        filePath = newPath;
    }
}

QString AFileMgr::joinPathAndFileName(const QString path, const QString fileName) {
    QDir dir(path);
    return dir.filePath(fileName);
}
