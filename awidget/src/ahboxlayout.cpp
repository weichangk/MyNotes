/*
 * @Author: weick
 * @Date: 2023-12-05 22:59:13
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:59:13
 */

#include "inc/ahboxlayout.h"

AHBoxLayout::AHBoxLayout(QWidget *parent) :
    QHBoxLayout(parent) {
    setSpacing(0);
    setContentsMargins(0, 0, 0, 0);
}

AHBoxLayout::~AHBoxLayout() {
}
