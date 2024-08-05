/*
 * @Author: weick 
 * @Date: 2024-05-26 08:29:37 
 * @Last Modified by: weick
 * @Last Modified time: 2024-08-06 00:35:15
 */

#pragma once
#include <QWidget>
#include <QPushButton>
#include "qtwidget/painter.h"
#include "qtwidget/tabwidget.h"
#include "qtwidget/translucentbackground.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void createUi();
    void sigConnect();

    void painterShow();
    void tabShow();
    void translucentShow();

private:
    QPushButton *painter_btn_ = nullptr;
    PainterWidget *painter_widget_ = nullptr;
    QPushButton *tab_btn_ = nullptr;
    TabWidget *tab_widget_ = nullptr;
    QPushButton *translucent_btn_ = nullptr;
    TranslucentBackgroundWidget *translucent_widget_ = nullptr;
};
