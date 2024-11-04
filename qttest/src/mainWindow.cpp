#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushbutton>
#include <QMap>
#include <QVariant>
#include "control/flowlayout.h"

using QtTestDemo = enum {
    QtTestDemoDialog = 0,
    QtTestDemoPainter,
    QtTestDemoTabWidget,
    QtTestDemoTranslucent,
    QtTestDemoLayoutMacBug,
    QtTestDemoLanguageChangeTest,
    QtTestDemoIconFontTest,
    QtTestDemo777,
    QtTestDemo888,
    QtTestDemo999,
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

    qttest_widget_ = new QWidget();
    main_tabwidget_->addTab(qttest_widget_, "qttest");

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

    setQtTestDemoBtns(qttest_widget_);

    setQtmaterialCoreDemoBtns(qtmaterial_core_widget_);
    setQtmaterialControlDemoBtns(qtmaterial_control_widget_);
    setQtmaterialComponentDemoBtns(qtmaterial_component_widget_);
    setQtmaterialOsxDemoBtns(qtmaterial_osx_widget_);
}

void MainWindow::setQtTestDemoBtns(QWidget *w) {
    auto demoFlowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QtTestDemo::QtTestDemoDialog, "Dialog");
    demoMap.insert(QtTestDemo::QtTestDemoPainter, "Painter");
    demoMap.insert(QtTestDemo::QtTestDemoTabWidget, "TabWidget");
    demoMap.insert(QtTestDemo::QtTestDemoTranslucent, "Translucent");
    demoMap.insert(QtTestDemo::QtTestDemoLayoutMacBug, "LayoutMacBug");
    demoMap.insert(QtTestDemo::QtTestDemoLanguageChangeTest, "LanguageChangeTest");
    demoMap.insert(QtTestDemo::QtTestDemoIconFontTest, "IconFontTest");
    demoMap.insert(QtTestDemo::QtTestDemo777, "777");
    demoMap.insert(QtTestDemo::QtTestDemo888, "888");
    demoMap.insert(QtTestDemo::QtTestDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = demoMap.begin(); iter != demoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QtTestDemo_Id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtTestDemo_Id").toInt();
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
    case QtTestDemo::QtTestDemoDialog:
        dialogShow();
        break;
    case QtTestDemo::QtTestDemoPainter:
        painterShow();
        break;
    case QtTestDemo::QtTestDemoTabWidget:
        tabShow();
        break;
    case QtTestDemo::QtTestDemoTranslucent:
        translucentShow();
        break;
    case QtTestDemo::QtTestDemoLayoutMacBug:
        layoutMacBugShow();
        break;
    case QtTestDemo::QtTestDemoLanguageChangeTest:
        languageChangeTestShow();
        break;
    case QtTestDemo::QtTestDemoIconFontTest:
        iconFontTestShow();
        break;
    case QtTestDemo::QtTestDemo777:
        break;
    case QtTestDemo::QtTestDemo888:
        break;
    case QtTestDemo::QtTestDemo999:
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
    if (!painter_widget_) {
        painter_widget_ = new PainterWidget();
    }
    painter_widget_->show();
}

void MainWindow::tabShow() {
    if (!tab_widget_) {
        tab_widget_ = new TabWidget();
    }
    tab_widget_->show();
}

void MainWindow::translucentShow() {
    if (!translucent_widget_) {
        translucent_widget_ = new TranslucentBackgroundWidget();
    }
    translucent_widget_->show();
}

void MainWindow::layoutMacBugShow() {
    if (!layoutmacbug_widget_) {
        layoutmacbug_widget_ = new LayoutMacBugWidget();
    }
    layoutmacbug_widget_->show();
}

void MainWindow::languageChangeTestShow() {
    if (!languagechange_widget_) {
        languagechange_widget_ = new LanguageChangeTest();
    }
    languagechange_widget_->show();
}

void MainWindow::iconFontTestShow() {
    if (!m_pIconFontTestWidget) {
        m_pIconFontTestWidget = new IconFontTestWidget();
    }
    m_pIconFontTestWidget->show();
}