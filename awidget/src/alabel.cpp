/*
 * @Author: weick
 * @Date: 2023-12-05 22:59:21
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:59:21
 */

#include "inc/alabel.h"

ALabel::ALabel(QWidget *parent) :
    QLabel(parent) {
    QSizePolicy sizePolicy = this->sizePolicy();
    sizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    sizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    this->setSizePolicy(sizePolicy);
}

ALabel::~ALabel() {
}
