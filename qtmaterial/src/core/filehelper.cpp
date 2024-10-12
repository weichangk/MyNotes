#include "inc/core/filehelper.h"
#include <QFile>
#include <QDir>

bool FileHelper::fileExists(const QString filePath) {
    QFile file(filePath);
    return file.exists();
}

QFileInfo FileHelper::fileInfo(const QString filePath) {
    QFile file(filePath);
    QFileInfo fileInfo(file);
    return fileInfo;
}

void FileHelper::renameIfExists(QString &filePath) {
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

QString FileHelper::joinPathAndFileName(const QString path, const QString fileName) {
    QDir dir(path);
    return dir.filePath(fileName);
}
