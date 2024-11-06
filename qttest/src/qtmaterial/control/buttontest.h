#pragma once
#include <QWidget>
#include "widget/button.h"

class ButtonTestWidget : public QWidget {
    Q_OBJECT

public:
    ButtonTestWidget(QWidget *parent = nullptr);
    ~ButtonTestWidget() override {}

private:
    widget::IconVectorButton *m_pBtn1 = nullptr;
    widget::IconVectorButton *m_pBtn2 = nullptr;
    widget::IconVectorButton *m_pBtn3 = nullptr;
    widget::IconVectorButton *m_pBtn4 = nullptr;
    widget::IconVectorButton *m_pBtn5 = nullptr;
    widget::IconVectorButton *m_pBtn6 = nullptr;
};
