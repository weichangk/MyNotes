Qt 的事件系统是其核心机制之一，支持图形界面响应用户操作（如鼠标点击、键盘输入）和系统事件（如窗口调整、定时器触发等）。Qt 的事件系统围绕 事件（QEvent） 和 事件处理机制 展开，包括事件的创建、投递、分发和处理。

### 事件系统的组成
- QEvent：事件基类
Qt 中所有事件的基类，不同事件类型通过 QEvent::Type 枚举来区分，如：
    - QEvent::MouseButtonPress（鼠标按下）
    - QEvent::KeyPress（键盘按下）
    - QEvent::Paint（绘制事件）
    - QEvent::Timer（定时器事件）

- 事件派生类（常用的有）
    - QMouseEvent：鼠标事件
    - QKeyEvent：键盘事件
    - QPaintEvent：重绘事件
    - QResizeEvent：窗口大小调整
    - QTimerEvent：定时器事
    - QCloseEvent：关闭窗口事件

### 事件的生命周期
- 事件产生
操作系统或 Qt 自身生成事件对象（如鼠标、键盘、定时器、绘制等）。
- 事件投递
Qt 通过 QCoreApplication::postEvent() 异步投递，或 QCoreApplication::sendEvent() 同步发送事件。
- 事件分发
Qt 会将事件分发给目标对象（一般是 QObject 的子类，如 QWidget）。
- 事件处理
对象通过从新实现事件处理函数、重新实现notifyz()函数、重新实现event()函数、向Application对象安装事件过滤器、向对象安装事件过滤器等方式处理事件


### 事件处理
#### 重新实现部件事件处理函数（事件处理虚函数）
这是最常见、最直接的方式，适合控件处理其专属事件。
```cpp
void MyWidget::mousePressEvent(QMouseEvent *event) override {
    qDebug() << "Mouse pressed!";
    QWidget::mousePressEvent(event);
}
```
常用的事件处理虚函数包括：
- mousePressEvent(QMouseEvent*)
- mouseMoveEvent(QMouseEvent*)
- keyPressEvent(QKeyEvent*)
- paintEvent(QPaintEvent*)
- resizeEvent(QResizeEvent*)
- closeEvent(QCloseEvent*)
特点：粒度细、类型安全、逻辑清晰，适合部件级的处理。

#### 重新实现 QApplication::notify() 函数
这是 Qt 事件系统最底层的钩子，适合全局事件监控。
```cpp
class MyApp : public QApplication {
public:
    using QApplication::QApplication;
    bool notify(QObject *receiver, QEvent *event) override {
        if (event->type() == QEvent::KeyPress)
            qDebug() << "全局键盘事件：" << receiver;
        return QApplication::notify(receiver, event);  // 保持默认派发
    }
};
```
特点：全局拦截，修改派发行为。但应谨慎使用，否则可能影响所有对象行为。

#### 向 QApplication 安装事件过滤器
适用于希望在应用层面监控所有对象的某类事件。
```cpp
class MyFilter : public QObject {
protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonPress)
            qDebug() << "全局鼠标事件：" << watched;
        return false;
    }
};

MyFilter *filter = new MyFilter;
qApp->installEventFilter(filter);  // 作用于整个应用
```
特点：非侵入式全局事件监听。可组合多个过滤器，易于解耦。

#### 重新实现 QObject::event() 函数
这是 Qt 所有事件的分发入口，适合统一拦截和分发对象的所有事件类型。
```cpp
bool MyWidget::event(QEvent *event) {
    if (event->type() == QEvent::Enter)
        qDebug() << "鼠标进入";
    return QWidget::event(event);  // 调用默认分发逻辑
}
```
特点：集中处理所有事件类型，适用于控件统一管理多个事件。

#### 在某个对象上安装事件过滤器
和向 QApplication 安装事件过滤器不同之处在于：只作用于特定对象，而非全局。
```cpp
QPushButton *btn = new QPushButton("按钮");
btn->installEventFilter(filter);  // 只监听按钮的事件
```
特点：局部事件拦截，适合在运行时动态控制行为（如拦截某个按钮的事件）。

#### 时间处理方式对比
| 序号 | 方式                        | 作用范围 | 特点          | 用途场景           |
| -- | ------------------------- | ---- | ----------- | -------------- |
| 1  | 重写事件处理函数                  | 控件内部 | 类型安全、代码清晰   | 正常处理控件交互事件     |
| 2  | 重写 `QApplication::notify` | 整个应用 | 最底层、可拦截所有事件 | 全局日志、行为分析等     |
| 3  | 向 `QApplication` 安装过滤器    | 整个应用 | 非侵入式全局监听    | 多模块共享的全局行为监听   |
| 4  | 重写 `event()`              | 控件内部 | 所有事件集中入口，灵活 | 统一处理不同事件       |
| 5  | 安装对象事件过滤器                 | 指定对象 | 局部拦截，动态管理   | 控件行为修改、鼠标行为追踪等 |

### 事件的传递
#### 事件传递的整体流程（Event Delivery）
总流程（简化版）：
```css
[事件源]（用户操作/系统触发）
    ↓
[Qt 事件队列]
    ↓
QApplication::notify()
    ↓
事件过滤器（全局或对象级）
    ↓
QObject::event()
    ↓
具体事件处理函数（如 mousePressEvent）
```

#### 事件的派发逻辑（以鼠标事件为例）
场景：用户点击了一个按钮控件
1. 操作系统产生鼠标事件（通过 OS 原生事件系统）
2. Qt 捕获后生成 QMouseEvent 对象
3. Qt 找到鼠标下的最前端控件（childAt）
4. 将事件派发给该控件（child widget）
5. 按如下顺序处理：

```text
事件过滤器（应用级） → 事件过滤器（对象级）
→ QObject::event() → QWidget::mousePressEvent()
```

如果事件未被处理或处理返回 false，可能：
- 向父对象传递（视事件类型而定，如焦点事件）
- 最终被忽略

#### 事件传播机制（特别注意）
事件传播机制分为两种情况：
| 类型           | 是否向上传递                       | 说明           |
| ------------ | ---------------------------- | ------------ |
| 输入事件（如鼠标、键盘） | ❌ 不会自动向父控件传递（除非显式转发）         | 鼠标只作用于最上层子控件 |
| 特殊事件（如焦点、状态） | ✅ 会向上传递（focus、changeEvent 等） | 由内部控件向外传播    |
手动传递事件给父控件
```cpp
// 子控件主动将事件传给父控件
QWidget::mousePressEvent(event); // 调用父类默认处理
```

#### 事件分发中的常见函数说明
| 函数/方法                  | 所属类              | 作用              |
| ---------------------- | ---------------- | --------------- |
| `sendEvent()`          | QCoreApplication | 同步分发，立即处理事件     |
| `postEvent()`          | QCoreApplication | 异步分发，放入事件队列     |
| `notify()`             | QApplication     | 事件分发总控，可重写拦截    |
| `event()`              | QObject          | 所有事件进入点，可重写统一处理 |
| `eventFilter()`        | QObject          | 事件过滤器入口函数       |
| `installEventFilter()` | QObject          | 安装事件过滤器         |

#### 图解 Qt 事件传递流程
```css
 ┌────────────┐       ┌────────────────┐
 │  操作系统   ├─────►│ QEvent 创建      │
 └────────────┘       └────────────────┘
                             │
                             ▼
                    ┌─────────────────────┐
                    │ QCoreApplication::  │
                    │  postEvent/sendEvent│
                    └─────────────────────┘
                             │
                             ▼
                    ┌─────────────────────┐
                    │ QApplication::notify│◄────────┐
                    └─────────────────────┘         │
                             │                      │（可重写拦截）
                             ▼                      │
 ┌──────────────────────────────┐                   │
 │ 检查事件过滤器（install 的）     │◄────────────┐     │
 └──────────────────────────────┘             │       │
                             │                │       │
                             ▼                │       │
                   ┌────────────────────┐     │       │
                   │ QObject::event()   │─────┘       │
                   └────────────────────┘             │
                             │                        │   
                             ▼                        │
           ┌──────────────────────────────┐           │
           │ 派发给具体处理函数，如：         │           │
           │ mousePressEvent/paintEvent等 │───────────┘
           └──────────────────────────────┘
```

#### 常见问题及建议
| 问题       | 原因         | 建议                                               |
| -------- | ---------- | ------------------------------------------------ |
| 鼠标事件无响应  | 控件没有启用鼠标响应 | 确保 `setMouseTracking(true)` 或 `setEnabled(true)` |
| 子控件吃掉事件  | 没调用基类方法    | 调用 `QWidget::mousePressEvent(event)` 保留默认行为      |
| 想全局监听事件  | 不清楚事件传递入口  | 使用 `installEventFilter(qApp)` 或重写 `notify()`     |
| 不希望事件被处理 | 想阻止后续处理    | 返回 `true` 表示“已处理”，事件将停止传递                        |
