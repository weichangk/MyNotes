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

   QFont iconFont = qtmaterialcore::Font::getIconFont(":/font/QtMaterialIconFont.ttf");

    //
    auto widgetVectorButton = new QWidget(this);
    widgetVectorButton->setFixedHeight(48);

    layout->addWidget(widgetVectorButton);

    auto layoutVectorButton = new QHBoxLayout(widgetVectorButton); 
    layoutVectorButton->setAlignment(Qt::AlignVCenter);
    layoutVectorButton->setContentsMargins(0, 0, 0, 0);

    m_pBtn_VectorButton_HW28_I20 = new qtmaterialwidget::VectorButton(this);
    m_pBtn_VectorButton_HW28_I20->setObjectName("VectorButton_HW28_I20");
    m_pBtn_VectorButton_HW28_I20->setCheckable(true);
    m_pBtn_VectorButton_HW28_I20->setFont(iconFont);
    m_pBtn_VectorButton_HW28_I20->setText(QChar(0xe665));
    layoutVectorButton->addWidget(m_pBtn_VectorButton_HW28_I20, 0, Qt::AlignVCenter);

    m_pBtn_VectorButton_HW32_I20 = new qtmaterialwidget::VectorButton(this);
    m_pBtn_VectorButton_HW32_I20->setObjectName("VectorButton_HW32_I20");
    m_pBtn_VectorButton_HW32_I20->setCheckable(true);
    m_pBtn_VectorButton_HW32_I20->setFont(iconFont);
    m_pBtn_VectorButton_HW32_I20->setText(QChar(0xe665));
    layoutVectorButton->addWidget(m_pBtn_VectorButton_HW32_I20, 0, Qt::AlignVCenter);

    m_pBtn_VectorButton_HW36_I20 = new qtmaterialwidget::VectorButton(this);
    m_pBtn_VectorButton_HW36_I20->setObjectName("VectorButton_HW36_I20");
    m_pBtn_VectorButton_HW36_I20->setCheckable(true);
    m_pBtn_VectorButton_HW36_I20->setFont(iconFont);
    m_pBtn_VectorButton_HW36_I20->setText(QChar(0xe665));
    layoutVectorButton->addWidget(m_pBtn_VectorButton_HW36_I20, 0, Qt::AlignVCenter);

    m_pBtn_VectorButton_HW28_R8_I20_B_Bg = new qtmaterialwidget::VectorButton(this);
    m_pBtn_VectorButton_HW28_R8_I20_B_Bg->setObjectName("VectorButton_HW28_R8_I20_B_Bg");
    m_pBtn_VectorButton_HW28_R8_I20_B_Bg->setCheckable(true);
    m_pBtn_VectorButton_HW28_R8_I20_B_Bg->setFont(iconFont);
    m_pBtn_VectorButton_HW28_R8_I20_B_Bg->setText(QChar(0xe665));
    layoutVectorButton->addWidget(m_pBtn_VectorButton_HW28_R8_I20_B_Bg, 0, Qt::AlignVCenter);

    m_pBtn_VectorButton_HW32_R8_I20_B_Bg = new qtmaterialwidget::VectorButton(this);
    m_pBtn_VectorButton_HW32_R8_I20_B_Bg->setObjectName("VectorButton_HW32_R8_I20_B_Bg");
    m_pBtn_VectorButton_HW32_R8_I20_B_Bg->setCheckable(true);
    m_pBtn_VectorButton_HW32_R8_I20_B_Bg->setFont(iconFont);
    m_pBtn_VectorButton_HW32_R8_I20_B_Bg->setText(QChar(0xe665));
    layoutVectorButton->addWidget(m_pBtn_VectorButton_HW32_R8_I20_B_Bg, 0, Qt::AlignVCenter);

    m_pBtn_VectorButton_HW36_R8_I20_B_Bg = new qtmaterialwidget::VectorButton(this);
    m_pBtn_VectorButton_HW36_R8_I20_B_Bg->setObjectName("VectorButton_HW36_R8_I20_B_Bg");
    m_pBtn_VectorButton_HW36_R8_I20_B_Bg->setCheckable(true);
    m_pBtn_VectorButton_HW36_R8_I20_B_Bg->setFont(iconFont);
    m_pBtn_VectorButton_HW36_R8_I20_B_Bg->setText(QChar(0xe665));
    layoutVectorButton->addWidget(m_pBtn_VectorButton_HW36_R8_I20_B_Bg, 0, Qt::AlignVCenter);

    QButtonGroup *buttonGroupVectorButton = new QButtonGroup(this);
    buttonGroupVectorButton->setExclusive(true);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW28_I20, 0);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW32_I20, 1);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW36_I20, 2);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW28_R8_I20_B_Bg, 4);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW32_R8_I20_B_Bg, 5);
    buttonGroupVectorButton->addButton(m_pBtn_VectorButton_HW36_R8_I20_B_Bg, 6);
    m_pBtn_VectorButton_HW36_R8_I20_B_Bg->setChecked(true);

    layoutVectorButton->addStretch();

    //
    auto widgetHorIconTextVectorButton = new QWidget(this);
    widgetHorIconTextVectorButton->setFixedHeight(48);

    layout->addWidget(widgetHorIconTextVectorButton);

    auto layoutHorIconTextVectorButton = new QHBoxLayout(widgetHorIconTextVectorButton);
    layoutHorIconTextVectorButton->setAlignment(Qt::AlignVCenter);
    layoutHorIconTextVectorButton->setContentsMargins(0, 0, 0, 0);

    m_pBtn_HorIconTextVectorButton_H28_I20_T14 = new qtmaterialwidget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H28_I20_T14->setObjectName("HorIconTextVectorButton_H28_I20_T14");
    m_pBtn_HorIconTextVectorButton_H28_I20_T14->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H28_I20_T14->setIconFont(iconFont);
    m_pBtn_HorIconTextVectorButton_H28_I20_T14->setIcon(QChar(0xe665));
    m_pBtn_HorIconTextVectorButton_H28_I20_T14->setText("我喜欢的");
    layoutHorIconTextVectorButton->addWidget(m_pBtn_HorIconTextVectorButton_H28_I20_T14, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextVectorButton_H32_I20_T14 = new qtmaterialwidget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H32_I20_T14->setObjectName("HorIconTextVectorButton_H32_I20_T14");
    m_pBtn_HorIconTextVectorButton_H32_I20_T14->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H32_I20_T14->setIconFont(iconFont);
    m_pBtn_HorIconTextVectorButton_H32_I20_T14->setIcon(QChar(0xe665));
    m_pBtn_HorIconTextVectorButton_H32_I20_T14->setText("本地下载");
    layoutHorIconTextVectorButton->addWidget(m_pBtn_HorIconTextVectorButton_H32_I20_T14, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextVectorButton_H36_I20_T14 = new qtmaterialwidget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H36_I20_T14->setObjectName("HorIconTextVectorButton_H36_I20_T14");
    m_pBtn_HorIconTextVectorButton_H36_I20_T14->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H36_I20_T14->setIconFont(iconFont);
    m_pBtn_HorIconTextVectorButton_H36_I20_T14->setIcon(QChar(0xe665));
    m_pBtn_HorIconTextVectorButton_H36_I20_T14->setText("最近播放呀呀");
    layoutHorIconTextVectorButton->addWidget(m_pBtn_HorIconTextVectorButton_H36_I20_T14, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg = new qtmaterialwidget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg->setObjectName("HorIconTextVectorButton_H28_R8_I20_T14_Bg");
    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg->setIconFont(iconFont);
    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg->setIcon(QChar(0xe665));
    m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg->setText("音乐云盘");
    layoutHorIconTextVectorButton->addWidget(m_pBtn_HorIconTextVectorButton_H28_R8_I20_T14_Bg, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg = new qtmaterialwidget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg->setObjectName("HorIconTextVectorButton_H32_R8_I20_T14_Bg");
    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg->setIconFont(iconFont);
    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg->setIcon(QChar(0xe665));
    m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg->setText("我的收藏");
    layoutHorIconTextVectorButton->addWidget(m_pBtn_HorIconTextVectorButton_H32_R8_I20_T14_Bg, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg = new qtmaterialwidget::HorIconTextVectorButton(this);
    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg->setObjectName("HorIconTextVectorButton_H36_R8_I20_T14_Bg");
    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg->setCheckable(true);
    m_pBtn_HorIconTextVectorButton_H36_R8_I20_T14_Bg->setIconFont(iconFont);
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
    auto widgetQPushButton = new QWidget(this);
    widgetQPushButton->setFixedHeight(48);

    layout->addWidget(widgetQPushButton);

    auto layoutQPushButton = new QHBoxLayout(widgetQPushButton);
    layoutQPushButton->setAlignment(Qt::AlignVCenter);
    layoutQPushButton->setContentsMargins(0, 0, 0, 0);

    m_pQPushButton_H28_R14_T14_B_Bg = new QPushButton(this);
    m_pQPushButton_H28_R14_T14_B_Bg->setObjectName("QPushButton_H28_R14_T14_B_Bg");
    m_pQPushButton_H28_R14_T14_B_Bg->setCheckable(true);
    m_pQPushButton_H28_R14_T14_B_Bg->setText("民谣");
    layoutQPushButton->addWidget(m_pQPushButton_H28_R14_T14_B_Bg, 0, Qt::AlignVCenter);

    m_pQPushButton_H32_R16_T14_B_Bg = new QPushButton(this);
    m_pQPushButton_H32_R16_T14_B_Bg->setObjectName("QPushButton_H32_R16_T14_B_Bg");
    m_pQPushButton_H32_R16_T14_B_Bg->setCheckable(true);
    m_pQPushButton_H32_R16_T14_B_Bg->setText("电子");
    layoutQPushButton->addWidget(m_pQPushButton_H32_R16_T14_B_Bg, 0, Qt::AlignVCenter);

    m_pQPushButton_H36_R18_T14_B_Bg = new QPushButton(this);
    m_pQPushButton_H36_R18_T14_B_Bg->setObjectName("QPushButton_H36_R18_T14_B_Bg");
    m_pQPushButton_H36_R18_T14_B_Bg->setCheckable(true);
    m_pQPushButton_H36_R18_T14_B_Bg->setText("轻音乐");
    layoutQPushButton->addWidget(m_pQPushButton_H36_R18_T14_B_Bg, 0, Qt::AlignVCenter);

    QButtonGroup *buttonGroup21 = new QButtonGroup(this);
    buttonGroup21->addButton(m_pQPushButton_H28_R14_T14_B_Bg, 0);
    buttonGroup21->addButton(m_pQPushButton_H32_R16_T14_B_Bg, 1);
    buttonGroup21->addButton(m_pQPushButton_H36_R18_T14_B_Bg, 2);

    layoutQPushButton->addStretch();

    //
    auto widgetIconButton = new QWidget(this);
    widgetIconButton->setFixedHeight(48);

    layout->addWidget(widgetIconButton);

    auto layoutIconButton = new QHBoxLayout(widgetIconButton); 
    layoutIconButton->setAlignment(Qt::AlignVCenter);
    layoutIconButton->setContentsMargins(0, 0, 0, 0);

    m_pIconButton_HW28 = new qtmaterialwidget::IconButton(this);
    m_pIconButton_HW28->setObjectName("IconButton_HW28");
    m_pIconButton_HW28->setIconSize(20);
    m_pIconButton_HW28->setNormalPixmapPath(":/qtmaterial/test/test20.png");
    layoutIconButton->addWidget(m_pIconButton_HW28, 0, Qt::AlignVCenter);

    m_pIconButton_HW32 = new qtmaterialwidget::IconButton(this);
    m_pIconButton_HW32->setObjectName("IconButton_HW32");
    m_pIconButton_HW32->setIconSize(20);
    m_pIconButton_HW32->setNormalPixmapPath(":/qtmaterial/test/test20.png");
    layoutIconButton->addWidget(m_pIconButton_HW32, 0, Qt::AlignVCenter);

    m_pIconButton_HW36 = new qtmaterialwidget::IconButton(this);
    m_pIconButton_HW36->setObjectName("IconButton_HW36");
    m_pIconButton_HW36->setIconSize(20);
    m_pIconButton_HW36->setNormalPixmapPath(":/qtmaterial/test/test20.png");
    layoutIconButton->addWidget(m_pIconButton_HW36, 0, Qt::AlignVCenter);

    m_pIconButton_HW28_R8_B_Bg = new qtmaterialwidget::IconButton(this);
    m_pIconButton_HW28_R8_B_Bg->setObjectName("IconButton_HW28_R8_B_Bg");
    m_pIconButton_HW28_R8_B_Bg->setIconSize(20);
    m_pIconButton_HW28_R8_B_Bg->setNormalPixmapPath(":/qtmaterial/test/test20.png");
    layoutIconButton->addWidget(m_pIconButton_HW28_R8_B_Bg, 0, Qt::AlignVCenter);

    m_pIconButton_HW32_R8_B_Bg = new qtmaterialwidget::IconButton(this);
    m_pIconButton_HW32_R8_B_Bg->setObjectName("IconButton_HW32_R8_B_Bg");
    m_pIconButton_HW32_R8_B_Bg->setIconSize(20);
    m_pIconButton_HW32_R8_B_Bg->setNormalPixmapPath(":/qtmaterial/test/test20.png");
    layoutIconButton->addWidget(m_pIconButton_HW32_R8_B_Bg, 0, Qt::AlignVCenter);

    m_pIconButton_HW32_R8_B_Bg = new qtmaterialwidget::IconButton(this);
    m_pIconButton_HW32_R8_B_Bg->setObjectName("IconButton_HW36_R8_B_Bg");
    m_pIconButton_HW32_R8_B_Bg->setIconSize(20);
    m_pIconButton_HW32_R8_B_Bg->setNormalPixmapPath(":/qtmaterial/test/test20.png");
    layoutIconButton->addWidget(m_pIconButton_HW32_R8_B_Bg, 0, Qt::AlignVCenter);

    layoutIconButton->addStretch();

    //
    auto widgetHorIconTextButton = new QWidget(this);
    widgetHorIconTextButton->setFixedHeight(48);

    layout->addWidget(widgetHorIconTextButton);

    auto layoutHorIconTextButton = new QHBoxLayout(widgetHorIconTextButton);
    layoutHorIconTextButton->setAlignment(Qt::AlignVCenter);
    layoutHorIconTextButton->setContentsMargins(0, 0, 0, 0);

    m_pBtn_HorIconTextButton_H28_T14 = new qtmaterialwidget::HorIconTextButton(this);
    m_pBtn_HorIconTextButton_H28_T14->setObjectName("HorIconTextButton_H28_T14");
    m_pBtn_HorIconTextButton_H28_T14->setIconSize(20);
    m_pBtn_HorIconTextButton_H28_T14->setCheckable(true);
    m_pBtn_HorIconTextButton_H28_T14->setNormalPixmapPath(":/qtmaterial/test/test20.png");
    m_pBtn_HorIconTextButton_H28_T14->setText("我喜欢的");
    layoutHorIconTextButton->addWidget(m_pBtn_HorIconTextButton_H28_T14, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextButton_H32_T14 = new qtmaterialwidget::HorIconTextButton(this);
    m_pBtn_HorIconTextButton_H32_T14->setObjectName("HorIconTextButton_H32_T14");
    m_pBtn_HorIconTextButton_H32_T14->setIconSize(20);
    m_pBtn_HorIconTextButton_H32_T14->setCheckable(true);
    m_pBtn_HorIconTextButton_H32_T14->setNormalPixmapPath(":/qtmaterial/test/test20.png");
    m_pBtn_HorIconTextButton_H32_T14->setText("本地下载");
    layoutHorIconTextButton->addWidget(m_pBtn_HorIconTextButton_H32_T14, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextButton_H36_T14 = new qtmaterialwidget::HorIconTextButton(this);
    m_pBtn_HorIconTextButton_H36_T14->setObjectName("HorIconTextButton_H36_T14");
    m_pBtn_HorIconTextButton_H36_T14->setIconSize(20);
    m_pBtn_HorIconTextButton_H36_T14->setCheckable(true);
    m_pBtn_HorIconTextButton_H36_T14->setNormalPixmapPath(":/qtmaterial/test/test20.png");
    m_pBtn_HorIconTextButton_H36_T14->setText("最近播放呀呀");
    layoutHorIconTextButton->addWidget(m_pBtn_HorIconTextButton_H36_T14, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextButton_H28_R8_T14_Bg = new qtmaterialwidget::HorIconTextButton(this);
    m_pBtn_HorIconTextButton_H28_R8_T14_Bg->setObjectName("HorIconTextButton_H28_R8_T14_Bg");
    m_pBtn_HorIconTextButton_H28_R8_T14_Bg->setIconSize(20);
    m_pBtn_HorIconTextButton_H28_R8_T14_Bg->setCheckable(true);
    m_pBtn_HorIconTextButton_H28_R8_T14_Bg->setNormalPixmapPath(":/qtmaterial/test/test20.png");;
    m_pBtn_HorIconTextButton_H28_R8_T14_Bg->setText("音乐云盘");
    layoutHorIconTextButton->addWidget(m_pBtn_HorIconTextButton_H28_R8_T14_Bg, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextButton_H32_R8_T14_Bg = new qtmaterialwidget::HorIconTextButton(this);
    m_pBtn_HorIconTextButton_H32_R8_T14_Bg->setObjectName("HorIconTextButton_H32_R8_T14_Bg");
    m_pBtn_HorIconTextButton_H32_R8_T14_Bg->setIconSize(20);
    m_pBtn_HorIconTextButton_H32_R8_T14_Bg->setCheckable(true);
    m_pBtn_HorIconTextButton_H32_R8_T14_Bg->setNormalPixmapPath(":/qtmaterial/test/test20.png");
    m_pBtn_HorIconTextButton_H32_R8_T14_Bg->setText("我的收藏");
    layoutHorIconTextButton->addWidget(m_pBtn_HorIconTextButton_H32_R8_T14_Bg, 0, Qt::AlignVCenter);

    m_pBtn_HorIconTextButton_H36_R8_T14_Bg = new qtmaterialwidget::HorIconTextButton(this);
    m_pBtn_HorIconTextButton_H36_R8_T14_Bg->setObjectName("HorIconTextButton_H36_R8_T14_Bg");
    m_pBtn_HorIconTextButton_H36_R8_T14_Bg->setIconSize(20);
    m_pBtn_HorIconTextButton_H36_R8_T14_Bg->setCheckable(true);
    m_pBtn_HorIconTextButton_H36_R8_T14_Bg->setNormalPixmapPath(":/qtmaterial/test/test20.png");
    m_pBtn_HorIconTextButton_H36_R8_T14_Bg->setText("我的播客");
    m_pBtn_HorIconTextButton_H36_R8_T14_Bg->setAdjustWidth(false);
    m_pBtn_HorIconTextButton_H36_R8_T14_Bg->setFixedSize(140, 36);
    layoutHorIconTextButton->addWidget(m_pBtn_HorIconTextButton_H36_R8_T14_Bg, 0, Qt::AlignVCenter);

    QButtonGroup *buttonGroupHorIconTextButton = new QButtonGroup(this);
    buttonGroupHorIconTextButton->setExclusive(true);
    buttonGroupHorIconTextButton->addButton(m_pBtn_HorIconTextButton_H28_T14, 0);
    buttonGroupHorIconTextButton->addButton(m_pBtn_HorIconTextButton_H32_T14, 1);
    buttonGroupHorIconTextButton->addButton(m_pBtn_HorIconTextButton_H36_T14, 2);
    buttonGroupHorIconTextButton->addButton(m_pBtn_HorIconTextButton_H28_R8_T14_Bg, 4);
    buttonGroupHorIconTextButton->addButton(m_pBtn_HorIconTextButton_H32_R8_T14_Bg, 5);
    buttonGroupHorIconTextButton->addButton(m_pBtn_HorIconTextButton_H36_R8_T14_Bg, 6);
    m_pBtn_HorIconTextButton_H36_R8_T14_Bg->setChecked(true);

    layoutHorIconTextButton->addStretch();

    //
    auto widgetBottomBorderButton = new QWidget(this);
    widgetBottomBorderButton->setFixedHeight(48);

    layout->addWidget(widgetBottomBorderButton);

    auto layoutBottomBorderButton = new QHBoxLayout(widgetBottomBorderButton);
    layoutBottomBorderButton->setAlignment(Qt::AlignVCenter);
    layoutBottomBorderButton->setSpacing(16);
    layoutBottomBorderButton->setContentsMargins(0, 0, 0, 0);

    m_pBtn_BottomBorderButton1 = new qtmaterialwidget::BottomBorderButton(this);
    m_pBtn_BottomBorderButton1->setObjectName("BottomBorderButton_H28_T16");
    m_pBtn_BottomBorderButton1->setCheckable(true);
    m_pBtn_BottomBorderButton1->setText("精选");
    layoutBottomBorderButton->addWidget(m_pBtn_BottomBorderButton1, 0, Qt::AlignVCenter);
    // m_pBtn_BottomBorderButton1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // m_pBtn_BottomBorderButton1->adjustSize();  // 调整按钮大小以适应文本

    m_pBtn_BottomBorderButton2 = new qtmaterialwidget::BottomBorderButton(this);
    m_pBtn_BottomBorderButton2->setObjectName("BottomBorderButton_H28_T16");
    m_pBtn_BottomBorderButton2->setCheckable(true);
    m_pBtn_BottomBorderButton2->setText("歌单广场");
    layoutBottomBorderButton->addWidget(m_pBtn_BottomBorderButton2, 0, Qt::AlignVCenter);
    // m_pBtn_BottomBorderButton2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // m_pBtn_BottomBorderButton2->adjustSize();  // 调整按钮大小以适应文本

    m_pBtn_BottomBorderButton3 = new qtmaterialwidget::BottomBorderButton(this);
    m_pBtn_BottomBorderButton3->setObjectName("BottomBorderButton_H28_T16");
    m_pBtn_BottomBorderButton3->setCheckable(true);
    m_pBtn_BottomBorderButton3->setText("排行榜");
    layoutBottomBorderButton->addWidget(m_pBtn_BottomBorderButton3, 0, Qt::AlignVCenter);
    // m_pBtn_BottomBorderButton3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // m_pBtn_BottomBorderButton3->adjustSize();  // 调整按钮大小以适应文本

    m_pBtn_BottomBorderButton4 = new qtmaterialwidget::BottomBorderButton(this);
    m_pBtn_BottomBorderButton4->setObjectName("BottomBorderButton_H28_T16");
    m_pBtn_BottomBorderButton4->setCheckable(true);
    m_pBtn_BottomBorderButton4->setText("歌手");
    layoutBottomBorderButton->addWidget(m_pBtn_BottomBorderButton4, 0, Qt::AlignVCenter);
    // m_pBtn_BottomBorderButton4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // m_pBtn_BottomBorderButton4->adjustSize();  // 调整按钮大小以适应文本

    m_pBtn_BottomBorderButton5 = new qtmaterialwidget::BottomBorderButton(this);
    m_pBtn_BottomBorderButton5->setObjectName("BottomBorderButton_H28_T16");
    m_pBtn_BottomBorderButton5->setCheckable(true);
    m_pBtn_BottomBorderButton5->setText("VIP");
    layoutBottomBorderButton->addWidget(m_pBtn_BottomBorderButton5, 0, Qt::AlignVCenter);
    // m_pBtn_BottomBorderButton5->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // m_pBtn_BottomBorderButton5->adjustSize();  // 调整按钮大小以适应文本

    layoutBottomBorderButton->addStretch();

    QButtonGroup *buttonGroupBottomBorderButton = new QButtonGroup(this);
    buttonGroupBottomBorderButton->addButton(m_pBtn_BottomBorderButton1, 0);
    buttonGroupBottomBorderButton->addButton(m_pBtn_BottomBorderButton2, 1);
    buttonGroupBottomBorderButton->addButton(m_pBtn_BottomBorderButton3, 2);
    buttonGroupBottomBorderButton->addButton(m_pBtn_BottomBorderButton4, 3);
    buttonGroupBottomBorderButton->addButton(m_pBtn_BottomBorderButton5, 4);
    m_pBtn_BottomBorderButton1->setChecked(true);

    //
    layout->addStretch();
}