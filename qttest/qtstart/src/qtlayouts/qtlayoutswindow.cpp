#include "qtlayoutswindow.h"
#include "../flowlayout.h"

#include <QPushbutton>
#include <QMap>
#include <QVariant>

using QtLayoutsID = enum {
    QtBoxLayoutID = 0,
    QtFormLayoutID,
    QtGridLayoutID,
    QtStackLayoutID,
};

QtLayoutsWindow::QtLayoutsWindow(QWidget *parent) :
    QWidget(parent) {
        createUi();
}

QtLayoutsWindow::~QtLayoutsWindow() {
}

void QtLayoutsWindow::createUi() {
    setWindowTitle("Qt 布局管理");
    setMinimumSize(1096, 680);
    setQtLayoutsBtns(this);
}

void QtLayoutsWindow::setQtLayoutsBtns(QWidget *w) {
    auto flowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> qtStartMap;
    qtStartMap.insert(QtLayoutsID::QtBoxLayoutID, "Qt 窗口与子窗口");
    qtStartMap.insert(QtLayoutsID::QtFormLayoutID, "Qt 窗口类型");
    qtStartMap.insert(QtLayoutsID::QtGridLayoutID, "Qt 窗口几何布局");
    qtStartMap.insert(QtLayoutsID::QtStackLayoutID, "Qt 模态和非模态对话框");

    QMap<int, QString>::Iterator iter;
    for (iter = qtStartMap.begin(); iter != qtStartMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QtLayoutsID", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtLayoutsID").toInt();
            qtLayoutsTestShow(id);
        });
        flowLayout->addWidget(btn);
    }
}

void QtLayoutsWindow::qtLayoutsTestShow(int id) {
    switch (id) {
    case QtLayoutsID::QtBoxLayoutID:
        qbox_layout_test();
        break;
    case QtLayoutsID::QtFormLayoutID:
        qform_layout_test();
        break;
    case QtLayoutsID::QtGridLayoutID:
        qgrid_layout_test();
        break;
    case QtLayoutsID::QtStackLayoutID:
        qstack_layout_test();
        break;
    }
}

void QtLayoutsWindow::qbox_layout_test() {
}

void QtLayoutsWindow::qform_layout_test() {
}

void QtLayoutsWindow::qgrid_layout_test() {
}

void QtLayoutsWindow::qstack_layout_test() {
}
