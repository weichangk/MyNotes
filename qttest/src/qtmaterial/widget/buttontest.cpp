#include "buttontest.h"
#include "core/font.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>

ButtonTestWidget::ButtonTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("ButtonTestWidget");
    setWindowTitle("Button Test");
    setFixedSize(800, 600);

    QString style = R"(
        QWidget#ButtonTestWidget {
            background-color: #f5f5f5;
        }
    )";
    setStyleSheet(style);

    auto layout = new QVBoxLayout(this); 

    auto widget1 = new QWidget(this);
    widget1->setFixedHeight(48);

    layout->addWidget(widget1);

    auto layout1 = new QHBoxLayout(widget1); 

    m_pBtn1 = new widget::VectorButton(this);
    m_pBtn1->setObjectName("VectorButton_I20");
    m_pBtn1->setFont(core::Font::currentIconFont());
    m_pBtn1->setText(QChar(0xe665));
    layout1->addWidget(m_pBtn1);

    m_pBtn2 = new widget::VectorButton(this);
    m_pBtn2->setObjectName("VectorButton_HW36_R8_I20_Bg");
    m_pBtn2->setFont(core::Font::currentIconFont());
    m_pBtn2->setText(QChar(0xe665));
    layout1->addWidget(m_pBtn2);

    layout1->addStretch();

    auto widget2 = new QWidget(this);
    widget2->setFixedHeight(48);

    layout->addWidget(widget2);

    auto layout2 = new QHBoxLayout(widget2);  

    m_pBtn11 = new widget::HorIconTextVectorButton(this);
    m_pBtn11->setObjectName("HorIconTextVectorButton_H36_I20_T14");
    m_pBtn11->setIconFont(core::Font::currentIconFont());
    m_pBtn11->setIcon(QChar(0xe665));
    m_pBtn11->setText("我喜欢的");
    layout2->addWidget(m_pBtn11);

    m_pBtn12 = new widget::HorIconTextVectorButton(this);
    m_pBtn12->setObjectName("HorIconTextVectorButton_H36_R8_I20_T14_Bg");
    m_pBtn12->setIconFont(core::Font::currentIconFont());
    m_pBtn12->setIcon(QChar(0xe665));
    m_pBtn12->setText("本地下载");
    layout2->addWidget(m_pBtn12);

    m_pBtn13 = new widget::HorIconTextVectorButton(this);
    m_pBtn13->setObjectName("HorIconTextVectorButton_H36_R8_I20_T14_Bg");
    m_pBtn13->setCheckable(true);
    m_pBtn13->setIconFont(core::Font::currentIconFont());
    m_pBtn13->setIcon(QChar(0xe665));
    m_pBtn13->setText("最近播放");
    layout2->addWidget(m_pBtn13);

    m_pBtn14 = new widget::HorIconTextVectorButton(this);
    m_pBtn14->setObjectName("HorIconTextVectorButton_H36_R8_I20_T14_Bg");
    m_pBtn14->setCheckable(true);
    m_pBtn14->setIconFont(core::Font::currentIconFont());
    m_pBtn14->setIcon(QChar(0xe665));
    m_pBtn14->setText("音乐云盘");
    layout2->addWidget(m_pBtn14);

    m_pBtn15 = new widget::HorIconTextVectorButton(this);
    m_pBtn15->setObjectName("HorIconTextVectorButton_H36_R8_I20_T14_Bg");
    m_pBtn15->setCheckable(true);
    m_pBtn15->setIconFont(core::Font::currentIconFont());
    m_pBtn15->setIcon(QChar(0xe665));
    m_pBtn15->setText("我的收藏");
    layout2->addWidget(m_pBtn15);

    m_pBtn16 = new widget::HorIconTextVectorButton(this);
    m_pBtn16->setObjectName("HorIconTextVectorButton_H36_R8_I20_T14_Bg");
    m_pBtn16->setCheckable(true);
    m_pBtn16->setIconFont(core::Font::currentIconFont());
    m_pBtn16->setIcon(QChar(0xe665));
    m_pBtn16->setText("我的播客");
    layout2->addWidget(m_pBtn16);

    layout2->addStretch();

    layout->addStretch();

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);
    buttonGroup->addButton(m_pBtn13, 0);
    buttonGroup->addButton(m_pBtn14, 1);
    buttonGroup->addButton(m_pBtn15, 2);
    buttonGroup->addButton(m_pBtn16, 3);

    m_pBtn13->setChecked(true);
    m_pBtn13->slotWidgetStateChecked(true);

    bool b = connect(buttonGroup, &QButtonGroup::idToggled, this, [&](int id, bool checked) {
        if (checked) {
            m_pBtn13->slotWidgetStateChecked(id == 0);
            m_pBtn14->slotWidgetStateChecked(id == 1);
            m_pBtn15->slotWidgetStateChecked(id == 2);
            m_pBtn16->slotWidgetStateChecked(id == 3);
        }
    });
}