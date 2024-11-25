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
    widget::Carousel *m_pCarousel = nullptr;
};