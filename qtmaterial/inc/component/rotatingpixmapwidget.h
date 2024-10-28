#pragma once
#include "qtmaterial_global.h"
#include <QWidget>
#include <QApplication>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>

class QTMATERIAL_EXPORT RotatingPixmapItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)

public:
    explicit RotatingPixmapItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr);
    void start();
    void stop();

private:
    QPropertyAnimation *m_RotationAnimation = 0;
};

class QTMATERIAL_EXPORT WidgetWithRotatingItem : public QWidget {
    Q_OBJECT
public:
    explicit WidgetWithRotatingItem(QPixmap pixmap, QWidget *parent = nullptr);
    explicit WidgetWithRotatingItem(QWidget *parent = nullptr);
    void start();
    void stop();

private:
    RotatingPixmapItem *m_RotatingPixmapItem = 0;
};
