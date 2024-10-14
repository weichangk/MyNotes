#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushbutton>
#include <QMap>
#include <QVariant>
#include "control/flowlayout.h"

using QCoreDemo = enum {
    QCoreDemo000 = 0,
    QCoreDemo111,
    QCoreDemo222,
    QCoreDemo333,
    QCoreDemo444,
    QCoreDemo555,
    QCoreDemo666,
    QCoreDemo777,
    QCoreDemo888,
    QCoreDemo999,
};

using QWidgetDemo = enum {
    QWidgetDemoDialog = 1000,
    QWidgetDemoPainter,
    QWidgetDemoTabWidget,
    QWidgetDemoTranslucent,
    QWidgetDemoLayoutMacBug,
    QWidgetDemo555,
    QWidgetDemo666,
    QWidgetDemo777,
    QWidgetDemo888,
    QWidgetDemo999,
};

using QGuiDemo = enum {
    QGuiDemo000 = 2000,
    QGuiDemo111,
    QGuiDemo222,
    QGuiDemo333,
    QGuiDemo444,
    QGuiDemo555,
    QGuiDemo666,
    QGuiDemo777,
    QGuiDemo888,
    QGuiDemo999,
};

using QmlDemo = enum {
    QmlDemo000 = 3000,
    QmlDemo111,
    QmlDemo222,
    QmlDemo333,
    QmlDemo444,
    QmlDemo555,
    QmlDemo666,
    QmlDemo777,
    QmlDemo888,
    QmlDemo999,
};

using QtmaterialCoreDemo = enum {
    QtmaterialCoreDemo000 = 4000,
    QtmaterialCoreDemo111,
    QtmaterialCoreDemo222,
    QtmaterialCoreDemo333,
    QtmaterialCoreDemo444,
    QtmaterialCoreDemo555,
    QtmaterialCoreDemo666,
    QtmaterialCoreDemo777,
    QtmaterialCoreDemo888,
    QtmaterialCoreDemo999,
};

using QtmaterialControlDemo = enum {
    QtmaterialControlDemo000 = 5000,
    QtmaterialControlDemo111,
    QtmaterialControlDemo222,
    QtmaterialControlDemo333,
    QtmaterialControlDemo444,
    QtmaterialControlDemo555,
    QtmaterialControlDemo666,
    QtmaterialControlDemo777,
    QtmaterialControlDemo888,
    QtmaterialControlDemo999,
};

using QtmaterialComponentDemo = enum {
    QtmaterialComponentDemo000 = 5000,
    QtmaterialComponentDemo111,
    QtmaterialComponentDemo222,
    QtmaterialComponentDemo333,
    QtmaterialComponentDemo444,
    QtmaterialComponentDemo555,
    QtmaterialComponentDemo666,
    QtmaterialComponentDemo777,
    QtmaterialComponentDemo888,
    QtmaterialComponentDemo999,
};

using QtmaterialOsxDemo = enum {
    QtmaterialOsxDemo000 = 7000,
    QtmaterialOsxDemo111,
    QtmaterialOsxDemo222,
    QtmaterialOsxDemo333,
    QtmaterialOsxDemo444,
    QtmaterialOsxDemo555,
    QtmaterialOsxDemo666,
    QtmaterialOsxDemo777,
    QtmaterialOsxDemo888,
    QtmaterialOsxDemo999,
};

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent) {
    createUi();
    sigConnect();
}

MainWindow::~MainWindow() {
}

void MainWindow::sigConnect() {
}

void MainWindow::createUi() {
    setMinimumSize(1096, 680);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    main_tabwidget_ = new QTabWidget();
    layout->addWidget(main_tabwidget_);

    q_tabwidget_ = new QTabWidget();
    main_tabwidget_->addTab(q_tabwidget_, "test");

    qcore_widget_ = new QWidget();
    qwidget_widget_ = new QWidget();
    qgui_widget_ = new QWidget();
    qml_widget_ = new QWidget();

    q_tabwidget_->addTab(qcore_widget_, "qcore");
    q_tabwidget_->addTab(qwidget_widget_, "qwidget");
    q_tabwidget_->addTab(qgui_widget_, "qgui");
    q_tabwidget_->addTab(qml_widget_, "qml");

    qtmaterial_tabwidget_ = new QTabWidget();
    main_tabwidget_->addTab(qtmaterial_tabwidget_, "qtmaterial");

    qtmaterial_core_widget_ = new QWidget();
    qtmaterial_control_widget_ = new QWidget();
    qtmaterial_component_widget_ = new QWidget();
    qtmaterial_osx_widget_ = new QWidget();

    qtmaterial_tabwidget_->addTab(qtmaterial_core_widget_, "core");
    qtmaterial_tabwidget_->addTab(qtmaterial_control_widget_, "control");
    qtmaterial_tabwidget_->addTab(qtmaterial_component_widget_, "component");
    qtmaterial_tabwidget_->addTab(qtmaterial_osx_widget_, "osx");

    setQCoreDemoBtns(qcore_widget_);
    setQWidgetDemoBtns(qwidget_widget_);
    setQGuiDemoBtns(qgui_widget_);
    setQmlDemoBtns(qml_widget_);

    setQtmaterialCoreDemoBtns(qtmaterial_core_widget_);
    setQtmaterialControlDemoBtns(qtmaterial_control_widget_);
    setQtmaterialComponentDemoBtns(qtmaterial_component_widget_);
    setQtmaterialOsxDemoBtns(qtmaterial_osx_widget_);
}

void MainWindow::setQCoreDemoBtns(QWidget *w) {
    auto demoFlowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QCoreDemo::QCoreDemo000, "000");
    demoMap.insert(QCoreDemo::QCoreDemo111, "111");
    demoMap.insert(QCoreDemo::QCoreDemo222, "222");
    demoMap.insert(QCoreDemo::QCoreDemo333, "333");
    demoMap.insert(QCoreDemo::QCoreDemo444, "444");
    demoMap.insert(QCoreDemo::QCoreDemo555, "555");
    demoMap.insert(QCoreDemo::QCoreDemo666, "666");
    demoMap.insert(QCoreDemo::QCoreDemo777, "777");
    demoMap.insert(QCoreDemo::QCoreDemo888, "888");
    demoMap.insert(QCoreDemo::QCoreDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = demoMap.begin(); iter != demoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QCoreDemo_Id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QCoreDemo_Id").toInt();
            test(id);
        });
        demoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQWidgetDemoBtns(QWidget *w) {
    auto demoFlowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QWidgetDemo::QWidgetDemoDialog, "Dialog");
    demoMap.insert(QWidgetDemo::QWidgetDemoPainter, "Painter");
    demoMap.insert(QWidgetDemo::QWidgetDemoTabWidget, "TabWidget");
    demoMap.insert(QWidgetDemo::QWidgetDemoTranslucent, "Translucent");
    demoMap.insert(QWidgetDemo::QWidgetDemoLayoutMacBug, "LayoutMacBug");
    demoMap.insert(QWidgetDemo::QWidgetDemo555, "555");
    demoMap.insert(QWidgetDemo::QWidgetDemo666, "666");
    demoMap.insert(QWidgetDemo::QWidgetDemo777, "777");
    demoMap.insert(QWidgetDemo::QWidgetDemo888, "888");
    demoMap.insert(QWidgetDemo::QWidgetDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = demoMap.begin(); iter != demoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QWidgetDemo_Id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QWidgetDemo_Id").toInt();
            test(id);
        });
        demoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQGuiDemoBtns(QWidget *w) {
    auto demoFlowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QGuiDemo::QGuiDemo000, "000");
    demoMap.insert(QGuiDemo::QGuiDemo111, "111");
    demoMap.insert(QGuiDemo::QGuiDemo222, "222");
    demoMap.insert(QGuiDemo::QGuiDemo333, "333");
    demoMap.insert(QGuiDemo::QGuiDemo444, "444");
    demoMap.insert(QGuiDemo::QGuiDemo555, "555");
    demoMap.insert(QGuiDemo::QGuiDemo666, "666");
    demoMap.insert(QGuiDemo::QGuiDemo777, "777");
    demoMap.insert(QGuiDemo::QGuiDemo888, "888");
    demoMap.insert(QGuiDemo::QGuiDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = demoMap.begin(); iter != demoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QGuiDemo_Id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QGuiDemo_Id").toInt();
            test(id);
        });
        demoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQmlDemoBtns(QWidget *w) {
    auto demoFlowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QmlDemo::QmlDemo000, "000");
    demoMap.insert(QmlDemo::QmlDemo111, "111");
    demoMap.insert(QmlDemo::QmlDemo222, "222");
    demoMap.insert(QmlDemo::QmlDemo333, "333");
    demoMap.insert(QmlDemo::QmlDemo444, "444");
    demoMap.insert(QmlDemo::QmlDemo555, "555");
    demoMap.insert(QmlDemo::QmlDemo666, "666");
    demoMap.insert(QmlDemo::QmlDemo777, "777");
    demoMap.insert(QmlDemo::QmlDemo888, "888");
    demoMap.insert(QmlDemo::QmlDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = demoMap.begin(); iter != demoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QmlDemo_Id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QmlDemo_Id").toInt();
            test(id);
        });
        demoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQtmaterialCoreDemoBtns(QWidget *w) {
    auto demoFlowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemo000, "000");
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemo111, "111");
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemo222, "222");
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemo333, "333");
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemo444, "444");
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemo555, "555");
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemo666, "666");
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemo777, "777");
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemo888, "888");
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = demoMap.begin(); iter != demoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QtmaterialCoreDemo_Id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtmaterialCoreDemo_Id").toInt();
            test(id);
        });
        demoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQtmaterialControlDemoBtns(QWidget *w) {
    auto demoFlowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QtmaterialControlDemo::QtmaterialControlDemo000, "000");
    demoMap.insert(QtmaterialControlDemo::QtmaterialControlDemo111, "111");
    demoMap.insert(QtmaterialControlDemo::QtmaterialControlDemo222, "222");
    demoMap.insert(QtmaterialControlDemo::QtmaterialControlDemo333, "333");
    demoMap.insert(QtmaterialControlDemo::QtmaterialControlDemo444, "444");
    demoMap.insert(QtmaterialControlDemo::QtmaterialControlDemo555, "555");
    demoMap.insert(QtmaterialControlDemo::QtmaterialControlDemo666, "666");
    demoMap.insert(QtmaterialControlDemo::QtmaterialControlDemo777, "777");
    demoMap.insert(QtmaterialControlDemo::QtmaterialControlDemo888, "888");
    demoMap.insert(QtmaterialControlDemo::QtmaterialControlDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = demoMap.begin(); iter != demoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QtmaterialControlDemo_Id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtmaterialControlDemo_Id").toInt();
            test(id);
        });
        demoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQtmaterialComponentDemoBtns(QWidget *w) {
    auto demoFlowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QtmaterialComponentDemo::QtmaterialComponentDemo000, "000");
    demoMap.insert(QtmaterialComponentDemo::QtmaterialComponentDemo111, "111");
    demoMap.insert(QtmaterialComponentDemo::QtmaterialComponentDemo222, "222");
    demoMap.insert(QtmaterialComponentDemo::QtmaterialComponentDemo333, "333");
    demoMap.insert(QtmaterialComponentDemo::QtmaterialComponentDemo444, "444");
    demoMap.insert(QtmaterialComponentDemo::QtmaterialComponentDemo555, "555");
    demoMap.insert(QtmaterialComponentDemo::QtmaterialComponentDemo666, "666");
    demoMap.insert(QtmaterialComponentDemo::QtmaterialComponentDemo777, "777");
    demoMap.insert(QtmaterialComponentDemo::QtmaterialComponentDemo888, "888");
    demoMap.insert(QtmaterialComponentDemo::QtmaterialComponentDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = demoMap.begin(); iter != demoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QtmaterialComponentDemo_Id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtmaterialComponentDemo_Id").toInt();
            test(id);
        });
        demoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQtmaterialOsxDemoBtns(QWidget *w) {
    auto demoFlowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QtmaterialOsxDemo::QtmaterialOsxDemo000, "000");
    demoMap.insert(QtmaterialOsxDemo::QtmaterialOsxDemo111, "111");
    demoMap.insert(QtmaterialOsxDemo::QtmaterialOsxDemo222, "222");
    demoMap.insert(QtmaterialOsxDemo::QtmaterialOsxDemo333, "333");
    demoMap.insert(QtmaterialOsxDemo::QtmaterialOsxDemo444, "444");
    demoMap.insert(QtmaterialOsxDemo::QtmaterialOsxDemo555, "555");
    demoMap.insert(QtmaterialOsxDemo::QtmaterialOsxDemo666, "666");
    demoMap.insert(QtmaterialOsxDemo::QtmaterialOsxDemo777, "777");
    demoMap.insert(QtmaterialOsxDemo::QtmaterialOsxDemo888, "888");
    demoMap.insert(QtmaterialOsxDemo::QtmaterialOsxDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = demoMap.begin(); iter != demoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QtmaterialOsxDemo_Id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtmaterialOsxDemo_Id").toInt();
            test(id);
        });
        demoFlowLayout->addWidget(btn);
    }
}

void MainWindow::test(int id) {
    switch (id) {
    case QCoreDemo::QCoreDemo000:
        break;
    case QCoreDemo::QCoreDemo111:
        break;
    case QCoreDemo::QCoreDemo222:
        break;
    case QCoreDemo::QCoreDemo333:
        break;
    case QCoreDemo::QCoreDemo444:
        break;
    case QCoreDemo::QCoreDemo555:
        break;
    case QCoreDemo::QCoreDemo666:
        break;
    case QCoreDemo::QCoreDemo777:
        break;
    case QCoreDemo::QCoreDemo888:
        break;
    case QCoreDemo::QCoreDemo999:
        break;
    }

    switch (id) {
    case QWidgetDemo::QWidgetDemoDialog:
        dialogShow();
        break;
    case QWidgetDemo::QWidgetDemoPainter:
        painterShow();
        break;
    case QWidgetDemo::QWidgetDemoTabWidget:
        tabShow();
        break;
    case QWidgetDemo::QWidgetDemoTranslucent:
        translucentShow();
        break;
    case QWidgetDemo::QWidgetDemoLayoutMacBug:
        layoutMacBugShow();
        break;
    case QWidgetDemo::QWidgetDemo555:
        break;
    case QWidgetDemo::QWidgetDemo666:
        break;
    case QWidgetDemo::QWidgetDemo777:
        break;
    case QWidgetDemo::QWidgetDemo888:
        break;
    case QWidgetDemo::QWidgetDemo999:
        break;
    }

    switch (id) {
    case QGuiDemo::QGuiDemo000:
        break;
    case QGuiDemo::QGuiDemo111:
        break;
    case QGuiDemo::QGuiDemo222:
        break;
    case QGuiDemo::QGuiDemo333:
        break;
    case QGuiDemo::QGuiDemo444:
        break;
    case QGuiDemo::QGuiDemo555:
        break;
    case QGuiDemo::QGuiDemo666:
        break;
    case QGuiDemo::QGuiDemo777:
        break;
    case QGuiDemo::QGuiDemo888:
        break;
    case QGuiDemo::QGuiDemo999:
        break;
    }

    switch (id) {
    case QmlDemo::QmlDemo000:
        break;
    case QmlDemo::QmlDemo111:
        break;
    case QmlDemo::QmlDemo222:
        break;
    case QmlDemo::QmlDemo333:
        break;
    case QmlDemo::QmlDemo444:
        break;
    case QmlDemo::QmlDemo555:
        break;
    case QmlDemo::QmlDemo666:
        break;
    case QmlDemo::QmlDemo777:
        break;
    case QmlDemo::QmlDemo888:
        break;
    case QmlDemo::QmlDemo999:
        break;
    }

    switch (id) {
    case QtmaterialCoreDemo::QtmaterialCoreDemo000:
        break;
    case QtmaterialCoreDemo::QtmaterialCoreDemo111:
        break;
    case QtmaterialCoreDemo::QtmaterialCoreDemo222:
        break;
    case QtmaterialCoreDemo::QtmaterialCoreDemo333:
        break;
    case QtmaterialCoreDemo::QtmaterialCoreDemo444:
        break;
    case QtmaterialCoreDemo::QtmaterialCoreDemo555:
        break;
    case QtmaterialCoreDemo::QtmaterialCoreDemo666:
        break;
    case QtmaterialCoreDemo::QtmaterialCoreDemo777:
        break;
    case QtmaterialCoreDemo::QtmaterialCoreDemo888:
        break;
    case QtmaterialCoreDemo::QtmaterialCoreDemo999:
        break;
    }

    switch (id) {
    case QtmaterialControlDemo::QtmaterialControlDemo000:
        break;
    case QtmaterialControlDemo::QtmaterialControlDemo111:
        break;
    case QtmaterialControlDemo::QtmaterialControlDemo222:
        break;
    case QtmaterialControlDemo::QtmaterialControlDemo333:
        break;
    case QtmaterialControlDemo::QtmaterialControlDemo444:
        break;
    case QtmaterialControlDemo::QtmaterialControlDemo555:
        break;
    case QtmaterialControlDemo::QtmaterialControlDemo666:
        break;
    case QtmaterialControlDemo::QtmaterialControlDemo777:
        break;
    case QtmaterialControlDemo::QtmaterialControlDemo888:
        break;
    case QtmaterialControlDemo::QtmaterialControlDemo999:
        break;
    }

    switch (id) {
    case QtmaterialComponentDemo::QtmaterialComponentDemo000:
        break;
    case QtmaterialComponentDemo::QtmaterialComponentDemo111:
        break;
    case QtmaterialComponentDemo::QtmaterialComponentDemo222:
        break;
    case QtmaterialComponentDemo::QtmaterialComponentDemo333:
        break;
    case QtmaterialComponentDemo::QtmaterialComponentDemo444:
        break;
    case QtmaterialComponentDemo::QtmaterialComponentDemo555:
        break;
    case QtmaterialComponentDemo::QtmaterialComponentDemo666:
        break;
    case QtmaterialComponentDemo::QtmaterialComponentDemo777:
        break;
    case QtmaterialComponentDemo::QtmaterialComponentDemo888:
        break;
    case QtmaterialComponentDemo::QtmaterialComponentDemo999:
        break;
    }

    switch (id) {
    case QtmaterialOsxDemo::QtmaterialOsxDemo000:
        break;
    case QtmaterialOsxDemo::QtmaterialOsxDemo111:
        break;
    case QtmaterialOsxDemo::QtmaterialOsxDemo222:
        break;
    case QtmaterialOsxDemo::QtmaterialOsxDemo333:
        break;
    case QtmaterialOsxDemo::QtmaterialOsxDemo444:
        break;
    case QtmaterialOsxDemo::QtmaterialOsxDemo555:
        break;
    case QtmaterialOsxDemo::QtmaterialOsxDemo666:
        break;
    case QtmaterialOsxDemo::QtmaterialOsxDemo777:
        break;
    case QtmaterialOsxDemo::QtmaterialOsxDemo888:
        break;
    case QtmaterialOsxDemo::QtmaterialOsxDemo999:
        break;
    }
}

void MainWindow::dialogShow() {
    Dialog dialog;
    dialog.exec();
}

void MainWindow::painterShow() {
    if(!painter_widget_) {
        painter_widget_ = new PainterWidget();
    }
    painter_widget_->show();
}

void MainWindow::tabShow() {
    if(!tab_widget_) {
        tab_widget_ = new TabWidget();
    }
    tab_widget_->show();
}

void MainWindow::translucentShow() {
    if(!translucent_widget_) {
        translucent_widget_ = new TranslucentBackgroundWidget();
    }
    translucent_widget_->show();
}

void MainWindow::layoutMacBugShow() {
    if(!layoutmacbug_widget_) {
        layoutmacbug_widget_ = new LayoutMacBugWidget();
    }
    layoutmacbug_widget_->show();
}