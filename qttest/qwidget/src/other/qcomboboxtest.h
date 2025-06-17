#pragma once
#include <QWidget>
#include <QComboBox>

class QComboBoxTestWidget : public QWidget {
    Q_OBJECT

public:
    QComboBoxTestWidget(QWidget *parent = nullptr);
    ~QComboBoxTestWidget() {
    }

private:
    void createUi();
    void sigConnect();

private:
    QComboBox *m_pComboBox = nullptr;
};