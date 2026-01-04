#pragma once
#include "event_handing.h"
#include "event_transfer.h"

#include <QWidget>

class QtEventWidget : public QWidget {
    Q_OBJECT

public:
    QtEventWidget(QWidget *parent = nullptr);
    ~QtEventWidget();

private:
    void createUi();
    void setTestBtns(QWidget *w);
    void qtEventTestShow(int id);
    void showEventHandingTest();
    void showEventFilterTest();

private:
    EventHanding::EventHandingWidget *m_pEventHandingWidget = nullptr;
    EventTransfer::EventTransferWidget *m_pEventTransferWidget = nullptr;
};