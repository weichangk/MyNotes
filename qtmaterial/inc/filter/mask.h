#pragma once
#include "qtmaterial_global.h"

namespace filter {
class QTMATERIAL_EXPORT Mask : public QObject {
    Q_OBJECT
public:
    Mask(QWidget *watchedWidget, QWidget *watchedWidgetParent);
    ~Mask();

    void setMaskColor(const QColor);
    QColor maskColor() const;

    void setWatchedWidgetRadius(int);
    int watchedWidgetRadius() const;

    void setMaskWidgetRadius(int);
    int maskWidgetRadius() const;

    void setFullMask(bool);
    bool fullMask() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void updateMaskGeometry();

private:
    QWidget *m_pWatchedWidget = nullptr;
    QWidget *m_pMaskWidget = nullptr;
    QWidget *m_pMask = nullptr;

    QColor m_colorMask = QColor(0, 0, 0, 255 * 0.7);
    int m_nWatchedWidgetRadius = 0;
    int m_nMaskWidgetRadius = 0;

    bool m_bFullMask = false;
};
} // namespace filter