#pragma once
#include <QDialog>
#include <QPushButton>
#include "translucentbackground.h"

class Dialog : public QDialog {
    Q_OBJECT

public:
    Dialog(QDialog *parent = nullptr);
    ~Dialog();

private:
    void createUi();
    void sigConnect();
    void translucentShow();

private:
    QPushButton *translucent_btn_ = nullptr;
    TranslucentBackgroundWidget *translucent_widget_ = nullptr;
};
