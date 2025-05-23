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

## 对话框QDialog

### 模态和非模态对话框

### 标准对话框

## 其他窗口部件

### QFrame