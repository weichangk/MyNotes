interview-qt.md
# Qt 技术面试知识点

> 基于 Wondershare Filmora Windows 客户端（C++17 + Qt 5.15）真实代码整理
> 项目路径：`E:\dcproject\filmora11-win`

---

## 目录

1. [Qt 对象模型](#1-qt-对象模型)
2. [信号与槽机制](#2-信号与槽机制)
3. [事件系统](#3-事件系统)
4. [Qt 内存管理](#4-qt-内存管理)
5. [自定义控件](#5-自定义控件)
6. [模型/视图架构](#6-模型视图架构)
7. [Qt 线程与并发](#7-qt-线程与并发)
8. [PIMPL 与 Q_DECLARE_PRIVATE](#8-pimpl-与-q_declare_private)
9. [资源系统与样式表](#9-资源系统与样式表)
10. [Qt 布局管理](#10-qt-布局管理)
11. [Qt 智能指针](#11-qt-智能指针)

---

## 1. Qt 对象模型

### 核心机制

Qt 对象模型（Qt Object Model）是 Qt 框架的基础，核心包括：

| 机制 | 宏/关键字 | 作用 |
|------|-----------|------|
| 元对象系统 | `Q_OBJECT` | 启用 MOC 代码生成，支持信号槽、`QMetaObject` |
| 对象树 | `parent` 参数 | 父对象销毁时自动销毁所有子对象 |
| 运行期类型信息 | `qobject_cast<>` | 安全的 Qt 对象类型转换 |
| 动态属性 | `Q_PROPERTY` | 属性系统，配合 QML 和设计器 |
| 反射 | `QMetaObject` | 运行期查询方法、属性、信号、枚举 |

### 项目代码示例

**Q_OBJECT 宏的使用**

```cpp
// 文件：Include/TemplateMode/FVideoTemplate/FVideoTemplatePresenter.h
class FVideoTemplatePresenter : public FFPresenter,
                                public IFFUndoRedoServiceEventObserver,
                                public IFFWSIDEventObserver
{
    Q_OBJECT             // 必须：启用 MOC、信号槽、qobject_cast 支持
    Q_DECLARE_PRIVATE(FVideoTemplatePresenter)

    // 声明信号（MOC 生成实现）
Q_SIGNALS:
    void sigPageIndexChanged(FVIDEOTEMPLATE::VideoTemplatePageIndex);
    void sigTemplateModePageChanged(FVIDEOTEMPLATE::TemplateModePageIndex);
    void sigBackTemplateView();
    void sigProjectChanged(bool);
    void sigWSIDStatusChanged(bool);
};
```

**对象树 + 父子关系**

```cpp
// 文件：Src/TemplateMode/FTemplateView/FTemplateMediaItemIconView.cpp
// 指定 parent = this，子控件随父控件自动销毁，无需手动 delete
m_selectionModel = new FFMediaItemSelectionModel(m_model, this);
```

### 常见面试题

**Q：`Q_OBJECT` 宏做了什么？不加会怎样？**

A：`Q_OBJECT` 触发 MOC（Meta-Object Compiler）生成额外的 `.cpp` 文件，包含：
- `metaObject()` 返回类的 `QMetaObject` 指针
- `qt_metacall()` 信号槽调度函数
- 信号的实现代码（emit 时调用）

不加则：信号无法 emit，`connect` 在运行期失败，`qobject_cast` 返回 null，`QMetaObject` 信息不完整。

**Q：`qobject_cast` 和 `dynamic_cast` 的区别？**

A：
- `qobject_cast` 基于 Qt 元对象系统，**不要求 RTTI**，性能更好，但只能用于 `QObject` 的子类
- `dynamic_cast` 基于 C++ RTTI，适用于任意多态类，但需开启 RTTI 且性能略差

**Q：MOC 是什么？何时运行？**

A：MOC（元对象编译器）是 Qt 的预处理器，读取含 `Q_OBJECT` 的头文件，生成 `moc_xxx.cpp`。CMake 中 `AUTOMOC ON` 自动处理，Visual Studio 构建时在编译步骤之前运行。

---

## 2. 信号与槽机制

### 连接语法对比

| 语法 | 特点 | 适用场景 |
|------|------|----------|
| 旧式字符串宏 `SIGNAL/SLOT` | 运行期检查，无类型安全 | Qt4 遗留代码 |
| 新式函数指针 `&Class::func` | 编译期类型检查，可连 lambda | Qt5 推荐 |
| Lambda 直接连接 | 最灵活，可捕获上下文 | 简单回调 |

### 项目代码示例

**新式函数指针语法**

```cpp
// 文件：Src/TemplateMode/FTemplateView/FTemplateMediaItemIconView.cpp

// 标准：sender, &SenderClass::signal, receiver, &ReceiverClass::slot
connect(itemDelegate, &FMediaItemIconDelegate::sigItemEnterEvent,
        this,         &FTemplateMediaItemIconView::slotItemEnterEvent);

// 信号转发信号：连接两个信号
connect(m_itemPainter, &FTemplateMediaItemPainter::sigPreviewMedia,
        this,          &FTemplateMediaItemIconView::sigTemplateHoverPreview);
```

**Lambda 作槽函数**

```cpp
// 文件：Src/TemplateMode/FTemplateView/FTemplateMediaItemIconView.cpp

// 简单回调：引用捕获 this，调用 update()
connect(itemDelegate, &FMediaItemIconDelegate::sigRepaint, this, [&]() {
    update();
});

// 带逻辑的回调
connect(m_model, &FTemplateMediaItemModel::sigFetchMoreSuccess, this, [&]() {
    restartCheckResourceThumbnailTimer();
});
```

**跨线程信号槽（QMetaObject::invokeMethod）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h
// 场景：异步任务完成后，将回调切换到 UI 线程执行

auto obj_continuation = [
    func    = std::forward<Function>(func),
    promise,
    context = QPointer<QObject>(context)   // 弱引用持有接收者
](QFuture<T> parent) mutable {

    // invokeMethod 将 lambda 投递到 context 所在线程的事件队列
    QMetaObject::invokeMethod(context,
        [func = std::move(func), promise = std::move(promise), parent]() mutable {
            SyncContinuation<...> job(std::forward<Function>(func), parent, std::move(promise));
            job.execute();
        }
    );

    // context 已销毁则取消 future
    if (context.isNull()) {
        promise.reportStarted();
        promise.reportCanceled();
        promise.reportFinished();
    }
};
```

### 常见面试题

**Q：`Qt::DirectConnection` 和 `Qt::QueuedConnection` 的区别？**

A：
- `DirectConnection`：槽函数在 `emit` 所在线程**同步**执行（如同普通函数调用）；默认用于同线程连接
- `QueuedConnection`：槽函数被**投递到接收者所在线程的事件队列**，异步执行；默认用于跨线程连接
- Qt 会根据发送者和接收者是否在同一线程**自动选择**连接类型（`AutoConnection`）

**Q：connect 的返回值是什么？有什么用？**

A：返回 `QMetaObject::Connection` 对象，可用于：
```cpp
auto conn = connect(sender, &S::signal, receiver, &R::slot);
disconnect(conn);  // 精确断开这一条连接
```
或用 `QObject::disconnect(conn)` / 成员 `connection.disconnect()`，适合需要精确管理连接生命周期的场景。

**Q：信号可以有返回值吗？**

A：不可以。信号的返回类型必须是 `void`，因为一个信号可能连接到多个槽，"哪个槽的返回值算数"是无定义的。如果需要返回值，应改用普通虚函数或 `QMetaObject::invokeMethod` 的 `Q_RETURN_ARG`。

**Q：Lambda 连接时的内存安全问题？**

A：如果 lambda 捕获了 `this`（指针），而 receiver 对象先被销毁，信号再次触发时会访问悬空指针导致崩溃。解决方案：
1. 传入 context 参数：`connect(sender, &S::sig, receiver, [=](){...})` —— receiver 销毁后自动断开连接
2. 用 `QPointer<T>` 捕获，检查 `isNull()`（项目中 FFFuture 的做法）

---

## 3. 事件系统

### 核心机制

Qt 事件系统流程：
```
OS/Qt 生成事件
    ↓
QApplication::notify(obj, event)
    ↓
QObject::event(event)       ← 可在此重写处理所有事件
    ↓
具体事件处理（mousePressEvent 等）
```

事件过滤器（eventFilter）在整个链路的最前端拦截：
```
QObject::installEventFilter(filter)
    ↓
filter->eventFilter(watched, event) → 返回 true 则吞掉事件，不再传递
```

### 项目代码示例

**自定义事件过滤器接口（IFFEventFilter）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFWidgets/FFEventFilter.h

// 纯虚接口：实现类处理特定事件
class FFWIDGETS_EXPORT IFFEventFilter
{
public:
    virtual bool onFilterEvent(QEvent* event) = 0;

    // 有默认实现：返回箭头光标（子类可覆盖）
    virtual QCursor getCursor() const
    {
        return Qt::ArrowCursor;
    }
};

// 管理器：持有多个过滤器并按序分发
class FFWIDGETS_EXPORT FFEventFilterManager
{
public:
    void installEventFilter(IFFEventFilter* filter);
    void removeEventFilter(IFFEventFilter* filter);
    bool dispatchEvent(QEvent* event);   // 逐个尝试，返回 true 表示已处理
    QCursor getCursor() const;

private:
    QList<IFFEventFilter*> m_oEventList;
    QCursor m_oCursor;
};
```

**Manager 的分发逻辑（责任链模式）**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Src/FFWidgets/FFEventFilter.cpp
bool FFEventFilterManager::dispatchEvent(QEvent* event)
{
    for (IFFEventFilter* pFilter : m_oEventList)
    {
        if (pFilter->onFilterEvent(event))  // 某个过滤器处理了事件
        {
            m_oCursor = pFilter->getCursor();
            return true;   // 吞掉事件，后续过滤器不再执行
        }
    }
    return false;  // 所有过滤器都未处理
}
```

**QObject::eventFilter 重写示例**

```cpp
// 文件：Include/TemplateMode/FTemplateView/FStreamPlayerInfoHelper.h
class FStreamPlayerPreviewFinishedHelper : public QObject
{
    Q_OBJECT
public:
    // 重写 eventFilter：监视特定 QObject 的事件
    bool eventFilter(QObject *watched, QEvent *event) override;
};
```

### 常见面试题

**Q：事件（Event）和信号（Signal）的区别？**

A：
- **事件**：来自 OS 或 Qt 的通知（鼠标点击、键盘输入、定时器），通过 `QApplication::notify` → `event()` 分发，可被 `eventFilter` 拦截；一对一处理
- **信号**：对象状态改变时发出的通知，由开发者在逻辑中 `emit`；可连接多个槽，是一对多的发布订阅

**Q：`event()` 和 `paintEvent()` 等具体事件函数的关系？**

A：`event()` 是统一入口，它根据 `QEvent::type()` 派发到对应的具体函数（`mousePressEvent`、`paintEvent`、`keyPressEvent` 等）。重写 `event()` 可处理所有事件；重写 `paintEvent()` 只处理绘制事件。

**Q：eventFilter 返回 true 和 false 的区别？**

A：返回 `true`：**吞掉**该事件，watched 对象的 `event()` 不会收到此事件；返回 `false`：**放行**，事件继续传递给 watched 对象正常处理。

---

## 4. Qt 内存管理

### 核心机制

1. **父子对象树**：父对象 `delete` 时，自动递归 `delete` 所有子对象
2. **`deleteLater()`**：将对象销毁延迟到当前事件循环结束，避免在槽函数中 `delete` 发送者
3. **`QScopedPointer`**：Qt 版 `unique_ptr`，离开作用域自动 delete
4. **`QPointer`**：弱引用，对象被 `delete` 后自动置 `null`（与 `weak_ptr` 类似，但无需 lock）
5. **`Q_DISABLE_COPY`**：禁止拷贝构造和赋值，防止意外的浅拷贝

### 项目代码示例

**`QScopedPointer` 实现 PIMPL**

```cpp
// 文件：Include/TemplateMode/FVideoTemplate/FVideoTemplatePresenter.h
class FVideoTemplatePresenter : public FFPresenter
{
private:
    // PIMPL 指针：FVideoTemplatePresenter 析构时自动 delete d
    QScopedPointer<FVideoTemplatePresenterPrivate> d;

    // 禁止拷贝：QObject 子类一般不应被拷贝
    Q_DISABLE_COPY(FVideoTemplatePresenter)
};
```

**`QPointer` 安全弱引用**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFObserverHelper.h
template<typename Subject>
class FFSafeSubject
{
private:
    // QPointer：Subject 被 delete 后自动变为 null，避免悬空指针
    QPointer<QObject>  m_pSafeQObjSubject;
    // FFPointer 是类似的自定义实现（针对 FFObject）
    FFPointer<FFObject> m_pSafeFFObjSubject;
};

// 使用时检查：
bool isNull() const {
    if (m_bSafeSubject)
        return m_pSafeFFObjSubject.isNull() && m_pSafeQObjSubject.isNull();
    else
        return !m_pSubject;
}
```

**`deleteLater()` 跨线程安全销毁 QObject**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h
// QFutureWatcher 在工作线程中创建，但必须在主线程销毁
// deleteLater 将销毁操作投递到主线程事件队列
watcherPtr = std::shared_ptr<QFutureWatcher<T>>(
    new QFutureWatcher<T>,
    std::bind(&QObject::deleteLater, std::placeholders::_1)  // 自定义删除器
);
```

### 常见面试题

**Q：Qt 的父子关系内存管理和 `shared_ptr` 有什么本质区别？**

A：
- Qt 父子关系：层级树形结构，父亡子亡；基于 `QObject`，侵入式
- `shared_ptr`：引用计数，最后一个持有者释放；非侵入式，适用任何对象

两者可以混用：`QScopedPointer<Child>` 持有子对象，`Child` 的构造传 `parent`——但 **parent 先析构** 会导致 double-free，需注意 PIMPL 中析构顺序（parent 析构 → d 析构）。

**Q：`QPointer` 和 `std::weak_ptr` 的区别？**

A：
- `QPointer<T>`：仅适用于 `QObject` 子类，不需要 `.lock()`，直接判 `isNull()` 或用 `->` 访问
- `std::weak_ptr<T>`：适用任意 `shared_ptr` 管理的对象，访问前必须 `.lock()` 升级为 `shared_ptr`

**Q：什么情况用 `deleteLater()` 而不是直接 `delete`？**

A：
1. 在槽函数中删除发送信号的对象（直接 delete 会让调用栈悬空）
2. 需要跨线程删除 QObject（直接 delete 不安全）
3. 需要让对象先处理完所有待处理事件再销毁

---

## 5. 自定义控件

### 核心步骤

1. 继承 `QWidget`（或其子类如 `QListView`、`QAbstractButton`）
2. 添加 `Q_OBJECT` 宏
3. 重写 `paintEvent()` 自定义绘制
4. 重写鼠标/键盘事件（`mousePressEvent` 等）
5. 定义信号供外部使用

### 项目代码示例

**自定义列表视图**

```cpp
// 文件：Include/TemplateMode/FTemplateView/FTemplateMediaItemIconView.h
class FTEMPLATEVIEW_EXPORT FTemplateMediaItemIconView : public FMediaItemIconView
{
    Q_OBJECT  // 必须：启用信号槽

public:
    explicit FTemplateMediaItemIconView(std::shared_ptr<FMediaContext> context,
                                        FFUIView* parent = nullptr);

    // 自定义信号：供外部连接
Q_SIGNALS:
    void sigTemplateItemClicked(IFFTemplateMediaItem* item);
    void sigTemplateHoverPreview(IFFTemplateMediaItem* item, bool bPreview);

private Q_SLOTS:
    // 私有槽：响应内部事件
    void slotItemEnterEvent(const QModelIndex&);
    void slotItemLeaveEvent(const QModelIndex&);
    void slotItemDoubleClicked(const QModelIndex& index);
};
```

**控件初始化（设置样式、间距等）**

```cpp
// 文件：Src/TemplateMode/FTemplateView/FTemplateMediaItemIconView.cpp
void FTemplateMediaItemIconView::init()
{
    // 设置列表视图属性
    FFUIStyle::SetBGColorType(listView(), FFSkin::ColorType::Component_Dialog1);
    dynamic_cast<QListView*>(listView())->setSpacing(16);

    // 连接内部 delegate 信号 → 本控件槽 → 触发重绘
    connect(itemDelegate, &FMediaItemIconDelegate::sigRepaint, this, [&]() {
        update();
    });
    connect(itemDelegate, &FMediaItemIconDelegate::sigItemEnterEvent,
            this, &FTemplateMediaItemIconView::slotItemEnterEvent);
}
```

### 常见面试题

**Q：`update()` 和 `repaint()` 的区别？**

A：
- `update()`：将重绘请求**加入事件队列**，Qt 会在合适时机合并多次 update 为一次 `paintEvent`，效率高
- `repaint()`：**立即同步**触发 `paintEvent`，用于对实时性要求极高的场景（通常不推荐）

**Q：如何实现双缓冲绘制（避免闪烁）？**

A：Qt 5 的 QWidget 默认开启双缓冲（`WA_OpaquePaintEvent` + 后台 buffer）。对于复杂绘制，可在 `paintEvent` 中先绘制到 `QPixmap` 缓冲，再将 `QPixmap` 一次性绘制到 `QPainter`。

---

## 6. 模型/视图架构

### 核心类层次

```
QAbstractItemModel
    ├── QAbstractListModel     → 列表数据
    ├── QAbstractTableModel    → 表格数据
    └── QStandardItemModel     → 通用树形数据

QAbstractItemView
    ├── QListView              → 列表显示
    ├── QTableView             → 表格显示
    └── QTreeView              → 树形显示

QAbstractItemDelegate
    └── QStyledItemDelegate    → 自定义绘制和编辑
```

### 项目代码示例

**自定义 Model + View + SelectionModel**

```cpp
// 文件：Src/TemplateMode/FTemplateView/FTemplateMediaItemIconView.cpp
void FTemplateMediaItemIconView::init()
{
    // Model：提供数据
    m_model = new FTemplateMediaItemModel(context(), this);

    // SelectionModel：管理选中状态（自定义，继承 QItemSelectionModel）
    m_selectionModel = new FFMediaItemSelectionModel(m_model, this);

    // 将 model 设置到 View（继承自 QListView）
    setModel(m_selectionModel);

    // 监听 Model 的数据变化信号
    connect(m_model, &FTemplateMediaItemModel::sigFetchMoreSuccess, this, [&]() {
        restartCheckResourceThumbnailTimer();
    });
}
```

**Model 实现要点（QAbstractItemModel 派生类必须实现的方法）**

```cpp
// 通用 Model 必须实现的 5 个纯虚函数：
class MyModel : public QAbstractItemModel
{
    // 1. 总行数
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    // 2. 总列数
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    // 3. 返回数据
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    // 4. 返回节点的 index
    QModelIndex index(int row, int col, const QModelIndex& parent) const override;
    // 5. 返回父节点
    QModelIndex parent(const QModelIndex& index) const override;
};
```

### 常见面试题

**Q：Model 数据变化后如何通知 View？**

A：Model 必须发出相应信号：
- `dataChanged(topLeft, bottomRight, roles)`：部分数据改变
- `beginInsertRows()` + `endInsertRows()`：插入行（必须成对调用）
- `beginRemoveRows()` + `endRemoveRows()`：删除行
- `beginResetModel()` + `endResetModel()`：全量重置（性能差，慎用）

**Q：`QModelIndex` 包含哪些信息？**

A：`row`、`column`、`internalPointer`（或 `internalId`）、`model`。`internalPointer` 是自定义节点数据指针，用于在 `data()`/`index()` 中快速定位数据，避免 O(n) 查找。

**Q：如何优化大数据量 Model 的性能？**

A：
1. 实现 `canFetchMore()` + `fetchMore()` 实现**懒加载**（项目中 `sigFetchMoreSuccess` 信号即此场景）
2. `data()` 中缓存计算结果，避免重复计算
3. 减少不必要的 `dataChanged` 范围（精确到最小改变区域）
4. 使用 `QAbstractItemModel::beginResetModel()` 时谨慎，会导致 View 完全重建

---

## 7. Qt 线程与并发

### 核心方式

| 方式 | 适用场景 |
|------|----------|
| `QThread` | 长时间运行的后台线程 |
| `QtConcurrent::run` | 简单的并行任务，自动线程池 |
| `QFuture` / `QFutureWatcher` | 异步任务结果监控 |
| `QMetaObject::invokeMethod` | 跨线程调用对象方法 |
| `QTimer::singleShot` | 延迟在主线程执行 |

### 项目代码示例

**条件编译：QThread 和 std::thread 双支持**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Src/FFAsync/FFThread.h

class FFThread
#ifdef USE_QT_THREAD
    : public QThread            // Qt 平台：使用 QThread
#endif
{
    // 策略模式：线程执行内容由 Delegate 决定
    class Delegate {
    public:
        virtual ~Delegate();
        virtual void entryThreadMain();  // 子类实现具体逻辑
    };

    FFThread(Delegate* delegate, const QString& threadName);

#ifndef USE_QT_THREAD
    std::unique_ptr<std::thread> m_thread;  // 标准线程
#endif
};
```

**QFuture + QFutureWatcher：监控异步任务**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h

FFFuture(const QFuture<T> &f)
{
    // shared_ptr + deleteLater：跨线程安全销毁 watcher
    watcherPtr = std::shared_ptr<QFutureWatcher<T>>(
        new QFutureWatcher<T>,
        std::bind(&QObject::deleteLater, std::placeholders::_1)
    );

    QFutureWatcher<T> *watcher = watcherPtr.get();

    // 监听 finished 信号：任务完成后执行 continuation
    QObject::connect(watcher, &QFutureWatcher<T>::finished, [watcher, this]() {
        if (m_continuation)
            m_continuation(watcher->future());
        delete this;
    });

    watcher->setFuture(f);  // 开始监控
}
```

**`QMetaObject::invokeMethod` 跨线程回调到 UI 线程**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h

// 场景：工作线程任务完成，切换到 UI 线程更新界面
QMetaObject::invokeMethod(
    context,          // 目标对象（决定在哪个线程执行）
    [func = std::move(func), promise = std::move(promise), parent]() mutable {
        // 此 lambda 在 context 所属线程（主线程）执行
        SyncContinuation<...> job(std::forward<Function>(func), parent, std::move(promise));
        job.execute();
    }
    // 默认 Qt::AutoConnection：自动决定 Direct 或 Queued
);
```

### 常见面试题

**Q：QThread 的 `run()` 和 `start()` 的区别？**

A：
- `start()`：在**原线程**调用，创建新线程并调用 `run()`，立刻返回
- `run()`：在**新线程**中执行，默认实现为 `exec()`（事件循环）；重写此函数实现线程逻辑

**Q：QThread 的 `moveToThread` 用法？**

A：将 QObject 移动到新线程，之后该对象的槽函数会在新线程的事件循环中执行：
```cpp
Worker *worker = new Worker();
QThread *thread = new QThread();
worker->moveToThread(thread);
connect(thread, &QThread::started, worker, &Worker::doWork);
connect(worker, &Worker::finished, thread, &QThread::quit);
thread->start();
```
注意：moveToThread 后不能有父对象。

**Q：直接在 QThread::run() 中使用 QObject 的注意事项？**

A：在 `run()` 中创建的 QObject 属于该线程，不能在其他线程访问。不要将在 UI 线程创建的 QObject 直接在 `run()` 中调用方法（需通过 `invokeMethod` 或信号槽队列连接）。

---

## 8. PIMPL 与 Q_DECLARE_PRIVATE

### 为什么用 PIMPL

1. **减少编译依赖**：头文件不暴露私有成员的具体类型，修改私有实现不触发级联重编译
2. **ABI 稳定**：动态库升级时，只要公共接口不变，二进制兼容
3. **隐藏实现细节**：SDK/库发布时保护内部细节

### 项目代码示例

**Qt 风格 PIMPL（Q_DECLARE_PRIVATE + QScopedPointer）**

```cpp
// ============ 头文件：FVideoTemplatePresenter.h ============
class FVideoTemplatePresenterPrivate;   // 前向声明，不包含实现头文件

class FVideoTemplatePresenter : public FFPresenter
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(FVideoTemplatePresenter)   // 展开为：
                                                  // inline FVideoTemplatePresenterPrivate* d_func()
                                                  // inline const FVideoTemplatePresenterPrivate* d_func() const
                                                  // friend class FVideoTemplatePresenterPrivate;

public:
    FVideoTemplatePresenter(QObject* parent = Q_NULLPTR);
    // ... 公共接口 ...

private:
    QScopedPointer<FVideoTemplatePresenterPrivate> d;  // 不透明 d-pointer
    Q_DISABLE_COPY(FVideoTemplatePresenter)
};

// ============ 实现文件：FVideoTemplatePresenter.cpp ============
class FVideoTemplatePresenterPrivate
{
public:
    // 所有私有成员都放这里
    QString m_sessionId;
    bool m_useFromPreviewPlayer = false;
    // ...
};
```

### 常见面试题

**Q：`Q_DECLARE_PRIVATE` 和直接写 `private` 成员的区别？**

A：`Q_DECLARE_PRIVATE` 配合 `QScopedPointer<D>` 实现了完整的 PIMPL 模式——私有数据通过前向声明类型的指针存储，头文件不包含具体类型，可以做到：
- 修改私有数据结构不重新编译包含此头文件的文件
- 直接写 `private` 成员则每次私有成员变动都会触发级联重编

---

## 9. 资源系统与样式表

### Qt 资源系统（.qrc）

- 将图片、QSS 样式表、字体等二进制文件打包进可执行文件或动态库
- CMake 中 `qt5_add_resources()` 或 `AUTORCC ON` 自动处理
- 访问路径格式：`:/prefix/path/to/file`

### 项目代码示例

**CMake 自动生成的 qrc 资源（FExportView）**

```
// 文件（自动生成）：
// Build/Src/FExportView/FExportView_autogen/include_Debug/EWIEGA46WW/qrc_FExportView_CMAKE_.cpp
// 这是 CMake AUTORCC 从 .qrc 文件生成的 C++ 资源文件，包含所有嵌入资源的二进制数据
```

**样式表（QSS）动态加载**

```cpp
// 项目中通过皮肤系统统一管理样式
// 类似用法（参考项目 FFSkinStyle 调用）：
qApp->setStyleSheet(":/styles/dark_theme.qss");

// 或针对单个控件：
widget->setStyleSheet("QListView { background: #2b2b2b; border: none; }");
```

### 常见面试题

**Q：qrc 资源和直接读取文件路径的区别？**

A：qrc 资源编译进二进制，部署时不需要额外文件，路径以 `:/` 开头；文件路径依赖外部文件存在，便于开发时热替换但部署复杂。

---

## 10. Qt 布局管理

### 常用布局

| 布局类 | 用途 |
|--------|------|
| `QHBoxLayout` | 水平排列 |
| `QVBoxLayout` | 垂直排列 |
| `QGridLayout` | 网格排列 |
| `QFormLayout` | 标签-字段对 |
| `QStackedLayout` | 堆叠（同一时刻只显示一个） |

### 项目代码示例

**控件属性设置（代替布局部分功能）**

```cpp
// 文件：Src/TemplateMode/FTemplateView/FTemplateMediaItemIconView.cpp
// 自定义视图中通过 setSpacing 等属性控制布局
dynamic_cast<QListView*>(listView())->setSpacing(16);  // Item 间距

// FFUIStyle 统一设置背景色（皮肤系统）
FFUIStyle::SetBGColorType(listView(), FFSkin::ColorType::Component_Dialog1);
```

### 常见面试题

**Q：`setSizePolicy` 的 `Expanding` 和 `Fixed` 的区别？**

A：`Fixed`：控件保持 `sizeHint()` 大小，不伸缩；`Expanding`：控件可以任意拉伸，占用所有多余空间。布局时 Qt 会优先分配空间给 `Expanding` 控件，`Fixed` 控件保持不变。

**Q：如何让两个控件等分空间？**

A：两个控件都设置 `sizePolicy` 为 `Expanding`（默认权重相同为1），或使用 `layout->setStretch(index, weight)` 按权重分配。

---

## 11. Qt 智能指针

### 四种 Qt 智能指针对比

| 类型 | 对应 STL | 特点 |
|------|---------|------|
| `QScopedPointer<T>` | `unique_ptr<T>` | 离开作用域自动 delete，不可拷贝 |
| `QSharedPointer<T>` | `shared_ptr<T>` | 引用计数，可拷贝 |
| `QWeakPointer<T>` | `weak_ptr<T>` | 弱引用 QSharedPointer，不增加计数 |
| `QPointer<T>` | — | 仅限 QObject，对象被 delete 后自动置 null |

### 项目代码示例

**`QScopedPointer` 实现 PIMPL（最常见用法）**

```cpp
// 文件：Include/TemplateMode/FVideoTemplate/FVideoTemplatePresenter.h
QScopedPointer<FVideoTemplatePresenterPrivate> d;
// FVideoTemplatePresenter 析构 → QScopedPointer 析构 → delete d → ~Private()
```

**`QPointer` 观察 QObject 生命周期**

```cpp
// 文件：3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFFuture.h
// 捕获接收者的 QPointer：接收者 delete 后 context.isNull() == true
context = QPointer<QObject>(context)

// 使用时检查：
if (context.isNull()) {
    // 接收者已销毁，不执行回调，取消 future
    promise.reportCanceled();
}
```

### 常见面试题

**Q：`QPointer` 和 `QSharedPointer` + `QWeakPointer` 的区别？**

A：
- `QPointer`：轻量，只适用于 `QObject`，不需要对象被 `QSharedPointer` 管理；直接 `delete` 对象后 `QPointer` 自动置 null（通过 `QObjectPrivate` 追踪）
- `QWeakPointer`：必须配合 `QSharedPointer` 使用，功能上与 `std::weak_ptr` 相似，要调用 `.toStrongRef()` 升级

**Q：什么时候用 `QScopedPointer` 而不是 `std::unique_ptr`？**

A：Qt 专有代码（如 PIMPL 配合 `Q_DECLARE_PRIVATE`）用 `QScopedPointer` 更习惯，API 更接近 Qt 风格。C++11 以后的纯 C++ 代码推荐用 `std::unique_ptr`，与标准库生态更兼容（可放入 STL 容器、作函数返回值等）。项目中两者都有使用。

---

## 附录：项目中涉及的核心文件索引

| 文件 | Qt 知识点 |
|------|-----------|
| `3rdparty/.../FFCore/FFSubject.h` | QList、QThread（线程检查）、Q_OBJECT 前置使用 |
| `3rdparty/.../FFCore/FFObserverHelper.h` | QPointer、Q_DISABLE_COPY、if constexpr + QObject |
| `3rdparty/.../FFCore/FFFuture.h` | QFuture/QFutureWatcher、QMetaObject::invokeMethod、QPointer、deleteLater |
| `3rdparty/.../FFAsync/FFThread.h` | QThread 条件编译封装 |
| `3rdparty/.../FFAsync/FFThreadPool.h` | 线程池（QObject 继承）、多线程任务调度 |
| `3rdparty/.../FFWidgets/FFEventFilter.h` | 事件过滤器接口设计 |
| `3rdparty/.../Src/FFWidgets/FFEventFilter.cpp` | 责任链分发事件 |
| `Include/.../FVideoTemplatePresenter.h` | Q_OBJECT、Q_DECLARE_PRIVATE、QScopedPointer、Q_DISABLE_COPY、多继承 |
| `Src/.../FTemplateMediaItemIconView.cpp` | connect 新式语法、lambda 槽、自定义 Model/View |
