#pragma once
#include "qtmaterial_global.h"
#include <QWidget>
#include <QPixmap>

class QTMATERIAL_EXPORT ThumbnailWidget : public QWidget {
    Q_OBJECT
public:
    ThumbnailWidget(QWidget *parent = nullptr);
    ~ThumbnailWidget();

    void setPixmap(QPixmap pixmap) {
        m_pixmap = pixmap;
        update();
    };
    void setRadiius(int radiius) {
        m_radiius = radiius;
        update();
    };
    void setHasBorder(bool b) {
        m_hasBorder = b;
    };

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    QPixmap m_pixmap = QPixmap();
    int m_radiius = 6;
    bool m_hasBorder = false;
};
