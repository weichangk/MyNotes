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