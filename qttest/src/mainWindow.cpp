#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushbutton>
#include <QMap>
#include <QVariant>
#include "widget/flowlayout.h"
#include "filter/mask.h"

using QtTestDemo = enum {
    QtTestDemoDialog = 0,
    QtTestDemoPainter,
    QtTestDemoTabWidget,
    QtTestDemoTranslucent,
    QtTestDemoLayoutMacBug,
    QtTestDemoLanguageChangeTest,
    QtTestDemoQssTest,
    QtTestDemo777,
    QtTestDemo888,
    QtTestDemo999,
};

using QtmaterialCoreDemo = enum {
    QtmaterialCoreDemoDefineTest = 1000,
    QtmaterialCoreDemoSubjectTest,
    QtmaterialCoreDemo222,
    QtmaterialCoreDemo333,
    QtmaterialCoreDemo444,
    QtmaterialCoreDemo555,
    QtmaterialCoreDemo666,
    QtmaterialCoreDemo777,
    QtmaterialCoreDemo888,
    QtmaterialCoreDemo999,
};

using QtmaterialWidgetDemo = enum {
    QtmaterialWidgetButtonTest = 2000,
    QtmaterialWidgetDemoLabelTest,
    QtmaterialWidgetDemoLineEditTest,
    QtmaterialWidgetDemoProgressBarTest,
    QtmaterialWidgetDemoCarouselTest,
    QtmaterialWidgetAvatarTest,
    QtmaterialWidgetDemo666,
    QtmaterialWidgetDemo777,
    QtmaterialWidgetDemo888,
    QtmaterialWidgetDemo999,
};

using QtmaterialFilterDemo = enum {
    QtmaterialFilterShadowTest = 3000,
    QtmaterialFilterDemoMoveTest,
    QtmaterialFilterDemo222,
    QtmaterialFilterDemo333,
    QtmaterialFilterDemo444,
    QtmaterialFilterDemo555,
    QtmaterialFilterDemo666,
    QtmaterialFilterDemo777,
    QtmaterialFilterDemo888,
    QtmaterialFilterDemo999,
};

using QtmaterialOsxDemo = enum {
    QtmaterialOsxDemo000 = 4000,
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
    qtmaterial_widget_widget_ = new QWidget();
    qtmaterial_filter_widget_ = new QWidget();
    qtmaterial_osx_widget_ = new QWidget();

    qtmaterial_tabwidget_->addTab(qtmaterial_core_widget_, "core");
    qtmaterial_tabwidget_->addTab(qtmaterial_widget_widget_, "widget");
    qtmaterial_tabwidget_->addTab(qtmaterial_filter_widget_, "filter");
    qtmaterial_tabwidget_->addTab(qtmaterial_osx_widget_, "osx");

    setQtTestDemoBtns(qttest_widget_);

    setQtmaterialCoreDemoBtns(qtmaterial_core_widget_);
    setQtmaterialWidgetDemoBtns(qtmaterial_widget_widget_);
    setQtmaterialFilterDemoBtns(qtmaterial_filter_widget_);
    setQtmaterialOsxDemoBtns(qtmaterial_osx_widget_);
}

void MainWindow::setQtTestDemoBtns(QWidget *w) {
    auto demoFlowLayout = new widget::FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QtTestDemo::QtTestDemoDialog, "Dialog");
    demoMap.insert(QtTestDemo::QtTestDemoPainter, "Painter");
    demoMap.insert(QtTestDemo::QtTestDemoTabWidget, "TabWidget");
    demoMap.insert(QtTestDemo::QtTestDemoTranslucent, "Translucent");
    demoMap.insert(QtTestDemo::QtTestDemoLayoutMacBug, "LayoutMacBug");
    demoMap.insert(QtTestDemo::QtTestDemoLanguageChangeTest, "LanguageChangeTest");
    demoMap.insert(QtTestDemo::QtTestDemoQssTest, "QssTest");
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
    auto demoFlowLayout = new widget::FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemoDefineTest, "Define Test");
    demoMap.insert(QtmaterialCoreDemo::QtmaterialCoreDemoSubjectTest, "Subject Test");
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

void MainWindow::setQtmaterialWidgetDemoBtns(QWidget *w) {
    auto demoFlowLayout = new widget::FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QtmaterialWidgetDemo::QtmaterialWidgetButtonTest, "Button Test");
    demoMap.insert(QtmaterialWidgetDemo::QtmaterialWidgetDemoLabelTest, "Label Test");
    demoMap.insert(QtmaterialWidgetDemo::QtmaterialWidgetDemoLineEditTest, "LineEdit Test");
    demoMap.insert(QtmaterialWidgetDemo::QtmaterialWidgetDemoProgressBarTest, "ProgressBar Test");
    demoMap.insert(QtmaterialWidgetDemo::QtmaterialWidgetDemoCarouselTest, "Carousel Test");
    demoMap.insert(QtmaterialWidgetDemo::QtmaterialWidgetAvatarTest, "Avatar Test");
    demoMap.insert(QtmaterialWidgetDemo::QtmaterialWidgetDemo666, "666");
    demoMap.insert(QtmaterialWidgetDemo::QtmaterialWidgetDemo777, "777");
    demoMap.insert(QtmaterialWidgetDemo::QtmaterialWidgetDemo888, "888");
    demoMap.insert(QtmaterialWidgetDemo::QtmaterialWidgetDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = demoMap.begin(); iter != demoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QtmaterialWidgetDemo_Id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtmaterialWidgetDemo_Id").toInt();
            test(id);
        });
        demoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQtmaterialFilterDemoBtns(QWidget *w) {
    auto demoFlowLayout = new widget::FlowLayout(w, 4, 4, 4);
    QMap<int, QString> demoMap;
    demoMap.insert(QtmaterialFilterDemo::QtmaterialFilterShadowTest, "Shadow Test");
    demoMap.insert(QtmaterialFilterDemo::QtmaterialFilterDemoMoveTest, "Move Test");
    demoMap.insert(QtmaterialFilterDemo::QtmaterialFilterDemo222, "222");
    demoMap.insert(QtmaterialFilterDemo::QtmaterialFilterDemo333, "333");
    demoMap.insert(QtmaterialFilterDemo::QtmaterialFilterDemo444, "444");
    demoMap.insert(QtmaterialFilterDemo::QtmaterialFilterDemo555, "555");
    demoMap.insert(QtmaterialFilterDemo::QtmaterialFilterDemo666, "666");
    demoMap.insert(QtmaterialFilterDemo::QtmaterialFilterDemo777, "777");
    demoMap.insert(QtmaterialFilterDemo::QtmaterialFilterDemo888, "888");
    demoMap.insert(QtmaterialFilterDemo::QtmaterialFilterDemo999, "999");

    QMap<int, QString>::Iterator iter;
    for (iter = demoMap.begin(); iter != demoMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(160, 24);
        btn->setText(iter.value());
        btn->setProperty("QtmaterialFilterDemo_Id", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtmaterialFilterDemo_Id").toInt();
            test(id);
        });
        demoFlowLayout->addWidget(btn);
    }
}

void MainWindow::setQtmaterialOsxDemoBtns(QWidget *w) {
    auto demoFlowLayout = new widget::FlowLayout(w, 4, 4, 4);
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
    case QtTestDemo::QtTestDemoQssTest:
        qssTestWidgetShow();
        break;
    case QtTestDemo::QtTestDemo777:
        break;
    case QtTestDemo::QtTestDemo888:
        break;
    case QtTestDemo::QtTestDemo999:
        break;
    }

    switch (id) {
    case QtmaterialCoreDemo::QtmaterialCoreDemoDefineTest:
        defineTestShow();
        break;
    case QtmaterialCoreDemo::QtmaterialCoreDemoSubjectTest:
        subjectTestShow();
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
    case QtmaterialWidgetDemo::QtmaterialWidgetButtonTest:
        buttonTestShow();
        break;
    case QtmaterialWidgetDemo::QtmaterialWidgetDemoLabelTest:
        labelTestShow();
        break;
    case QtmaterialWidgetDemo::QtmaterialWidgetDemoLineEditTest:
        lineEditTestShow();
        break;
    case QtmaterialWidgetDemo::QtmaterialWidgetDemoProgressBarTest:
        progressBarTestShow();
        break;
    case QtmaterialWidgetDemo::QtmaterialWidgetDemoCarouselTest:
        carouselTestShow();
        break;
    case QtmaterialWidgetDemo::QtmaterialWidgetAvatarTest:
        avatarTestShow();
        break;
    case QtmaterialWidgetDemo::QtmaterialWidgetDemo666:
        break;
    case QtmaterialWidgetDemo::QtmaterialWidgetDemo777:
        break;
    case QtmaterialWidgetDemo::QtmaterialWidgetDemo888:
        break;
    case QtmaterialWidgetDemo::QtmaterialWidgetDemo999:
        break;
    }

    switch (id) {
    case QtmaterialFilterDemo::QtmaterialFilterShadowTest:
        shadowTestShow();
        break;
    case QtmaterialFilterDemo::QtmaterialFilterDemoMoveTest:
        moveTestShow();
        break;
    case QtmaterialFilterDemo::QtmaterialFilterDemo222:
        break;
    case QtmaterialFilterDemo::QtmaterialFilterDemo333:
        break;
    case QtmaterialFilterDemo::QtmaterialFilterDemo444:
        break;
    case QtmaterialFilterDemo::QtmaterialFilterDemo555:
        break;
    case QtmaterialFilterDemo::QtmaterialFilterDemo666:
        break;
    case QtmaterialFilterDemo::QtmaterialFilterDemo777:
        break;
    case QtmaterialFilterDemo::QtmaterialFilterDemo888:
        break;
    case QtmaterialFilterDemo::QtmaterialFilterDemo999:
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
    auto mask = new filter::PopupMask(&dialog, this);
    mask->setFullMask(true);
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

void MainWindow::qssTestWidgetShow() {
    if (!m_pQssWidget) {
        m_pQssWidget = new QssTestWidget();
    }
    m_pQssWidget->show();
}

void MainWindow::defineTestShow() {
    if (!m_pDefineTestWidget) {
        m_pDefineTestWidget = new DefineTestWidget();
    }
    m_pDefineTestWidget->show();
}

void MainWindow::subjectTestShow() {
    if (!m_pSubjectTestWidget) {
        m_pSubjectTestWidget = new SubjectTestWidget();
    }
    m_pSubjectTestWidget->show();
}

void MainWindow::buttonTestShow() {
    if (!m_pButtonTestWidget) {
        m_pButtonTestWidget = new ButtonTestWidget();
    }
    m_pButtonTestWidget->show();
}

void MainWindow::labelTestShow() {
    if (!m_pLabelTestWidget) {
        m_pLabelTestWidget = new LabelTestWidget();
    }
    m_pLabelTestWidget->show();
}

void MainWindow::lineEditTestShow() {
    if (!m_pLineEditTestWidget) {
        m_pLineEditTestWidget = new LineEditTestWidget();
    }
    m_pLineEditTestWidget->show();
}

void MainWindow::progressBarTestShow() {
    if (!m_pProgressBarTestWidget) {
        m_pProgressBarTestWidget = new ProgressBarTestWidget();
    }
    m_pProgressBarTestWidget->show();
}

void MainWindow::carouselTestShow() {
    if (!m_pCarouselTestWidget) {
        m_pCarouselTestWidget = new CarouselTestWidget();
    }
    m_pCarouselTestWidget->show();
}

void MainWindow::avatarTestShow() {
    if (!m_pAvatarTestWidget) {
        m_pAvatarTestWidget = new AvatarTestWidget();
    }
    m_pAvatarTestWidget->show();
}

void MainWindow::shadowTestShow() {
    if (!m_pShadowTestWidget) {
        m_pShadowTestWidget = new ShadowTestWidget();
    }
    m_pShadowTestWidget->show();
}

void MainWindow::moveTestShow() {
    if (!m_pMoveTestWidget) {
        m_pMoveTestWidget = new MoveTestWidget();
    }
    m_pMoveTestWidget->show();
}