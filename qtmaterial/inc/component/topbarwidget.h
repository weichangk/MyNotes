#pragma once
#include "qtmaterial_global.h"
#include "canmovewidget.h"
#include <QWidget>
#include <QPushButton>

class QTMATERIAL_EXPORT TopbarWidget : public CanMoveWidget {
    Q_OBJECT
public:
    TopbarWidget(QWidget *parent);
    ~TopbarWidget();
    QWidget *contentWidget();
    void setNormalVisible(bool visible);
    void setCloseBtnTopRight10Radius();
    void setMinVisible(bool visible);
    void setMaxVisible(bool visible);
    void setCloseVisible(bool visible);

Q_SIGNALS:
    void sigMin();
    void sigMax();
    void sigNormal();
    void sigClose();

protected:
    void createUi();
    void sigConnect();

private:
    QPushButton *m_minBtn;
    QPushButton *m_maxBtn;
    QPushButton *m_normalBtn;
    QPushButton *m_closeBtn;
    QPushButton *m_minMacBtn;
    QPushButton *m_maxMacBtn;
    QPushButton *m_normalMacBtn;
    QPushButton *m_closeMacBtn;
    QWidget *m_contentWidget;
};
