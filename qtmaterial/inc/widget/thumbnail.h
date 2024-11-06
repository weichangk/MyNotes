#pragma once
#include "qtmaterial_global.h"
#include <QWidget>
#include <QPixmap>

namespace widget {
class QTMATERIAL_EXPORT Thumbnail : public QWidget {
    Q_OBJECT
public:
    Thumbnail(QWidget *parent = nullptr);
    ~Thumbnail();

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
} // namespace widget
