#include "qsstest.h"

#include <QHBoxLayout>
#include <QDebug>

namespace ns {
NsPushButton::NsPushButton(QWidget *parent) :
    QPushButton(parent) {
}
} // namespace ns

MyPushButton::MyPushButton(QWidget *parent) :
    QPushButton(parent) {
}

HasChildPushButton::HasChildPushButton(QWidget *parent) :
    QPushButton(parent) {
    auto layout = new QHBoxLayout(this);
    m_pText = new QLabel(this);
    layout->addWidget(m_pText, 0, Qt::AlignCenter);
}

void HasChildPushButton::setObjectName(const QString &name) {
    QPushButton::setObjectName(name);
    if (m_pText) {
        m_pText->setObjectName(name + "Text");
    }
}

void HasChildPushButton::setText(const QString &text) {
    m_pText->setText(text);
}

QssTestWidget::QssTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("QssTestWidget");
    setWindowTitle("Qss Test");
    setFixedSize(800, 600);

    auto layout = new QHBoxLayout(this); 

    m_pNsPushButton = new ns::NsPushButton(this);
    m_pNsPushButton->setFixedSize(100, 50);
    layout->addWidget(m_pNsPushButton, 0, Qt::AlignCenter);

    qDebug() << m_pNsPushButton->metaObject()->className();

    m_pNsPushButton1 = new ns::NsPushButton (this);
    m_pNsPushButton1->setObjectName("NsPushButtonXXX");
    m_pNsPushButton1->setFixedSize(100, 50);
    layout->addWidget(m_pNsPushButton1, 0, Qt::AlignCenter);

    m_pPushButton1 = new MyPushButton (this);
    m_pPushButton1->setFixedSize(100, 50);
    layout->addWidget(m_pPushButton1, 0, Qt::AlignCenter);

    m_pHasChildPushButton = new HasChildPushButton(this);
    // m_pHasChildPushButton->setObjectName("HasChildPushButton");
    m_pHasChildPushButton->setFixedSize(100, 50);
    m_pHasChildPushButton->setText("HasChildPushButton");
    layout->addWidget(m_pHasChildPushButton, 0, Qt::AlignCenter);

    layout->addStretch();
}