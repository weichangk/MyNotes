```cpp
void MacSetWindowResizeEnable(QWidget *pWidget)
{
    NSView * view = (NSView *)pWidget->winId();
    NSWindow * nsWin = [view window];
    [nsWin setStyleMask:[nsWin styleMask] | NSWindowStyleMaskResizable];
}
```

NSWindowStyleMaskResizable 是 macOS 中的窗口样式之一，表示允许用户拖动窗口边缘来调整大小。
若使用的是 Qt 6，则推荐使用 QNativeInterface::QCocoaWindow 来获取原生对象，避免使用 winId()。