/*
 * @Author: weick
 * @Date: 2024-05-11 07:02:23
 * @Last Modified by: weick
 * @Last Modified time: 2024-05-11 07:18:04
 */

#include "inc/aapppath.h"
#include <QStandardPaths>
#include <QDir>

static QString app_name_ = "";
static QString log_name_ = "log";

void AAppPath::setAppName(const QString &name) {
    app_name_ = name;
}

void AAppPath::setLogName(const QString &name) {
    log_name_ = name;
}

QString AAppPath::appDirPath() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return path;
}

QString AAppPath::appLogPath() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    path = QDir(path).filePath(log_name_);
    QDir().mkpath(path);
    return path;
}

QString AAppPath::appProgramDataPath() {
    Q_ASSERT(!app_name_.isEmpty());
    QString path = "C:/ProgramData";// QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    path = QDir(path).filePath(app_name_);
    QDir().mkpath(path);
    return path;
}