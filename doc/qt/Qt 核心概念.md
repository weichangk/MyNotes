Qt 是一个跨平台的 C++ 图形用户界面应用程序开发框架，其核心概念主要围绕<b>对象模型、事件系统、模块化架构和图形视图系统</b>展开。以下是 Qt 中几个非常重要的核心概念：

## Qt 模块化架构
- 核心模块：
    - QtCore：基础类，如 QObject, QString, QVariant, QTimer；
    - QtGui：图形、字体、图像处理；
    - QtWidgets：传统桌面 UI 控件；
    - QtQuick / QtQml：声明式 UI 开发；
    - QtNetwork, QtMultimedia, QtSql 等用于扩展功能。
- 用户可按需链接模块，减少程序体积。

## QObject 与信号槽机制
- QObject 是所有 Qt 对象的基类，支持内存管理、事件系统等功能。
- 信号与槽（Signal & Slot）是 Qt 的核心通信机制，用于对象间解耦通信：
    ```cpp
    connect(sender, SIGNAL(signalName()), receiver, SLOT(slotName()));
    ```
- 支持函数指针、新语法、Lambda 绑定等形式。

## 事件系统（Event System）
- Qt 的 GUI 主要通过事件驱动。
- 所有事件（如鼠标点击、键盘输入）都封装为 QEvent 派生类。
- 事件处理机制：
    - 重写 event()、eventFilter()、keyPressEvent() 等虚函数；
    - 安装事件过滤器 installEventFilter() 拦截特定对象的事件。

## 元对象系统（Meta-Object System）
- 支持运行时类型识别、信号槽机制、动态属性等功能
- 使用 Q_OBJECT 宏的类可以启用元对象系统，借助 moc（元对象编译器）生成中间代码。
- 提供：
    - QObject::metaObject() 获取类型信息；
    - QMetaObject、QMetaProperty、QMetaMethod 反射操作。

## 布局管理器与绘图系统
- Qt 提供灵活的布局管理器（QVBoxLayout, QHBoxLayout, QGridLayout 等），可自动管理控件位置和大小。
- 绘图系统以 QPainter 为核心，可绘制文本、图像、矢量图等。

## 资源系统（Qt Resource System）
- 通过 .qrc 文件将图片、UI 文件等资源编译进可执行文件。
- 使用 qrc:/path/to/resource 访问。

## Qt 样式表 QSS（Qt Style Sheets）
- 用于美化 UI 控件外观，类似 HTML 的 CSS；
- 通过 setStyleSheet() 或 .qss 文件应用；
- 可设置颜色、字体、圆角、边框、渐变等；

## Model/View 架构
- 用于处理和显示数据：
- QAbstractItemModel 为基础模型；
- QTableView, QTreeView, QListView 为视图；
- 支持自定义数据源、代理显示等功能。

## 跨平台性与信号槽线程安全
- Qt 可在 Windows、macOS、Linux 等平台无差别构建。
- 信号槽机制支持跨线程通信，自动封装为事件队列，保证线程安全。