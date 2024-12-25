#include "shadowtest.h"

#include <QVBoxLayout>
#include <QLabel>

ShadowTestWidget::ShadowTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("ShadowTestWidget");
    setWindowTitle("Shadow Test");
    setFixedSize(800, 600);

    auto layout = new QVBoxLayout(this);

    QLabel *label = new QLabel(this);
    label->setText("Shadow Test");

    layout->addWidget(label, 0, Qt::AlignCenter);

    m_pShadowEffect = new qtmaterialfilter::ShadowEffect(this);
}

ShadowTestWidget::~ShadowTestWidget() {
    m_pShadowEffect->release();
}