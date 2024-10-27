#include "component/importguidewidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

ImportGuideWidget::ImportGuideWidget(QWidget *parent) :
    QWidget(parent) {
    createUi();
    sigConnect();
}

ImportGuideWidget::~ImportGuideWidget() {
}

void ImportGuideWidget::createUi() {
    import_btn_ = new QPushButton(this);
    import_btn_->setFixedSize(200, 200);

    auto btn_layout = new QVBoxLayout();
    btn_layout->addStretch();
    btn_layout->addWidget(import_btn_);
    btn_layout->addStretch();

    auto mainLayout = new QHBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addLayout(btn_layout);
    mainLayout->addStretch();
}

void ImportGuideWidget::sigConnect() {
    connect(import_btn_, &QPushButton::clicked, this, &ImportGuideWidget::sigClicked);
}