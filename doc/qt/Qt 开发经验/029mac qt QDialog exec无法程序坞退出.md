# macOS 下 Qt QDialog 使用 exec 打开时 Dock 无法右键退出问题

## 问题描述

在 macOS 上使用 Qt 开发桌面应用时，如果程序主窗口为 `QDialog`，并通过 `exec()` 启动，当用户尝试从 Dock（程序坞）右键点击“退出”时，**无响应或无法正常退出**。

## 产生原因

- `QDialog::exec()` 启动的是 **局部事件循环**，并非整个应用程序的主事件循环。
- 如果 `QApplication::exec()` 没有运行，macOS 无法正常传递退出事件（`NSApplication::terminate:`）到 Qt 应用。
- 没有主窗口时（如 `QMainWindow` 或 `QWidget`），应用在 Dock 中的生命周期管理异常。

## 示例代码（问题场景）

```cpp
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDialog dlg;
    dlg.exec(); // 启动模态对话框

    return 0;   // 没有调用 app.exec()
}
```

## 方案一：使用 show() + app.exec()

使用非模态方式显示 QDialog，并启动 Qt 的主事件循环：

```
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDialog dlg;
    dlg.show(); // 非模态方式显示

    return app.exec(); // 启动主事件循环，Dock 可正常退出
}
```
## 方案二：引入主窗口（QMainWindow 或 QWidget）
即使你的应用最终只需要一个 QDialog，也建议创建一个主窗口来维持主事件循环和生命周期管理：
```
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget mainWindow;
    mainWindow.show(); // 主窗口，确保生命周期完整

    QDialog dlg(&mainWindow);
    dlg.exec(); // 作为模态对话框显示

    return app.exec(); // 保持事件循环
}
```