#pragma once
#include <QWidget>

void frame_family();

class FrameFamilyWidget : public QWidget {
    Q_OBJECT
public:
    explicit FrameFamilyWidget(QWidget *parent = nullptr);
    ~FrameFamilyWidget();
};
