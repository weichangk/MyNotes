在 Qt 里，MVVM（Model-View-ViewModel）并不像 WPF 那样是“官方强约束架构”，但 Qt 的：

- QObject 元对象系统
- Signal / Slot
- Property（Q_PROPERTY）
- Model/View 框架
- QML Binding

天然非常适合实现 MVVM。

Qt 中真正最接近 MVVM 的，其实是：

- C++ Backend（ViewModel + Model）
- QML（View）
- 属性绑定（Binding）
- Signal/Slot（通知机制）

------

# 一、先理解 Qt 中 MVVM 的角色

经典 MVVM：

| 层        | 作用              |
| --------- | ----------------- |
| Model     | 数据              |
| View      | UI                |
| ViewModel | UI状态 + 业务逻辑 |

Qt 对应：

| MVVM      | Qt 对应                      |
| --------- | ---------------------------- |
| Model     | QObject / QAbstractItemModel |
| View      | QML / QWidget                |
| ViewModel | QObject + Q_PROPERTY         |
| Binding   | QML 属性绑定                 |
| Command   | Signal/Slot                  |

------

# 二、Qt 为什么天然适合 MVVM

核心原因：

Qt 有“响应式属性系统”。

例如：

```
Text {
    text: vm.userName
}
```

这里：

```
vm.userName
```

不是简单 getter。

它背后：

- 有 Q_PROPERTY
- 有 notify signal
- 有 binding engine

一旦数据变化：

```
emit userNameChanged();
```

UI 自动刷新。

这就是 MVVM 的核心：

> 数据驱动 UI，而不是 UI 主动刷新。

------

# 三、Qt MVVM 的核心实现机制

------

# 1. QObject + Q_PROPERTY

这是 ViewModel 的基础。

例如：

```
class LoginViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString userName
               READ userName
               WRITE setUserName
               NOTIFY userNameChanged)

public:
    QString userName() const { return m_userName; }

    void setUserName(const QString& name)
    {
        if (m_userName == name)
            return;

        m_userName = name;
        emit userNameChanged();
    }

signals:
    void userNameChanged();

private:
    QString m_userName;
};
```

这里：

Q_PROPERTY 本质上把：

```
getter/setter/signal
```

注册到 Qt 元对象系统。

这样 QML 才能感知变化。

------

# 2. QML Binding（真正的 MVVM 核心）

例如：

```
TextField {
    text: vm.userName

    onTextChanged: {
        vm.userName = text
    }
}
```

这里形成了：

```
UI <-> ViewModel
```

双向绑定。

Qt 内部会：

- 追踪依赖
- 建立 binding graph
- 数据变化时重新求值

类似：

```
Reactive System
```

这和：

- Vue
- React Signals
- WPF Binding

本质类似。

------

# 3. notify signal

这是 Qt 实现“自动刷新”的关键。

例如：

```
emit userNameChanged();
```

QML engine 收到后：

```
重新计算依赖该属性的所有 binding
```

因此：

```
Text {
    text: vm.userName
}
```

自动刷新。

------

# 四、Qt Widgets 怎么做 MVVM

很多人误区：

> 只有 QML 才能 MVVM

其实 QWidget 也能。

只是没有自动 binding。

你需要：

- connect()
- 手动同步

例如：

```
connect(ui->lineEdit,
        &QLineEdit::textChanged,
        vm,
        &LoginViewModel::setUserName);

connect(vm,
        &LoginViewModel::userNameChanged,
        this,
        [=]{
            ui->label->setText(vm->userName());
        });
```

本质仍然是：

```
View <-> ViewModel
```

只是：

QWidget 没有 QML 那种 declarative binding。

------

# 五、Qt 官方 Model/View 与 MVVM 的关系

Qt 有自己的：

```
Model/View Architecture
```

例如：

- QListView
- QTableView
- QTreeView
- QAbstractItemModel

很多人误以为这是 MVVM。

其实：

这是 MVC/MVP 风格。

因为：

```
QAbstractItemModel
```

同时承担：

- 数据
- 部分业务逻辑

它不完全等于 ViewModel。

------

# 六、真正 Qt 企业项目里的 MVVM

实际工程通常：

```
QML UI
   ↓
ViewModel(QObject)
   ↓
Service
   ↓
Repository
   ↓
Data
```

例如：

```
LoginPage.qml
    ↓
LoginViewModel
    ↓
UserService
    ↓
HttpClient
```

------

# 七、Qt MVVM 最大优势

------

## 1. 天然响应式

QML Binding 非常强。

比 QWidget：

- 更少胶水代码
- 更少 connect
- 更少手动刷新

------

## 2. 解耦 UI

QML 不关心：

- 数据来源
- 网络
- DB

只关心：

```
text: vm.userName
```

------

## 3. 易测试

ViewModel 是纯 QObject：

可以单测。

无需 UI。

------

# 八、Qt MVVM 最大问题

------

## 1. QML/C++ 边界复杂

例如：

- ownership
- 生命周期
- 线程
- QVariant
- JS/C++ 转换

大型项目容易混乱。

------

## 2. Binding 循环

例如：

```
a = b
b = a
```

会：

```
binding loop detected
```

------

## 3. notify signal 漏发

如果：

```
emit xxxChanged();
```

忘记发：

UI 不更新。

这是经典问题。

------

# 九、面试回答（推荐版）

如果面试问：

“Qt 是如何实现 MVVM 的？”

你可以这样答：

------

Qt 本身没有严格定义 MVVM 框架，但 Qt 的 QObject、Q_PROPERTY、Signal/Slot、QML Binding 天然适合实现 MVVM。

在 Qt 中：

- View 通常是 QML 或 QWidget
- Model 是数据层
- ViewModel 一般继承 QObject

ViewModel 会通过：

```
Q_PROPERTY + notify signal
```

把数据暴露给 QML。

QML Binding 会自动监听属性变化：

```
text: vm.userName
```

当 ViewModel 发出：

```
emit userNameChanged()
```

QML 引擎会重新计算 binding，从而自动刷新 UI。

因此 Qt 的 MVVM 本质是：

```
元对象系统 + 属性系统 + 信号槽 + 响应式 binding
```

实现的数据驱动 UI。