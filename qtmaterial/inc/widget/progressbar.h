#pragma once
#include "qtmaterial_global.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QTimer>

namespace widget {
class LoopProgressBar;
class LoopProgressChunk : public QWidget {
    Q_OBJECT
public:
    explicit LoopProgressChunk(QWidget *parent = nullptr);
    ~LoopProgressChunk() override {
    }

protected:
    void showEvent(QShowEvent *) override;
    virtual void paintEvent(QPaintEvent *) override;

private slots:
    void slotTimeout();

private:
    QTimer *m_pTimer = nullptr;
    QLinearGradient m_ChunkColor;

    const int m_nTimerInterval = 10;
    int m_nOffset1 = 0;
    int m_nOffset2 = 0;
    int m_nChunkWidth = 0;
    int m_nEvery10msSpeed = 4;
    int m_nRadius = 8;

    friend class LoopProgressBar;
};

class QTMATERIAL_EXPORT LoopProgressBar : public QWidget {
    Q_OBJECT
public:
    explicit LoopProgressBar(QWidget *parent = nullptr);
    ~LoopProgressBar() override {
    }

    void setBackGroundColor(const QColor &);
    void setMargins(const QMargins &);
    void setRadius(int);
    void setChunkColor(const QLinearGradient &);
    void setChunkWidth(int);
    void setEvery10msSpeed(int);

protected:
    void showEvent(QShowEvent *) override;
    virtual void paintEvent(QPaintEvent *) override;

private:
    QVBoxLayout *m_Layout = nullptr;
    LoopProgressChunk *m_pChunkWidget = nullptr;
    QColor m_BackGroundColor;
    QMargins m_Margins = QMargins(2, 2, 2, 2);
    int m_nRadius = 8;
};
} // namespace widget