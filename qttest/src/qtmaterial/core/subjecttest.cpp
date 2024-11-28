#include "subjecttest.h"

#include <QHBoxLayout>
#include <QDebug>

SubjectTestWidget::SubjectTestWidget(QWidget *parent) :
    QWidget(parent) {
    setObjectName("SubjectTestWidget");
    setWindowTitle("Subject Test");
    setFixedSize(800, 600);

    auto layout = new QHBoxLayout(this);

    m_pBtn1 = new QPushButton("Btn1", this);
    layout->addWidget(m_pBtn1);

    p_mSubjectTestEventObserverWidget = new SubjectTestEventObserverWidget(this);
    p_mSubjectTestEventObserverWidget->setVisible(false);

    connect(m_pBtn1, &QPushButton::clicked, this, [=](){
        p_mSubjectTestEventObserverWidget->test(100);
    });
}

SubjectTestEventObserverWidget::SubjectTestEventObserverWidget(QWidget *parent) :
    QWidget(parent) {
    m_pService = new SubjectTestEventObserverWidgetService();
    m_pService->attach(this);
}

SubjectTestEventObserverWidget::~SubjectTestEventObserverWidget() {
    m_pService->detach(this);
    delete m_pService;
}

void SubjectTestEventObserverWidget::subjectTestEvent(int n) {
    qDebug() << "SubjectTestEventObserverWidget::subjectTestEvent" << n;
}

void SubjectTestEventObserverWidget::test(int n) {
    m_pService->testEvent(n);
}

void SubjectTestEventObserverWidgetService::attach(ISubjectTestEventObserver *observer) {
    m_Observer.attach(observer);
}

void SubjectTestEventObserverWidgetService::detach(ISubjectTestEventObserver *observer) {
    m_Observer.detach(observer);
}

void SubjectTestEventObserverWidgetService::testEvent(int n) {
    m_Observer.notify(&ISubjectTestEventObserver::subjectTestEvent, n);
}
