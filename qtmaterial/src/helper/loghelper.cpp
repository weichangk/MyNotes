#include "helper/loghelper.h"
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QTextStream>

void LogHelper::outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QString contextInfo = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString currentDate = QString("(%1)").arg(currentDateTime);

    QString message;
    QString text;
    QString fileName;

    switch (type) {
    case QtDebugMsg:
        text = QString("Debug:");
        message = QString("%1 %2 %3 %4").arg(text).arg(contextInfo).arg(msg).arg(currentDate);
        fileName = "logDebug.txt";
        log(fileName, message);
        break;

    case QtInfoMsg:
        text = QString("Info:");
        message = QString("%1 %2 %3 %4").arg(text).arg(contextInfo).arg(msg).arg(currentDate);
        fileName = "logInfo.txt";
        log(fileName, message);
        break;

    case QtWarningMsg:
        text = QString("Warning:");
        message = QString("%1 %2 %3 %4").arg(text).arg(contextInfo).arg(msg).arg(currentDate);
        fileName = "logWarning.txt";
        log(fileName, message);
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        message = QString("%1 %2 %3 %4").arg(text).arg(contextInfo).arg(msg).arg(currentDate);
        fileName = "logCritical.txt";
        log(fileName, message);
        break;

    case QtFatalMsg:
        text = QString("Fatal:");
        message = QString("%1 %2 %3 %4").arg(text).arg(contextInfo).arg(msg).arg(currentDate);
        fileName = "logFatal.txt";
        log(fileName, message);
        break;
    }
}

void LogHelper::writeLine(QString logFileName, QString logMessage) {
    QFile file(logFileName);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&file);
    textStream << logMessage << "\r\n";
    file.flush();
    file.close();
}

void LogHelper::log(QString logFileName, QString logMessage) {
    static QMutex mutex;
    mutex.lock();

    if (m_bEnableTerminalLog) {
        QTextStream stream(stdout);
        stream << logMessage << "\r\n";
        stream.flush();
    }

    if (m_bEnableFileLog) {
        writeLine(logFileName, logMessage);
    }

    mutex.unlock();
}
