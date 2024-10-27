#include "component/funcpanelwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>

FuncPanelWidget::FuncPanelWidget(QWidget *parent, int id) :
    QWidget(parent),
    m_Id(id) {
    createUi();
}

FuncPanelWidget::~FuncPanelWidget() {
}

void FuncPanelWidget::createUi() {
    setObjectName("FuncPanelWidget");
    auto mainLayout = new QHBoxLayout(this);

    auto leftLayout = new QVBoxLayout();
    mainLayout->addLayout(leftLayout);

    m_Name = new QLabel(this);
    m_Name->setObjectName("FuncPanelWidget_m_Name");

    m_Dec = new QLabel(this);
    m_Dec->setObjectName("FuncPanelWidget_m_Dec");

    leftLayout->addWidget(m_Name);
    leftLayout->addStretch();
    leftLayout->addWidget(m_Dec);

    m_Icon = new QLabel(this);
    m_Icon->setObjectName("FuncPanelWidget_m_Icon");
    mainLayout->addStretch();
    mainLayout->addWidget(m_Icon);
}

void FuncPanelWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        emit sigClicked(m_Id);
    }
}

void FuncPanelWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    int left, top, right, bottom;
    layout()->getContentsMargins(&left, &top, &right, &bottom);
    m_Icon->setFixedSize(height() - top - bottom, height() - top - bottom);
}
