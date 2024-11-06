#include "core/languagechange.h"
#include <QEvent>

namespace core {
LanguageChange::LanguageChange(QObject *parent) :
    QObject(parent), watched_(parent) {
    watched_->installEventFilter(this);
}

LanguageChange::~LanguageChange() {
}

bool LanguageChange::eventFilter(QObject *watched, QEvent *event) {
    if (watched_ == watched) {
        if (event->type() == QEvent::LanguageChange) {
            emit sigLanguageChanged();
        }
    }
    return QObject::eventFilter(watched, event);
}
} // namespace core