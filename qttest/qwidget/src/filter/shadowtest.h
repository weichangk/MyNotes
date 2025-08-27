#pragma once
#include "filter/shadowwindow.h"

#include <QWidget>

class ShadowTestWidget : public QWidget {
    Q_OBJECT

public:
    ShadowTestWidget(QWidget *parent = nullptr);
    ~ShadowTestWidget();

private:
    QtmFilter::ShadowEffect *m_pShadowEffect = nullptr;
};