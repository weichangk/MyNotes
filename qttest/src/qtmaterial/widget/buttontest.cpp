#include "buttontest.h"
#include "core/font.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

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
    m_pBtn1->setObjectName("VectorButton_Size24");
    m_pBtn1->setFont(core::Font::currentIconFont());
    m_pBtn1->setText(QChar(0xe665));
    layout1->addWidget(m_pBtn1);

    m_pBtn2 = new widget::VectorButton(this);
    m_pBtn2->setObjectName("VectorButton_Size26");
    m_pBtn2->setFont(core::Font::currentIconFont());
    m_pBtn2->setText(QChar(0xe665));
    layout1->addWidget(m_pBtn2);

    m_pBtn3 = new widget::VectorButton(this);
    m_pBtn3->setObjectName("VectorButton_Size28");
    m_pBtn3->setFont(core::Font::currentIconFont());
    m_pBtn3->setText(QChar(0xe665));
    layout1->addWidget(m_pBtn3);

    m_pBtn4 = new widget::VectorButton(this);
    m_pBtn4->setObjectName("VectorButton_Size24RoundBg");
    m_pBtn4->setFont(core::Font::currentIconFont());
    m_pBtn4->setText(QChar(0xe665));
    layout1->addWidget(m_pBtn4);

    m_pBtn5 = new widget::VectorButton(this);
    m_pBtn5->setObjectName("VectorButton_Size26RoundBg");
    m_pBtn5->setFont(core::Font::currentIconFont());
    m_pBtn5->setText(QChar(0xe665));
    layout1->addWidget(m_pBtn5);

    m_pBtn6 = new widget::VectorButton(this);
    m_pBtn6->setObjectName("VectorButton_Size28RoundBg");
    m_pBtn6->setFont(core::Font::currentIconFont());
    m_pBtn6->setText(QChar(0xe665));
    layout1->addWidget(m_pBtn6);

    layout1->addStretch();

    auto widget2 = new QWidget(this);
    widget2->setFixedHeight(48);

    layout->addWidget(widget2);

    auto layout2 = new QHBoxLayout(widget2);  

    m_pBtn11 = new widget::HorIconTextVectorButton(this);
    m_pBtn11->setObjectName("HorIconTextVectorButton_Size24");
    m_pBtn11->setIconFont(core::Font::currentIconFont());
    m_pBtn11->setIcon(QChar(0xe665));
    m_pBtn11->setText("我喜欢的");
    layout2->addWidget(m_pBtn11);

    m_pBtn12 = new widget::HorIconTextVectorButton(this);
    m_pBtn12->setObjectName("HorIconTextVectorButton_Size26");
    m_pBtn12->setFont(core::Font::currentIconFont());
    m_pBtn12->setIcon(QChar(0xe665));
    m_pBtn12->setText("本地下载");
    layout2->addWidget(m_pBtn12);

    m_pBtn13 = new widget::HorIconTextVectorButton(this);
    m_pBtn13->setObjectName("HorIconTextVectorButton_Size28");
    m_pBtn13->setFont(core::Font::currentIconFont());
    m_pBtn13->setIcon(QChar(0xe665));
    m_pBtn13->setText("最近播放");
    layout2->addWidget(m_pBtn13);

    m_pBtn14 = new widget::HorIconTextVectorButton(this);
    m_pBtn14->setObjectName("HorIconTextVectorButton_Size24RoundBg");
    m_pBtn14->setFont(core::Font::currentIconFont());
    m_pBtn14->setIcon(QChar(0xe665));
    m_pBtn14->setText("音乐云盘");
    layout2->addWidget(m_pBtn14);

    m_pBtn15 = new widget::HorIconTextVectorButton(this);
    m_pBtn15->setObjectName("HorIconTextVectorButton_Size26RoundBg");
    m_pBtn15->setFont(core::Font::currentIconFont());
    m_pBtn15->setIcon(QChar(0xe665));
    m_pBtn15->setText("我的收藏");
    layout2->addWidget(m_pBtn15);

    m_pBtn16 = new widget::HorIconTextVectorButton(this);
    m_pBtn16->setObjectName("HorIconTextVectorButton_Size28RoundBg");
    m_pBtn16->setFont(core::Font::currentIconFont());
    m_pBtn16->setIcon(QChar(0xe665));
    m_pBtn16->setText("我的播客");
    layout2->addWidget(m_pBtn16);

    layout2->addStretch();

    layout->addStretch();
}