#include "buttontest.h"
#include "core/font.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>

ButtonTestWidget::ButtonTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("ButtonTestWidget");
    setWindowTitle("Button Test");
    setFixedSize(1000, 600);

    QString style = R"(
        QWidget#ButtonTestWidget {
            background-color: #f5f5f5;
        }
    )";
    setStyleSheet(style);

    auto layout = new QVBoxLayout(this); 

    //
    auto widgetVectorButton = new QWidget(this);
    widgetVectorButton->setFixedHeight(48);

    layout->addWidget(widgetVectorButton);

    auto layoutwidgetVectorButton = new QHBoxLayout(widgetVectorButton); 
    layoutwidgetVectorButton->setAlignment(Qt::AlignVCenter);
    layoutwidgetVectorButton->setContentsMargins(0, 0, 0, 0);

    m_pBtn_VectorButton_HW28_I20 = new widget::VectorButton(this);
    m_pBtn_VectorButton_HW28_I20->setObjectName("VectorButton_HW28_I20");
    m_pBtn_VectorButton_HW28_I20->setCheckable(true);
    m_pBtn_VectorButton_HW28_I20->setFont(core::Font::currentIconFont());
    m_pBtn_VectorButton_HW28_I20->setText(QChar(0xe665));
    layoutwidgetVectorButton->addWidget(m_pBtn_VectorButton_HW28_I20, 0, Qt::AlignVCenter);

    m_pBtn_VectorButton_HW32_I20 = new widget::VectorButton(this);
    m_pBtn_VectorButton_HW32_I20->setObjectName("VectorButton_HW32_I20");
    m_pBtn_VectorButton_HW32_I20->setCheckable(true);
    m_pBtn_VectorButton_HW32_I20->setFont(core::Font::currentIconFont());
    m_pBtn_VectorButton_HW32_I20->setText(QChar(0xe665));
    layoutwidgetVectorButton->addWidget(m_pBtn_VectorButton_HW32_I20, 0, Qt::AlignVCenter);

    m_pBtn_VectorButton_HW36_I20 = new widget::VectorButton(this);
    m_pBtn_VectorButton_HW36_I20->setObjectName("VectorButton_HW36_I20");
    m_pBtn_VectorButton_HW36_I20->setCheckable(true);
    m_pBtn_VectorButton_HW36_I20->setFont(core::Font::currentIconFont());
    m_pBtn_VectorButton_HW36_I20->setText(QChar(0xe665));
    layoutwidgetVectorButton->addWidget(m_pBtn_VectorButton_HW36_I20, 0, Qt::AlignVCenter);

    m_pBtn_VectorButton_HW28_R8_I20_B_Bg = new widget::VectorButton(this);
    m_pBtn_VectorButton_HW28_R8_I20_B_Bg->setObjectName("VectorButton_HW28_R8_I20_B_Bg");
    m_pBtn_VectorButton_HW28_R8_I20_B_Bg->setCheckable(true);
    m_pBtn_VectorButton_HW28_R8_I20_B_Bg->setFont(core::Font::currentIconFont());
    m_pBtn_VectorButton_HW28_R8_I20_B_Bg->setText(QChar(0xe665));
    layoutwidgetVectorButton->addWidget(m_pBtn_VectorButton_HW28_R8_I20_B_Bg, 0, Qt::AlignVCenter);

    m_pBtn_VectorButton_HW32_R8_I20_B_Bg = new widget::VectorButton(this);
    m_pBtn_VectorButton_HW32_R8_I20_B_Bg->setObjectName("VectorButton_HW32_R8_I20_B_Bg");
    m_pBtn_VectorButton_HW32_R8_I20_B_Bg->setCheckable(true);
    m_pBtn_VectorButton_HW32_R8_I20_B_Bg->setFont(core::Font::currentIconFont());
    m_pBtn_VectorButton_HW32_R8_I20_B_Bg->setText(QChar(0xe665));
    layoutwidgetVectorButton->addWidget(m_pBtn_VectorButton_HW32_R8_I20_B_Bg, 0, Qt::AlignVCenter);

    m_pBtn_VectorButton_HW36_R8_I20_B_Bg = new widget::VectorButton(this);
    m_pBtn_VectorButton_HW36_R8_I20_B_Bg->setObjectName("VectorButton_HW36_R8_I20_B_Bg");
    m_pBtn_VectorButton_HW36_R8_I20_B_Bg->setCheckable(true);
    m_pBtn_VectorButton_HW36_R8_I20_B_Bg->setFont(core::Font::currentIconFont());
    m_pBtn_VectorButton_HW36_R8_I20_B_Bg->setText(QChar(0xe665));
    layoutwidgetVectorButton->addWidget(m_pBtn_VectorButton_HW36_R8_I20_B_Bg, 0, Qt::AlignVCenter);

    QButtonGroup *buttonGroupVectorButton = new QButtonGroup(this);
    buttonGroupVectorButton->setExclusive(true);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW28_I20, 0);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW32_I20, 1);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW36_I20, 2);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW28_R8_I20_B_Bg, 4);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW32_R8_I20_B_Bg, 5);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW36_R8_I20_B_Bg, 6);
    m_pBtn_VectorButton_HW36_R8_I20_B_Bg->setChecked(true);

    layoutwidgetVectorButton->addStretch();

    //
    auto widgetHorIconTextVectorButton = new QWidget(this);
    widgetHorIconTextVectorButton->setFixedHeight(48);

    layout->addWidget(widgetHorIconTextVectorButton);

    auto layoutHorIconTextVectorButton = new QHBoxLayout(widgetHorIconTextVectorButton);
    layoutHorIconTextVectorButton->setAlignment(Qt::AlignVCenter);
    layoutHorIconTextVectorButton->setContentsMargins(0, 0, 0, 0);

    m_pBtn_HorIconTextVectorButton_H28_I20_T14 = new widget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H28_I20_T14->setObjectName("HorIconTextVectorButton_H28_I20_T14");
    m_pBtn_HorIconTextVectorButton_H28_I20_T14->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H28_I20_T14->setIconFont(core::Font::currentIconFont());
    m_pBtn_HorIconTextVectorButton_H28_I20_T14->setIcon(QChar(0xe665));
    m_pBtn_HorIconTextVectorButton_H28_I20_T14->setText("我喜欢的");
    layoutHorIconTextVectorButton->addWidget(m_pBtn_HorIconTextVectorButton_H28_I20_T14, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextVectorButton_H32_I20_T14 = new widget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H32_I20_T14->setObjectName("HorIconTextVectorButton_H32_I20_T14");
    m_pBtn_HorIconTextVectorButton_H32_I20_T14->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H32_I20_T14->setIconFont(core::Font::currentIconFont());
    m_pBtn_HorIconTextVectorButton_H32_I20_T14->setIcon(QChar(0xe665));
    m_pBtn_HorIconTextVectorButton_H32_I20_T14->setText("本地下载");
    layoutHorIconTextVectorButton->addWidget(m_pBtn_HorIconTextVectorButton_H32_I20_T14, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextVectorButton_H36_I20_T14 = new widget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H36_I20_T14->setObjectName("HorIconTextVectorButton_H36_I20_T14");
    m_pBtn_HorIconTextVectorButton_H36_I20_T14->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H36_I20_T14->setIconFont(core::Font::currentIconFont());
    m_pBtn_HorIconTextVectorButton_H36_I20_T14->setIcon(QChar(0xe665));
    m_pBtn_HorIconTextVectorButton_H36_I20_T14->setText("最近播放呀呀");
    layoutHorIconTextVectorButton->addWidget(m_pBtn_HorIconTextVectorButton_H36_I20_T14, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg = new widget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg->setObjectName("HorIconTextVectorButton_H28_R8_I20_T14_Bg");
    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg->setIconFont(core::Font::currentIconFont());
    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg->setIcon(QChar(0xe665));
    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg->setText("音乐云盘");
    layoutHorIconTextVectorButton->addWidget(m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg = new widget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg->setObjectName("HorIconTextVectorButton_H32_R8_I20_T14_Bg");
    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg->setIconFont(core::Font::currentIconFont());
    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg->setIcon(QChar(0xe665));
    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg->setText("我的收藏");
    layoutHorIconTextVectorButton->addWidget(m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg = new widget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg->setObjectName("HorIconTextVectorButton_H36_R8_I20_T14_Bg");
    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg->setIconFont(core::Font::currentIconFont());
    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg->setIcon(QChar(0xe665));
    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg->setText("我的播客");
    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg->setAdjustWidth(false);
    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg->setFixedSize(140, 36);
    layoutHorIconTextVectorButton->addWidget(m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg, 0, Qt::AlignVCenter);

    QButtonGroup *buttonGroupHorIconTextVectorButton = new QButtonGroup(this);
    buttonGroupHorIconTextVectorButton->setExclusive(true);
    buttonGroupHorIconTextVectorButton->addButton(m_pBtn_HorIconTextVectorButton_H28_I20_T14, 0);
    buttonGroupHorIconTextVectorButton->addButton(m_pBtn_HorIconTextVectorButton_H32_I20_T14, 1);
    buttonGroupHorIconTextVectorButton->addButton(m_pBtn_HorIconTextVectorButton_H36_I20_T14, 2);
    buttonGroupHorIconTextVectorButton->addButton(m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg, 4);
    buttonGroupHorIconTextVectorButton->addButton(m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg, 5);
    buttonGroupHorIconTextVectorButton->addButton(m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg, 6);
    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg->setChecked(true);

    layoutHorIconTextVectorButton->addStretch();

    //
    auto widget3 = new QWidget(this);
    widget3->setFixedHeight(48);

    layout->addWidget(widget3);

    auto layout3 = new QHBoxLayout(widget3);
    layout3->setAlignment(Qt::AlignVCenter);
    layout3->setContentsMargins(0, 0, 0, 0);

    m_pBtn21 = new QPushButton(this);
    m_pBtn21->setObjectName("QPushButton_H28_R14_T14_B_Bg");
    m_pBtn21->setCheckable(true);
    m_pBtn21->setText("民谣");
    layout3->addWidget(m_pBtn21, 0, Qt::AlignVCenter);

    m_pBtn22 = new QPushButton(this);
    m_pBtn22->setObjectName("QPushButton_H32_R16_T14_B_Bg");
    m_pBtn22->setCheckable(true);
    m_pBtn22->setText("电子");
    layout3->addWidget(m_pBtn22, 0, Qt::AlignVCenter);

    m_pBtn23 = new QPushButton(this);
    m_pBtn23->setObjectName("QPushButton_H36_R18_T14_B_Bg");
    m_pBtn23->setCheckable(true);
    m_pBtn23->setText("轻音乐");
    layout3->addWidget(m_pBtn23, 0, Qt::AlignVCenter);

    layout3->addStretch();

    //
    auto widget4 = new QWidget(this);
    widget4->setFixedHeight(48);

    layout->addWidget(widget4);

    auto layout4 = new QHBoxLayout(widget4); 

    m_pBtn31 = new widget::IconButton(this);
    m_pBtn31->setObjectName("IconButton_HW24");
    m_pBtn31->setNormalPixmapPath(":/test/light/first-1-16.png");
    layout4->addWidget(m_pBtn31);

    m_pBtn32 = new widget::IconButton(this);
    m_pBtn32->setObjectName("IconButton_HW24_R6_Bg");
    m_pBtn32->setNormalPixmapPath(":/test/light/first-1-16.png");
    layout4->addWidget(m_pBtn32);

    layout4->addStretch();

    //
    auto widget5 = new QWidget(this);
    widget5->setFixedHeight(48);

    layout->addWidget(widget5);

    auto layout5 = new QHBoxLayout(widget5); 

    m_pBtn41 = new widget::HorIconTextButton(this);
    m_pBtn41->setObjectName("HorIconTextButton_I20");
    m_pBtn41->setNormalPixmapPath(":/test/light/first-1-16.png");
    m_pBtn41->setText("动态");
    layout5->addWidget(m_pBtn41);

    m_pBtn42 = new widget::HorIconTextButton(this);
    m_pBtn42->setObjectName("HorIconTextButton_I20_Bg");
    m_pBtn42->setNormalPixmapPath(":/test/light/first-1-16.png");
    m_pBtn42->setText("我的音乐网盘");
    layout5->addWidget(m_pBtn42);

    layout5->addStretch();

    //
    layout->addStretch();

    //
    QButtonGroup *buttonGroup21 = new QButtonGroup(this);
    buttonGroup21->addButton(m_pBtn21, 0);
    buttonGroup21->addButton(m_pBtn22, 1);
    buttonGroup21->addButton(m_pBtn23, 2);
}