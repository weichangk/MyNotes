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