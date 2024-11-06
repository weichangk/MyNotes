#include "widget/navbar.h"
#include <QVBoxLayout>
#include <QVariant>

namespace widget {
Navbar::Navbar(QMap<int, QVariantList> data, QWidget *parent) :
    QWidget(parent),
    m_Data(data) {
    createUi();
    sigConnect();
}

Navbar::~Navbar() {
}

void Navbar::createUi() {
    auto mainLayout = new QVBoxLayout(this);
    QMap<int, QVariantList>::Iterator iter;
    for (iter = m_Data.begin(); iter != m_Data.end(); ++iter) {
        QPixmap pix(24 * 4, 24);
        pix.load(iter.value().at(0).toString());
        auto btn = new IconText(this);
        btn->setObjectName("m_FuncBtn");
        btn->setFixedHeight(32);
        btn->getIcon()->setFixedSize(24, 24);
        btn->getIcon()->setFourPixmap(pix);
        btn->getText()->setText(iter.value().at(1).toString());
        connect(btn, &IconText::sigClicked, this, [=]() {
            exclusive(btn);
            emit sigClicked(iter.key());
        });
        mainLayout->addWidget(btn);
    }
    mainLayout->addStretch();
}

void Navbar::sigConnect() {
}

void Navbar::exclusive(IconText *widget) {
    QList<IconText *> btns = findChildren<IconText *>();
    for (int i = 0; i < btns.size(); i++) {
        btns[i]->setState(FourStateImage::StyleStatus::Normal);
    }
    widget->setState(FourStateImage::StyleStatus::Checked);
}
} // namespace widget