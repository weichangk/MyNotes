#include "qtwidgetsdialog.h"
#include "../flowlayout.h"

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

QtWidgetsDialog::QtWidgetsDialog(QWidget *parent) :
    QDialog(parent) {
    createUi();
}

QtWidgetsDialog::~QtWidgetsDialog() {
}

void QtWidgetsDialog::createUi() {
    setWindowTitle("Qt 窗口部件");
    setMinimumSize(1096, 680);
    setQtWidgetsBtns(this);
}

void QtWidgetsDialog::setQtWidgetsBtns(QWidget *w) {
    auto flowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> qtStartMap;
    qtStartMap.insert(QtWidgetsID::QtWidgets0, "Qt 窗口与子窗口");

    QMap<int, QString>::Iterator iter;
    for (iter = qtStartMap.begin(); iter != qtStartMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QtWidgetsID", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtWidgetsID").toInt();
            // qtStartShow(id);
        });
        flowLayout->addWidget(btn);
    }
}