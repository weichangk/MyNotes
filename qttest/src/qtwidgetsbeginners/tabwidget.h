/*
 * @Author: weick 
 * @Date: 2024-07-26 07:45:45 
 * @Last Modified by: weick
 * @Last Modified time: 2024-07-26 07:50:01
 */
#pragma once
#include <QWidget>
#include <QTabWidget>

class TabWidget : public QWidget {
    Q_OBJECT

public:
    TabWidget(QWidget *parent = nullptr);
    ~TabWidget();

private:
    void createUi();
    void sigConnect();

private:
    QTabWidget *tab_widget_;
};