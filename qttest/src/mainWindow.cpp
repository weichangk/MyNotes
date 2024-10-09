/*
 * @Author: weick
 * @Date: 2024-05-26 08:30:54
 * @Last Modified by: weick
 * @Last Modified time: 2024-08-06 00:36:26
 */

#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushbutton>
#include <QMap>
#include <QVariant>
#include "../awidget/inc/aflowlayout.h"

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

using ACoreDemo = enum {
    ACoreDemo000 = 4000,
    ACoreDemo111,
    ACoreDemo222,
    ACoreDemo333,
    ACoreDemo444,
    ACoreDemo555,
    ACoreDemo666,
    ACoreDemo777,
    ACoreDemo888,
    ACoreDemo999,
};

using AWidgetDemo = enum {
    AWidgetDemo000 = 5000,
    AWidgetDemo111,
    AWidgetDemo222,
    AWidgetDemo333,
    AWidgetDemo444,
    AWidgetDemo555,
    AWidgetDemo666,
    AWidgetDemo777,
    AWidgetDemo888,
    AWidgetDemo999,
};

using AGuiDemo = enum {
    AGuiDemo000 = 6000,
    AGuiDemo111,
    AGuiDemo222,
    AGuiDemo333,
    AGuiDemo444,
    AGuiDemo555,
    AGuiDemo666,
    AGuiDemo777,
    AGuiDemo888,
    AGuiDemo999,
};

using AmlDemo = enum {
    AmlDemo000 = 8000,
    AmlDemo111,
    AmlDemo222,
    AmlDemo333,
    AmlDemo444,
    AmlDemo555,
    AmlDemo666,
    AmlDemo777,
    AmlDemo888,
    AmlDemo999,
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

    qcore_widget_ = new QWidget();
    qwidget_widget_ = new QWidget();
    qgui_widget_ = new QWidget();
    qml_widget_ = new QWidget();

    acore_widget_ = new QWidget();
    awidget_widget_ = new QWidget();
    agui_widget_ = new QWidget();
    aml_widget_ = new QWidget();

    main_tabwidget_->addTab(q_tabwidget_, "test");
    q_tabwidget_->addTab(qcore_widget_, "qcore");
    q_tabwidget_->addTab(qwidget_widget_, "qwidget");
    q_tabwidget_->addTab(qgui_widget_, "qgui");
    q_tabwidget_->addTab(qml_widget_, "qml");

    main_tabwidget_->addTab(acore_widget_, "acore");
    main_tabwidget_->addTab(awidget_widget_, "awidget");
    main_tabwidget_->addTab(agui_widget_, "agui");
    main_tabwidget_->addTab(aml_widget_, "aml");

    setQCoreDemoBtns(qcore_widget_);
    setQWidgetDemoBtns(qwidget_widget_);
    setQGuiDemoBtns(qgui_widget_);
    setQmlDemoBtns(qml_widget_);

    setACoreDemoBtns(acore_widget_);
    setAWidgetDemoBtns(awidget_widget_);
    setAGuiDemoBtns(agui_widget_);
    setAmlDemoBtns(aml_widget_);
}

void MainWindow::setQCoreDemoBtns(QWidget *w) {
    auto testDemoFlowLayout = new AFlowLayout(w, 4, 4, 4);
    QMap<int, QString> testDemoMap;
    testDemoMap.insert(QCoreDemo::QCoreDemo000, "000");
    testDemoMap.insert(QCoreDemo::QCoreDemo111, "111");
    testDemoMap.insert(QCoreDemo::QCoreDemo222, "222");
    testDemoMap.insert(QCoreDemo::QCoreDemo333, "333");
    testDemoMap.insert(QCoreDemo::QCoreDemo444, "444");
    testDemoMap.insert(QCoreDemo::QCoreDemo555, "555");
    testDemoMap.insert(QCoreDemo::QCoreDemo666, "666");
    testDemoMap.insert(QCoreDemo::QCoreDemo777, "777");
    testDemoMap.insert(QCoreDemo::QCoreDemo888, "888");
    testDemoMap.insert(QCoreDemo::QCoreDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = testDemoMap.begin(); iter != testDemoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("qcore_id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("qcore_id").toInt();
            test(id);
        });
        testDemoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQWidgetDemoBtns(QWidget *w) {
    auto testDemoFlowLayout = new AFlowLayout(w, 4, 4, 4);
    QMap<int, QString> testDemoMap;
    testDemoMap.insert(QWidgetDemo::QWidgetDemoDialog, "Dialog");
    testDemoMap.insert(QWidgetDemo::QWidgetDemoPainter, "Painter");
    testDemoMap.insert(QWidgetDemo::QWidgetDemoTabWidget, "TabWidget");
    testDemoMap.insert(QWidgetDemo::QWidgetDemoTranslucent, "Translucent");
    testDemoMap.insert(QWidgetDemo::QWidgetDemoLayoutMacBug, "LayoutMacBug");
    testDemoMap.insert(QWidgetDemo::QWidgetDemo555, "555");
    testDemoMap.insert(QWidgetDemo::QWidgetDemo666, "666");
    testDemoMap.insert(QWidgetDemo::QWidgetDemo777, "777");
    testDemoMap.insert(QWidgetDemo::QWidgetDemo888, "888");
    testDemoMap.insert(QWidgetDemo::QWidgetDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = testDemoMap.begin(); iter != testDemoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("qwidget_id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("qwidget_id").toInt();
            test(id);
        });
        testDemoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQGuiDemoBtns(QWidget *w) {
    auto testDemoFlowLayout = new AFlowLayout(w, 4, 4, 4);
    QMap<int, QString> testDemoMap;
    testDemoMap.insert(QGuiDemo::QGuiDemo000, "000");
    testDemoMap.insert(QGuiDemo::QGuiDemo111, "111");
    testDemoMap.insert(QGuiDemo::QGuiDemo222, "222");
    testDemoMap.insert(QGuiDemo::QGuiDemo333, "333");
    testDemoMap.insert(QGuiDemo::QGuiDemo444, "444");
    testDemoMap.insert(QGuiDemo::QGuiDemo555, "555");
    testDemoMap.insert(QGuiDemo::QGuiDemo666, "666");
    testDemoMap.insert(QGuiDemo::QGuiDemo777, "777");
    testDemoMap.insert(QGuiDemo::QGuiDemo888, "888");
    testDemoMap.insert(QGuiDemo::QGuiDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = testDemoMap.begin(); iter != testDemoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("qgui_id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("qgui_id").toInt();
            test(id);
        });
        testDemoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQmlDemoBtns(QWidget *w) {
    auto testDemoFlowLayout = new AFlowLayout(w, 4, 4, 4);
    QMap<int, QString> testDemoMap;
    testDemoMap.insert(QmlDemo::QmlDemo000, "000");
    testDemoMap.insert(QmlDemo::QmlDemo111, "111");
    testDemoMap.insert(QmlDemo::QmlDemo222, "222");
    testDemoMap.insert(QmlDemo::QmlDemo333, "333");
    testDemoMap.insert(QmlDemo::QmlDemo444, "444");
    testDemoMap.insert(QmlDemo::QmlDemo555, "555");
    testDemoMap.insert(QmlDemo::QmlDemo666, "666");
    testDemoMap.insert(QmlDemo::QmlDemo777, "777");
    testDemoMap.insert(QmlDemo::QmlDemo888, "888");
    testDemoMap.insert(QmlDemo::QmlDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = testDemoMap.begin(); iter != testDemoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("qml_id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("qml_id").toInt();
            test(id);
        });
        testDemoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setACoreDemoBtns(QWidget *w) {
    auto aCoreDemoFlowLayout = new AFlowLayout(w, 4, 4, 4);
    QMap<int, QString> aCoreDemoMap;
    aCoreDemoMap.insert(ACoreDemo::ACoreDemo000, "000");
    aCoreDemoMap.insert(ACoreDemo::ACoreDemo111, "111");
    aCoreDemoMap.insert(ACoreDemo::ACoreDemo222, "222");
    aCoreDemoMap.insert(ACoreDemo::ACoreDemo333, "333");
    aCoreDemoMap.insert(ACoreDemo::ACoreDemo444, "444");
    aCoreDemoMap.insert(ACoreDemo::ACoreDemo555, "555");
    aCoreDemoMap.insert(ACoreDemo::ACoreDemo666, "666");
    aCoreDemoMap.insert(ACoreDemo::ACoreDemo777, "777");
    aCoreDemoMap.insert(ACoreDemo::ACoreDemo888, "888");
    aCoreDemoMap.insert(ACoreDemo::ACoreDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = aCoreDemoMap.begin(); iter != aCoreDemoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("acore_id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("acore_id").toInt();
            test(id);
        });
        aCoreDemoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setAWidgetDemoBtns(QWidget *w) {
    auto aWidgetDemoFlowLayout = new AFlowLayout(w, 4, 4, 4);
    QMap<int, QString> aWidgetDemoMap;
    aWidgetDemoMap.insert(AWidgetDemo::AWidgetDemo000, "000");
    aWidgetDemoMap.insert(AWidgetDemo::AWidgetDemo111, "111");
    aWidgetDemoMap.insert(AWidgetDemo::AWidgetDemo222, "222");
    aWidgetDemoMap.insert(AWidgetDemo::AWidgetDemo333, "333");
    aWidgetDemoMap.insert(AWidgetDemo::AWidgetDemo444, "444");
    aWidgetDemoMap.insert(AWidgetDemo::AWidgetDemo555, "555");
    aWidgetDemoMap.insert(AWidgetDemo::AWidgetDemo666, "666");
    aWidgetDemoMap.insert(AWidgetDemo::AWidgetDemo777, "777");
    aWidgetDemoMap.insert(AWidgetDemo::AWidgetDemo888, "888");
    aWidgetDemoMap.insert(AWidgetDemo::AWidgetDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = aWidgetDemoMap.begin(); iter != aWidgetDemoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("awidget_id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("awidget_id").toInt();
            test(id);
        });
        aWidgetDemoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setAGuiDemoBtns(QWidget *w) {
    auto aGuiDemoFlowLayout = new AFlowLayout(w, 4, 4, 4);
    QMap<int, QString> aGuiDemoMap;
    aGuiDemoMap.insert(AGuiDemo::AGuiDemo000, "000");
    aGuiDemoMap.insert(AGuiDemo::AGuiDemo111, "111");
    aGuiDemoMap.insert(AGuiDemo::AGuiDemo222, "222");
    aGuiDemoMap.insert(AGuiDemo::AGuiDemo333, "333");
    aGuiDemoMap.insert(AGuiDemo::AGuiDemo444, "444");
    aGuiDemoMap.insert(AGuiDemo::AGuiDemo555, "555");
    aGuiDemoMap.insert(AGuiDemo::AGuiDemo666, "666");
    aGuiDemoMap.insert(AGuiDemo::AGuiDemo777, "777");
    aGuiDemoMap.insert(AGuiDemo::AGuiDemo888, "888");
    aGuiDemoMap.insert(AGuiDemo::AGuiDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = aGuiDemoMap.begin(); iter != aGuiDemoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("agui_id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("agui_id").toInt();
            test(id);
        });
        aGuiDemoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setAmlDemoBtns(QWidget *w) {
    auto amlDemoFlowLayout = new AFlowLayout(w, 4, 4, 4);
    QMap<int, QString> amlDemoMap;
    amlDemoMap.insert(AmlDemo::AmlDemo000, "000");
    amlDemoMap.insert(AmlDemo::AmlDemo111, "111");
    amlDemoMap.insert(AmlDemo::AmlDemo222, "222");
    amlDemoMap.insert(AmlDemo::AmlDemo333, "333");
    amlDemoMap.insert(AmlDemo::AmlDemo444, "444");
    amlDemoMap.insert(AmlDemo::AmlDemo555, "555");
    amlDemoMap.insert(AmlDemo::AmlDemo666, "666");
    amlDemoMap.insert(AmlDemo::AmlDemo777, "777");
    amlDemoMap.insert(AmlDemo::AmlDemo888, "888");
    amlDemoMap.insert(AmlDemo::AmlDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = amlDemoMap.begin(); iter != amlDemoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("aml_id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("aml_id").toInt();
            test(id);
        });
        amlDemoFlowLayout->addWidget(btn);
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
    case ACoreDemo::ACoreDemo000:
        break;
    case ACoreDemo::ACoreDemo111:
        break;
    case ACoreDemo::ACoreDemo222:
        break;
    case ACoreDemo::ACoreDemo333:
        break;
    case ACoreDemo::ACoreDemo444:
        break;
    case ACoreDemo::ACoreDemo555:
        break;
    case ACoreDemo::ACoreDemo666:
        break;
    case ACoreDemo::ACoreDemo777:
        break;
    case ACoreDemo::ACoreDemo888:
        break;
    case ACoreDemo::ACoreDemo999:
        break;
    }

    switch (id) {
    case AWidgetDemo::AWidgetDemo000:
        break;
    case AWidgetDemo::AWidgetDemo111:
        break;
    case AWidgetDemo::AWidgetDemo222:
        break;
    case AWidgetDemo::AWidgetDemo333:
        break;
    case AWidgetDemo::AWidgetDemo444:
        break;
    case AWidgetDemo::AWidgetDemo555:
        break;
    case AWidgetDemo::AWidgetDemo666:
        break;
    case AWidgetDemo::AWidgetDemo777:
        break;
    case AWidgetDemo::AWidgetDemo888:
        break;
    case AWidgetDemo::AWidgetDemo999:
        break;
    }

    switch (id) {
    case AGuiDemo::AGuiDemo000:
        break;
    case AGuiDemo::AGuiDemo111:
        break;
    case AGuiDemo::AGuiDemo222:
        break;
    case AGuiDemo::AGuiDemo333:
        break;
    case AGuiDemo::AGuiDemo444:
        break;
    case AGuiDemo::AGuiDemo555:
        break;
    case AGuiDemo::AGuiDemo666:
        break;
    case AGuiDemo::AGuiDemo777:
        break;
    case AGuiDemo::AGuiDemo888:
        break;
    case AGuiDemo::AGuiDemo999:
        break;
    }

    switch (id) {
    case AmlDemo::AmlDemo000:
        break;
    case AmlDemo::AmlDemo111:
        break;
    case AmlDemo::AmlDemo222:
        break;
    case AmlDemo::AmlDemo333:
        break;
    case AmlDemo::AmlDemo444:
        break;
    case AmlDemo::AmlDemo555:
        break;
    case AmlDemo::AmlDemo666:
        break;
    case AmlDemo::AmlDemo777:
        break;
    case AmlDemo::AmlDemo888:
        break;
    case AmlDemo::AmlDemo999:
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