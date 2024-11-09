#pragma once
#include <QWidget>
#include "widget/button.h"

class ButtonTestWidget : public QWidget {
    Q_OBJECT

public:
    ButtonTestWidget(QWidget *parent = nullptr);
    ~ButtonTestWidget() override {}

private:
    widget::VectorButton *m_pBtn1 = nullptr;
    widget::VectorButton *m_pBtn2 = nullptr;
    widget::VectorButton *m_pBtn3 = nullptr;
    widget::VectorButton *m_pBtn4 = nullptr;
    widget::VectorButton *m_pBtn5 = nullptr;
    widget::VectorButton *m_pBtn6 = nullptr;
};
