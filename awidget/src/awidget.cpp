/*
 * @Author: weick
 * @Date: 2023-12-05 23:00:04
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 23:00:04
 */

#include "inc/awidget.h"
#include <QEvent>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

AWidget::AWidget(QWidget *parent) :
    QWidget(parent) {
    setFocusPolicy(Qt::ClickFocus);
    setAttribute(Qt::WA_StyledBackground);
}

AWidget::~AWidget() {
}
