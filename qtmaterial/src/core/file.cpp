#include "core/file.h"
#include <QFile>
#include <QDir>

namespace core {
bool File::fileExists(const QString filePath) {
    QFile file(filePath);
    return file.exists();
}

QFileInfo File::fileInfo(const QString filePath) {
    QFile file(filePath);
    QFileInfo fileInfo(file);
    return fileInfo;
}

void File::renameIfExists(QString &filePath) {
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

QString File::joinPathAndFileName(const QString path, const QString fileName) {
    QDir dir(path);
    return dir.filePath(fileName);
}
} // namespace core