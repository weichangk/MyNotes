#include "qtwidgetswindow.h"
#include "../flowlayout.h"
#include "windows_and_sub_widgets.h"
#include "window_type.h"

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
    }
}

void QtWidgetsWindow::windows_and_sub_widgets_test() {
    windows_and_sub_widgets();
}

void QtWidgetsWindow::window_type_test() {
    window_type();
}