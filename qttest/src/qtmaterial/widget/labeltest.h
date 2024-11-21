#pragma once
#include <QWidget>
#include "widget/label.h"

class LabelTestWidget : public QWidget {
    Q_OBJECT

public:
    LabelTestWidget(QWidget *parent = nullptr);
    ~LabelTestWidget() override {}

private:
    widget::VectorLabel *m_pLab1 = nullptr;
    widget::VectorLabel *m_pLab2 = nullptr;
    widget::VectorLabel *m_pLab3 = nullptr;
    widget::VectorLabel *m_pLab4 = nullptr;
    widget::VectorLabel *m_pLab5 = nullptr;
    widget::VectorLabel *m_pLab6 = nullptr;

    widget::CarouselLabel *m_pCarouselLabel1 = nullptr;
    widget::CarouselLabel *m_pCarouselLabel2 = nullptr;
    widget::CarouselLabel *m_pCarouselLabel3 = nullptr;
    widget::CarouselLabel *m_pCarouselLabel4 = nullptr;
};
