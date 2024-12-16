#pragma once
#include "widget/avatar.h"

#include <QWidget>

class AvatarTestWidget : public QWidget {
    Q_OBJECT

public:
    explicit AvatarTestWidget(QWidget *parent = nullptr);
    ~AvatarTestWidget() override {
    }

private:
    widget::Avatar *m_pAvatar1 = nullptr;
    widget::Avatar *m_pAvatar2 = nullptr;
};