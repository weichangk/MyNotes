#pragma once
#include "qtmaterial_global.h"

class QTMATERIAL_EXPORT ShadowExt : public QObject {
    Q_OBJECT
public:
    ShadowExt(QWidget *parent);
    ~ShadowExt();

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
