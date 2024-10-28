#pragma once
#include "qtmaterial_global.h"
#include "icontextwidget.h"
#include <QWidget>
#include <QMap>

class QTMATERIAL_EXPORT NavbarWidget : public QWidget {
    Q_OBJECT
public:
    NavbarWidget(QMap<int, QVariantList> data, QWidget *parent = 0);
    ~NavbarWidget();

Q_SIGNALS:
    void sigClicked(int index);

protected:
    void createUi();
    void sigConnect();

private:
    void exclusive(IconTextWidget *widget);

private:
    // QVariantList: icon text
    QMap<int, QVariantList> m_Data;
};
