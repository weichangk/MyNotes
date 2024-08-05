/*
 * @Author: weick 
 * @Date: 2024-08-06 00:29:13 
 * @Last Modified by: weick
 * @Last Modified time: 2024-08-06 00:30:06
 */

#pragma once
#include <QWidget>
#include <QPainter>

class TranslucentBackgroundWidget : public QWidget {
    Q_OBJECT

public:
    TranslucentBackgroundWidget(QWidget *parent = nullptr);
    ~TranslucentBackgroundWidget();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void createUi();
    void sigConnect();
};