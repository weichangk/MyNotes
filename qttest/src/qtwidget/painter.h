/*
 * @Author: weick 
 * @Date: 2024-05-25 23:59:08 
 * @Last Modified by: weick
 * @Last Modified time: 2024-05-26 08:46:54
 */

#pragma once
#include <QWidget>
#include <QPainter>

class PainterWidget : public QWidget {
    Q_OBJECT

public:
    PainterWidget(QWidget *parent = nullptr);
    ~PainterWidget();

protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    void createUi();
    void sigConnect();
};

class Painter {
public:
    void draw2Diamond(QPainter* painter, QPoint startPoint, QPoint endPoint, const QColor& borderColor, const QColor& contentColor);
};