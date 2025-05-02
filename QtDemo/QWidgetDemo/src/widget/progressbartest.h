#pragma once
#include "widget/progressbar.h"

#include <QWidget>
#include <QProgressBar>

class ProgressBarTestWidget : public QWidget {
    Q_OBJECT

public:
    ProgressBarTestWidget(QWidget *parent = nullptr);
    ~ProgressBarTestWidget() override {
    }

private:
    QProgressBar *m_pProgressBar = nullptr;
    qtmaterialwidget::LoopProgressBar *m_pProgressBar1 = nullptr;
};