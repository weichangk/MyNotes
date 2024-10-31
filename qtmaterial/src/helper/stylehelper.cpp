#include "helper/stylehelper.h"
#include <QDir>
#include <QApplication>
#include <QWidget>
#include <QDirIterator>
#include <QDebug>

void StyleHelper::setStyleToApp(const QString &qssFolder) {
    QString allStyle;
    QDirIterator it(qssFolder, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isFile()  && it.fileInfo().suffix() == "qss") {
            QString path = it.filePath();
            qDebug() << path;
            QFile file(path);
            file.open(QIODevice::ReadOnly);
            QString style = file.readAll();
            if (!style.isEmpty()) {
                allStyle += style;
            }
            file.close();
        }
    }
    qApp->setStyleSheet(allStyle);
}

void StyleHelper::setStyleToWidget(QWidget *widget, const QString &fileName) {
    QFile fileQss(fileName);
    fileQss.open(QFile::ReadOnly);
    widget->setStyleSheet(fileQss.readAll());
    fileQss.close();
}
