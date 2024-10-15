#pragma once
#include "qtmaterial_global.h"

class QTMATERIAL_EXPORT LanguageChangeHelper : public QObject {
    Q_OBJECT

public:
    LanguageChangeHelper(QObject *parent);
    ~LanguageChangeHelper();

Q_SIGNALS:
    void sigLanguageChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QObject *watched_ = nullptr;
};