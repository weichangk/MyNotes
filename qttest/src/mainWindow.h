/*
 * @Author: weick 
 * @Date: 2024-05-26 08:29:37 
 * @Last Modified by: weick
 * @Last Modified time: 2024-07-26 07:59:30
 */

#pragma once
#include <QWidget>
#include <QPushButton>
#include "qtwidget/painter.h"
#include "qtwidget/tabwidget.h"

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

private:
    QPushButton *painter_btn_ = nullptr;
    PainterWidget *painter_widget_ = nullptr;
    QPushButton *tab_btn_ = nullptr;
    TabWidget *tab_widget_ = nullptr;
};
