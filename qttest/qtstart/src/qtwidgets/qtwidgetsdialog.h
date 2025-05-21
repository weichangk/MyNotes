#pragma once
#include <QDialog>

class QtWidgetsDialog : public QDialog {
    Q_OBJECT

public:
    QtWidgetsDialog(QWidget *parent = nullptr);
    ~QtWidgetsDialog();

private:
    void createUi();
    void setQtWidgetsBtns(QWidget *w);
    void qtWidgetsTestShow(int id);
    
    void windows_and_sub_widgets_test();
};
