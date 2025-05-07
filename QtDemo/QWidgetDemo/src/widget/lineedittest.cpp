#include "lineedittest.h"

#include <QVBoxLayout>

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

    m_pSearchLineEdit = new qtmaterialwidget::SearchLineEdit(this);
    m_pSearchLineEdit->setFixedWidth(250);
    m_pSearchLineEdit->setPlaceholderText("Enter your name here");

    layout->addWidget(m_pSearchLineEdit);
    layout->addStretch();
}