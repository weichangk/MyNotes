#include "labeltest.h"
#include "core/font.h"

#include <QHBoxLayout>

LabelTestWidget::LabelTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("LabelTestWidget");
    setWindowTitle("Label Test");
    setFixedSize(800, 600);

    QString style = R"(
        QWidget#LabelTestWidget {
            background-color: #f5f5f5;
        }
    )";
    setStyleSheet(style);

    auto layout = new QHBoxLayout(this); 

    m_pLab1 = new widget::VectorLabel(this);
    m_pLab1->setObjectName("VectorLabel_Size24");
    m_pLab1->setFont(core::Font::currentIconFont());
    m_pLab1->setText(QChar(0xe665));
    layout->addWidget(m_pLab1);

    m_pLab2 = new widget::VectorLabel(this);
    m_pLab2->setObjectName("VectorLabel_Size26");
    m_pLab2->setFont(core::Font::currentIconFont());
    m_pLab2->setText(QChar(0xe665));
    layout->addWidget(m_pLab2);

    m_pLab3 = new widget::VectorLabel(this);
    m_pLab3->setObjectName("VectorLabel_Size28");
    m_pLab3->setFont(core::Font::currentIconFont());
    m_pLab3->setText(QChar(0xe665));
    layout->addWidget(m_pLab3);

    m_pLab4 = new widget::VectorLabel(this);
    m_pLab4->setObjectName("VectorLabel_Size24RoundBg");
    m_pLab4->setFont(core::Font::currentIconFont());
    m_pLab4->setText(QChar(0xe665));
    layout->addWidget(m_pLab4);

    m_pLab5 = new widget::VectorLabel(this);
    m_pLab5->setObjectName("VectorLabel_Size26RoundBg");
    m_pLab5->setFont(core::Font::currentIconFont());
    m_pLab5->setText(QChar(0xe665));
    layout->addWidget(m_pLab5);

    m_pLab6 = new widget::VectorLabel(this);
    m_pLab6->setObjectName("VectorLabel_Size28RoundBg");
    m_pLab6->setFont(core::Font::currentIconFont());
    m_pLab6->setText(QChar(0xe665));
    layout->addWidget(m_pLab6);

    layout->addStretch();
}