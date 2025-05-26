#pragma once
#include <QWidget>
#include <QDialog>
#include <QPushButton>

void window_switch();

class WindowSwitchWidget : public QWidget {
    Q_OBJECT
public:
    explicit WindowSwitchWidget(QWidget *parent = nullptr);
    ~WindowSwitchWidget();
    
Q_SIGNALS:
    void sigExit();

private slots:
    void onShowDialogButtonClicked();
    void onOnlyShowDialogButtonClicked();
    void onExitButtonClicked();

private:
    QPushButton *m_pShowDialogButton = nullptr;
    QPushButton *m_pOnlyShowDialogButton = nullptr;
    QPushButton *m_pExitButton = nullptr;
};

class WindowSwitchDialog : public QDialog {
    Q_OBJECT
public:
    explicit WindowSwitchDialog(QWidget *parent = nullptr);
    ~WindowSwitchDialog();

private slots:
    void onAcceptButtonClicked();

private:
    QPushButton *m_pAcceptButton = nullptr;
};
