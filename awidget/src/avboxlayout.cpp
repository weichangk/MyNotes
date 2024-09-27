/*
 * @Author: weick
 * @Date: 2023-12-05 22:59:55
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:59:55
 */

#include "inc/avboxlayout.h"

AVBoxLayout::AVBoxLayout(QWidget *parent) :
    QVBoxLayout(parent) {
    setSpacing(0);
    setContentsMargins(0, 0, 0, 0);
}

AVBoxLayout::~AVBoxLayout() {
}
