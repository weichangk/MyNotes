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
#pragma once
#include <QWidget>

void modal_dialog();

class ModalDialogWidget : public QWidget {
    Q_OBJECT
public:
    explicit ModalDialogWidget(QWidget *parent = nullptr);
    ~ModalDialogWidget();
};
```
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

### 多窗口切换
```cpp
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
```
```cpp
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
```
### 标准对话框
```cpp
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
```
```cpp
#include "standard_dialog.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QErrorMessage>
#include <QWizardPage>
#include <QLineEdit>
#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>

/*
Qt 标准对话框:
    - 颜色对话框
    - 文件对话框
    - 字体对话框
    - 输入对话框
    - 消息对话框
    - 进度对话框
    - 错误信息对话框
    - 向导页面
*/

void standard_dialog() {
    StandardDialogWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &StandardDialogWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

StandardDialogWidget::StandardDialogWidget(QWidget *parent) :
    QWidget(parent) {
    m_pOpenColorDialogBtn = new QPushButton(this);
    m_pOpenFileDialogBtn = new QPushButton(this);
    m_pOpenFontDialogBtn = new QPushButton(this);
    m_pOpenInputDialogBtn = new QPushButton(this);
    m_pOpenMessageBoxBtn = new QPushButton(this);
    m_pOpenProgressDialogBtn = new QPushButton(this);
    m_pOpenErrorMessageBtn = new QPushButton(this);
    m_pOpenWizardPageBtn = new QPushButton(this);

    m_pOpenColorDialogBtn->setText("颜色对话框");
    m_pOpenFileDialogBtn->setText("文件对话框");
    m_pOpenFontDialogBtn->setText("字体对话框");
    m_pOpenInputDialogBtn->setText("输入对话框");
    m_pOpenMessageBoxBtn->setText("消息对话框");
    m_pOpenProgressDialogBtn->setText("进度对话框");
    m_pOpenErrorMessageBtn->setText("错误信息对话框");
    m_pOpenWizardPageBtn->setText("向导页面");

    m_pOpenColorDialogBtn->setGeometry(10, 10, 200, 30);
    m_pOpenFileDialogBtn->setGeometry(10, 50, 200, 30);
    m_pOpenFontDialogBtn->setGeometry(10, 90, 200, 30);
    m_pOpenInputDialogBtn->setGeometry(10, 130, 200, 30);
    m_pOpenMessageBoxBtn->setGeometry(10, 170, 200, 30);
    m_pOpenProgressDialogBtn->setGeometry(10, 210, 200, 30);
    m_pOpenErrorMessageBtn->setGeometry(10, 250, 200, 30);
    m_pOpenWizardPageBtn->setGeometry(10, 290, 200, 30);

    connect(m_pOpenColorDialogBtn, &QPushButton::clicked, this, &StandardDialogWidget::openColorDialog);
    connect(m_pOpenFileDialogBtn, &QPushButton::clicked, this, &StandardDialogWidget::openFileDialog);
    connect(m_pOpenFontDialogBtn, &QPushButton::clicked, this, &StandardDialogWidget::openFontDialog);
    connect(m_pOpenInputDialogBtn, &QPushButton::clicked, this, &StandardDialogWidget::openInputDialog);
    connect(m_pOpenMessageBoxBtn, &QPushButton::clicked, this, &StandardDialogWidget::openMessageBox);
    connect(m_pOpenProgressDialogBtn, &QPushButton::clicked, this, &StandardDialogWidget::openProgressDialog);
    connect(m_pOpenErrorMessageBtn, &QPushButton::clicked, this, &StandardDialogWidget::openErrorMessage);
    connect(m_pOpenWizardPageBtn, &QPushButton::clicked, this, &StandardDialogWidget::openWizardPage);
}

StandardDialogWidget::~StandardDialogWidget() {
}

void StandardDialogWidget::openColorDialog() {
    // QColor color = QColorDialog::getColor(Qt::red, this, "颜色对话框", QColorDialog::ShowAlphaChannel);

    QColorDialog dialog(Qt::red, this);               // 创建对象
    dialog.setOption(QColorDialog::ShowAlphaChannel); // 显示alpha选项
    dialog.exec();                                    // 以模态方式运行对话框
    QColor color = dialog.currentColor();             // 获取当前颜色
    qDebug() << "color: " << color;
}

void StandardDialogWidget::openFileDialog() {
    // QString fileName = QFileDialog::getOpenFileName(this, "文件对话框"), "D:", "图片文件(*png *jpg);;文本文件(*txt)"));
    // qDebug() << "fileName:" << fileName;

    QStringList fileNames = QFileDialog::getOpenFileNames(this, "文件对话框", "D:", "图片文件(*png *jpg)");
    qDebug() << "fileNames:" << fileNames;
}

void StandardDialogWidget::openFontDialog() {
    // ok用于标记是否按下了“OK”按钮
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this);
    // 如果按下“OK”按钮，那么让“字体对话框”按钮使用新字体
    // 如果按下“Cancel”按钮，那么输出信息
    if (ok) {
        m_pOpenFontDialogBtn->setFont(font);
    } else {
        qDebug() << "没有选择字体！";
    }
}

void StandardDialogWidget::openInputDialog() {
    bool ok;
    // 获取字符串
    QString string = QInputDialog::getText(this, "输入字符串对话框", "请输入用户名：", QLineEdit::Normal, "admin", &ok);
    if (ok) {
        qDebug() << "string:" << string;
    }
    // 获取整数
    int value1 = QInputDialog::getInt(this, "输入整数对话框", "请输入-1000到1000之间的数值", 100, -1000, 1000, 10, &ok);
    if (ok) {
        qDebug() << "value1:" << value1;
    }
    // 获取浮点数
    double value2 = QInputDialog::getDouble(this, "输入浮点数对话框", "请输入-1000到1000之间的数值", 0.00, -1000, 1000, 2, &ok);
    if (ok) {
        qDebug() << "value2:" << value2;
    }
    QStringList items;
    items << "条目1" << "条目2";
    // 获取条目
    QString item = QInputDialog::getItem(this, "输入条目对话框", "请选择或输入一个条目", items, 0, true, &ok);
    if (ok) {
        qDebug() << "item:" << item;
    }
}

void StandardDialogWidget::openMessageBox() {
    // 问题对话框
    int ret1 = QMessageBox::question(this, "问题对话框", "你了解Qt吗？", QMessageBox::Yes, QMessageBox::No);
    if (ret1 == QMessageBox::Yes) {
        qDebug() << "问题！";
    }
    // 提示对话框
    int ret2 = QMessageBox::information(this, "提示对话框", "这是Qt书籍！", QMessageBox::Ok);
    if (ret2 == QMessageBox::Ok) {
        qDebug() << "提示！";
    }
    // 警告对话框
    int ret3 = QMessageBox::warning(this, "警告对话框", "不能提前结束！", QMessageBox::Abort);
    if (ret3 == QMessageBox::Abort) {
        qDebug() << "警告！";
    }
    // 错误对话框
    int ret4 = QMessageBox::critical(this, "严重错误对话框", "发现一个严重错误！现在要关闭所有文件！", QMessageBox::YesAll);
    if (ret4 == QMessageBox::YesAll) {
        qDebug() << "错误";
    }
    // 关于对话框
    QMessageBox::about(this, "关于对话框", "Qt 标准对话框的学习案例！");
}

void StandardDialogWidget::openProgressDialog() {
    QProgressDialog dialog("文件复制进度", "取消", 0, 50000, this);
    dialog.setWindowTitle("进度对话框");       // 设置窗口标题
    dialog.setWindowModality(Qt::WindowModal); // 将对话框设置为模态
    dialog.show();
    for (int i = 0; i < 50000; i++) {      // 演示复制进度
        dialog.setValue(i);                // 设置进度条的当前值
        QCoreApplication::processEvents(); // 避免界面冻结
        if (dialog.wasCanceled()) break;   // 按下取消按钮则中断
    }
    dialog.setValue(50000); // 这样才能显示100%，因为for循环中少加了一个数
    qDebug() << "复制结束！";
}

void StandardDialogWidget::openErrorMessage() {
    auto errordlg = new QErrorMessage(this);
    errordlg->setWindowTitle(tr("错误信息对话框"));
    errordlg->showMessage(tr("这里是出错信息！"));
}

void StandardDialogWidget::openWizardPage() {
    auto createPage1 = []() {
        QWizardPage *page = new QWizardPage;
        page->setTitle(tr("介绍"));
        return page;
    };

    auto createPage2 = []() {
        QWizardPage *page = new QWizardPage;
        page->setTitle(tr("用户选择信息"));
        return page;
    };

    auto createPage3 = []() {
        QWizardPage *page = new QWizardPage;
        page->setTitle(tr("结束"));
        return page;
    };

    QWizard wizard(this);
    wizard.setWindowTitle(tr("向导对话框"));
    wizard.addPage(createPage1());
    wizard.addPage(createPage2());
    wizard.addPage(createPage3());
    wizard.exec();
}
```
## QFrame 类族
### QFrame 的使用
```
#include "frame_use.h"

#include <QEventLoop>
#include <QFrame>
#include <QLabel>
#include <QMetaEnum>
/*
在 Qt 中，QFrame 是一个用于绘制边框和背景的基本界面控件，它继承自 QWidget，主要作用是作为其他控件的容器或用于视觉分隔，提供简单的框架结构

QFrame 并不是一个功能控件（如按钮、标签），而是一个视觉辅助组件，主要提供以下功能：
1. 显示边框或凹槽：可通过 frameShape 和 frameShadow 设置不同的边框风格。
2. 布局容器：可在其内部放置其他控件，作为一个分组区域。
3. 视觉分隔线：通过设置为 HLine 或 VLine，可以作为一条水平/垂直的线来分隔界面区域。

常用属性
1. setFrameShape(QFrame::Shape)设置边框的形状，常用值有：
    - NoFrame：无边框
    - Box：盒子边框
    - Panel：面板样式
    - HLine：水平线
    - VLine：垂直线
    - StyledPanel：风格化面板（常用于分组）

2. setFrameShadow(QFrame::Shadow)设置边框的阴影样式，常用值有：
    - Plain：无阴影
    - Raised：凸起效果
    - Sunken：凹陷效果

3. setLineWidth() / setMidLineWidth()设置边框线的宽度。

在 Qt 中，QFrame 默认的边框颜色是由当前的 样式（Style） 决定的，比如 Fusion、Windows、macOS 等平台样式。要自定义边框颜色，有以下几种常见方法：
1. 方法一：使用 Qt 样式表
2. 方法二：子类化 QFrame 自定义绘制
3. 方法三：直接设置 QPalette

使用 QSS 或 setStyleSheet("QFrame { border: ... }")，QFrame 的 setFrameShape 和 setFrameShadow 设置就会失效，如何同时保留原始立体边框效果并改变颜色，要做到这一点，需要手动绘制边框或使用 QPalette 设置 Panel 风格的颜色
    - 原始立体边框的颜色如何通过 QPalette 设置，具体可查看 ColorRole 属性，不同的 Shape 和 Shadow 属性对不同的 ColorRole 起作用
        QFrame* frame = new QFrame(this);
        frame->setFrameShape(QFrame::Panel);
        frame->setFrameShadow(QFrame::Sunken);
        frame->setLineWidth(3);
        frame->setMidLineWidth(2);
        QPalette p = frame->palette();
        p.setColor(QPalette::Dark, Qt::red);     // 影响外框暗边（Sunken 时）
        p.setColor(QPalette::Light, Qt::green);  // 影响外框亮边
        p.setColor(QPalette::Mid, Qt::blue);     // 理论上影响中线（部分平台）
        frame->setPalette(p);
        frame->setAutoFillBackground(true);

    - 彻底自绘 Line + MidLine 改变边框颜色
        void paintEvent(QPaintEvent* event) override {
            QPainter painter(this);
            QRect outer = rect().adjusted(0, 0, -1, -1);

            int lw = lineWidth();
            int mlw = midLineWidth();

            // Line（外框）
            painter.setPen(QPen(lineColor, lw));
            painter.drawRect(outer);

            // MidLine（内框）
            if (mlw > 0) {
                QRect inner = outer.adjusted(lw, lw, -lw, -lw);
                painter.setPen(QPen(midLineColor, mlw));
                painter.drawRect(inner);
            }

总结建议：
- 保留阴影/边框风格 + 修改颜色	            重写 paintEvent()
- 快速设置边框颜色但无阴影	                setStyleSheet()
- 想要平台风格的 Panel/边框效果	            使用 setFrameShape + QPalette
- 想要兼顾美观与灵活                       使用 QStyleOptionFrame 和自绘

*/ 

void frame_use() {
    FrameUseWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &FrameUseWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

QString shapeEnumName(QFrame::Shape shape) {
    const QMetaObject &metaObject = QFrame::staticMetaObject;
    int index = metaObject.indexOfEnumerator("Shape");
    QMetaEnum metaEnum = metaObject.enumerator(index);
    return metaEnum.valueToKey(shape);
}

QString shadowEnumName(QFrame::Shadow shadow) {
    const QMetaObject &metaObject = QFrame::staticMetaObject;
    int index = metaObject.indexOfEnumerator("Shadow");
    QMetaEnum metaEnum = metaObject.enumerator(index);
    return metaEnum.valueToKey(shadow);
}

FrameUseWidget::FrameUseWidget(QWidget *parent) :
    QWidget(parent) {
    QList<QFrame::Shape> shapeList = {QFrame::Box, QFrame::Panel, QFrame::StyledPanel, QFrame::HLine, QFrame::VLine, QFrame::WinPanel};
    QList<QFrame::Shadow> shadowList = {QFrame::Plain, QFrame::Raised, QFrame::Sunken};

    int lineWidth = 8;
    int midLineWidth = 4;
    int frameWidth = 240;
    int frameHeight = 120;

    int left = 10, top = 10;

    for (int shapeIndex = 0; shapeIndex < shapeList.count(); shapeIndex++) {
        for (int shadowIndex = 0; shadowIndex < shadowList.count(); shadowIndex++) {
            auto frame = new QFrame(this);
            // 设置边框颜色
            QPalette framePalette = frame->palette();
            framePalette.setColor(QPalette::Dark, Qt::red);    // 影响外框暗边
            framePalette.setColor(QPalette::Light, Qt::green); // 影响外框亮边
            framePalette.setColor(QPalette::Mid, Qt::blue);    // 理论上影响中线（部分平台）
            framePalette.setColor(QPalette::WindowText, Qt::yellow);
            frame->setPalette(framePalette);
            frame->setAutoFillBackground(true);

            auto label = new QLabel(frame);
            QPalette labelPalette = label->palette();
            labelPalette.setColor(QPalette::WindowText, Qt::black);
            label->setPalette(labelPalette);
            label->setAutoFillBackground(true);
            label->setText(QString("QFrame:%1 | QFrame:%2").arg(shapeEnumName(shapeList[shapeIndex])).arg(shadowEnumName(shadowList[shadowIndex])));
            label->move(20, 20);

            frame->setLineWidth(lineWidth);
            frame->setMidLineWidth(midLineWidth);
            frame->setFrameShape(shapeList[shapeIndex]);
            frame->setFrameShadow(shadowList[shadowIndex]);
            // 以上两个函数可以使用setFrameStyle(shapeList[shadowIndex] | shadowList[shadowIndex])代替

            int x = (frameWidth + left) * shadowIndex + left;
            int y = (frameHeight + top) * shapeIndex + top;
            frame->setGeometry(x, y, frameWidth, frameHeight);
        }
    }

    setFixedSize((frameWidth + left) * shadowList.count() + left, (frameHeight + top) * shapeList.count() + top);
}

FrameUseWidget::~FrameUseWidget() {
}
```