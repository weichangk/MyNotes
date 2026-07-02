# WinDbg 面试文档（Qt/C++/Windows方向）

# 一、WinDbg 是什么？

WinDbg 是 Microsoft 提供的 Windows 平台原生调试器。

主要用于：

- 崩溃分析（Crash Dump）
- 卡死分析（Hang Dump）
- 性能问题定位
- 内存泄漏分析
- 死锁分析
- 用户态 / 内核态调试
- 驱动调试

支持：

- EXE
- DLL
- Qt
- MFC
- STL
- COM
- Driver

常用于：

- Qt/C++ 工业软件
- 游戏引擎
- 浏览器
- 驱动开发
- 大型 Windows 客户端

------

# 二、WinDbg 和 Visual Studio Debugger 区别

| 对比项     | WinDbg | Visual Studio |
| ---------- | ------ | ------------- |
| 崩溃分析   | 强     | 一般          |
| Dump 分析  | 非常强 | 较弱          |
| 内核调试   | 支持   | 不支持        |
| 死锁分析   | 强     | 一般          |
| 内存分析   | 强     | 一般          |
| 学习成本   | 高     | 低            |
| 自动化分析 | 强     | 一般          |
| 符号系统   | 完整   | 一般          |

面试回答：

Visual Studio 更适合开发调试。

WinDbg 更适合：

- 线上 dump 分析
- 用户现场问题分析
- 系统级问题定位
- 死锁/卡死问题
- 内存问题

------

# 三、什么是 Dump？

Dump 是进程某一时刻的内存快照。

可以理解为：

“程序运行现场截图”。

Dump 内通常包含：

- 线程信息
- 调用栈
- 寄存器
- 模块信息
- 堆信息
- 异常信息
- 部分内存

因此即使用户现场无法复现：

仍然可以通过 dump 分析程序当时正在做什么。

------

# 四、Dump 的类型

## 1）Mini Dump

最常用。

包含：

- 调用栈
- 线程
- 异常信息

优点：

- 文件小
- 易上传
- 足够定位大部分崩溃

常用于线上崩溃收集。

## 2）Full Dump

完整进程内存。

优点：

- 可分析内存
- 可分析对象
- 可分析 STL/Qt 容器

缺点：

- 非常大

常用于：

- 疑难问题
- 内存泄漏
- 数据损坏

------

# 五、如何生成 Dump

## 1）代码生成

```cpp
MiniDumpWriteDump(
    GetCurrentProcess(),
    GetCurrentProcessId(),
    hFile,
    MiniDumpWithThreadInfo,
    nullptr,
    nullptr,
    nullptr);
```

常用于：

- 崩溃自动导出
- 卡顿自动导出
- watchdog 机制

## 2）任务管理器生成

右键进程：

```text
创建转储文件
```

## 3）WinDbg 生成

```text
.dump /ma xxx.dmp
```

------

# 六、什么是符号（PDB）

PDB（Program Database）记录：

- 函数名
- 行号
- 类型信息
- 变量信息

没有符号：

```text
0x7ff812341234
```

有符号：

```text
Qt5Core!QEventLoop::exec
```

因此：

Dump 分析核心依赖 PDB。

------

# 七、WinDbg 常用命令

# 1）设置符号

```bash
.symfix
.reload
```

或者：

```bash
.sympath SRV*c:\symbols*https://msdl.microsoft.com/download/symbols
.reload
```

# 2）自动分析

```bash
!analyze -v
```

作用：

- 分析异常
- 分析崩溃线程
- 输出调用栈
- 推测问题原因

# 3）查看调用栈

```bash
k
```

常用：

```bash
kv
kp
kb
```

查看所有线程：

```bash
~*k
```

# 4）查看线程

```bash
~
```

切换线程：

```bash
~3s
```

# 5）查看寄存器

```bash
r
```

# 6）反汇编

```bash
u
```

例如：

```bash
u rip
```

# 7）查看模块

```bash
lm
```

查看指定模块：

```bash
lmvm xxx
```

# 8）断点

```bash
bp xxx!func
```

查看断点：

```bash
bl
```

删除断点：

```bash
bc *
```

------

# 八、崩溃分析流程

面试高频。

## 第一步：加载 dump

```bash
windbg -z xxx.dmp
```

## 第二步：加载符号

```bash
.symfix
.reload
```

## 第三步：自动分析

```bash
!analyze -v
```

查看：

- 异常类型
- 崩溃模块
- 崩溃线程

## 第四步：查看调用栈

```bash
k
```

重点关注：

- 崩溃函数
- 空指针
- STL/Qt容器
- 第三方库

## 第五步：分析根因

常见：

- 空指针
- 野指针
- 越界
- use-after-free
- 死锁
- 栈溢出

------

# 九、卡死（Hang）分析流程

这是 Qt 客户端面试非常高频的问题。

## 核心思路

卡死时：

看线程正在执行什么。

## 常用命令

```bash
~*k
```

查看所有线程栈。

## UI线程判断

通常包含：

```text
QEventLoop::exec
DispatchMessageW
```

说明是 UI 线程。

## 常见卡死原因

### 1）UI线程做耗时计算

例如：

```text
opencv
halcon
resize
memcpy
```

### 2）死锁

例如：

```text
WaitForSingleObject
QMutex
std::mutex
condition_variable
```

### 3）IO阻塞

例如：

```text
recv
send
ReadFile
sqlite
```

------

# 十、死锁分析

## 死锁典型调用栈

```text
ntdll!NtWaitForSingleObject
KernelBase!WaitForSingleObjectEx
std::mutex::lock
```

或者：

```text
QMutex::lock
```

说明线程正在等待锁。

如果多个线程互相等待：

就是死锁。

## 分析方法

```bash
~*k
```

查看：

- 谁持有锁
- 谁等待锁
- 是否循环等待

------

# 十一、内存问题分析

## 1）空指针

典型异常：

```text
EXCEPTION_ACCESS_VIOLATION
```

地址：

```text
0000000000000000
```

说明空指针。

## 2）野指针

随机地址：

```text
0xfeeefeee
0xdddddddd
```

通常：

- 对象已释放
- 内存被覆盖

## 3）栈溢出

异常：

```text
STACK_OVERFLOW
```

通常：

- 无限递归
- 超大局部变量

------

# 十二、Qt 项目中的 WinDbg 分析

## 1）Qt UI线程

典型栈：

```text
Qt5Core!QEventLoop::exec
Qt5Widgets!QApplication::exec
```

## 2）信号槽问题

```text
QMetaObject::activate
```

说明：

- signal 正在触发
- slot 执行耗时

## 3）线程池问题

```text
QThreadPool
QtConcurrent
```

可分析：

- 线程竞争
- future 卡死
- 线程池耗尽

------

# 十三、线上问题定位流程（面试重点）

一个完整回答模板。

## 用户反馈软件崩溃

### 第一步

收集 dump。

### 第二步

使用 WinDbg：

```bash
!analyze -v
```

分析异常。

### 第三步

查看调用栈：

```bash
k
```

定位崩溃函数。

### 第四步

结合源码：

- 分析对象生命周期
- 分析线程安全
- 分析越界

### 第五步

修复并验证。

------

# 十四、性能问题分析

## 核心思想

Dump 可以看到：

“程序卡住时正在执行哪个函数”。

因此可以定位：

- CPU热点
- 死循环
- 大计算
- IO阻塞

## 典型案例

UI线程：

```text
opencv_world480!cv::resize
```

说明：

OpenCV 大图缩放在 UI线程执行。

优化：

- moveToThread
- QtConcurrent
- 线程池

------

# 十五、WinDbg 高频面试题

# 1）WinDbg 和 VS 调试器区别？

回答：

VS 更偏开发调试。

WinDbg 更偏：

- dump分析
- 系统级问题
- 死锁
- 内存问题
- 内核调试

# 2）为什么 dump 可以分析卡顿？

因为 dump 保存了：

- 所有线程调用栈
- 当前执行函数
- 等待状态

所以可以看到：

程序卡住时正在执行什么。

# 3）为什么没有源码也能分析？

因为：

- dump 保存机器码
- PDB 保存符号

WinDbg 可以还原函数调用关系。

# 4）线上无法复现怎么办？

回答：

通过：

- 自动导出 dump
- 用户现场 dump
- watchdog dump

进行离线分析。

# 5）如何分析死锁？

回答：

通过：

```bash
~*k
```

查看线程等待关系。

重点关注：

```text
WaitForSingleObject
QMutex
std::mutex
```

# 6）如何分析 UI 卡死？

回答：

查看 UI线程调用栈。

重点看：

是否在 UI线程执行：

- OpenCV
- HALCON
- IO
- 大循环

------

# 十六、项目经验回答模板

## 示例：Qt 工业视觉项目

“项目中曾遇到用户现场偶发卡死，无法本地复现。

后来我们加入 watchdog 机制：

当 UI线程超过一定时间没有响应时，自动触发 MiniDumpWriteDump 导出 dump。

之后通过 WinDbg：

```bash
~*k
```

分析所有线程调用栈。

发现 UI线程卡在 OpenCV 图像缩放函数。

原因是大图 resize 放在主线程执行。

后续通过 moveToThread + 线程池优化解决。”

这是非常典型且加分的回答。

------

# 十七、建议掌握的能力

## 初级

- 会打开 dump
- 会加载符号
- 会看调用栈
- 会看崩溃函数

## 中级

- 会分析死锁
- 会分析线程等待
- 会分析 UI卡死
- 会分析 STL/Qt对象

## 高级

- Heap分析
- Handle泄漏
- 内存损坏
- 内核调试
- Driver调试
- 自动化dump系统

------

# 十八、推荐学习顺序

## 第一阶段

掌握：

```bash
!analyze -v
k
~*k
```

## 第二阶段

学习：

- 死锁
- 多线程
- 内存问题

## 第三阶段

学习：

- heap
- page heap
- gflags
- UMDH

## 第四阶段

学习：

- kernel dump
- driver 调试

------

# 十九、面试总结（重点背诵）

WinDbg 最大价值：

- 可以分析线上无法复现的问题
- 可以分析崩溃现场
- 可以分析卡死现场
- 可以分析线程等待关系
- 可以定位性能热点

核心原理：

Dump 保存了程序某一时刻的运行现场。

而 WinDbg 可以解析：

- 线程
- 调用栈
- 异常
- 模块
- 内存

因此能够离线还原问题发生时程序正在做什么。