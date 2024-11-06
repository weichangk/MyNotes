#include "filter/lang.h"
#include <QEvent>

namespace filter {
Lang::Lang(QObject *parent) :
    QObject(parent), watched_(parent) {
    watched_->installEventFilter(this);
}

Lang::~Lang() {
}

bool Lang::eventFilter(QObject *watched, QEvent *event) {
    if (watched_ == watched) {
        if (event->type() == QEvent::LanguageChange) {
            emit sigLanguageChange();
        }
    }
    return QObject::eventFilter(watched, event);
}
} // namespace filter