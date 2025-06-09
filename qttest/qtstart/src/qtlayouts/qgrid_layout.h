#pragma once
#include <QWidget>

void qgrid_layout();

class QGridLayoutWidget : public QWidget {
    Q_OBJECT
public:
    explicit QGridLayoutWidget(QWidget *parent = nullptr);
    ~QGridLayoutWidget();
};
