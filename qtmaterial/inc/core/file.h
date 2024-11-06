#pragma once
#include "qtmaterial_global.h"
#include <QFileInfo>

namespace core {
class QTMATERIAL_EXPORT File {
public:
    static bool fileExists(const QString filePath);
    static QFileInfo fileInfo(const QString filePath);
    static void renameIfExists(QString &filePath);
    static QString joinPathAndFileName(const QString path, const QString fileName);
};
} // namespace core