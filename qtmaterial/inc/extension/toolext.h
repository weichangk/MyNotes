#pragma once
#include "qtmaterial_global.h"

class QTMATERIAL_EXPORT ToolExt : public QObject {
    Q_OBJECT
public:
    explicit ToolExt(QWidget *parent);
    ~ToolExt() override {
    }
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QWidget *m_pWatchedObj = nullptr;
};
