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
    widget::VectorButton *m_pBtn_VectorButton_HW28_I20 = nullptr;
    widget::VectorButton *m_pBtn_VectorButton_HW32_I20 = nullptr;
    widget::VectorButton *m_pBtn_VectorButton_HW36_I20 = nullptr;
    widget::VectorButton *m_pBtn_VectorButton_HW28_R8_I20_B_Bg = nullptr;
    widget::VectorButton *m_pBtn_VectorButton_HW32_R8_I20_B_Bg = nullptr;
    widget::VectorButton *m_pBtn_VectorButton_HW36_R8_I20_B_Bg = nullptr;

    widget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H28_I20_T14 = nullptr;
    widget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H32_I20_T14 = nullptr;
    widget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H36_I20_T14 = nullptr;
    widget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg = nullptr;
    widget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg = nullptr;
    widget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg = nullptr;

    QPushButton *m_pBtn21 = nullptr;
    QPushButton *m_pBtn22 = nullptr;
    QPushButton *m_pBtn23 = nullptr;

    widget::IconButton *m_pBtn31 = nullptr;
    widget::IconButton *m_pBtn32 = nullptr;

    widget::HorIconTextButton *m_pBtn41 = nullptr;
    widget::HorIconTextButton *m_pBtn42 = nullptr;

    widget::BottomBorderButton *m_pBtn_BottomBorderButton1 = nullptr;
    widget::BottomBorderButton *m_pBtn_BottomBorderButton2 = nullptr;
    widget::BottomBorderButton *m_pBtn_BottomBorderButton3 = nullptr;
    widget::BottomBorderButton *m_pBtn_BottomBorderButton4 = nullptr;
    widget::BottomBorderButton *m_pBtn_BottomBorderButton5 = nullptr;
};
