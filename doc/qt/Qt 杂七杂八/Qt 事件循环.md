在 Qt 中，进入事件循环（event loop）通常是通过调用 QApplication::exec() 或 QCoreApplication::exec() 实现的。这个调用会启动主事件循环，程序会开始响应用户输入、定时器、网络事件等。

## GUI 程序进入事件循环
如果你的程序是图形界面应用，使用的是 QApplication
```cpp
#include <QApplication>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);  // 创建应用对象
    QPushButton button("Hello, Qt!");
    button.show();                 // 显示窗口
    return app.exec();             // 进入事件循环
}
```

## 非GUI（控制台）程序进入事件循环
如果是控制台应用程序（比如不涉及图形界面），使用 QCoreApplication
```cpp
#include <QCoreApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTimer::singleShot(1000, []() {
        qDebug("Timer triggered");
        QCoreApplication::quit();  // 1秒后退出事件循环
    });

    return app.exec();  // 启动事件循环
}
```

## 手动创建子事件循环
某些特殊场景中你可能需要手动创建子事件循环，比如等待某个信号返回
```cpp
#include <QEventLoop>
#include <QTimer>

QEventLoop loop;
QTimer::singleShot(1000, &loop, &QEventLoop::quit);
loop.exec();  // 进入子事件循环，1秒后退出
```

## 主事件循环和子事件循环的区别
主事件循环和子事件循环的关系可以简单理解为：主事件循环是整个 Qt 应用运行的基础，而子事件循环是主事件循环内部临时嵌套的一层“等待机制”，它们之间有层级和依赖关系。

主事件循环是由 QApplication::exec() 或 QCoreApplication::exec() 启动的，exec() 会阻塞在此处，直到应用退出，是 Qt 应用程序运行的核心。
- 它负责处理用户输入（鼠标、键盘）
- 管理窗口事件（显示、移动、关闭）
- 调度信号和槽
- 管理定时器、网络等异步事件

子事件循环（通过 QEventLoop 创建）是在主事件循环之内嵌套的一层事件循环，可以临时“卡住当前函数的执行”来等待某些事件完成，比如：
- 同步等待网络请求结果
- 等待某个窗口关闭
- 等待用户交互完成
- 实现类似“阻塞式信号等待”