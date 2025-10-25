#include "slidertest.h"

#include <QVBoxLayout>

SliderTestWidget::SliderTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("SliderTestWidget");
    setWindowTitle("Slider Test");
    setFixedSize(800, 600);

    QString style = R"(
        QWidget#SliderTestWidget {
            background-color: #f5f5f5;
        }
    )";
    setStyleSheet(style);

    m_pSlider1 = new QSlider(this);
    m_pSlider1->setOrientation(Qt::Horizontal);
    m_pSlider1->setMinimum(0);
    m_pSlider1->setMaximum(100);
    m_pSlider1->setPageStep(1);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_pSlider1);
    layout->addStretch();
}