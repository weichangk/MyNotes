#pragma once

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
    void qtWidgetsDialogShow();
};
