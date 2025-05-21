#include <QWidget>
#include <QLabel>
#include <QEventLoop>
#include <QTimer>

void windows_and_sub_widgets()
{
    /*
    窗口与子部件：
    1.Qt中把没有嵌套到其他部件中的部件称为窗口，QMainWindow和QDialog及其子类是最一般的窗口类型。
    2.窗口是没有父对象的部件，所以也称为顶级部件（top level widget）。
    3.与窗口相对的是非窗口部件，又称为子部件（child widget），在Qt中大部分部件被作为子部件，嵌入别的窗口中，有父对象。
    */

    // 新建QWidget类对象，默认parent参数是0，所以它是个窗口
    QWidget *widget = new QWidget(0, Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    // 设置窗口标题
    widget->setWindowTitle(QObject::tr("我是widget"));

    // 新建QLabel对象，默认parent参数是0，所以它是个窗口
    QLabel *label = new QLabel(0, Qt::SplashScreen | Qt::WindowStaysOnTopHint);
    label->setWindowTitle(QObject::tr("我是label"));
    // 设置要显示的信息
    label->setText(QObject::tr("label:我是个窗口"));
    // 改变部件大小，以便能显示出完整的内容
    label->resize(180, 20);
    // 设置背景颜色
    label->setStyleSheet("background-color:yellow");

    // label2指定了父窗口为widget，所以不是窗口，是父窗口的子部件
    QLabel *label2 = new QLabel(widget);
    label2->setText(QObject::tr("label2:我不是独立窗口，只是widget的子部件"));
    label2->resize(250, 20);

    // 在屏幕上显示出来
    label->show();
    widget->show();

    // 进入子事件循环，5秒后退出
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    // 销毁窗口对象
    delete label;
    delete widget;

    // 在Qt中销毁父对象时自动销毁子对象，所以label2不需要手动销毁
    // delete label2
}