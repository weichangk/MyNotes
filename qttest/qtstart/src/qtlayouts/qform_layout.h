#pragma once
#include <QWidget>

void qform_layout();

class QFormLayoutWidget : public QWidget {
    Q_OBJECT
public:
    explicit QFormLayoutWidget(QWidget *parent = nullptr);
    ~QFormLayoutWidget();
};
