下面给你一份 **MFC 面试高频核心点总结（重点围绕：消息机制 / 消息泵 / 架构）**，基本覆盖面试官最爱问的深层问题。

------

# 一、MFC 消息机制（核心必问）

## 1. Windows 消息机制是什么？

Windows 是基于 **消息驱动模型（Message-driven）**：

- 用户操作（点击/键盘/绘制）
- 系统封装成消息（WM_XXX）
- 放入消息队列
- 程序循环取出并处理

常见消息：

- `WM_LBUTTONDOWN`（鼠标左键）
- `WM_PAINT`（绘制）
- `WM_DESTROY`（窗口销毁）

------

## 2. MFC 如何处理消息？

MFC 用的是：

👉 **消息映射（Message Map）机制**

本质：
 用宏 + 静态表，把“消息 → 函数”绑定起来

```
BEGIN_MESSAGE_MAP(CMyDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON1, &CMyDlg::OnButtonClick)
END_MESSAGE_MAP()
```

底层做的事情：

- 维护一个消息映射表
- 查表分发函数调用

------

## 3. Windows 消息 vs MFC 消息映射区别？

| 项目     | Win32 API   | MFC        |
| -------- | ----------- | ---------- |
| 处理方式 | switch-case | 消息映射表 |
| 结构     | 过程式      | 面向对象   |
| 扩展性   | 差          | 好         |

------

## 4. 消息处理顺序（非常高频）

👉 一个消息在 MFC 中的流转：

1. 窗口接收到消息
2. AfxWndProc 入口函数
3. CWnd::WindowProc
4. 查消息映射表
5. 调用对应成员函数
6. 没有匹配 → 走默认处理（DefWindowProc）

------

## 5. 消息分类（必须会）

### （1）队列消息（Queued Message）

通过消息队列：

- 键盘
- 鼠标
- WM_PAINT
- WM_TIMER

------

### （2）非队列消息（Non-queued Message）

直接发送：

- SendMessage
- WM_CREATE
- WM_DESTROY

👉 不进队列，直接调用处理函数

------

# 二、MFC 消息泵（Message Pump）

## 1. 什么是消息泵？

👉 本质是一个 **while循环 + GetMessage**

```
while (GetMessage(&msg, NULL, 0, 0))
{
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
```

------

## 2. MFC 的消息泵在哪？

在：

👉 `CWinApp::Run()`

简化结构：

```
int CWinApp::Run()
{
    while (GetMessage(...))
    {
        PumpMessage();
    }
}
```

------

## 3. 消息泵作用

- 从消息队列取消息
- 分发给窗口
- 保证 UI 程序持续响应

👉 没有消息泵 = 程序“假死”

------

## 4. PostMessage vs SendMessage（必问）

| 类型        | 是否进队列 | 是否阻塞         |
| ----------- | ---------- | ---------------- |
| PostMessage | 是         | 非阻塞           |
| SendMessage | 否         | 阻塞（同步调用） |

------

# 三、MFC 架构（非常高频）

## 1. MFC 三大核心架构

### ✔ 1）CWinApp（应用程序对象）

- 程序入口
- 初始化资源
- 消息循环

```
CMyApp theApp;
```

------

### ✔ 2）CWnd（窗口基类）

所有窗口的基类：

- CFrameWnd（主窗口）
- CDialog（对话框）
- CView（视图）

------

### ✔ 3）Document/View 架构（重点）

👉 经典设计模式

### 结构：

```
Document（数据）
   ↓
View（显示）
   ↓
Frame（窗口容器）
```

------

### 举例：

一个文本编辑器：

- Document：文本内容
- View：显示文本
- Frame：窗口

------

## 2. Document/View 优点

- 数据与界面分离
- 支持多视图
- 易扩展（类似 MVC）

------

## 3. MFC 整体架构关系

```
WinMain
  ↓
CWinApp
  ↓
消息泵（Run）
  ↓
CWnd WindowProc
  ↓
消息映射表
  ↓
具体处理函数
```

------

# 四、面试高频追问（非常重要）

## Q1：MFC 为什么要用消息映射，而不是虚函数？

👉 因为 Windows 消息是“ID驱动”，不是面向对象事件模型
 → 虚函数无法覆盖所有 WM_XXX

------

## Q2：消息映射本质是什么？

👉 静态表 + 宏 + 函数指针

------

## Q3：WM_PAINT 为什么不能用 SendMessage？

👉 因为绘制必须进入队列（WM_PAINT 是低优先级合并消息）

------

## Q4：PostMessage 为什么更常用于 UI？

👉 不阻塞 UI 线程，提高响应性

------

## Q5：MFC 本质是什么？

👉 Win32 API + C++ 封装 + 消息驱动框架

------

# 五、一句话面试总结（可背）

👉 **MFC 是对 Win32 API 的 C++ 封装框架，通过消息泵循环获取 Windows 消息，再通过消息映射表分发到对应的类成员函数，从而实现事件驱动的 GUI 应用结构。**