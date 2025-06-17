#pragma once
#include "filter/arrowwindow.h"

#include <QWidget>

class ArrowMaskTestWidget : public QWidget {
    Q_OBJECT

public:
    ArrowMaskTestWidget(QWidget *parent = nullptr);
    ~ArrowMaskTestWidget();
};