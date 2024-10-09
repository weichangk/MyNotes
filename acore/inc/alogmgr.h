/*
 * @Author: weick
 * @Date: 2023-12-05 22:48:11
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:48:11
 */

#pragma once
#include "acore_global.h"

class ACORE_EXPORT ALogMgr {
public:
    static void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    static void writeLine(QString logFileName, QString logMessage);
    static void log(QString logFileName, QString logMessage);

private:
    static const bool m_bEnableTerminalLog = 1;
    static const bool m_bEnableFileLog = 1;
};
