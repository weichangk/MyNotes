/*
 * @Author: weick
 * @Date: 2023-12-05 22:57:53
 * @Last Modified by:   weick
 * @Last Modified time: 2023-12-05 22:57:53
 */

#pragma once
#include "awidget_global.h"

class AWIDGET_EXPORT AShadowEffect : public QObject {
    Q_OBJECT
public:
    AShadowEffect(QWidget *parent);
    ~AShadowEffect();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QPixmap ninePatchPixmap(const QPixmap &srcPixmap, int horzSplit, int vertSplit, int dstWidth, int dstHeight);

private:
    QWidget *m_parentWidget;
    QWidget *m_shadowWidget;
    QPixmap m_pixmap;
    QPixmap m_maskPixmap;
};
