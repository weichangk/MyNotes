/*
 * @Author: weick 
 * @Date: 2024-05-23 21:34:45 
 * @Last Modified by: weick
 * @Last Modified time: 2024-08-06 01:11:22
 */

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include "mainWindow.h"
#include "maindialog.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    MainDialog d;

    QWidget widget;
    widget.setFixedSize(300, 100);

    QHBoxLayout *layout = new QHBoxLayout(&widget);
    QPushButton *button1 = new QPushButton("Open Main Window");
    QPushButton *button2 = new QPushButton("Open Dialog");
    layout->addWidget(button1);
    layout->addWidget(button2);
    QObject::connect(button1, &QPushButton::clicked, &w, &MainWindow::show);
    QObject::connect(button2, &QPushButton::clicked, &d, &MainDialog::exec);

    widget.show();
    return a.exec();
}
