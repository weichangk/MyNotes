#pragma once
#include <QWidget>
#include "widget/lineedit.h"

class LineEditTestWidget : public QWidget {
    Q_OBJECT

public:
    LineEditTestWidget(QWidget *parent = nullptr);
    ~LineEditTestWidget() override {}

private:
    widget::SearchLineEdit *m_pSearchLineEdit = nullptr;
};