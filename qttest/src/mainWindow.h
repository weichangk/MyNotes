/*
 * @Author: weick 
 * @Date: 2024-05-26 08:29:37 
 * @Last Modified by: weick
 * @Last Modified time: 2024-05-26 09:36:03
 */

#pragma once
#include <QWidget>
#include <QPushButton>
#include "qtwidgetsbeginners/painter.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void createUi();
    void sigConnect();

    void painterTest();

private:
    QPushButton *painter_btn_ = nullptr;
    PainterWidget *painter_widget_ = nullptr;
};
