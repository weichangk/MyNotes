#include "component/navbarwidget.h"
#include <QVBoxLayout>
#include <QVariant>

NavbarWidget::NavbarWidget(QMap<int, QVariantList> data, QWidget *parent) :
    QWidget(parent),
    m_Data(data) {
    createUi();
    sigConnect();
}

NavbarWidget::~NavbarWidget() {
}

void NavbarWidget::createUi() {
    auto mainLayout = new QVBoxLayout(this);
    QMap<int, QVariantList>::Iterator iter;
    for (iter = m_Data.begin(); iter != m_Data.end(); ++iter) {
        QPixmap pix(24 * 4, 24);
        pix.load(iter.value().at(0).toString());
        auto btn = new IconTextWidget(this);
        btn->setObjectName("m_FuncBtn");
        btn->setFixedHeight(32);
        btn->getIcon()->setFixedSize(24, 24);
        btn->getIcon()->setFourPixmap(pix);
        btn->getText()->setText(iter.value().at(1).toString());
        connect(btn, &IconTextWidget::sigClicked, this, [=]() {
            exclusive(btn);
            emit sigClicked(iter.key());
        });
        mainLayout->addWidget(btn);
    }
    mainLayout->addStretch();
}

void NavbarWidget::sigConnect() {
}

void NavbarWidget::exclusive(IconTextWidget *widget) {
    QList<IconTextWidget *> btns = findChildren<IconTextWidget *>();
    for (int i = 0; i < btns.size(); i++) {
        btns[i]->setState(FourStateImageWidget::StyleStatus::Normal);
    }
    widget->setState(FourStateImageWidget::StyleStatus::Checked);
}
