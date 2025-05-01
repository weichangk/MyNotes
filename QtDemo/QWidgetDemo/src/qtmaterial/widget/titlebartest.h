#pragma once
#include "widget/titlebar.h"

#include <QWidget>

class TitlebarTestWidget : public QWidget {
    Q_OBJECT

public:
    explicit TitlebarTestWidget(QWidget *parent = nullptr);
    ~TitlebarTestWidget() override {
    }

private:
    qtmaterialwidget::Titlebar *m_pTitlebar = nullptr;
};