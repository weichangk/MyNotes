/*
 * @Author: weick 
 * @Date: 2024-05-26 08:29:37 
 * @Last Modified by: weick
 * @Last Modified time: 2024-08-06 00:35:15
 */

#pragma once
#include <QWidget>
#include <QTabWidget>

// #include <QPushButton>
// #include "qtwidget/painter.h"
// #include "qtwidget/tabwidget.h"
// #include "qtwidget/translucentbackground.h"

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
    
    // void painterShow();
    // void tabShow();
    // void translucentShow();

private:
    QTabWidget *tab_widget_ = nullptr;

    QTabWidget *tab_test_widget_ = nullptr;
    QWidget *tab_test_qcore_widget_ = nullptr;
    QWidget *tab_test_qwidget_widget_ = nullptr;
    QWidget *tab_test_qgui_widget_ = nullptr;
    QWidget *tab_test_qml_widget_ = nullptr;

    QWidget *tab_acore_widget_ = nullptr;
    QWidget *tab_awidget_widget_ = nullptr;
    QWidget *tab_agui_widget_ = nullptr;
    QWidget *tab_aml_widget_ = nullptr;

    // QPushButton *painter_btn_ = nullptr;
    // PainterWidget *painter_widget_ = nullptr;
    // QPushButton *tab_btn_ = nullptr;
    // TabWidget *tab_widget_ = nullptr;
    // QPushButton *translucent_btn_ = nullptr;
    // TranslucentBackgroundWidget *translucent_widget_ = nullptr;
};
