#include "osx/widgethelper.h"
#import <Cocoa/Cocoa.h>

/*
常用的窗口等级（Window Levels）：
kCGBaseWindowLevelKey:
基本窗口等级，普通应用程序窗口通常使用这个级别。
默认窗口层级。

kCGMinimumWindowLevelKey:
最低窗口等级，用于一些背景窗口或透明窗口。
位于其他窗口之下。

kCGNormalWindowLevelKey:
正常窗口等级，应用程序窗口的默认等级。

kCGFloatingWindowLevelKey:
浮动窗口等级，常用于浮动面板或工具窗口。
例如系统的工具栏窗口，通常位于普通窗口之上。

kCGTornOffMenuWindowLevelKey:
撕开的菜单窗口等级，用于显示从菜单中“撕下”的菜单项。

kCGDockWindowLevelKey:
Dock 窗口的等级，常用于 macOS 的 Dock 或其他固定在屏幕边缘的窗口。

kCGMainMenuWindowLevelKey:
主菜单窗口等级，主要用于系统或应用程序的主菜单。

kCGStatusWindowLevelKey:
状态窗口等级，用于状态栏窗口。

kCGModalPanelWindowLevelKey:
模态面板窗口等级，主要用于模态对话框，它们会阻塞其他窗口的操作。

kCGUtilityWindowLevelKey:
实用窗口等级，通常用于非关键性、工具类的窗口。
比正常窗口高，但不如浮动窗口高。

kCGScreenSaverWindowLevelKey:
屏保窗口等级，用于屏幕保护程序，几乎覆盖一切其他窗口。

kCGMaximumWindowLevelKey:
最高窗口等级，置顶窗口的等级，通常用于全屏显示的窗口或覆盖所有其他窗口。

kCGOverlayWindowLevelKey:
覆盖窗口等级，通常用于在屏幕上绘制一些叠加内容，常用于游戏、媒体播放器等。

调整窗口级别的典型场景：
正常应用窗口：kCGNormalWindowLevelKey。
工具窗口或浮动面板：kCGFloatingWindowLevelKey。
模态对话框：kCGModalPanelWindowLevelKey。
覆盖所有窗口的全屏应用：kCGMaximumWindowLevelKey 或 kCGScreenSaverWindowLevelKey。
*/

void MacSetWidgetLevel(QWidget *w, int level)
{
    if (!w)
    {
        return;
    }

    NSView *pNSview = reinterpret_cast<NSView *>(w->winId());
    if (!pNSview)
    {
        return;
    }

    NSWindow *pNSWin = [pNSview window];
    if (!pNSWin)
    {
        return;
    }

    [pNSWin setLevel:level];
}

void setWidgetStaysOnTop(QWidget *w)
{
    if (!w)
    {
        return;
    }

    NSView *pNSview = reinterpret_cast<NSView *>(w->winId());
    if (!pNSview)
    {
        return;
    }

    NSWindow *pNSWin = [pNSview window];
    if (!pNSWin)
    {
        return;
    }

    // 设置窗口置顶
    [pNSWin setLevel:CGWindowLevelForKey(kCGMaximumWindowLevelKey)];
}