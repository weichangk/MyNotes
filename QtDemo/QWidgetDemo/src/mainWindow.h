#pragma once
#include <QWidget>
#include <QTabWidget>
#include "other/dialog.h"
#include "other/painter.h"
#include "other/tabwidget.h"
#include "other/translucentbackground.h"
#include "other/layout.h"
#include "other/languagechange.h"
#include "other/qsstest.h"
#include "core/definetest.h"
#include "core/subjecttest.h"
#include "widget/buttontest.h"
#include "widget/labeltest.h"
#include "widget/lineedittest.h"
#include "widget/progressbartest.h"
#include "widget/carouseltest.h"
#include "widget/avatartest.h"
#include "widget/titlebartest.h"
#include "filter/shadowtest.h"
#include "filter/movetest.h"

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
    void setQtmaterialWidgetDemoBtns(QWidget *w);
    void setQtmaterialFilterDemoBtns(QWidget *w);
    void setQtmaterialOsxDemoBtns(QWidget *w);
    
    void test(int id);

    // qttest
    void dialogShow();
    void painterShow();
    void tabShow();
    void translucentShow();
    void layoutMacBugShow();
    void languageChangeTestShow();
    void qssTestWidgetShow();

    // qtmaterial core
    void defineTestShow();
    void subjectTestShow();

    // qtmaterial widget
    void buttonTestShow();
    void labelTestShow();
    void lineEditTestShow();
    void progressBarTestShow();
    void carouselTestShow();
    void avatarTestShow();
    void titleBarTestShow();

    // qtmaterial filter
    void shadowTestShow();
    void moveTestShow();

private:
    QTabWidget *main_tabwidget_ = nullptr;

    QWidget *qttest_widget_ = nullptr;

    QTabWidget *qtmaterial_tabwidget_ = nullptr;
    QWidget *qtmaterial_core_widget_ = nullptr;
    QWidget *qtmaterial_widget_widget_ = nullptr;
    QWidget *qtmaterial_filter_widget_ = nullptr;
    QWidget *qtmaterial_osx_widget_ = nullptr;

    // qttest
    PainterWidget *painter_widget_ = nullptr;
    TabWidget *tab_widget_ = nullptr;
    TranslucentBackgroundWidget *translucent_widget_ = nullptr;
    LayoutMacBugWidget *layoutmacbug_widget_ = nullptr;
    LanguageChangeTest *languagechange_widget_ = nullptr;
    QssTestWidget *m_pQssWidget = nullptr;

    // qtmaterial core
    DefineTestWidget *m_pDefineTestWidget = nullptr;
    SubjectTestWidget *m_pSubjectTestWidget = nullptr;

    // qtmaterial widget
    ButtonTestWidget *m_pButtonTestWidget = nullptr;
    LabelTestWidget *m_pLabelTestWidget = nullptr;
    LineEditTestWidget *m_pLineEditTestWidget = nullptr;
    ProgressBarTestWidget *m_pProgressBarTestWidget = nullptr;
    CarouselTestWidget *m_pCarouselTestWidget = nullptr;
    AvatarTestWidget *m_pAvatarTestWidget = nullptr;
    TitlebarTestWidget *m_pTitlebarTestWidget = nullptr;

    // qtmaterial filter
    ShadowTestWidget *m_pShadowTestWidget = nullptr;
    MoveTestWidget *m_pMoveTestWidget = nullptr;
};
