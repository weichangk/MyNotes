#pragma once
#include <QWidget>

void qbox_layout();

class QBoxLayoutWidget : public QWidget {
    Q_OBJECT
public:
    explicit QBoxLayoutWidget(QWidget *parent = nullptr);
    ~QBoxLayoutWidget();
};
