#include "core/apppath.h"
#include <QStandardPaths>
#include <QDir>

namespace core {
static QString app_name_ = "";
static QString log_name_ = "log";

void AppPath::setAppName(const QString &name) {
    app_name_ = name;
}

void AppPath::setLogName(const QString &name) {
    log_name_ = name;
}

QString AppPath::appDirPath() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return path;
}

QString AppPath::appLogPath() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    path = QDir(path).filePath(log_name_);
    QDir().mkpath(path);
    return path;
}

QString AppPath::appProgramDataPath() {
    Q_ASSERT(!app_name_.isEmpty());
    QString path = "C:/ProgramData"; // QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    path = QDir(path).filePath(app_name_);
    QDir().mkpath(path);
    return path;
}
} // namespace core