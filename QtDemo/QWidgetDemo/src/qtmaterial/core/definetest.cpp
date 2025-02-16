#include "definetest.h"
#include "core/define.h"

#include <QHBoxLayout>
#include <QDebug>

int checkAge(int age) {
    IF_TRUE_RETURN_VALUE(age < 18, -1);
    return 0;
}

DefineTestWidget::DefineTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("DefineTestWidget");
    setWindowTitle("Define Test");
    setFixedSize(800, 600);

    auto layout = new QHBoxLayout(this); 

    m_pBtn1 = new QPushButton("Btn1", this);
    layout->addWidget(m_pBtn1);

    m_pBtn2 = new QPushButton("Btn2", this);
    layout->addWidget(m_pBtn2);

    m_pBtn3 = new QPushButton("Btn3", this);
    layout->addWidget(m_pBtn3);

    m_pBtn4 = new QPushButton("Btn4", this);
    layout->addWidget(m_pBtn4);

    m_pBtn5 = new QPushButton("Btn5", this);
    layout->addWidget(m_pBtn5);

    m_pBtn6 = new QPushButton("Btn6", this);
    layout->addWidget(m_pBtn6);

    layout->addStretch();


    connect(m_pBtn1, &QPushButton::clicked, this, [this]() {
        int a = checkAge(10);
        qDebug() << a;
    });
}