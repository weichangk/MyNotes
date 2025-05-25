#include "modal_dialog.h"
#include <QDialog>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

void modal_dialog() {
    /*
    QDialog 类是所有对话框窗口类的基类，常被分为模态和非模态对话框
        1.QDialog 使用 show 打开非模态窗口
        2.QDialog 使用 setModal(true) + show 打开模态窗口,不阻塞当前线程
        3.QDialog 使用 setWindowModality(Qt::ApplicationModal) + show 打开模态窗口,不阻塞当前线程
        4.QDialog 使用 setWindowModality(Qt::WindowModal) + show 打开模态窗口,相对父窗口模态,不阻塞当前线程
        5.QDialog 使用 exec 打开模态窗口,阻塞当前线程
    QWidget 也可以实现模态窗口
        QWidget 为顶层窗口时使用 setWindowModality(Qt::ApplicationModal) 打开模态窗口,不阻塞当前线程
     */

    ModalDialogWidget w;
    w.show();

    // 进入子事件循环，5秒后退出
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();
}

ModalDialogWidget::ModalDialogWidget(QWidget *parent) :
    QWidget(parent) {

    // 非模态,立即释放
    // QDialog dlg(this);
    // dlg.setFixedSize(300, 300);
    // dlg.show();

    // 非模态,动态分配需手动释放
    // QDialog *dlg = new QDialog(this);
    // dlg->setFixedSize(300, 300);
    // dlg->show();

    // 模态,阻塞当前线程
    QDialog dlg(this);
    dlg.setFixedSize(300, 300);
    dlg.exec();

    // 模态,非阻塞当前线程:setModal(true) + show()
    // QDialog *dlg = new QDialog(this);
    // dlg->setModal(true); // 默认设置setWindowModality(Qt::ApplicationModal);
    // dlg->show();
    // qDebug() << "仍然会立即执行到这里";

    // 模态,所有其他窗口不可点击,非阻塞当前线程:setWindowModality(Qt::ApplicationModal); + show()
    // QDialog *dlg = new QDialog();
    // dlg->setWindowModality(Qt::ApplicationModal);
    // dlg->show();
    // qDebug() << "仍然会立即执行到这里";

    // 模态,父窗口不可点击,非阻塞当前线程:setWindowModality(Qt::WindowModal); + show()
    // QDialog *dlg = new QDialog(this);
    // dlg->setWindowModality(Qt::WindowModal);
    // dlg->show();
    // qDebug() << "仍然会立即执行到这里";

    // 顶层窗口设置setWindowModality(Qt::ApplicationModal);也有模态效果,非阻塞当前线程
    // QWidget *dlg = new QWidget();
    // dlg->setFixedSize(300, 300);
    // dlg->setWindowModality(Qt::ApplicationModal);
    // dlg->show();
    // qDebug() << "仍然会立即执行到这里";

}

ModalDialogWidget::~ModalDialogWidget() {
}
