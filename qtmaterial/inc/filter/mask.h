#pragma once
#include "qtmaterial_global.h"

#include <QBitmap>

namespace filter {
class QTMATERIAL_EXPORT PopupMask : public QObject {
    Q_OBJECT
public:
    PopupMask(QWidget *watchedWidget, QWidget *watchedWidgetParent);
    ~PopupMask();

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

class QTMATERIAL_EXPORT RadiusMask : public QObject {
    Q_OBJECT
public:
    enum RoundType { 
        Round_All,
        Round_Left,
        Round_Right,
        Round_Top,
        Round_Bottom,
        Round_Left_Bottom,
        Round_Right_Bottom 
    };

public:
    RadiusMask(QWidget *);
    ~RadiusMask() override {
    }

    void setPramas(RoundType roundType, int maskMargin, int xRadius, int yRadius);
    void updateMask();

protected:
    bool eventFilter(QObject *, QEvent *) override;

private:
    QBitmap generateMask();

private:
    QWidget *m_pWatchedWidget = nullptr;
    RoundType m_eRoundType = RoundType::Round_All;
	int m_nMaskMargin = 0;
	int m_nXRadius = 8;
	int m_nYRadius = 8;
};

} // namespace filter