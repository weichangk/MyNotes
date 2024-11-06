#pragma once
#include "qtmaterial_global.h"

namespace filter {
class QTMATERIAL_EXPORT Lang : public QObject {
    Q_OBJECT

public:
    Lang(QObject *parent);
    ~Lang();

Q_SIGNALS:
    void sigLanguageChange();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QObject *watched_ = nullptr;
};
} // namespace filter