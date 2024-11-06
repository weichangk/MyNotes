#pragma once
#include "qtmaterial_global.h"
#include <QWidget>

namespace widget {
class QTMATERIAL_EXPORT Arrow : public QWidget {
    Q_OBJECT
public:
    Arrow(QWidget *parent = nullptr);
    // 设置阴影宽度
    void setShadowWidth(int width);
    // 设置边框圆角
    void setBorderRadius(int radius);
    // 设置小三角形方向
    void setTriangleDirection(int direction);
    // 设置上或下的小三角起始位置
    void setTriangleX(int x);
    // 设置左或右的小三角起始位置
    void setTriangleY(int y);
    // 设置小三角宽和高
    void setTriangleInfo(int width, int height);
    // 设置中间区域
    void setCenterWidget(QWidget *widget);

protected:
    void paintEvent(QPaintEvent *);

private:
    // 阴影宽度
    int m_shadowWidth = 14;
    // 边框圆角
    int m_borderRadius = 6;
    // 小三角形方向，0123左上右下
    int m_triangleDirection = 0;
    // 小三角起始位置，在上或下边框的三角形距离左边框的距离
    int m_triangleX = 50;
    // 小三角起始位置，在左或右边框的三角形距离上边框的距离
    int m_triangleY = 50;
    // 小三角的宽度
    int m_triangleWidth = 14;
    // 小三角高度
    int m_triangleHeight = 10;
};
} // namespace widget