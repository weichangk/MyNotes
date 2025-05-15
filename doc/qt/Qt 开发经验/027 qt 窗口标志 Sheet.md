# `setWindowFlags(Qt::Sheet);` 的作用

在 macOS 上，`Qt::Sheet` 使窗口成为 **工作表（Sheet）**，通常用于 **模态对话框**，并且会附加到 **父窗口** 之上，而不是独立弹出。

## **使用场景**
- `Qt::Sheet` 主要用于 macOS，它使窗口以 **滑入效果**（从父窗口顶部下滑）显示。
- 适用于 **模态窗口**，如 **警告框、文件对话框** 等。
- 模态窗口带有**蒙层**效果。

## **示例**
```cpp
QDialog *dialog = new QDialog(this);
dialog->setWindowFlags(Qt::Sheet);
dialog->setModal(true);  // 确保模态
dialog->show();
```

## **注意事项**
- 仅在 macOS 生效，在 Windows 和 Linux 上，Qt::Sheet 没有特别效果，窗口行为类似 Qt::Dialog。
- 必须有父窗口，否则 Qt::Sheet 可能不会生效。
- 适用于模态窗口，通常需要 setModal(true) 让它成为模态对话框，否则可能表现异常。