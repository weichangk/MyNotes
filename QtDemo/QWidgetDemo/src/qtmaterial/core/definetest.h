#pragma once
#include <QWidget>
#include <QPushButton>

class DefineTestWidget : public QWidget {
    Q_OBJECT

public:
    DefineTestWidget(QWidget *parent = nullptr);
    ~DefineTestWidget() override {}

private:
    QPushButton *m_pBtn1 = nullptr;
    QPushButton *m_pBtn2 = nullptr;
    QPushButton *m_pBtn3 = nullptr;
    QPushButton *m_pBtn4 = nullptr;
    QPushButton *m_pBtn5 = nullptr;
    QPushButton *m_pBtn6 = nullptr;
};
