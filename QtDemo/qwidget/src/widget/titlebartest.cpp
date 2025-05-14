#include "titlebartest.h"

#include <QVBoxLayout>

TitlebarTestWidget::TitlebarTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("TitlebarTestWidget");
    setWindowTitle("Titlebar Test");
    setWindowFlags(Qt::FramelessWindowHint);
    setMinimumSize(1000, 600);
    

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_pTitlebar = new qtmaterialwidget::Titlebar(this);
    m_pTitlebar->setBackgroundColor(QColor("#266df8"));
    m_pTitlebar->setCloseBtnNormalPixmapPath(":/test/img/mac_close_n.png");
    m_pTitlebar->setCloseBtnHoverPixmapPath(":/test/img/mac_close_h.png");
    m_pTitlebar->setCloseBtnPressedPixmapPath(":/test/img/mac_close_p.png");
    m_pTitlebar->setCloseBtnDisablePixmapPath(":/test/img/mac_close_d.png");
    m_pTitlebar->setMinBtnNormalPixmapPath(":/test/img/mac_minimum_n.png");
    m_pTitlebar->setMinBtnHoverPixmapPath(":/test/img/mac_minimum_h.png");
    m_pTitlebar->setMinBtnPressedPixmapPath(":/test/img/mac_minimum_p.png");
    m_pTitlebar->setMinBtnDisablePixmapPath(":/test/img/mac_minimum_d.png");
    m_pTitlebar->setMaxBtnNormalPixmapPath(":/test/img/mac_maxmum_n.png");
    m_pTitlebar->setMaxBtnHoverPixmapPath(":/test/img/mac_maxmum_h.png");
    m_pTitlebar->setMaxBtnPressedPixmapPath(":/test/img/mac_maxmum_p.png");
    m_pTitlebar->setMaxBtnDisablePixmapPath(":/test/img/mac_maxmum_d.png");

    layout->addWidget(m_pTitlebar);
    layout->addStretch();

    //
    connect(m_pTitlebar, &qtmaterialwidget::Titlebar::sigIgnoreClose, this, [&](){
        return true;
    });
}