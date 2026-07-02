MFC 是 **Microsoft Foundation Classes（微软基础类库）** 的缩写，是微软提供的一套 **基于 C++ 的 Windows 应用程序开发框架**。

------

## 1. MFC 是什么

MFC 本质上是对 Windows API 的一层 **C++ 封装**，目的是让开发 Windows 桌面程序更简单。

它把原本需要直接调用 Win32 API 的复杂操作，封装成类和对象，比如：

- 窗口（Window）
- 按钮（Button）
- 对话框（Dialog）
- 文档/视图结构（Document/View）

------

## 2. MFC 能做什么

常用于开发传统 Windows 桌面软件，例如：

- GUI 工具软件
- 工业控制软件
- 老版本 Windows 客户端程序
- 一些企业内部工具

------

## 3. MFC 的核心特点

### ✔ 面向对象封装 Win32 API

Win32 API 是 C 风格的，而 MFC 用 C++ 类来封装：

- `CWnd` → 窗口基类
- `CDialog` → 对话框
- `CButton` → 按钮控件

------

### ✔ 消息映射机制（重点）

MFC 用宏机制处理 Windows 消息，例如：

```
BEGIN_MESSAGE_MAP(CMyDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON1, &CMyDlg::OnButtonClick)
END_MESSAGE_MAP()
```

本质是把 Windows 的消息机制（WM_XXX）映射到 C++ 函数。

------

### ✔ 文档 / 视图架构（Document/View）

这是 MFC 很经典的设计模式：

- Document：数据
- View：界面显示
- Frame：窗口框架

适合编辑器类软件（如早期 Office 风格程序）。

------

## 4. MFC 的现状（很重要）

现在 MFC 属于 **“历史遗留技术”**：

- 优点：成熟、稳定、接近底层、性能好
- 缺点：开发效率低、UI 现代化差、学习曲线陡

现代 Windows 开发更常用：

- Qt
- .NET / WPF / WinUI
- Electron（跨平台 UI）

------

## 5. 一句话总结

👉 **MFC 就是“用 C++ 写 Windows 桌面程序的旧式框架，是 Win32 API 的面向对象封装”。**