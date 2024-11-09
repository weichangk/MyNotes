#include "widget/label.h"

namespace widget {
VectorLabel::VectorLabel(QWidget *parent) :
    QLabel(parent) {
    setAlignment(Qt::AlignCenter);
}
}// namespace widget