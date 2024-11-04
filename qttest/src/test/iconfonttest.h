#pragma once
#include <QWidget>
#include <QPushButton>

class IconFontTestWidget : public QWidget {
    Q_OBJECT

public:
    IconFontTestWidget(QWidget *parent = nullptr);
    ~IconFontTestWidget() override {}

private:
    QPushButton *m_pBtn1 = nullptr;
};
