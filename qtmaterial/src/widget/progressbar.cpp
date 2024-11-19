#include "widget/progressbar.h"
#include "filter/mask.h"

#include <QPainter>
#include <QPainterPath>

namespace widget {
LoopProgressChunk::LoopProgressChunk(QWidget *parent) :
    QWidget(parent) {

    m_ChunkColor =  QLinearGradient(0, 0, 1, 0); 
    m_ChunkColor.setCoordinateMode(QGradient::ObjectBoundingMode);
    m_ChunkColor.setColorAt(0, QColor("#F15AFF"));
    m_ChunkColor.setColorAt(1, QColor("#A957FE"));

    m_pTimer = new QTimer(this);
    connect(m_pTimer, &QTimer::timeout, this, &LoopProgressChunk::slotTimeout);
    m_pTimer->start(m_nTimerInterval);

    auto mask = new filter::RadiusMask(this);
}

void LoopProgressChunk::showEvent(QShowEvent *event) {
    if(m_nChunkWidth <= 0 || m_nChunkWidth >= width()) {
        m_nChunkWidth = width() / 2;
    }
    m_nOffset1 = 0 - m_nChunkWidth;
    m_nOffset2 = 0 - m_nChunkWidth - width();
}

void LoopProgressChunk::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    QRect rect = this->rect();
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_ChunkColor);

    QRect chunkRect1(m_nOffset1, 0, m_nChunkWidth, rect.height());
    painter.drawRoundedRect(chunkRect1, m_nRadius, m_nRadius);

    QRect chunkRect2(m_nOffset2, 0, m_nChunkWidth, rect.height());
    painter.drawRoundedRect(chunkRect2, m_nRadius, m_nRadius);
}

void LoopProgressChunk::slotTimeout() {
    m_nOffset1 += m_nEvery10msSpeed; 
    if (m_nOffset1 >= (this->width() * 2 - m_nChunkWidth)) {
        m_nOffset1 = 0 - m_nChunkWidth;
    }

    m_nOffset2 += m_nEvery10msSpeed;
    if (m_nOffset2 >= this->width()) {
        m_nOffset2 = 0 - width();
    }
    update();
}

LoopProgressBar::LoopProgressBar(QWidget *parent) :
    QWidget(parent) {

    m_Layout = new QVBoxLayout(this);
    m_Layout->setContentsMargins(m_Margins);

    m_pChunkWidget = new LoopProgressChunk(this);
    m_Layout->addWidget(m_pChunkWidget, 1);

    m_BackGroundColor = QColor("#0A090D");
}

void LoopProgressBar::setBackGroundColor(const QColor &color) {
    m_BackGroundColor = color;
}

void LoopProgressBar::setMargins(const QMargins &margins) {
    m_Margins = margins;
    m_Layout->setContentsMargins(m_Margins);
}

void LoopProgressBar::setRadius(int radius) {
    m_nRadius = radius;
    m_pChunkWidget->m_nRadius = radius;
}

void LoopProgressBar::setChunkColor(const QLinearGradient &color) {
    m_pChunkWidget->m_ChunkColor = color;
}

void LoopProgressBar::setChunkWidth(int width) {
    m_pChunkWidget->m_nChunkWidth = width;
}

void LoopProgressBar::setEvery10msSpeed(int speed) {
    m_pChunkWidget->m_nEvery10msSpeed = speed;
}

void LoopProgressBar::showEvent(QShowEvent *event) {
}

void LoopProgressBar::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    QRect rect = this->rect();
    painter.setBrush(m_BackGroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect, m_nRadius, m_nRadius);
}
} // namespace widget