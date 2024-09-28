/*
 * @Author: weick
 * @Date: 2024-08-06 00:56:04
 * @Last Modified by: weick
 * @Last Modified time: 2024-08-06 00:56:46
 */

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
