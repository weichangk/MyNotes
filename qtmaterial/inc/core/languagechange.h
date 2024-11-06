#pragma once
#include "qtmaterial_global.h"

namespace core {
class QTMATERIAL_EXPORT LanguageChange : public QObject {
    Q_OBJECT

public:
    LanguageChange(QObject *parent);
    ~LanguageChange();

Q_SIGNALS:
    void sigLanguageChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QObject *watched_ = nullptr;
};
} // namespace core