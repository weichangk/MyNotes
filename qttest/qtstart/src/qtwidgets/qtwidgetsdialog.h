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

private:

};
