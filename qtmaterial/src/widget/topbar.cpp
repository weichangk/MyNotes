#include "widget/topbar.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QFile>

namespace widget {
Topbar::Topbar(QWidget *parent) :
    CanMove(parent) {
    createUi();
    sigConnect();
}

Topbar::~Topbar() {
}

QWidget *Topbar::contentWidget() {
    return m_contentWidget;
}

void Topbar::setNormalVisible(bool visible) {
#ifdef Q_OS_MAC
#else
    m_maxBtn->setVisible(!visible);
    m_normalBtn->setVisible(visible);
#endif
}

void Topbar::setCloseBtnTopRight10Radius() {
    m_closeBtn->setObjectName("Topbar_m_closeBtn_10Radius");
    m_closeBtn->setStyle(m_closeBtn->style());
}

void Topbar::setMinVisible(bool visible) {
    m_minBtn->setVisible(visible);
    m_minMacBtn->setVisible(visible);
}

void Topbar::setMaxVisible(bool visible) {
    m_maxBtn->setVisible(visible);
    m_maxMacBtn->setVisible(visible);
}
void Topbar::setCloseVisible(bool visible) {
    m_closeBtn->setVisible(visible);
    m_closeMacBtn->setVisible(visible);
}

void Topbar::createUi() {
    setFixedHeight(40); // 36

    m_minMacBtn = new QPushButton(this);
    m_minMacBtn->setObjectName("Topbar_m_minMacBtn");
    m_minMacBtn->setFixedSize(40, 40);
    m_maxMacBtn = new QPushButton(this);
    m_maxMacBtn->setObjectName("Topbar_m_maxMacBtn");
    m_maxMacBtn->setFixedSize(40, 40);
    m_normalMacBtn = new QPushButton(this);
    m_normalMacBtn->setObjectName("Topbar_m_normalMacBtn");
    m_normalMacBtn->setFixedSize(40, 40);
    m_closeMacBtn = new QPushButton(this);
    m_closeMacBtn->setObjectName("Topbar_m_closeMacBtn");
    m_closeMacBtn->setFixedSize(40, 40);

    m_contentWidget = new QWidget(this);
    m_contentWidget->setAttribute(Qt::WA_TranslucentBackground);

    m_minBtn = new QPushButton(this);
    m_minBtn->setObjectName("Topbar_m_minBtn");
    m_minBtn->setFixedSize(40, 40);
    m_maxBtn = new QPushButton(this);
    m_maxBtn->setObjectName("Topbar_m_maxBtn");
    m_maxBtn->setFixedSize(40, 40);
    m_normalBtn = new QPushButton(this);
    m_normalBtn->setObjectName("Topbar_m_normalBtn");
    m_normalBtn->setFixedSize(40, 40);
    m_closeBtn = new QPushButton(this);
    m_closeBtn->setObjectName("Topbar_m_closeBtn");
    m_closeBtn->setFixedSize(40, 40);

    auto lyt = new QHBoxLayout(this);
    lyt->setContentsMargins(0, 0, 0, 0);
    lyt->setSpacing(0);
    lyt->addWidget(m_minMacBtn);
    lyt->addWidget(m_maxMacBtn);
    lyt->addWidget(m_normalMacBtn);
    lyt->addWidget(m_closeMacBtn);
    lyt->addWidget(m_contentWidget, 1);
    lyt->addWidget(m_minBtn);
    lyt->addWidget(m_maxBtn);
    lyt->addWidget(m_normalBtn);
    lyt->addWidget(m_closeBtn);

//
#ifdef Q_OS_MAC
    m_minMacBtn->setVisible(true);
    m_maxMacBtn->setVisible(true);
    m_normalMacBtn->setVisible(true);
    m_closeMacBtn->setVisible(true);
    m_minBtn->setVisible(false);
    m_maxBtn->setVisible(false);
    m_normalBtn->setVisible(false);
    m_closeBtn->setVisible(false);
#else
    m_minMacBtn->setVisible(false);
    m_maxMacBtn->setVisible(false);
    m_normalMacBtn->setVisible(false);
    m_closeMacBtn->setVisible(false);
    m_minBtn->setVisible(true);
    setNormalVisible(false);
    m_closeBtn->setVisible(true);
#endif
}

void Topbar::sigConnect() {
    connect(m_minBtn, &QPushButton::clicked, this, &Topbar::sigMin);
    connect(m_closeBtn, &QPushButton::clicked, this, &Topbar::sigClose);
    connect(m_maxBtn, &QPushButton::clicked, this, [=]() {
        emit sigMax();
        setNormalVisible(true);
    });
    connect(m_normalBtn, &QPushButton::clicked, this, [=]() {
        emit sigNormal();
        setNormalVisible(false);
    });
}
} // namespace widget