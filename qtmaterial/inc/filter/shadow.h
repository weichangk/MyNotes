#pragma once
#include "qtmaterial_global.h"

#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

namespace filter {
class QTMATERIAL_EXPORT Shadow : public QObject {
    Q_OBJECT
public:
    Shadow(QWidget *parent);
    ~Shadow();

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

class QTMATERIAL_EXPORT ShadowEffect : public QWidget {
    Q_OBJECT
public:
    ShadowEffect(QWidget *parent);
    ~ShadowEffect() override {
    }
    // ShadowEffect 是 tool 窗体，为了实现 ShadowEffect 在 parent 下方，没有给 ShadowEffect 设置 parent 父对象，所以使用 ShadowEffect 时需要手动释放!
    void release();
    void setMargins(const QMargins &);
    void setEffectParams(int radius, double offsetX, double offsetY, double blurRadius, const QColor &color);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void updateGeometry();

private:
    QVBoxLayout *m_pLayout = nullptr;
    QMargins m_margins = {16, 16, 16, 16};
    QWidget *m_pBackgroundWidget = nullptr;
    QWidget *m_pParentWidget = nullptr;
    QGraphicsDropShadowEffect *m_pShadowEffect = nullptr;

    int m_nRadius = 8;
    double m_dOffsetX = 0;
    double m_dOffsetY = 0;
    double m_dBlurRadius = 16.0;
    QColor m_color = Qt::gray;
};
} // namespace filter
