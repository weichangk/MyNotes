#pragma once
#include "qtwidgets/qtwidgetswindow.h"

#include <QWidget>

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void createUi();
    void setQtStartBtns(QWidget *w);
    void qtStartShow(int id);
    void qtWidgetsWindowShow();

private:
    QtWidgetsWindow *m_pQtWidgetsWindow = nullptr;
};
