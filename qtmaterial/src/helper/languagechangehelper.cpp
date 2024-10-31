#include "helper/languagechangehelper.h"
#include <QEvent>

LanguageChangeHelper::LanguageChangeHelper(QObject *parent) :
    QObject(parent), watched_(parent) {
    watched_->installEventFilter(this);
}

LanguageChangeHelper::~LanguageChangeHelper() {
}

bool LanguageChangeHelper::eventFilter(QObject *watched, QEvent *event) {
    if (watched_ == watched) {
        if (event->type() == QEvent::LanguageChange) {
            emit sigLanguageChanged();
        }
    }
    return QObject::eventFilter(watched, event);
}