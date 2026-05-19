这个岗位本质上是：

> **Windows 客户端开发 + Qt UI工程 + C++基础 + 工程化能力**

而且关键词非常明显：

- 网易游戏平台
- Windows 客户端
- 组件化
- 技术攻关
- Qt（加分项但实际大概率会问很深）
- 现代 C++
- Windows 消息机制

所以面试重点不会只是“会不会 Qt 控件”。

更像：

> “你能不能写大型 Windows 客户端？”

------

# 一、这个岗位的真实技术画像

从 JD 推导：

| JD                | 面试真实考察                  |
| ----------------- | ----------------------------- |
| Qt桌面端开发      | Qt 架构/UI/事件/线程/性能     |
| windows客户端界面 | Win32 消息机制 / 窗口系统     |
| 组件化设计        | 模块解耦 / 插件化 / 接口设计  |
| 技术攻关          | 崩溃、卡顿、渲染、线程问题    |
| 现代C++           | C++11/14/17 高频              |
| STL               | 容器/算法/智能指针            |
| TCP/IP HTTP       | 网络通信基础                  |
| Windows开发经验   | DLL、消息循环、Hook、COM、DPI |

------

# 二、面试内容优先级（非常重要）

我按网易这种客户端岗位的真实概率排序：

# 第一优先级（必问）

## 1. C++ 基础与现代 C++

这是核心。

一定会深问。

------

## 高频问题

### 对象模型

- 虚函数原理
- vtable
- 多态实现
- 构造/析构顺序
- RTTI
- dynamic_cast 原理
- 内存布局

你最近问的 RTTI、元对象系统，其实已经踩中高频了。

------

### 智能指针

高频：

- shared_ptr 原理
- weak_ptr 为什么存在
- unique_ptr 使用场景
- 循环引用

扩展：

- Qt parent-child 和 shared_ptr 冲突
- QObject 为什么不建议 shared_ptr

------

### move语义

重点：

- 左值右值
- std::move
- 完美转发
- emplace_back 为什么快
- 移动构造 vs 拷贝构造

------

### STL

常问：

| 类型              | 高频问题         |
| ----------------- | ---------------- |
| vector            | 扩容机制         |
| map/unordered_map | 红黑树 vs 哈希   |
| list              | 为什么缓存不友好 |
| deque             | 底层结构         |
| string            | SSO              |

算法：

- sort
- lower_bound
- 二分
- hash

------

### 多线程

非常高频。

重点：

- mutex
- condition_variable
- atomic
- 死锁
- 生产者消费者

高级：

- 内存模型
- CAS
- ABA问题

------

# 第二优先级（网易非常重视）

# 2. Windows 开发

这个岗位不是纯 Qt。

一定会问 Win32。

------

# 必问内容

## Windows消息机制

这是核心中的核心。

------

### 高频问题

#### 什么是消息循环？

```
while(GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
```

必须会讲：

- 消息队列
- 消息泵
- 派发
- 窗口过程

------

#### SendMessage vs PostMessage

超级高频。

必须能讲：

| SendMessage     | PostMessage  |
| --------------- | ------------ |
| 同步            | 异步         |
| 阻塞            | 不阻塞       |
| 直接调用WndProc | 放入消息队列 |

延伸：

- 跨线程
- 死锁
- UI卡死

------

#### HWND是什么？

必须知道：

> HWND 本质是窗口句柄，用于标识窗口对象。

------

#### 窗口创建流程

会问：

- WinMain
- RegisterClassEx
- CreateWindowEx
- WndProc

------

#### 常见Windows消息

至少知道：

| 消息         | 作用         |
| ------------ | ------------ |
| WM_PAINT     | 绘制         |
| WM_SIZE      | 窗口大小变化 |
| WM_TIMER     | 定时器       |
| WM_CLOSE     | 关闭         |
| WM_DESTROY   | 销毁         |
| WM_MOUSEMOVE | 鼠标         |
| WM_KEYDOWN   | 键盘         |

------

# 3. Qt 核心（非常可能深挖）

虽然 JD 写“加分项”。

但真实情况：

> 做游戏平台客户端，Qt 很可能是核心UI框架。

所以会问很深。

------

# 高频 Qt 面试题

## QObject为什么禁止拷贝？

你今天问的这个：

这是典型高级面试题。

必须会。

------

## 信号槽机制

高频：

- connect原理
- queued connection
- direct connection
- 跨线程信号槽

高级：

- 为什么线程安全
- 底层事件投递

------

## Qt事件机制

必须会：

- event loop
- event()
- eventFilter()
- mousePressEvent()

------

## 元对象系统

高频：

- moc是什么
- Q_OBJECT作用
- Q_PROPERTY
- 反射机制

------

## Qt线程

重点：

### QThread 正确使用方式

高频陷阱：

> 不推荐继承QThread写业务。

推荐：

```
QObject + moveToThread
```

------

### 线程通信

- queued connection
- invokeMethod

------

## Qt内存管理

必须：

- parent-child
- deleteLater
- QObject 生命周期

------

## Qt Model/View

游戏平台客户端很可能问。

必须知道：

- model/view 架构
- MVC区别
- QAbstractItemModel

------

## Qt绘制

高概率。

游戏平台UI经常涉及：

- 自绘
- 高性能绘制
- 动画

重点：

- QPainter
- 双缓冲
- paintEvent

------

# 第三优先级

# 4. 网络编程

不会特别深。

但一定会问基础。

------

## TCP/IP

必须：

- 三次握手
- 四次挥手
- TIME_WAIT
- 粘包拆包

------

## HTTP

必须：

- GET POST
- keep-alive
- websocket
- HTTPS

------

## Qt网络

可能会问：

- QNetworkAccessManager
- QTcpSocket

------

# 第四优先级

# 5. 架构设计（网易很重视）

因为JD写了：

> 组件化设计

这意味着：

> 一定会考工程能力。

------

# 高频架构问题

## 如何设计一个组件化客户端？

例如：

- 登录模块
- 下载模块
- 更新模块
- UI模块

考察：

- 解耦
- 接口
- 生命周期

------

## 插件化

可能问：

- DLL
- Qt插件系统
- QPluginLoader

------

## 如何设计消息总线

比如：

- EventBus
- Observer

------

## 如何处理大型项目编译慢

高级题：

- PIMPL
- 前向声明
- 模块拆分

------

# 五、网易非常爱问的“项目拷打”

这个比八股更重要。

------

# 必问

## 1. 你项目里最难的问题？

一定提前准备。

最好是：

- 崩溃
- 卡顿
- 死锁
- UI渲染
- 内存泄漏

------

## 2. 如何定位问题？

必须会：

### Windows

- dump
- windbg
- visual studio 调试

### Qt

- GammaRay
- QObject tree
- 信号槽分析

------

## 3. 如何优化性能？

高频。

例如：

- UI卡顿
- 主线程阻塞
- repaint过多
- 大量QObject

------

## 4. 如何做线程安全？

网易很喜欢。

------

# 六、极高概率手撕/口撕题

------

# C++手撕

## 高频

- 单例
- 线程池
- LRU
- producer-consumer
- shared_ptr简化版

------

# 算法

不会 LeetCode Hard。

但：

## 高频：

- 二分
- 链表
- hash
- 排序
- topK

------

# 七、你这个岗位最应该重点准备的内容（重要）

你的岗位不是：

> “Qt控件开发”

而是：

> “Windows客户端高级开发”

所以：

# 优先级应该这样：

## 第一梯队（必须）

### C++

- 对象模型
- 智能指针
- move语义
- STL
- 多线程

------

### Windows

- 消息循环
- SendMessage/PostMessage
- WndProc
- HWND
- UI线程

------

### Qt核心

- 信号槽
- 事件系统
- 元对象
- QThread
- QObject生命周期

------

# 第二梯队

### 网络

- TCP
- HTTP

------

### 架构

- 插件化
- 组件化
- 模块解耦

------

# 八、我建议你重点准备的“杀手级高频题”

这些非常像网易会问的：

------

# Qt

- QObject为什么不能拷贝？
- 信号槽跨线程为什么安全？
- event 和 signal 有什么区别？
- deleteLater原理？
- Qt 为什么需要 moc？
- Qt事件循环原理？
- QThread 为什么不推荐继承？

------

# Windows

- SendMessage vs PostMessage
- Windows消息循环原理
- 为什么UI线程不能阻塞？
- WM_PAINT什么时候触发？
- Hook了解吗？
- DLL注入了解吗？（可能）

------

# C++

- shared_ptr原理
- vector扩容
- unordered_map为什么快
- 什么是右值引用
- move之后对象还能用吗
- lambda底层是什么

------

# 多线程

- 死锁如何避免
- atomic一定线程安全吗
- condition_variable原理
- Qt线程通信机制

------

# 九、网易面试风格（很重要）

网易客户端通常特点：

| 特点       | 描述          |
| ---------- | ------------- |
| 深挖项目   | 会连续追问    |
| 重基础     | C++基础很深   |
| 不只Qt     | Windows也重要 |
| 爱问原理   | 不满足于API   |
| 重工程经验 | 真实问题解决  |

所以：

> 不要只背 API。

而要：

> “为什么这样设计？”

------

# 十、你现在最该做的准备路线

建议：

# 第1阶段（最重要）

## C++ + Qt核心

你已经开始了。

继续：

- QObject
- 元对象
- 信号槽
- 线程
- 内存模型

------

# 第2阶段

## Windows

重点突击：

- 消息循环
- Win32
- SendMessage/PostMessage
- 窗口创建流程

------

# 第3阶段

## 项目拷打

提前准备：

- 技术难点
- 性能优化
- 崩溃定位
- 线程问题