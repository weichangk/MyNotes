#include "lineedittest.h"

#include <QVBoxLayout>
#include <QIntValidator>

LineEditTestWidget::LineEditTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("LineEditTestWidget");
    setWindowTitle("LineEdit Test");
    setFixedSize(800, 600);

    QString style = R"(
        QWidget#LineEditTestWidget {
            background-color: #f5f5f5;
        }
    )";
    setStyleSheet(style);

    auto layout = new QVBoxLayout(this);

    m_pSearchLineEdit = new QtmWidget::SearchLineEdit(this);
    m_pSearchLineEdit->setFixedWidth(250);
    m_pSearchLineEdit->setPlaceholderText("Enter your name here");
    layout->addWidget(m_pSearchLineEdit);

    m_pQLineEdit1 = new QLineEdit(this);
    m_pQLineEdit1->setFixedSize(40, 24);
    m_pQLineEdit1->setAlignment(Qt::AlignCenter);
    m_pQLineEdit1->setValidator(new QIntValidator(-100, 100, this));
    m_pQLineEdit1->setText("0");
    layout->addWidget(m_pQLineEdit1);

    m_pUnitLineEdit1 = new QtmWidget::UnitLineEdit(this);
    m_pUnitLineEdit1->setFixedSize(120, 24);
    m_pUnitLineEdit1->setAlignment(Qt::AlignCenter);
    m_pUnitLineEdit1->setUnit("kg");
    m_pUnitLineEdit1->setPrefix("+");
    layout->addWidget(m_pUnitLineEdit1);


    layout->addStretch();
}