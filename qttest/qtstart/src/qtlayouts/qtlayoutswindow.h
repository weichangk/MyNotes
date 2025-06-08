#pragma once
#include <QWidget>

class QtLayoutsWindow : public QWidget {
    Q_OBJECT

public:
    QtLayoutsWindow(QWidget *parent = nullptr);
    ~QtLayoutsWindow();

private:
    void createUi();
    void setQtLayoutsBtns(QWidget *w);
    void qtLayoutsTestShow(int id);

    void qbox_layout_test();
    void qform_layout_test();
    void qgrid_layout_test();
    void qstack_layout_test();
};