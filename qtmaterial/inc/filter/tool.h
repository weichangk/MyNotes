#pragma once
#include "qtmaterial_global.h"

namespace filter {
class QTMATERIAL_EXPORT Tool : public QObject {
    Q_OBJECT
public:
    explicit Tool(QWidget *parent);
    ~Tool() override {
    }
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QWidget *m_pWatchedObj = nullptr;
};
} // namespace filter