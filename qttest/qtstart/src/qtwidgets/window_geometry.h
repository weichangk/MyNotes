
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