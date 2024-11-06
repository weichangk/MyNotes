#pragma once
#include "qtmaterial_global.h"
#include "icontext.h"
#include <QWidget>
#include <QMap>

namespace widget {
class QTMATERIAL_EXPORT Navbar : public QWidget {
    Q_OBJECT
public:
    Navbar(QMap<int, QVariantList> data, QWidget *parent = 0);
    ~Navbar();

Q_SIGNALS:
    void sigClicked(int index);

protected:
    void createUi();
    void sigConnect();

private:
    void exclusive(IconText *widget);

private:
    // QVariantList: icon text
    QMap<int, QVariantList> m_Data;
};
} // namespace widget
