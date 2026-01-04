#include "event_demo.h"
#include "../flowlayout.h"

#include <QPushbutton>
#include <QMap>
#include <QVariant>

QtEventWidget::QtEventWidget(QWidget *parent) :
    QWidget(parent) {
    createUi();
}

QtEventWidget::~QtEventWidget() {
}

void QtEventWidget::createUi() {
    setWindowTitle("Qt 事件系统");
    setMinimumSize(1096, 680);
    setTestBtns(this);
}

void QtEventWidget::setTestBtns(QWidget *w) {
    auto flowLayout = new FlowLayout(w, 4, 4, 4);
    QMap<int, QString> qtStartMap;
    qtStartMap.insert(0, "Qt 事件处理");
    qtStartMap.insert(1, "Qt 事件传递");
    qtStartMap.insert(2, "Qt 自定义事件");

    QMap<int, QString>::Iterator iter;
    for (iter = qtStartMap.begin(); iter != qtStartMap.end(); ++iter) {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(240, 24);
        btn->setText(iter.value());
        btn->setProperty("QtEventID", iter.key());
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            int id = btn->property("QtEventID").toInt();
            qtEventTestShow(id);
        });
        flowLayout->addWidget(btn);
    }
}

void QtEventWidget::qtEventTestShow(int id) {
    switch (id) {
    case 0:
        showEventHandingTest();
        break;
    case 1:
        showEventFilterTest();
        break;
    case 2:
        // showCustomEventTest();
        break;
    }
}

void QtEventWidget::showEventHandingTest() {
    if (!m_pEventHandingWidget) {
        m_pEventHandingWidget = new EventHanding::EventHandingWidget();
    }
    m_pEventHandingWidget->show();
}

void QtEventWidget::showEventFilterTest() {
    if (!m_pEventTransferWidget) {
        m_pEventTransferWidget = new EventTransfer::EventTransferWidget();
    }
    m_pEventTransferWidget->show();
}