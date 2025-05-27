#pragma once
#include <QWidget>
#include <QPushButton>

void standard_dialog();

class StandardDialogWidget : public QWidget {
    Q_OBJECT
public:
    explicit StandardDialogWidget(QWidget *parent = nullptr);
    ~StandardDialogWidget();

private slots:
    void openColorDialog();
    void openFileDialog();
    void openFontDialog();
    void openInputDialog();
    void openMessageBox();
    void openProgressDialog();
    void openErrorMessage();
    void openWizardPage();

private:
    QPushButton *m_pOpenColorDialogBtn = nullptr;
    QPushButton *m_pOpenFileDialogBtn = nullptr;
    QPushButton *m_pOpenFontDialogBtn = nullptr;
    QPushButton *m_pOpenInputDialogBtn = nullptr;
    QPushButton *m_pOpenMessageBoxBtn = nullptr;
    QPushButton *m_pOpenProgressDialogBtn = nullptr;
    QPushButton *m_pOpenErrorMessageBtn = nullptr;
    QPushButton *m_pOpenWizardPageBtn = nullptr;
};

