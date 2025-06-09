#pragma once
#include <QWidget>

void qstack_layout();

class QStackLayoutWidget : public QWidget {
    Q_OBJECT
public:
    explicit QStackLayoutWidget(QWidget *parent = nullptr);
    ~QStackLayoutWidget();
};
