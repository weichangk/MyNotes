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
    void modal_dialog_test();
    void window_switch_test();
    void standard_dialog_test();
    void frame_use_test();
    void frame_family_test();
};
