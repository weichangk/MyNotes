#include "buttontest.h"
#include "core/font.h"

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

    auto layout = new QHBoxLayout(this); 

    m_pBtn1 = new widget::IconVectorButton(this);
    m_pBtn1->setObjectName("IconVectorButton_Size24");
    m_pBtn1->setFont(core::Font::currentIconFont());
    m_pBtn1->setText(QChar(0xe665));
    layout->addWidget(m_pBtn1);

    m_pBtn2 = new widget::IconVectorButton(this);
    m_pBtn2->setObjectName("IconVectorButton_Size26");
    m_pBtn2->setFont(core::Font::currentIconFont());
    m_pBtn2->setText(QChar(0xe665));
    layout->addWidget(m_pBtn2);

    m_pBtn3 = new widget::IconVectorButton(this);
    m_pBtn3->setObjectName("IconVectorButton_Size28");
    m_pBtn3->setFont(core::Font::currentIconFont());
    m_pBtn3->setText(QChar(0xe665));
    layout->addWidget(m_pBtn3);

    m_pBtn4 = new widget::IconVectorButton(this);
    m_pBtn4->setObjectName("IconVectorButton_Size24RoundBg");
    m_pBtn4->setFont(core::Font::currentIconFont());
    m_pBtn4->setText(QChar(0xe665));
    layout->addWidget(m_pBtn4);

    m_pBtn5 = new widget::IconVectorButton(this);
    m_pBtn5->setObjectName("IconVectorButton_Size26RoundBg");
    m_pBtn5->setFont(core::Font::currentIconFont());
    m_pBtn5->setText(QChar(0xe665));
    layout->addWidget(m_pBtn5);

    m_pBtn6 = new widget::IconVectorButton(this);
    m_pBtn6->setObjectName("IconVectorButton_Size28RoundBg");
    m_pBtn6->setFont(core::Font::currentIconFont());
    m_pBtn6->setText(QChar(0xe665));
    layout->addWidget(m_pBtn6);

    layout->addStretch();
}