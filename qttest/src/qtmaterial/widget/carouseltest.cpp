#include "carouseltest.h"
#include "widget/carousel.h"

#include <QVBoxLayout>

CarouselTestWidget::CarouselTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("CarouselTestWidget");
    setWindowTitle("Carousel Test");
    setFixedSize(1000, 600);

    auto layout = new QVBoxLayout(this);

    m_pCarousel = new qtmaterialwidget::Carousel(this);
    m_pCarousel->setFixedSize(380, 140);

    layout->addWidget(m_pCarousel, 0, Qt::AlignCenter);

    QVector<qtmaterialwidget::Carousel::Data> carousels;

    qtmaterialwidget::Carousel::Data item1;
    item1.image = QImage(":/test/light/carousel/109951170188223427.jpg");
    item1.name = "109951170188223427";
    item1.description = "";
    item1.url = "";
    carousels.append(item1);

    qtmaterialwidget::Carousel::Data item2;
    item2.image = QImage(":/test/light/carousel/109951170188234221.jpg");
    item2.name = "109951170188234221";
    item2.description = "";
    item2.url = "";
    carousels.append(item2);  

    qtmaterialwidget::Carousel::Data item3;
    item3.image = QImage(":/test/light/carousel/109951170188242621.jpg");
    item3.name = "109951170188242621";
    item3.description = "";
    item3.url = "";
    carousels.append(item3);

    qtmaterialwidget::Carousel::Data item4;
    item4.image = QImage(":/test/light/carousel/109951170188248870.jpg");
    item4.name = "109951170188248870";
    item4.description = "";
    item4.url = "";
    carousels.append(item4);

    qtmaterialwidget::Carousel::Data item5;
    item5.image = QImage(":/test/light/carousel/109951170188276901.jpg");
    item5.name = "109951170188276901";
    item5.description = "";
    item5.url = "";
    carousels.append(item5);

    qtmaterialwidget::Carousel::Data item6;
    item6.image = QImage(":/test/light/carousel/109951170188279640.jpg");
    item6.name = "109951170188279640";
    item6.description = "";
    item6.url = "";
    carousels.append(item6);

    qtmaterialwidget::Carousel::Data item7;
    item7.image = QImage(":/test/light/carousel/109951170188293560.jpg");
    item7.name = "109951170188293560";
    item7.description = "";
    item7.url = "";
    carousels.append(item7);

    qtmaterialwidget::Carousel::Data item8;
    item8.image = QImage(":/test/light/carousel/109951170188294750.jpg");
    item8.name = "109951170188294750";
    item8.description = "";
    item8.url = "";
    carousels.append(item8);

    qtmaterialwidget::Carousel::Data item9;
    item9.image = QImage(":/test/light/carousel/109951170188304539.jpg");
    item9.name = "109951170188304539";
    item9.description = "";
    item9.url = "";
    carousels.append(item9);

    m_pCarousel->setCarousels(carousels);

}