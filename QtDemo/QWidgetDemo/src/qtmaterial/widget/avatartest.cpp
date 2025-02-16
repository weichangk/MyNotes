#include "avatartest.h"

#include <QVBoxLayout>

AvatarTestWidget::AvatarTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("AvatarTestWidget");
    setWindowTitle("Avatar Test");
    setFixedSize(1000, 600);

    auto layout = new QVBoxLayout(this);

    m_pAvatar1 = new qtmaterialwidget::Avatar(this);
    m_pAvatar1->setFixedSize(100, 100);
    m_pAvatar1->setBorderRadius(50);
    m_pAvatar1->setAvatar(":/test/avatar");

    m_pAvatar2 = new qtmaterialwidget::Avatar(this);
    m_pAvatar2->setFixedSize(100, 100);
    m_pAvatar2->setBorderRadius(50);
    m_pAvatar2->setAvatar(":/test/avatar");
    m_pAvatar2->setHasBorder(true);

    layout->addWidget(m_pAvatar1, 0, Qt::AlignCenter);
    layout->addWidget(m_pAvatar2, 0, Qt::AlignCenter);
    layout->addStretch();

}