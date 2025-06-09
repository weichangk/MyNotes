
在 Qt 中，**布局管理器（Layout Manager）**用于自动管理控件（部件）的大小和位置。相比于手动设置控件坐标，使用布局管理器可以让界面更加灵活、响应式，适应不同平台和窗口尺寸。

### QLayout 类关系图
![QLayout 类关系图](./assets/QLayout%20%E7%B1%BB%E5%85%B3%E7%B3%BB%E5%9B%BE.png)



### 常见布局管理器

| 类名             | 说明                                         |
| ---------------- | -------------------------------------------- |
| `QHBoxLayout`    | 水平布局，将控件从左到右排列                 |
| `QVBoxLayout`    | 垂直布局，将控件从上到下排列                 |
| `QGridLayout`    | 网格布局，按行列方式排列控件                 |
| `QFormLayout`    | 表单布局，常用于设置界面（标签+输入框）      |
| `QStackedLayout` | 堆叠布局，一次只显示一个控件（比如多页切换） |



### Qt 基本布局管理器（QBoxLayout）

```cpp
#pragma once
#include <QWidget>

void qbox_layout();

class QBoxLayoutWidget : public QWidget {
    Q_OBJECT
public:
    explicit QBoxLayoutWidget(QWidget *parent = nullptr);
    ~QBoxLayoutWidget();
};
```

```cpp
#include "qbox_layout.h"

#include <QEventLoop>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

void qbox_layout() {
    QBoxLayoutWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &QBoxLayoutWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

QBoxLayoutWidget::QBoxLayoutWidget(QWidget *parent) :
    QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QPushButton("按钮1"));
    layout->addWidget(new QPushButton("按钮2"));
    layout->addWidget(new QPushButton("按钮3"));

    auto hlayout = new QHBoxLayout;
    hlayout->addWidget(new QPushButton("按钮4"));
    hlayout->addWidget(new QPushButton("按钮5"));
    hlayout->addWidget(new QPushButton("按钮6"));

    layout->addLayout(hlayout);
    
    setLayout(layout);
}

QBoxLayoutWidget::~QBoxLayoutWidget() {
}
```



### Qt 窗体布局管理器（QFormLayout）

```cpp
#pragma once
#include <QWidget>

void qform_layout();

class QFormLayoutWidget : public QWidget {
    Q_OBJECT
public:
    explicit QFormLayoutWidget(QWidget *parent = nullptr);
    ~QFormLayoutWidget();
};
```

```cpp
#include "qform_layout.h"

#include <QEventLoop>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

void qform_layout() {
    QFormLayoutWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &QFormLayoutWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

QFormLayoutWidget::QFormLayoutWidget(QWidget *parent) :
    QWidget(parent) {
    QFormLayout *layout = new QFormLayout;
    layout->addRow("用户名:", new QLineEdit);
    layout->addRow("密码:", new QLineEdit);
    setLayout(layout);
}

QFormLayoutWidget::~QFormLayoutWidget() {
}
```

### Qt 栅格布局管理器（QGridLayout）

```cpp
#pragma once
#include <QWidget>

void qgrid_layout();

class QGridLayoutWidget : public QWidget {
    Q_OBJECT
public:
    explicit QGridLayoutWidget(QWidget *parent = nullptr);
    ~QGridLayoutWidget();
};
```

```cpp
#include "qgrid_layout.h"

#include <QEventLoop>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

void qgrid_layout() {
    QGridLayoutWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &QGridLayoutWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

QGridLayoutWidget::QGridLayoutWidget(QWidget *parent) :
    QWidget(parent) {
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("姓名:"), 0, 0);
    layout->addWidget(new QLineEdit, 0, 1);
    layout->addWidget(new QLabel("年龄:"), 1, 0);
    layout->addWidget(new QLineEdit, 1, 1);
    setLayout(layout);
}

QGridLayoutWidget::~QGridLayoutWidget() {
}
```

### Qt 堆叠布局管理器（QStackLayout）

```cpp
#pragma once
#include <QWidget>

void qstack_layout();

class QStackLayoutWidget : public QWidget {
    Q_OBJECT
public:
    explicit QStackLayoutWidget(QWidget *parent = nullptr);
    ~QStackLayoutWidget();
};
```

```cpp
#include "qstack_layout.h"

#include <QEventLoop>
#include <QStackedLayout>
#include <QPushButton>
#include <QLabel>

void qstack_layout() {
    QStackLayoutWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &QStackLayoutWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

QStackLayoutWidget::QStackLayoutWidget(QWidget *parent) :
    QWidget(parent) {

    QPushButton *btn1 = new QPushButton("切换到页面1");
    QPushButton *btn2 = new QPushButton("切换到页面2");

    // 页面内容
    QWidget *page1 = new QWidget;
    page1->setStyleSheet("background-color: lightblue");
    page1->setLayout(new QVBoxLayout);
    page1->layout()->addWidget(new QLabel("这是页面1"));

    QWidget *page2 = new QWidget;
    page2->setStyleSheet("background-color: lightgreen");
    page2->setLayout(new QVBoxLayout);
    page2->layout()->addWidget(new QLabel("这是页面2"));

    // 核心：专门创建一个容器 Widget，并设置为 QStackedLayout 的载体
    QWidget *stackedContainer = new QWidget;
    QStackedLayout *stack = new QStackedLayout(stackedContainer);
    stack->addWidget(page1);
    stack->addWidget(page2);

    // 外层主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(btn1);
    mainLayout->addWidget(btn2);
    mainLayout->addWidget(stackedContainer); // ✅ 正确地添加 widget（而不是 layout）

    // 切换页面
    QObject::connect(btn1, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(0);
    });
    QObject::connect(btn2, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(1);
    });
}

QStackLayoutWidget::~QStackLayoutWidget() {
}
```



### QFormLayout 和 QGridLayout的区别

QFormLayout 和 QGridLayout 确实可以实现类似的布局效果，特别是在表单（label + editor）布局中，但它们的设计目的、使用便捷性和语义性不同。下面是详细的对比分析和应用建议：

| 对比项       | `QFormLayout`            | `QGridLayout`             |
| --------- | ------------------------ | ------------------------- |
| **用途定位**  | 表单布局（label + 控件）         | 通用网格布局                    |
| **语义明确**  | ✅ 是表单布局的语义化封装            | ❌ 没有语义，只是按格子排             |
| **使用简洁**  | ✅ 简洁直观，只需 addRow()       | ❌ 需显式设置行列坐标               |
| **适配性**   | ✅ 在调整窗口大小时自动对齐 label 和字段 | ⚠️ 需手动设置 alignment 和 span |
| **灵活性**   | ❌ 灵活性较低，只适合两列或一列布局       | ✅ 可随意设置行列跨越、复杂布局          |
| **视觉一致性** | ✅ 自动统一 label 宽度          | ❌ 不自动对齐，需要手动设置            |
| **推荐场景**  | 表单、设置页                   | 各种通用布局，如网格控件、复杂面板         |

### QStackedLayout 注意事项

QStackedLayout 是一个特殊的布局器，它必须直接应用于某个 QWidget（它内部控制显示哪个 widget），不能作为一个普通子布局来嵌入使用。否则，在运行时调用 stack->setCurrentIndex() 时，未正确绑定父控件，会导致 UI 崩溃或不可见控件访问异常。

| 错误点                              | 正确做法                                                      |
| -------------------------------- | --------------------------------------------------------- |
| 将 `QStackedLayout` 直接 add 到其他布局  | ❌ 不要直接添加 layout                                           |
| 没有为 `QStackedLayout` 提供所属 widget | ❌ 可能导致切换时崩溃                                               |
| 正确方式                             | ✅ 用一个 `QWidget` 容器作为 `QStackedLayout` 的宿主                 |
| 切换方式                             | `stack->setCurrentIndex(i)` 或 `stack->setCurrentWidget()` |



### 布局管理器中部件父对象问题

布局管理器设置到部件后，会自动成为其父布局，往布局里添加控件，控件的父对象会自动变为所属的容器部件

```cpp
QWidget *centralWidget = new QWidget(this);
QVBoxLayout *layout = new QVBoxLayout;
layout->addWidget(new QPushButton("按钮"));  // 按钮父对象将自动设置为 centralWidget
centralWidget->setLayout(layout);
```

### 布局中控件`QSizePolicy` 大小策略和`Stretch Factor`缩放因子 

| 常量                            | 描述                                                         |
| ------------------------------- | ------------------------------------------------------------ |
| `QSizePolicy::Fixed`            | 只能使用 `sizeHint()` 提供的值，**无法伸缩**。控件尺寸是固定的。 |
| `QSizePolicy::Minimum`          | 使用 `sizeHint()` 提供的大小作为**最小值**，控件**可以被拉伸**。 |
| `QSizePolicy::Maximum`          | 使用 `sizeHint()` 提供的大小作为**最大值**，控件**可以被压缩**。 |
| `QSizePolicy::Preferred`        | 使用 `sizeHint()` 提供的大小为理想值，**允许被拉伸或压缩**。 |
| `QSizePolicy::Expanding`        | 与 `Preferred` 类似，但表明控件**希望获得更多空间**（更倾向于拉伸）。 |
| `QSizePolicy::MinimumExpanding` | 使用 `sizeHint()` 作为最小值，并**倾向于获得额外空间**（类似 Expanding 但有底线）。 |
| `QSizePolicy::Ignored`          | 忽略 `sizeHint()` 的值，布局器可完全决定尺寸（用于极度灵活的控件）。 |

```
QPushButton *btn = new QPushButton("按钮");
// 水平拉伸（Expanding），垂直保持默认大小（Preferred）
QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Preferred);
btn->setSizePolicy(policy);
```

在 Qt 中，“**大小策略（QSizePolicy）**”和“**缩放因子（Stretch Factor）”** 是配合布局器一起使用、**影响控件大小分配**的两个重要概念，但它们的职责不同：

| 概念               | 作用                            | 控制方式                             |
| ------------------ | ------------------------------- | ------------------------------------ |
| **QSizePolicy**    | 控件是否可以拉伸/收缩（能不能） | `setSizePolicy()`                    |
| **Stretch Factor** | 控件分配多少比例空间（分多少）  | `layout->addWidget(widget, stretch)` |

当多个控件都允许拉伸时（如使用 `Expanding`），可以通过“**拉伸因子**”控制它们**各自占多少空间**：

```
layout->addWidget(widget1, 1);  // 占1份
layout->addWidget(widget2, 2);  // 占2份
// 窗口总空间剩余300像素时：widget1 得到 100 像素；widget2 得到 200 像素。
// 拉伸因子只在控件的大小策略允许“Expanding”或“Preferred”等拉伸的前提下才起作用。
```



### 布局大小约束属性

| 常量                            | 描述                                                         |
| ------------------------------- | ------------------------------------------------------------ |
| `QLayout::SetDefaultConstraint` | 主窗口大小设置为 `minimumSize()` 的值，除非部件已设置了最小大小。通常这是默认行为。 |
| `QLayout::SetFixedSize`         | 主窗口大小设置为 `sizeHint()` 的值，**无法改变大小**（窗口尺寸固定）。 |
| `QLayout::SetMinimumSize`       | 主窗口的**最小大小**设为 `minimumSize()` 的值，**无法再缩小**，但可以放大。 |
| `QLayout::SetMaximumSize`       | 主窗口的**最大大小**设为 `maximumSize()` 的值，**无法再放大**，但可以缩小。 |
| `QLayout::SetMinAndMaxSize`     | 主窗口的最小大小设为 `minimumSize()`，最大大小设为 `maximumSize()`，**窗口尺寸在范围内调整**。 |
| `QLayout::SetNoConstraint`      | **布局不对窗口大小作任何限制**，完全由外部控制窗口大小。     |

在你设置好布局（`QLayout`）之后，可调用以下函数来控制窗口大小行为

```cpp
layout->setSizeConstraint(QLayout::SetFixedSize);
```

### 布局小技巧

- 使用 `addStretch()` 增加弹性空间，让控件靠边或居中。
- 使用 `setSpacing()` 设置控件间距。
- 使用 `setContentsMargins()` 设置边距。
- 使用 `QSpacerItem` 自定义弹性空间。
