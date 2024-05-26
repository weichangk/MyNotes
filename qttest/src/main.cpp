/*
 * @Author: weick 
 * @Date: 2024-05-23 21:34:45 
 * @Last Modified by: weick
 * @Last Modified time: 2024-05-26 08:51:53
 */

#include <QApplication>
#include "mainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
