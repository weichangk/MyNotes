#pragma once
#include <QWidget>
#include "widget/lineedit.h"
#include <QLineEdit>

class LineEditTestWidget : public QWidget {
    Q_OBJECT

public:
    LineEditTestWidget(QWidget *parent = nullptr);
    ~LineEditTestWidget() override {}

private:
    QtmWidget::SearchLineEdit *m_pSearchLineEdit = nullptr;
    QLineEdit *m_pQLineEdit1 = nullptr;
    QtmWidget::UnitLineEdit *m_pUnitLineEdit1 = nullptr;
};