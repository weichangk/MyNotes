#pragma once
#include "qtwidgets/qtwidgetswindow.h"
#include "qtlayouts/qtlayoutswindow.h"
#include "qtevent/event_demo.h"

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
    void qtLayoutsWindowShow();
    void qtEventDemoShow();

private:
    QtWidgetsWindow *m_pQtWidgetsWindow = nullptr;
    QtLayoutsWindow *m_pQtLayoutsWindow = nullptr;
    QtEventWidget *m_pQtEventWidget = nullptr;
};
