#include "helper/apppathhelper.h"
#include <QStandardPaths>
#include <QDir>

static QString app_name_ = "";
static QString log_name_ = "log";

void AppPathHelper::setAppName(const QString &name) {
    app_name_ = name;
}

void AppPathHelper::setLogName(const QString &name) {
    log_name_ = name;
}

QString AppPathHelper::appDirPath() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return path;
}

QString AppPathHelper::appLogPath() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    path = QDir(path).filePath(log_name_);
    QDir().mkpath(path);
    return path;
}

QString AppPathHelper::appProgramDataPath() {
    Q_ASSERT(!app_name_.isEmpty());
    QString path = "C:/ProgramData";// QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    path = QDir(path).filePath(app_name_);
    QDir().mkpath(path);
    return path;
}