#include "iconfonttest.h"
#include "helper/fonthelper.h"

#include <QHBoxLayout>

IconFontTestWidget::IconFontTestWidget(QWidget *parent) :
    QWidget(parent) {
    setWindowTitle("IconFont Test");
    setFixedSize(800, 600);

    auto layout = new QHBoxLayout(this); 
    m_pBtn1 = new QPushButton("Button1", this);
    m_pBtn1->setFixedSize(100, 24);
    m_pBtn1->setFont(FontHelper::currentIconFont());
    m_pBtn1->setText(QChar(0xe665));

    layout->addWidget(m_pBtn1);
}