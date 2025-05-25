## 基础窗口部件 QWidget

QWidget类关系图：

![QWidget类关系图](./assets/QWidget%E7%B1%BB%E5%85%B3%E7%B3%BB%E5%9B%BE.png)

### 窗口与子部件
```cpp
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
```
### 窗口类型
```cpp

#include <QWidget>

void window_type() {
    /*
    在 Qt 中，窗口类型是由 Qt::WindowType 枚举类表示的，它控制窗口的行为和外观（例如是否是顶层窗口、对话框、工具窗口、弹出窗口等）。常用的窗口类型包括
    一、顶层窗口类型（常用于主窗口）
        窗口类型	         描述
        Qt::Widget	        默认类型。没有特别的窗口标志，通常是子控件。
        Qt::Window	        独立窗口（带标题栏和边框），可以作为主窗口。
        Qt::Dialog	        对话框窗口（可模态）。
        Qt::Sheet	        macOS 特有，模态表单对话框。
        Qt::Drawer	        macOS 特有，从窗口侧边滑出的抽屉窗口。
        Qt::Popup	        弹出窗口，常用于菜单、下拉框等，点击外部区域会自动关闭。
        Qt::Tool	        工具窗口，通常是浮动的小窗口（比如工具栏）。
        Qt::ToolTip	        工具提示窗口，没有标题栏，自动消失。
        Qt::SplashScreen	启动画面窗口，显示在应用程序启动时。
        Qt::Desktop	        桌面窗口对象，代表整个屏幕（很少用）。

    二、窗口标志（可以和窗口类型组合使用）
        这些标志控制窗口的样式、边框、是否可关闭/最大化等，通常和窗口类型配合使用：
        标志	                        描述
        Qt::CustomizeWindowHint	        关闭默认窗口装饰，完全自定义。
        Qt::WindowTitleHint	            窗口有标题栏。
        Qt::WindowSystemMenuHint	    系统菜单（右键菜单）。
        Qt::WindowMinimizeButtonHint	最小化按钮。
        Qt::WindowMaximizeButtonHint	最大化按钮。
        Qt::WindowCloseButtonHint	    关闭按钮。
        Qt::WindowStaysOnTopHint	    窗口总在最前面。
        Qt::FramelessWindowHint	        无边框窗口（无标题栏、无边框）。

    三、应用场景推荐类型
        应用场景	                    推荐类型
        主窗口（主界面）	             Qt::Window
        弹窗（设置界面）	             Qt::Dialog
        右键菜单、下拉列表	             Qt::Popup
        工具提示	                    Qt::ToolTip
        工具条浮窗	                    Qt::Tool
        无边框自定义窗口	             Qt::FramelessWindowHint

    四、注意事项
        在 Qt 中，如果你为一个 QWidget 设置了父对象，那么某些窗口类型标志（尤其是 Qt::Window 系列）会部分失效或表现不符合预期。
        只有在没有父对象（即顶层窗口）时，Qt::WindowType 才能真正发挥作用。
        情况	                                                    结果
        new QWidget(nullptr, Qt::Window)	                        ✅ 创建顶层窗口，Qt::Window 生效，有标题栏和边框。
        new QWidget(parent, Qt::Window)	                            ⚠️ 不会成为顶层窗口，仅作为父控件内的子控件。标志大多无效。
        setParent(someParent) 后再 setWindowFlags(Qt::Window)	    ⚠️ 不会成为顶层窗口，仍是嵌套控件。
        widget->setParent(nullptr) 后再 setWindowFlags(Qt::Window)	✅ 变成独立窗口，标志生效。
    */

    // 设置窗口类型的方法：
    // 1.在构造窗口时指定
    QWidget *window1 = new QWidget(nullptr, Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    window1->setWindowTitle(QObject::tr("我是window1"));

    // 2.或者使用 setWindowFlags() 设置
    QWidget *window2 = new QWidget();
    window2->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    window2->setWindowTitle(QObject::tr("我是window2"));

    window1->show();
    window1->move(200, 200);
    window2->show();
    window2->move(400, 400);
}
```

### 窗口几何布局

![窗口几何布局](./assets/%E7%AA%97%E5%8F%A3%E5%87%A0%E4%BD%95%E5%B8%83%E5%B1%80.png)

```cpp
#include <QWidget>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

void window_geometry() {
    /*
    在 Qt 中，每个控件（QWidget）都具有一个几何属性 geometry()，它表示控件在其父窗口中的位置和大小

    1. 包含窗口边框（frame）的几何信息
        这些函数考虑了 标题栏、边框，用于处理窗口在 屏幕上的实际位置，用于窗口位置管理、居中、屏幕对齐等。
        x() / y()	                    顶层窗口左上角（含边框）相对于父窗口或屏幕的坐标
        pos()	                        等价于 (x(), y())，位置包含边框
        move()	                        设置窗口左上角位置（包含边框）
        frameGeometry()	                返回一个 QRect，表示窗口整个外框（标题栏+边框+内容区）的位置和尺寸

    2. 不包含边框的“内容区”几何信息
        这些函数只针对实际显示内容区域（不含边框/标题栏），常用于控件尺寸计算、绘图、内容布局。
        geometry()	                    返回 QRect，表示内容区在父控件中的位置和大小
        width() / height()	            内容区的宽高
        rect()	                        同样返回内容区的 QRect（左上为 (0,0)，宽高同上）
        size()	                        返回内容区的 QSize(width, height)
    */

    QWidget widget;
    // 设置窗口大小
    widget.resize(400, 300);
    // 设置窗口位置   
    widget.move(200, 100);         
    widget.show();

    int x = widget.x();
    qDebug("x: %d", x);            // 输出x的值
    int y = widget.y();
    qDebug("y: %d", y);
    QRect geometry = widget.geometry();
    QRect frame = widget.frameGeometry();
    qDebug() << "geometry: " << geometry << "frame: " << frame;

    // 进入子事件循环，5秒后退出
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();
}
```

## 对话框QDialog

### 模态和非模态对话框
```cpp
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
```
### 标准对话框

## 其他窗口部件

### QFrame