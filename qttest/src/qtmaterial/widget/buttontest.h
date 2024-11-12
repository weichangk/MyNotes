#pragma once
#include "widget/button.h"

#include <QWidget>
#include <QPushButton>

class ButtonTestWidget : public QWidget {
    Q_OBJECT

public:
    ButtonTestWidget(QWidget *parent = nullptr);
    ~ButtonTestWidget() override {}

private:
    widget::VectorButton *m_pBtn1 = nullptr;
    widget::VectorButton *m_pBtn2 = nullptr;

    widget::HorIconTextVectorButton *m_pBtn11 = nullptr;
    widget::HorIconTextVectorButton *m_pBtn12 = nullptr;
    widget::HorIconTextVectorButton *m_pBtn13 = nullptr;
    widget::HorIconTextVectorButton *m_pBtn14 = nullptr;
    widget::HorIconTextVectorButton *m_pBtn15 = nullptr;
    widget::HorIconTextVectorButton *m_pBtn16 = nullptr;

    QPushButton *m_pBtn21 = nullptr;
    QPushButton *m_pBtn22 = nullptr;
    QPushButton *m_pBtn23 = nullptr;
};
