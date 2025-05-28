#include "qtwidgetswindow.h"
#include "../flowlayout.h"
#include "windows_and_sub_widgets.h"
#include "window_type.h"
#include "window_geometry.h"
#include "modal_dialog.h"
#include "window_switch.h"
#include "standard_dialog.h"
#include "frame_use.h"
#include "frame_family.h"

#include <QPushbutton>
#include <QMap>
#include <QVariant>

using QtWidgetsID = enum {
    QtWidgets0 = 0,
    QtWidgets1,
    QtWidgets2,
    QtWidgets3,
    QtWidgets4,
    QtWidgets5,
    QtWidgets6,
    QtWidgets7,
    QtWidgets8,
    QtWidgets9,
};

QtWidgetsWindow::QtWidgetsWindow(QWidget *parent) :
    QWidget(parent) {
    createUi();
}

QtWidgetsWindow::~QtWidgetsWindow() {
}

void QtWidgetsWindow::createUi() {
    setWindowTitle("Qt 窗口部件");
    setMinimumSize(1096, 680);
    setQtWidgetsBtns(this);
}

void QtWidgetsWindow::setQtWidgetsBtns(QWidget *w) {
    auto flowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> qtStartMap;
    qtStartMap.insert(QtWidgetsID::QtWidgets0, "Qt 窗口与子窗口");
    qtStartMap.insert(QtWidgetsID::QtWidgets1, "Qt 窗口类型");
    qtStartMap.insert(QtWidgetsID::QtWidgets2, "Qt 窗口几何布局");
    qtStartMap.insert(QtWidgetsID::QtWidgets3, "Qt 模态和非模态对话框");
    qtStartMap.insert(QtWidgetsID::QtWidgets4, "Qt 多窗口切换");
    qtStartMap.insert(QtWidgetsID::QtWidgets5, "Qt 标准对话框");
    qtStartMap.insert(QtWidgetsID::QtWidgets6, "Qt QFrame使用");
    qtStartMap.insert(QtWidgetsID::QtWidgets7, "Qt QFrame类族");

    QMap<int, QString>::Iterator iter;
    for (iter = qtStartMap.begin(); iter != qtStartMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QtWidgetsID", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtWidgetsID").toInt();
            qtWidgetsTestShow(id);
        });
        flowLayout->addWidget(btn);
    }
}

void QtWidgetsWindow::qtWidgetsTestShow(int id) {
    switch (id) {
    case QtWidgetsID::QtWidgets0:
        windows_and_sub_widgets_test();
        break;
    case QtWidgetsID::QtWidgets1:
        window_type_test();
        break;
    case QtWidgetsID::QtWidgets2:
        window_geometry_test();
        break;
    case QtWidgetsID::QtWidgets3:
        modal_dialog_test();
        break;
    case QtWidgetsID::QtWidgets4:
        window_switch_test();
        break;
    case QtWidgetsID::QtWidgets5:
        standard_dialog_test();
        break;
    case QtWidgetsID::QtWidgets6:
        frame_use_test();
        break;
    case QtWidgetsID::QtWidgets7:
        frame_family_test();
        break;
    }
}

void QtWidgetsWindow::windows_and_sub_widgets_test() {
    windows_and_sub_widgets();
}

void QtWidgetsWindow::window_type_test() {
    window_type();
}

void QtWidgetsWindow::window_geometry_test() {
    window_geometry();
}

void QtWidgetsWindow::modal_dialog_test() {
    modal_dialog();
}

void QtWidgetsWindow::window_switch_test() {
    window_switch();
}

void QtWidgetsWindow::standard_dialog_test() {
    standard_dialog();
}

void QtWidgetsWindow::frame_use_test() {
    frame_use();
}

void QtWidgetsWindow::frame_family_test() {
    frame_family();
}