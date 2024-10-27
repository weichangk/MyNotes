#pragma once
#include "qtmaterial_global.h"
#include <QWidget>
#include <QLabel>

class QTMATERIAL_EXPORT FuncPanelWidget : public QWidget {
    Q_OBJECT
public:
    explicit FuncPanelWidget(QWidget *parent = 0, int id = 0);
    ~FuncPanelWidget();
    QLabel *getName() {
        return m_Name;
    }
    QLabel *getDec() {
        return m_Dec;
    }
    QLabel *getIcon() {
        return m_Icon;
    }
    QLayout *getLayout() {
        return layout();
    }

Q_SIGNALS:
    void sigClicked(int id);

protected:
    void createUi();
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    int m_Id = 0;
    QLabel *m_Name = 0;
    QLabel *m_Dec = 0;
    QLabel *m_Icon = 0;
};
