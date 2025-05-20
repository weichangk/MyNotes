#include "mainwindow.h"
#include "flowlayout.h"
#include "qtwidgets/qtwidgetsdialog.h"

#include <QPushbutton>
#include <QMap>
#include <QVariant>

using EQtStartID = enum {
    QtStart0 = 0,
    QtStart1,
    QtStart2,
    QtStart3,
    QtStart4,
    QtStart5,
    QtStart6,
    QtStart7,
    QtStart8,
    QtStart9,
    QtStart10,
    QtStart11,
    QtStart12,
};

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent) {
    createUi();
}

MainWindow::~MainWindow() {
}

void MainWindow::createUi() {
    setWindowTitle("Qt 快速入门");
    setFixedSize(500, 150);
    setQtStartBtns(this);
}

void MainWindow::setQtStartBtns(QWidget *w) {
    auto flowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> qtStartMap;
    qtStartMap.insert(EQtStartID::QtStart0, "Qt 窗口部件");
    qtStartMap.insert(EQtStartID::QtStart1, "Qt 布局管理");
    qtStartMap.insert(EQtStartID::QtStart2, "Qt 事件系统");
    qtStartMap.insert(EQtStartID::QtStart3, "Qt 对象模型");
    qtStartMap.insert(EQtStartID::QtStart4, "Qt 样式");
    qtStartMap.insert(EQtStartID::QtStart5, "Qt 绘图");
    qtStartMap.insert(EQtStartID::QtStart6, "Qt 视图模型");
    qtStartMap.insert(EQtStartID::QtStart7, "Qt 数据库");
    qtStartMap.insert(EQtStartID::QtStart8, "Qt XML JSON");
    qtStartMap.insert(EQtStartID::QtStart9, "Qt WebEngine");
    qtStartMap.insert(EQtStartID::QtStart10, "Qt 网络编程");
    qtStartMap.insert(EQtStartID::QtStart11, "Qt 进程线程");
    qtStartMap.insert(EQtStartID::QtStart12, "Qt 多媒体");

    QMap<int, QString>::Iterator iter;
    for (iter = qtStartMap.begin(); iter != qtStartMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QtStartID", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtStartID").toInt();
            qtStartShow(id);
        });
        flowLayout->addWidget(btn);
    }
}

void MainWindow::qtStartShow(int id) {
    switch (id) {
    case EQtStartID::QtStart0:
        qtWidgetsDialogShow();
        break;
    }
}

void MainWindow::qtWidgetsDialogShow() {
    QtWidgetsDialog dlg;
    dlg.exec();
}
