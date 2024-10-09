/*
 * @Author: weick
 * @Date: 2024-01-20 00:11:23
 * @Last Modified by: weick
 * @Last Modified time: 2024-01-20 00:13:24
 */


#pragma once
#include "acore_global.h"
#include <QFileInfo>

class ACORE_EXPORT AFileMgr {
public:
    static bool fileExists(const QString filePath);
    static QFileInfo fileInfo(const QString filePath);
    static void renameIfExists(QString &filePath);
    static QString joinPathAndFileName(const QString path, const QString fileName);
};
