#pragma once
#include <QWidget>

void modal_dialog();

class ModalDialogWidget : public QWidget {
    Q_OBJECT
public:
    explicit ModalDialogWidget(QWidget *parent = nullptr);
    ~ModalDialogWidget();
};

