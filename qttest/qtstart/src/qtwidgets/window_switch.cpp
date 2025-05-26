#include "window_switch.h"

#include <QEventLoop>

/*
在 Qt 中，经常用使用到多个窗口组成且各窗口之间进行切换的场景
下面是使用 QWidget 和 QDialog 进行切换的简单例子
了解 QDialog 对话框 exec() 的使用
了解 Qt 的信号和槽使用
*/

void window_switch() {
    WindowSwitchDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        WindowSwitchWidget w;
        w.show();
        QEventLoop loop;
        QObject::connect(&w, &WindowSwitchWidget::sigExit, &loop, &QEventLoop::quit);
        loop.exec();
    }
}

WindowSwitchWidget::WindowSwitchWidget(QWidget *parent) :
    QWidget(parent) {
    m_pShowDialogButton = new QPushButton(this);
    m_pShowDialogButton->setGeometry(10, 10, 200, 30);
    m_pShowDialogButton->setText("打开对话框");
    m_pOnlyShowDialogButton = new QPushButton(this);
    m_pOnlyShowDialogButton->setGeometry(10, 50, 200, 30);
    m_pOnlyShowDialogButton->setText("关闭窗口,打开对话框");
    m_pExitButton = new QPushButton(this);
    m_pExitButton->setGeometry(10, 90, 200, 30);
    m_pExitButton->setText("退出");
    connect(m_pShowDialogButton, &QPushButton::clicked, this, &WindowSwitchWidget::onShowDialogButtonClicked);
    connect(m_pOnlyShowDialogButton, &QPushButton::clicked, this, &WindowSwitchWidget::onOnlyShowDialogButtonClicked);
    connect(m_pExitButton, &QPushButton::clicked, this, &WindowSwitchWidget::onExitButtonClicked);
}

WindowSwitchWidget::~WindowSwitchWidget() {
}

void WindowSwitchWidget::onShowDialogButtonClicked() {
    QDialog *dialog = new QDialog(this);
    dialog->show();
}

void WindowSwitchWidget::onOnlyShowDialogButtonClicked() {
    // 先关闭主界面,其实它是隐藏起来了,并没有真正退出
    close();
    // 然后新建WindowSwitchDialog对象
    WindowSwitchDialog dlg;
    // 如果按下了 m_pAcceptButton 按钮,则再次显示主界面
    // 否则,因为现在已经没有显示的界面,所以程序将退出
    // 当直接点击对话框右上角的关闭按钮（X）时,默认行为是调用 reject()
    if (dlg.exec() == QDialog::Accepted) {
        show();
    }
}

void WindowSwitchWidget::onExitButtonClicked() {
    emit sigExit();
}

WindowSwitchDialog::WindowSwitchDialog(QWidget *parent) :
    QDialog(parent) {
    m_pAcceptButton = new QPushButton(this);
    m_pAcceptButton->setGeometry(10, 10, 100, 30);
    m_pAcceptButton->setText("打开窗口");
    connect(m_pAcceptButton, &QPushButton::clicked, this, &WindowSwitchDialog::onAcceptButtonClicked);
}

WindowSwitchDialog::~WindowSwitchDialog() {
}

void WindowSwitchDialog::onAcceptButtonClicked() {
    accept();
}