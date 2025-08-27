#pragma once
#include <QWidget>
#include <QLabel>
#include "widget/label.h"

class LabelTestWidget : public QWidget {
    Q_OBJECT

public:
    LabelTestWidget(QWidget *parent = nullptr);
    ~LabelTestWidget() override {}

private:
    QtmWidget::VectorLabel *m_pLab1 = nullptr;
    QtmWidget::VectorLabel *m_pLab2 = nullptr;
    QtmWidget::VectorLabel *m_pLab3 = nullptr;
    QtmWidget::VectorLabel *m_pLab4 = nullptr;
    QtmWidget::VectorLabel *m_pLab5 = nullptr;
    QtmWidget::VectorLabel *m_pLab6 = nullptr;

    QtmWidget::CarouselLabel *m_pCarouselLabel1 = nullptr;
    QtmWidget::CarouselLabel *m_pCarouselLabel2 = nullptr;
    QtmWidget::CarouselLabel *m_pCarouselLabel3 = nullptr;
    QtmWidget::CarouselLabel *m_pCarouselLabel4 = nullptr;

    QtmWidget::DiscountLabel *m_pDiscountLabel = nullptr;

    QLabel *m_pWrapModelLab= nullptr;
};
