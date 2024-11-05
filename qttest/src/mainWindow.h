#pragma once
#include <QWidget>
#include <QTabWidget>
#include "test/dialog.h"
#include "test/painter.h"
#include "test/tabwidget.h"
#include "test/translucentbackground.h"
#include "test/layout.h"
#include "test/languagechange.h"
#include "qtmaterial/control/buttontest.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void createUi();
    void sigConnect();
    void setQtTestDemoBtns(QWidget *w);

    void setQtmaterialCoreDemoBtns(QWidget *w);
    void setQtmaterialControlDemoBtns(QWidget *w);
    void setQtmaterialComponentDemoBtns(QWidget *w);
    void setQtmaterialOsxDemoBtns(QWidget *w);
    
    void test(int id);

    // qttest
    void dialogShow();
    void painterShow();
    void tabShow();
    void translucentShow();
    void layoutMacBugShow();
    void languageChangeTestShow();;

    // qtmaterial control
    void buttonTestShow();

private:
    QTabWidget *main_tabwidget_ = nullptr;

    QWidget *qttest_widget_ = nullptr;

    QTabWidget *qtmaterial_tabwidget_ = nullptr;
    QWidget *qtmaterial_core_widget_ = nullptr;
    QWidget *qtmaterial_control_widget_ = nullptr;
    QWidget *qtmaterial_component_widget_ = nullptr;
    QWidget *qtmaterial_osx_widget_ = nullptr;

    // qttest
    PainterWidget *painter_widget_ = nullptr;
    TabWidget *tab_widget_ = nullptr;
    TranslucentBackgroundWidget *translucent_widget_ = nullptr;
    LayoutMacBugWidget *layoutmacbug_widget_ = nullptr;
    LanguageChangeTest *languagechange_widget_ = nullptr;

    // qtmaterial control
    ButtonTestWidget *m_pButtonTestWidget = nullptr;
};
