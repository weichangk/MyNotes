#pragma once
#include "widget/carousel.h"

#include <QWidget>

class CarouselTestWidget : public QWidget {
    Q_OBJECT

public:
    explicit CarouselTestWidget(QWidget *parent = nullptr);
    ~CarouselTestWidget() override {
    }

private:
    qtmaterialwidget::Carousel *m_pCarousel = nullptr;
};