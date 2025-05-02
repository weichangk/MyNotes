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