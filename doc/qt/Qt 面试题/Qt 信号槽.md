
## Qt 信号槽本质
Qt 信号槽本质是基于 **元对象系统（Meta-Object System）+ moc 代码生成机制**实现的。

类中只要加了 **Q_OBJECT**，moc 会为该类生成额外代码，包括：

- signal 的实现函数（本质是调用 `QMetaObject::activate()`）
- metaObject 元信息（信号、槽、参数索引表）
- `qt_metacall()` 用于槽函数的动态调用

------

### 🔹 signal 本质

signal 不是普通函数声明，它在 moc 里被展开成一个函数实现：

```
emit signal(x);
→ 实际调用 QMetaObject::activate()
```

------

### 🔹 connect 本质

`connect()` 会在 QObject 内部维护一张 **连接表（Connection list）**：

- 记录 sender、signal index、receiver、slot index
- 本质是“信号索引 → 槽函数映射关系”

------

### 🔹 emit 执行流程

当 emit 信号时：

1. 调用 `QMetaObject::activate()`
2. 查找该 signal 对应的所有 connection
3. 根据连接类型执行：
   - Direct：直接调用槽函数
   - Queued：封装成 `QMetaCallEvent` 投递到目标线程事件队列

------

### 🔹 槽函数调用本质

最终槽函数调用是通过：

```
qt_metacall() → switch-case → 直接函数调用
```

------

### 🔹 一句话总结

Qt 信号槽机制本质是：
 **moc 生成元对象代码 + QObject 维护信号槽连接表 + QMetaObject::activate 分发调用 +（跨线程时通过事件队列实现异步调用）**。



## Qt 信号槽的连接方式

Qt 信号槽的连接方式主要有以下几种，面试可以这样简洁回答：

------

### 🔹 1. DirectConnection（直接连接）

**信号发出后，立即调用槽函数，在发送者线程执行。**

- 等同于函数直接调用
- 不经过事件循环
- 同线程默认常用

👉 重点：**同步、立即执行**

------

### 🔹 2. QueuedConnection（队列连接）

**槽函数在接收者线程的事件循环中执行。**

- 跨线程通信标准方式
- signal 被封装成 `QMetaCallEvent`
- 投递到目标线程 event loop

👉 重点：**异步、线程安全、依赖事件循环**

------

### 🔹 3. AutoConnection（默认）

**Qt 自动选择连接方式**

- 同线程 → Direct
- 跨线程 → Queued

👉 重点：**默认方式，自动判断线程**

Qt 的 `AutoConnection` 本质是在 `QObject::connect()` 内部根据 **sender 和 receiver 的线程归属**自动选择连接类型。

在 Qt 源码 `QObject::connect()` 内部，大致逻辑是：

```
if (connectionType == Qt::AutoConnection) {
    if (sender->thread() == receiver->thread())
        connectionType = Qt::DirectConnection;
    else
        connectionType = Qt::QueuedConnection;
}
```

------

### 🔹 4. BlockingQueuedConnection（阻塞队列连接）

**跨线程同步调用方式**

- 发送线程阻塞等待槽执行完成
- 本质是 queued + wait

👉 重点：**跨线程同步、可能死锁（同线程禁止）**

`BlockingQueuedConnection` 主要用于 **跨线程的同步调用场景**，即：

> 发送线程发出信号后，需要**等待接收线程的槽函数执行完成并返回结果**。（发送线程等待接受线程计算结果）但使用不当容易造成死锁或卡顿。

### 🔹 5. UniqueConnection（修饰标志）

**防止重复连接**

- 同一个 signal-slot 只连接一次

👉 重点：**避免重复触发**

------

### 🔥 一句话总结

Qt 提供四种核心连接方式：
 **Direct（同步直接调用）、Queued（跨线程异步队列）、Auto（自动选择）、BlockingQueued（跨线程同步阻塞）**，其中跨线程通信推荐使用 QueuedConnection。



## 信号可以重载吗？

**可以，Qt 的 signal 支持重载。**

------

### 🔹 如何实现重载？

本质就是 **函数参数列表不同（C++函数重载机制）**：

```
signals:
    void valueChanged(int value);
    void valueChanged(double value);
```

------

### 🔹 连接时要注意什么？（重点）

由于 signal 重载后存在多个同名函数，使用 `connect` 时必须**显式指定函数指针类型**：

```
connect(obj,
        static_cast<void (Obj::*)(int)>(&Obj::valueChanged),
        this,
        &Receiver::onValueChanged);
```

------

### 🔹 为什么要这样做？

因为：

- 编译器无法仅通过函数名判断具体重载版本
- 需要消除歧义（disambiguation）

------

### 🔥 一句话总结

Qt 的信号支持函数重载，本质是 C++ 函数重载机制，但在 connect 时必须通过函数指针或 static_cast 显式指定具体信号版本，否则会产生歧义编译失败。
