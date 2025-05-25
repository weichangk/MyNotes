#pragma once
#include <QWidget>

class QtWidgetsWindow : public QWidget {
    Q_OBJECT

public:
    QtWidgetsWindow(QWidget *parent = nullptr);
    ~QtWidgetsWindow();

private:
    void createUi();
    void setQtWidgetsBtns(QWidget *w);
    void qtWidgetsTestShow(int id);

    void windows_and_sub_widgets_test();
    void window_type_test();
    void window_geometry_test();
};
