#pragma once
#include <QWidget>
#include <QSlider>

class SliderTestWidget : public QWidget {
    Q_OBJECT

public:
    SliderTestWidget(QWidget *parent = nullptr);
    ~SliderTestWidget() override {}

private:
    QSlider *m_pSlider1 = nullptr;
};