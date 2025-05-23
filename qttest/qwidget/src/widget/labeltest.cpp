#include "labeltest.h"
#include "core/font.h"

#include <QHBoxLayout>
#include <QApplication>

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

   QFont iconFont = qtmaterialcore::Font::getIconFont(":/font/QtMaterialIconFont.ttf");
   
    auto layout = new QVBoxLayout(this); 

    auto widgetVectorLabel = new QWidget(this);
    widgetVectorLabel->setFixedHeight(48);

    layout->addWidget(widgetVectorLabel);

    auto layoutwidgetVectorLabel = new QHBoxLayout(widgetVectorLabel); 
    layoutwidgetVectorLabel->setAlignment(Qt::AlignVCenter);
    layoutwidgetVectorLabel->setContentsMargins(0, 0, 0, 0);

    m_pLab1 = new qtmaterialwidget::VectorLabel(this);
    m_pLab1->setObjectName("VectorLabel_Size24");
    m_pLab1->setFont(iconFont);
    m_pLab1->setText(QChar(0xe665));
    layoutwidgetVectorLabel->addWidget(m_pLab1, 0, Qt::AlignVCenter);

    m_pLab2 = new qtmaterialwidget::VectorLabel(this);
    m_pLab2->setObjectName("VectorLabel_Size26");
    m_pLab2->setFont(iconFont);
    m_pLab2->setText(QChar(0xe665));
    layoutwidgetVectorLabel->addWidget(m_pLab2, 0, Qt::AlignVCenter);

    m_pLab3 = new qtmaterialwidget::VectorLabel(this);
    m_pLab3->setObjectName("VectorLabel_Size28");
    m_pLab3->setFont(iconFont);
    m_pLab3->setText(QChar(0xe665));
    layoutwidgetVectorLabel->addWidget(m_pLab3, 0, Qt::AlignVCenter);

    m_pLab4 = new qtmaterialwidget::VectorLabel(this);
    m_pLab4->setObjectName("VectorLabel_Size24RoundBg");
    m_pLab4->setFont(iconFont);
    m_pLab4->setText(QChar(0xe665));
    layoutwidgetVectorLabel->addWidget(m_pLab4, 0, Qt::AlignVCenter);

    m_pLab5 = new qtmaterialwidget::VectorLabel(this);
    m_pLab5->setObjectName("VectorLabel_Size26RoundBg");
    m_pLab5->setFont(iconFont);
    m_pLab5->setText(QChar(0xe665));
    layoutwidgetVectorLabel->addWidget(m_pLab5, 0, Qt::AlignVCenter);

    m_pLab6 = new qtmaterialwidget::VectorLabel(this);
    m_pLab6->setObjectName("VectorLabel_Size28RoundBg");
    m_pLab6->setFont(iconFont);
    m_pLab6->setText(QChar(0xe665));
    layoutwidgetVectorLabel->addWidget(m_pLab6, 0, Qt::AlignVCenter);

    layoutwidgetVectorLabel->addStretch();

    //
    m_pCarouselLabel1 = new qtmaterialwidget::CarouselLabel(this);
    m_pCarouselLabel1->setObjectName("MyCarouselLabel");
    m_pCarouselLabel1->setText("这是一段滚动的文字，滚动滚动！！", QFont(), Qt::red, 20, 30, true);
    layout->addWidget(m_pCarouselLabel1, 0, Qt::AlignLeft);

    m_pCarouselLabel2 = new qtmaterialwidget::CarouselLabel(this);
    m_pCarouselLabel2->setObjectName("MyCarouselLabel");
    m_pCarouselLabel2->setText("这是一段滚动的文字，滚动滚动！！", QFont(), Qt::blue, 80, 20, true);
    layout->addWidget(m_pCarouselLabel2, 0, Qt::AlignLeft);

    m_pCarouselLabel3 = new qtmaterialwidget::CarouselLabel(this);
    m_pCarouselLabel3->setObjectName("MyCarouselLabel");
    m_pCarouselLabel3->setText("这是一段滚动的文字，滚动滚动！！", QFont(), Qt::red, 20, 30);
    m_pCarouselLabel3->setFixedSize(150, 40);
    layout->addWidget(m_pCarouselLabel3, 0, Qt::AlignLeft);

    m_pCarouselLabel4 = new qtmaterialwidget::CarouselLabel(this);
    m_pCarouselLabel4->setObjectName("MyCarouselLabel");
    QFont font4 = QFont();
    font4.setPixelSize(24);
    font4.setBold(true);
    m_pCarouselLabel4->setText("这是一段滚动的文字，滚动滚动！！", font4, Qt::blue, 20, 10);
    m_pCarouselLabel4->setFixedSize(700, 60);
    layout->addWidget(m_pCarouselLabel4, 0, Qt::AlignLeft);

    QString carouselLabelStyle = R"(
        #MyCarouselLabel {
            background-color: #000000;
        }
    )";
    m_pCarouselLabel1->setStyleSheet(carouselLabelStyle);
    m_pCarouselLabel2->setStyleSheet(carouselLabelStyle);
    m_pCarouselLabel3->setStyleSheet(carouselLabelStyle);
    m_pCarouselLabel4->setStyleSheet(carouselLabelStyle);
    
    m_pDiscountLabel = new qtmaterialwidget::DiscountLabel(this);
    m_pDiscountLabel->setFixedSize(80, 16);
    QFont discountFont = QApplication::font();
    discountFont.setPixelSize(11);
    m_pDiscountLabel->setText("限时6.4折", discountFont, QColor("#ecd4cf"), 20, 40);
    layout->addWidget(m_pDiscountLabel, 0, Qt::AlignLeft);

    layout->addStretch();
}