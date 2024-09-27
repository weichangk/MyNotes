/*
 * @Author: weick
 * @Date: 2024-04-12 07:56:10
 * @Last Modified by: weick
 * @Last Modified time: 2024-04-12 07:59:37
 */

#include "inc/astackedlayout.h" 

AStackedLayout::AStackedLayout(QWidget *parent) :
    QStackedLayout(parent) {
    setSpacing(0);
    setContentsMargins(0, 0, 0, 0);
}
AStackedLayout::AStackedLayout(QLayout *parentLayout) :
    QStackedLayout(parentLayout) {
    setSpacing(0);
    setContentsMargins(0, 0, 0, 0);
}
AStackedLayout::~AStackedLayout() {
}