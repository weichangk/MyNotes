/*
 * @Author: weick 
 * @Date: 2024-05-26 08:29:37 
 * @Last Modified by: weick
 * @Last Modified time: 2024-08-06 00:35:15
 */

#pragma once
#include <QWidget>
#include <QTabWidget>
#include "qtwidget/dialog.h"
#include "qtwidget/painter.h"
#include "qtwidget/tabwidget.h"
#include "qtwidget/translucentbackground.h"
#include "qtwidget/layout.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void createUi();
    void sigConnect();
    void setQCoreDemoBtns(QWidget *w);
    void setQWidgetDemoBtns(QWidget *w);
    void setQGuiDemoBtns(QWidget *w);
    void setQmlDemoBtns(QWidget *w);
    void setACoreDemoBtns(QWidget *w);
    void setAWidgetDemoBtns(QWidget *w);
    void setAGuiDemoBtns(QWidget *w);
    void setAmlDemoBtns(QWidget *w);
    
    void test(int id);

    // qwidget
    void dialogShow();
    void painterShow();
    void tabShow();
    void translucentShow();
    void layoutMacBugShow();

private:
    QTabWidget *main_tabwidget_ = nullptr;

    QTabWidget *q_tabwidget_ = nullptr;
    QWidget *qcore_widget_ = nullptr;
    QWidget *qwidget_widget_ = nullptr;
    QWidget *qgui_widget_ = nullptr;
    QWidget *qml_widget_ = nullptr;

    QWidget *acore_widget_ = nullptr;
    QWidget *awidget_widget_ = nullptr;
    QWidget *agui_widget_ = nullptr;
    QWidget *aml_widget_ = nullptr;

    // qwidget
    PainterWidget *painter_widget_ = nullptr;
    TabWidget *tab_widget_ = nullptr;
    TranslucentBackgroundWidget *translucent_widget_ = nullptr;
    LayoutMacBugWidget *layoutmacbug_widget_ = nullptr;
};
