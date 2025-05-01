#pragma once
#include "core/subject.h"

#include <QWidget>
#include <QPushButton>

class ISubjectTestEventObserverWidgetService;
class SubjectTestEventObserverWidget;

class SubjectTestWidget : public QWidget {
    Q_OBJECT
public:
    SubjectTestWidget(QWidget *parent = nullptr);
    ~SubjectTestWidget() override {
    }

private:
    QPushButton *m_pBtn1 = nullptr;
    SubjectTestEventObserverWidget *p_mSubjectTestEventObserverWidget = nullptr;
};

class ISubjectTestEventObserver {
public:
    virtual void subjectTestEvent(int n) = 0;
};

class SubjectTestEventObserverWidget : public QWidget, public ISubjectTestEventObserver {
    Q_OBJECT
public:
    SubjectTestEventObserverWidget(QWidget *parent = nullptr);
    ~SubjectTestEventObserverWidget();
    void subjectTestEvent(int n) override;
    void test(int n);

private:
    ISubjectTestEventObserverWidgetService *m_pService = nullptr;
};

class ISubjectTestEventObserverWidgetService {
public:
    virtual void attach(ISubjectTestEventObserver *observer) = 0;
    virtual void detach(ISubjectTestEventObserver *observer) = 0;
    virtual void testEvent(int n) = 0;
};

class SubjectTestEventObserverWidgetService : public ISubjectTestEventObserverWidgetService {
public:
    void attach(ISubjectTestEventObserver *observer);
    void detach(ISubjectTestEventObserver *observer);
    void testEvent(int n);

private:
    qtmaterialcore::Subject<ISubjectTestEventObserver> m_Observer;
    // 常用于封装功能接口的服务，回调调用 m_Observer.notify 再响应监听者
};