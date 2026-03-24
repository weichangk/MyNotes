# QObject 与对象树

## 1. QObject 概述

`QObject` 是 Qt 对象模型的核心基类，几乎所有 Qt 类都直接或间接继承自它。它提供了以下关键能力：

- **对象树（Object Tree）**：父子层级关系与自动内存管理
- **信号与槽（Signals & Slots）**：类型安全的对象间通信机制
- **元对象系统（Meta-Object System）**：运行时类型信息、动态属性、反射
- **事件系统（Event System）**：事件的接收与过滤
- **国际化支持**：通过 `tr()` 实现字符串翻译
- **定时器**：内置的定时器支持

### 1.1 QObject 的声明要求

任何继承 `QObject` 的类，都必须在类声明的**私有区域**添加 `Q_OBJECT` 宏：

```cpp
#include <QObject>

class MyObject : public QObject
{
    Q_OBJECT  // 必须在类的开头，启用元对象系统

public:
    explicit MyObject(QObject *parent = nullptr);
    ~MyObject();

signals:
    void valueChanged(int newValue);

public slots:
    void setValue(int value);

private:
    int m_value = 0;
};
```

`Q_OBJECT` 宏的作用：
- 启用元对象编译器（moc）为该类生成元信息代码
- 使类支持信号与槽
- 使类支持 `qobject_cast<>()`
- 使类支持动态属性
- 使类支持 `tr()` 国际化

> ⚠️ 忘记添加 `Q_OBJECT` 是初学者最常见的错误之一，会导致信号槽连接失败、`qobject_cast` 返回 `nullptr` 等问题。

### 1.2 QObject 的三大限制

| 限制 | 原因 |
|------|------|
| **不可拷贝** | `QObject` 删除了拷贝构造函数和拷贝赋值运算符。每个对象有唯一身份（identity），拷贝语义与对象身份冲突 |
| **不可移动** | 对象可能被信号槽连接、事件过滤器、定时器等引用，移动会破坏这些关联 |
| **不建议存储在容器值类型中** | 应使用指针容器 `QList<MyObject*>` 而非 `QList<MyObject>` |

```cpp
// ❌ 编译错误：QObject 不可拷贝
MyObject a;
MyObject b = a;  // error: copy constructor is deleted

// ✅ 正确做法：使用指针
QList<MyObject*> objects;
objects.append(new MyObject(parentObj));
```

---

## 2. 对象树（Object Tree）

### 2.1 什么是对象树

Qt 使用**树形结构**来组织 `QObject` 对象。每个 `QObject` 可以有一个父对象（parent）和任意数量的子对象（children），形成一棵树：

```
QMainWindow (根节点)
├── QMenuBar
│   ├── QMenu ("File")
│   │   ├── QAction ("Open")
│   │   └── QAction ("Save")
│   └── QMenu ("Edit")
├── QToolBar
│   ├── QAction ("Cut")
│   └── QAction ("Copy")
├── QStatusBar
└── CentralWidget (QWidget)
    ├── QVBoxLayout
    ├── QLabel
    └── QPushButton
```

### 2.2 建立父子关系

有两种方式建立父子关系：

**方式一：构造时指定 parent**

```cpp
// parent 在构造函数中指定
QObject *parent = new QObject();
QObject *child1 = new QObject(parent);  // child1 的父对象是 parent
QObject *child2 = new QObject(parent);  // child2 的父对象也是 parent
```

**方式二：调用 `setParent()`**

```cpp
QObject *parent = new QObject();
QObject *child = new QObject();
child->setParent(parent);  // 运行时设置父对象
```

**对于 QWidget，还可以通过布局自动设置**

```cpp
QWidget *parentWidget = new QWidget();
QVBoxLayout *layout = new QVBoxLayout(parentWidget);

QPushButton *btn = new QPushButton("Click Me");
layout->addWidget(btn);  // btn 的 parent 自动设置为 parentWidget
```

### 2.3 查询对象树

```cpp
QObject *obj = new QObject();
QObject *child1 = new QObject(obj);
child1->setObjectName("child1");
QObject *child2 = new QObject(obj);
child2->setObjectName("child2");
QObject *grandChild = new QObject(child1);
grandChild->setObjectName("grandChild");

// 获取父对象
QObject *p = child1->parent();        // 返回 obj

// 获取直接子对象列表
const QObjectList &children = obj->children();  // 返回 {child1, child2}

// 按名称查找子对象（递归查找）
QObject *found = obj->findChild<QObject*>("grandChild");  // 找到 grandChild

// 按类型查找所有子对象
QList<QPushButton*> buttons = widget->findChildren<QPushButton*>();

// 按名称 + 类型查找
QPushButton *btn = widget->findChild<QPushButton*>("okButton");

// 限制查找深度：只查找直接子对象
QObject *direct = obj->findChild<QObject*>("grandChild", Qt::FindDirectChildrenOnly);
// 返回 nullptr，因为 grandChild 不是 obj 的直接子对象
```

### 2.4 对象树与内存管理

**核心规则：当父对象被销毁时，它会自动销毁所有子对象。**

这是 Qt 内存管理的基石。你只需要管理根对象的生命周期，子对象会被自动清理：

```cpp
void createUI()
{
    // 只需 delete mainWindow，所有子控件自动销毁
    QMainWindow *mainWindow = new QMainWindow();

    QWidget *central = new QWidget(mainWindow);
    mainWindow->setCentralWidget(central);

    QVBoxLayout *layout = new QVBoxLayout(central);
    QPushButton *btn1 = new QPushButton("Button 1", central);
    QPushButton *btn2 = new QPushButton("Button 2", central);
    layout->addWidget(btn1);
    layout->addWidget(btn2);

    mainWindow->show();
    // 当 mainWindow 被 delete 时，central、layout、btn1、btn2 全部自动释放
}
```

**销毁顺序**：父对象析构时，先销毁所有子对象，再销毁自身。子对象的销毁顺序与创建顺序相反（LIFO）。

### 2.5 对象树销毁的详细过程

```cpp
class MyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        qDebug() << objectName() << "constructed";
    }
    ~MyWidget() override
    {
        qDebug() << objectName() << "destroyed";
    }
};

// 演示销毁顺序
void demo()
{
    MyWidget *root = new MyWidget();
    root->setObjectName("Root");

    MyWidget *childA = new MyWidget(root);
    childA->setObjectName("ChildA");

    MyWidget *childB = new MyWidget(root);
    childB->setObjectName("ChildB");

    MyWidget *grandChild = new MyWidget(childA);
    grandChild->setObjectName("GrandChild");

    delete root;
}

// 输出：
// "GrandChild" destroyed    ← 最深的先销毁
// "ChildA" destroyed        ← ChildA 的子对象已销毁，然后销毁自身
// "ChildB" destroyed        ← 与 ChildA 同级，后创建的先销毁
// "Root" destroyed          ← 最后销毁根对象
```

---

## 3. 对象树中的常见陷阱与最佳实践

### 3.1 栈对象的析构顺序问题

**⚠️ 危险：子对象在父对象之前析构**

```cpp
void dangerous()
{
    QWidget parent;
    QWidget child(&parent);  // child 的父对象是 parent

    // 函数结束时，栈对象按声明的逆序析构：
    // 1. child 先析构 → child 从 parent 的子列表中移除（正常）
    // 2. parent 再析构 → 尝试销毁子列表（此时 child 已不在其中，OK）
    // ✅ 这种情况是安全的，因为 child 析构时会自动从 parent 中解除注册
}
```

```cpp
void alsoDangerous()
{
    QWidget child;
    QWidget parent;
    child.setParent(&parent);

    // 函数结束时：
    // 1. parent 先析构（后声明的先析构）→ 试图 delete child → child 是栈对象！
    // ❌ 双重析构！未定义行为！程序崩溃！
}
```

**最佳实践：确保子对象在父对象之后声明，或者在堆上创建子对象。**

```cpp
// ✅ 安全：子对象在父对象之后声明
void safe()
{
    QWidget parent;        // 先声明
    QWidget child(&parent); // 后声明 → 先析构 → 安全
}

// ✅ 更推荐：在堆上创建对象
void recommended()
{
    QWidget *parent = new QWidget();
    QWidget *child = new QWidget(parent);  // 堆对象，由 parent 管理
    parent->show();
    // 后续 delete parent 即可
}
```

### 3.2 手动 delete 子对象

手动 `delete` 一个子对象是安全的——它会在析构时自动从父对象的子列表中移除：

```cpp
QObject *parent = new QObject();
QObject *child1 = new QObject(parent);
QObject *child2 = new QObject(parent);

qDebug() << parent->children().size();  // 2

delete child1;  // child1 自动从 parent 的子列表中移除

qDebug() << parent->children().size();  // 1

delete parent;  // 只会 delete child2
```

### 3.3 deleteLater() — 延迟删除

在信号槽的回调中直接 `delete` 发送信号的对象是危险的。应使用 `deleteLater()`：

```cpp
// ❌ 危险：在槽函数中直接 delete 发送者
connect(button, &QPushButton::clicked, [button]() {
    delete button;  // button 的 clicked 信号还在执行栈中！
});

// ✅ 安全：延迟到事件循环的下一轮再删除
connect(button, &QPushButton::clicked, [button]() {
    button->deleteLater();  // 安全，会在回到事件循环时删除
});
```

`deleteLater()` 的工作机制：
1. 向事件队列投递一个 `QDeferredDeleteEvent`
2. 当前代码继续执行完毕
3. 事件循环处理该事件时，才真正调用 `delete`

### 3.4 动态改变父对象

```cpp
QObject *parent1 = new QObject();
parent1->setObjectName("Parent1");
QObject *parent2 = new QObject();
parent2->setObjectName("Parent2");

QObject *child = new QObject(parent1);
child->setObjectName("Child");

qDebug() << parent1->children().size();  // 1
qDebug() << parent2->children().size();  // 0

// 改变父对象
child->setParent(parent2);

qDebug() << parent1->children().size();  // 0 — child 已从 parent1 移除
qDebug() << parent2->children().size();  // 1 — child 现在属于 parent2

// 设置为 nullptr 使其成为独立对象
child->setParent(nullptr);
// 此时需要手动管理 child 的生命周期
```

> 对于 `QWidget`，改变 parent 还会影响窗口的显示行为：无 parent 的 QWidget 是顶层窗口（有标题栏），有 parent 的是嵌入式子控件。

---

## 4. QObject 的核心功能

### 4.1 对象名称（objectName）

每个 `QObject` 可以设置一个字符串名称，用于调试和对象查找：

```cpp
QPushButton *btn = new QPushButton("OK");
btn->setObjectName("okButton");

// 通过名称查找
QPushButton *found = parent->findChild<QPushButton*>("okButton");

// 在 QSS 样式表中使用对象名称
// QPushButton#okButton { color: red; }
```

### 4.2 类型转换：qobject_cast

`qobject_cast<>()` 是 Qt 提供的类型安全的向下转换，类似 `dynamic_cast` 但更快（不依赖 RTTI，而是使用元对象系统）：

```cpp
QObject *obj = new QPushButton("Test");

// 安全的类型转换
QPushButton *btn = qobject_cast<QPushButton*>(obj);
if (btn) {
    btn->setText("Clicked!");  // 转换成功
}

// 错误的类型转换
QLabel *label = qobject_cast<QLabel*>(obj);
if (!label) {
    qDebug() << "obj is not a QLabel";  // 输出此行
}
```

`qobject_cast` vs `dynamic_cast`：

| 特性 | `qobject_cast` | `dynamic_cast` |
|------|----------------|----------------|
| 前提条件 | 类必须有 `Q_OBJECT` 宏 | 类必须有虚函数（多态） |
| 性能 | 快（基于元对象系统的字符串比较） | 较慢（基于 RTTI） |
| 跨 DLL 边界 | ✅ 安全 | ⚠️ 某些编译器/平台有问题 |
| 支持多重继承 | 仅限第一个基类是 QObject | 完全支持 |

### 4.3 运行时类型信息

```cpp
QObject *obj = new QPushButton("Test");

// 获取类名
qDebug() << obj->metaObject()->className();  // "QPushButton"

// 判断是否是某个类的实例
qDebug() << obj->inherits("QPushButton");   // true
qDebug() << obj->inherits("QAbstractButton"); // true
qDebug() << obj->inherits("QWidget");        // true
qDebug() << obj->inherits("QLabel");         // false
```

### 4.4 内置定时器

`QObject` 提供了轻量级的定时器接口：

```cpp
class MyObject : public QObject
{
    Q_OBJECT
public:
    void startWork()
    {
        // 启动定时器，每 1000ms 触发一次
        m_timerId = startTimer(1000);

        // 也可以使用 std::chrono
        m_fastTimerId = startTimer(std::chrono::milliseconds(100));
    }

    void stopWork()
    {
        killTimer(m_timerId);
        killTimer(m_fastTimerId);
    }

protected:
    void timerEvent(QTimerEvent *event) override
    {
        if (event->timerId() == m_timerId) {
            qDebug() << "1 second tick";
        } else if (event->timerId() == m_fastTimerId) {
            qDebug() << "100ms tick";
        }
    }

private:
    int m_timerId = 0;
    int m_fastTimerId = 0;
};
```

> 对于更高级的定时器需求，推荐使用 `QTimer` 类（它也是 `QObject` 的子类）。

### 4.5 事件过滤器

`QObject` 可以监控其他对象收到的事件：

```cpp
class KeyFilter : public QObject
{
    Q_OBJECT
protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                qDebug() << watched->objectName() << "pressed Escape";
                return true;  // 事件已处理，不再传递
            }
        }
        return QObject::eventFilter(watched, event);  // 继续传递
    }
};

// 使用
KeyFilter *filter = new KeyFilter(this);
lineEdit->installEventFilter(filter);  // lineEdit 的所有事件先经过 filter
```

---

## 5. 对象树在 QWidget 中的体现

### 5.1 窗口层级与对象树

对于 `QWidget` 及其子类，对象树同时决定了**窗口的视觉层级**：

```cpp
QWidget *window = new QWidget();         // 顶层窗口（无 parent）
window->setWindowTitle("My App");
window->resize(400, 300);

QWidget *panel = new QWidget(window);    // window 的子控件，绘制在 window 上
panel->setGeometry(10, 10, 200, 200);
panel->setStyleSheet("background: lightblue;");

QPushButton *btn = new QPushButton("Click", panel);  // panel 的子控件
btn->move(20, 20);
// btn 绘制在 panel 上，panel 绘制在 window 上

window->show();  // 显示 window 同时显示 panel 和 btn
```

### 5.2 布局管理器与对象树

布局管理器会自动管理子控件的 parent：

```cpp
QWidget *container = new QWidget();
QHBoxLayout *layout = new QHBoxLayout(container);  // layout 的 parent 是 container

QPushButton *btn1 = new QPushButton("A");  // 暂时没有 parent
QPushButton *btn2 = new QPushButton("B");

layout->addWidget(btn1);  // btn1 的 parent 自动设为 container
layout->addWidget(btn2);  // btn2 的 parent 自动设为 container

// 此时 container->children() 包含 layout、btn1、btn2
```

### 5.3 使用 dumpObjectTree() 调试

`QObject::dumpObjectTree()` 可以打印完整的对象树结构，非常适合调试：

```cpp
QMainWindow mainWindow;
mainWindow.setObjectName("MainWindow");

QWidget *central = new QWidget(&mainWindow);
central->setObjectName("CentralWidget");
mainWindow.setCentralWidget(central);

QVBoxLayout *layout = new QVBoxLayout(central);
QPushButton *btn = new QPushButton("Test", central);
btn->setObjectName("TestButton");
QLabel *label = new QLabel("Hello", central);
label->setObjectName("HelloLabel");
layout->addWidget(btn);
layout->addWidget(label);

mainWindow.dumpObjectTree();
```

输出类似于：

```
QMainWindow::MainWindow
    QWidget::CentralWidget
        QVBoxLayout::
        QPushButton::TestButton
        QLabel::HelloLabel
    QMenuBar::
    QMainWindowLayout::
    QToolBar::
    QStatusBar::
```

---

## 6. 综合实战示例

### 6.1 自定义组件的对象树设计

```cpp
// 一个自定义的对话框组件
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle(tr("Settings"));

        // 所有子控件都以 this 为 parent，构成对象树
        auto *mainLayout = new QVBoxLayout(this);

        // 用户名输入区
        auto *nameGroup = new QGroupBox(tr("User Info"), this);
        auto *nameLayout = new QHBoxLayout(nameGroup);
        auto *nameLabel = new QLabel(tr("Name:"), nameGroup);
        m_nameEdit = new QLineEdit(nameGroup);
        nameLayout->addWidget(nameLabel);
        nameLayout->addWidget(m_nameEdit);

        // 选项区
        m_darkMode = new QCheckBox(tr("Dark Mode"), this);
        m_autoSave = new QCheckBox(tr("Auto Save"), this);

        // 按钮区
        auto *buttonLayout = new QHBoxLayout();
        auto *okBtn = new QPushButton(tr("OK"), this);
        auto *cancelBtn = new QPushButton(tr("Cancel"), this);
        buttonLayout->addStretch();
        buttonLayout->addWidget(okBtn);
        buttonLayout->addWidget(cancelBtn);

        mainLayout->addWidget(nameGroup);
        mainLayout->addWidget(m_darkMode);
        mainLayout->addWidget(m_autoSave);
        mainLayout->addLayout(buttonLayout);

        // 连接信号槽
        connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    }

    // delete SettingsDialog 时，所有子控件自动释放
    // 不需要在析构函数中手动 delete 任何子控件

private:
    QLineEdit *m_nameEdit;
    QCheckBox *m_darkMode;
    QCheckBox *m_autoSave;
};
```

### 6.2 动态创建与销毁子对象

```cpp
class DynamicPanel : public QWidget
{
    Q_OBJECT
public:
    explicit DynamicPanel(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_layout = new QVBoxLayout(this);
        auto *addBtn = new QPushButton("Add Item", this);
        auto *clearBtn = new QPushButton("Clear All", this);

        m_layout->addWidget(addBtn);
        m_layout->addWidget(clearBtn);

        connect(addBtn, &QPushButton::clicked, this, &DynamicPanel::addItem);
        connect(clearBtn, &QPushButton::clicked, this, &DynamicPanel::clearItems);
    }

public slots:
    void addItem()
    {
        auto *item = new QLabel(
            QString("Item %1").arg(++m_counter), this);
        m_layout->addWidget(item);
        m_items.append(item);
    }

    void clearItems()
    {
        // 手动 delete 子对象是安全的
        qDeleteAll(m_items);  // delete 每个 item，自动从对象树中移除
        m_items.clear();
        m_counter = 0;
    }

private:
    QVBoxLayout *m_layout;
    QList<QLabel*> m_items;
    int m_counter = 0;
};
```

### 6.3 利用对象树实现资源管理模式

```cpp
// 利用 QObject 对象树实现 RAII 风格的资源管理
class DatabaseConnection : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseConnection(const QString &name, QObject *parent = nullptr)
        : QObject(parent), m_name(name)
    {
        setObjectName(name);
        qDebug() << "Database connection" << m_name << "opened";
    }

    ~DatabaseConnection() override
    {
        qDebug() << "Database connection" << m_name << "closed";
    }

private:
    QString m_name;
};

class AppServices : public QObject
{
    Q_OBJECT
public:
    explicit AppServices(QObject *parent = nullptr) : QObject(parent)
    {
        // 所有连接都是 AppServices 的子对象
        new DatabaseConnection("MainDB", this);
        new DatabaseConnection("CacheDB", this);
        new DatabaseConnection("LogDB", this);
    }
    // 析构时自动关闭所有数据库连接
};

// 使用
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    AppServices services;  // 栈上创建，程序结束时自动释放
    // ... 业务逻辑 ...

    return app.exec();
}
// 输出：
// Database connection "LogDB" closed
// Database connection "CacheDB" closed
// Database connection "MainDB" closed
```

---

## 7. 总结

### 对象树的核心要点

| 要点 | 说明 |
|------|------|
| 自动内存管理 | 父对象析构时自动 `delete` 所有子对象 |
| 安全的手动删除 | 子对象被 `delete` 时自动从父对象的子列表中移除 |
| 栈对象注意声明顺序 | 子对象必须在父对象之后声明 |
| `deleteLater()` | 在信号槽回调中安全删除对象 |
| `findChild`/`findChildren` | 按名称或类型在对象树中搜索 |
| `dumpObjectTree()` | 打印对象树结构，调试利器 |
| QWidget 视觉层级 | 对象树决定了控件的绘制和事件传播层级 |

### 最佳实践清单

1. **堆上创建子对象** — 减少栈对象析构顺序的风险
2. **始终指定 parent** — 避免内存泄漏，让父对象管理生命周期
3. **不要手动 delete 有 parent 的对象**（除非你清楚在做什么）— 让对象树自动管理
4. **使用 `deleteLater()`** — 而非直接 `delete`，尤其在信号槽上下文中
5. **给对象设置 `objectName`** — 方便调试和样式表引用
6. **善用 `dumpObjectTree()`** — 遇到对象生命周期问题时第一时间查看对象树结构
7. **注意顶层窗口的释放** — 设置 `Qt::WA_DeleteOnClose` 属性让窗口关闭时自动删除

```cpp
// 推荐的顶层窗口创建方式
auto *dialog = new SettingsDialog(this);
dialog->setAttribute(Qt::WA_DeleteOnClose);  // 关闭时自动 delete
dialog->show();
```
