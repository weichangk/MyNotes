#pragma once
#include <QWidget>
#include <QPushButton>

class LayoutMacBugWidget : public QWidget {
    Q_OBJECT

public:
    LayoutMacBugWidget(QWidget *parent = nullptr);
    ~LayoutMacBugWidget();

private:
    void createUi();
    void sigConnect();

private:
    QPushButton *btn1;
    QPushButton *btn2;
    QPushButton *btn3;

    QPushButton *btn4;
    QPushButton *btn5;
    QPushButton *btn6;
};