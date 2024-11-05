#pragma once
#include <QWidget>
#include "control/button.h"

class ButtonTestWidget : public QWidget {
    Q_OBJECT

public:
    ButtonTestWidget(QWidget *parent = nullptr);
    ~ButtonTestWidget() override {}

private:
    IconVectorButton *m_pBtn1 = nullptr;
    IconVectorButton *m_pBtn2 = nullptr;
    IconVectorButton *m_pBtn3 = nullptr;
    IconVectorButton *m_pBtn4 = nullptr;
    IconVectorButton *m_pBtn5 = nullptr;
    IconVectorButton *m_pBtn6 = nullptr;
};
