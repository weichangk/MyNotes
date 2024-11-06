#pragma once
#include "qtmaterial_global.h"
#include <QWidget>
#include <QPushButton>

namespace widget {
class QTMATERIAL_EXPORT ImportGuide : public QWidget {
    Q_OBJECT
public:
    explicit ImportGuide(QWidget *parent = nullptr);
    ~ImportGuide();

Q_SIGNALS:
    void sigClicked();

protected:
    void createUi();
    void sigConnect();

private:
    QPushButton *import_btn_;
};
} // namespace widget