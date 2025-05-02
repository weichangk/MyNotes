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
    qtmaterialwidget::VectorButton *m_pBtn_VectorButton_HW28_I20 = nullptr;
    qtmaterialwidget::VectorButton *m_pBtn_VectorButton_HW32_I20 = nullptr;
    qtmaterialwidget::VectorButton *m_pBtn_VectorButton_HW36_I20 = nullptr;
    qtmaterialwidget::VectorButton *m_pBtn_VectorButton_HW28_R8_I20_B_Bg = nullptr;
    qtmaterialwidget::VectorButton *m_pBtn_VectorButton_HW32_R8_I20_B_Bg = nullptr;
    qtmaterialwidget::VectorButton *m_pBtn_VectorButton_HW36_R8_I20_B_Bg = nullptr;

    qtmaterialwidget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H28_I20_T14 = nullptr;
    qtmaterialwidget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H32_I20_T14 = nullptr;
    qtmaterialwidget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H36_I20_T14 = nullptr;
    qtmaterialwidget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg = nullptr;
    qtmaterialwidget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg = nullptr;
    qtmaterialwidget::HorIconTextVectorButton *m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg = nullptr;

    QPushButton *m_pQPushButton_H28_R14_T14_B_Bg = nullptr;
    QPushButton *m_pQPushButton_H32_R16_T14_B_Bg = nullptr;
    QPushButton *m_pQPushButton_H36_R18_T14_B_Bg = nullptr;

    qtmaterialwidget::IconButton *m_pIconButton_HW28 = nullptr;
    qtmaterialwidget::IconButton *m_pIconButton_HW32 = nullptr;
    qtmaterialwidget::IconButton *m_pIconButton_HW36 = nullptr;
    qtmaterialwidget::IconButton *m_pIconButton_HW28_R8_B_Bg = nullptr;
    qtmaterialwidget::IconButton *m_pIconButton_HW32_R8_B_Bg = nullptr;
    qtmaterialwidget::IconButton *m_pIconButton_HW36_R8_B_Bg = nullptr;

    qtmaterialwidget::HorIconTextButton *m_pBtn_HorIconTextButton_H28_T14 = nullptr;
    qtmaterialwidget::HorIconTextButton *m_pBtn_HorIconTextButton_H32_T14 = nullptr;
    qtmaterialwidget::HorIconTextButton *m_pBtn_HorIconTextButton_H36_T14 = nullptr;
    qtmaterialwidget::HorIconTextButton *m_pBtn_HorIconTextButton_H28_R8_T14_Bg = nullptr;
    qtmaterialwidget::HorIconTextButton *m_pBtn_HorIconTextButton_H32_R8_T14_Bg = nullptr;
    qtmaterialwidget::HorIconTextButton *m_pBtn_HorIconTextButton_H36_R8_T14_Bg = nullptr;

    qtmaterialwidget::BottomBorderButton *m_pBtn_BottomBorderButton1 = nullptr;
    qtmaterialwidget::BottomBorderButton *m_pBtn_BottomBorderButton2 = nullptr;
    qtmaterialwidget::BottomBorderButton *m_pBtn_BottomBorderButton3 = nullptr;
    qtmaterialwidget::BottomBorderButton *m_pBtn_BottomBorderButton4 = nullptr;
    qtmaterialwidget::BottomBorderButton *m_pBtn_BottomBorderButton5 = nullptr;
};
