#include "qcomboboxtest.h"

#include <QVBoxLayout>

QComboBoxTestWidget::QComboBoxTestWidget(QWidget *parent) :
    QWidget(parent) {
    createUi();
    sigConnect();
}

void QComboBoxTestWidget::createUi() {
    setObjectName("QComboBoxTestWidget");
    setWindowTitle("QComboBox Test");
    setFixedSize(800, 600);

    auto layout = new QVBoxLayout(this); 

    m_pComboBox = new QComboBox(this);
    m_pComboBox->setFixedSize(240, 40);
    m_pComboBox->addItem("Item 1");
    m_pComboBox->addItem("Item 2");
    m_pComboBox->addItem("Item 3");
    m_pComboBox->addItem("Item 4");
    m_pComboBox->addItem("Item 5");

    layout->addStretch();
    layout->addWidget(m_pComboBox);
}

void QComboBoxTestWidget::sigConnect() {
}