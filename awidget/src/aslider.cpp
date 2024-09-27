/*
 * @Author: weick 
 * @Date: 2024-03-30 22:02:50 
 * @Last Modified by: weick
 * @Last Modified time: 2024-03-30 22:04:49
 */

#include "inc/aslider.h"

ASlider::ASlider(QWidget *parent) :
    QSlider(parent) {
}

ASlider::ASlider(Qt::Orientation orientation, QWidget *parent) :
    ASlider(parent) {
    this->setOrientation(orientation);
}

ASlider::~ASlider() {
}