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

    QPushButton *m_pQPushButton_H28_R14_T14_B_Bg = nullptr;
    QPushButton *m_pQPushButton_H32_R16_T14_B_Bg = nullptr;
    QPushButton *m_pQPushButton_H36_R18_T14_B_Bg = nullptr;

    widget::IconButton *m_pIconButton_HW28 = nullptr;
    widget::IconButton *m_pIconButton_HW32 = nullptr;
    widget::IconButton *m_pIconButton_HW36 = nullptr;
    widget::IconButton *m_pIconButton_HW28_R8_B_Bg = nullptr;
    widget::IconButton *m_pIconButton_HW32_R8_B_Bg = nullptr;
    widget::IconButton *m_pIconButton_HW36_R8_B_Bg = nullptr;

    widget::HorIconTextButton *m_pBtn_HorIconTextButton_H28_T14 = nullptr;
    widget::HorIconTextButton *m_pBtn_HorIconTextButton_H32_T14 = nullptr;
    widget::HorIconTextButton *m_pBtn_HorIconTextButton_H36_T14 = nullptr;
    widget::HorIconTextButton *m_pBtn_HorIconTextButton_H28_R8_T14_Bg = nullptr;
    widget::HorIconTextButton *m_pBtn_HorIconTextButton_H32_R8_T14_Bg = nullptr;
    widget::HorIconTextButton *m_pBtn_HorIconTextButton_H36_R8_T14_Bg = nullptr;

    widget::BottomBorderButton *m_pBtn_BottomBorderButton1 = nullptr;
    widget::BottomBorderButton *m_pBtn_BottomBorderButton2 = nullptr;
    widget::BottomBorderButton *m_pBtn_BottomBorderButton3 = nullptr;
    widget::BottomBorderButton *m_pBtn_BottomBorderButton4 = nullptr;
    widget::BottomBorderButton *m_pBtn_BottomBorderButton5 = nullptr;
};
