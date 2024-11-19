#include "progressbartest.h"
#include "filter/mask.h"

#include <QHBoxLayout>

ProgressBarTestWidget::ProgressBarTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("ProgressBarTestWidget");
    setWindowTitle("ProgressBar Test");
    setFixedSize(800, 600);

    auto layout = new QVBoxLayout(this); 

    m_pProgressBar = new QProgressBar(this);
    m_pProgressBar->setFixedHeight(24);
    m_pProgressBar->setObjectName("QProgressBar_Test");
    m_pProgressBar->setRange(0, 0);

    auto maskProgressBar = new filter::RadiusMask(m_pProgressBar);

    layout->addWidget(m_pProgressBar);

    QString progressBarStyleSheet = R"(
        QProgressBar#QProgressBar_Test
        {
            color:rgba(255,255,255,255);
            font-size:14px;
            background:#0A090D;
            border-radius: 8px;
            padding:2px 0px;
        }

        QProgressBar#QProgressBar_Test::chunk 
        {
            border-radius: 8px;
            background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #F15AFF, stop:1 #A957FE);
        }
    )";
    m_pProgressBar->setStyleSheet(progressBarStyleSheet);

    m_pProgressBar1 = new widget::LoopProgressBar(this);
    m_pProgressBar1->setFixedHeight(24);
    layout->addWidget(m_pProgressBar1);

    layout->addStretch();
}