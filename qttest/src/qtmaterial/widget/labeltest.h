#pragma once
#include <QWidget>
#include "widget/label.h"

class LabelTestWidget : public QWidget {
    Q_OBJECT

public:
    LabelTestWidget(QWidget *parent = nullptr);
    ~LabelTestWidget() override {}

private:
    qtmaterialwidget::VectorLabel *m_pLab1 = nullptr;
    qtmaterialwidget::VectorLabel *m_pLab2 = nullptr;
    qtmaterialwidget::VectorLabel *m_pLab3 = nullptr;
    qtmaterialwidget::VectorLabel *m_pLab4 = nullptr;
    qtmaterialwidget::VectorLabel *m_pLab5 = nullptr;
    qtmaterialwidget::VectorLabel *m_pLab6 = nullptr;

    qtmaterialwidget::CarouselLabel *m_pCarouselLabel1 = nullptr;
    qtmaterialwidget::CarouselLabel *m_pCarouselLabel2 = nullptr;
    qtmaterialwidget::CarouselLabel *m_pCarouselLabel3 = nullptr;
    qtmaterialwidget::CarouselLabel *m_pCarouselLabel4 = nullptr;

    qtmaterialwidget::DiscountLabel *m_pDiscountLabel = nullptr;
};
