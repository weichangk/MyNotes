#pragma once
#include "qtmaterial_global.h"
#include <QWidget>
#include <QPushButton>

class QTMATERIAL_EXPORT ImportGuideWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImportGuideWidget(QWidget *parent = nullptr);
    ~ImportGuideWidget();

Q_SIGNALS:
    void sigClicked();

protected:
    void createUi();
    void sigConnect();
    
private:
    QPushButton *import_btn_;
};