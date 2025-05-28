#pragma once
#include <QWidget>

void frame_use();

class FrameUseWidget : public QWidget {
    Q_OBJECT
public:
    explicit FrameUseWidget(QWidget *parent = nullptr);
    ~FrameUseWidget();
};
