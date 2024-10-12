#pragma once
#include "inc/qtmaterial_global.h"

class QTMATERIAL_EXPORT LogHelper {
public:
    static void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    static void writeLine(QString logFileName, QString logMessage);
    static void log(QString logFileName, QString logMessage);

private:
    static const bool m_bEnableTerminalLog = 1;
    static const bool m_bEnableFileLog = 1;
};
