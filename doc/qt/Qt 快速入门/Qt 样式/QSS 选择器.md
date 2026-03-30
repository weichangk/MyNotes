# QSS 选择器

QSS（Qt Style Sheets）选择器用于指定样式规则作用于哪些控件，语法借鉴自 CSS，但针对 Qt 控件体系做了专门扩展。掌握选择器是编写高效 QSS 的核心。

---

## 1. 选择器类型总览

| 选择器类型 | 语法示例 | 说明 |
|---|---|---|
| 通用选择器 | `*` | 匹配所有控件 |
| 类型选择器 | `QPushButton` | 匹配该类及其**子类**的所有实例 |
| 类选择器 | `.QPushButton` | 仅匹配该类的实例，**不含子类** |
| ID 选择器 | `#myButton` | 匹配 `objectName` 为 `myButton` 的控件 |
| 属性选择器 | `QPushButton[flat="true"]` | 匹配具有指定属性值的控件 |
| 后代选择器 | `QDialog QPushButton` | 匹配 QDialog 内部（任意层级）的 QPushButton |
| 子选择器 | `QDialog > QPushButton` | 匹配 QDialog 的**直接子级** QPushButton |

---

## 2. 通用选择器（Universal Selector）

```css
* {
    font-family: "Microsoft YaHei";
    font-size: 14px;
}
```

- 匹配应用中**所有**控件
- 常用于设置全局默认字体、颜色等
- **性能影响大**，尽量减少使用，优先用类型选择器

---

## 3. 类型选择器（Type Selector）

```css
QPushButton {
    background-color: #3498db;
    color: white;
    border-radius: 4px;
    padding: 6px 16px;
}
```

**关键特性：类型选择器会匹配该类及其所有子类。**

```cpp
// 自定义按钮继承自 QPushButton
class MyButton : public QPushButton { Q_OBJECT };
```

```css
/* 以下规则同时作用于 QPushButton 和 MyButton */
QPushButton {
    background-color: #3498db;
}
```

### 常用类型选择器示例

```css
/* 标签 */
QLabel {
    color: #333333;
}

/* 输入框 */
QLineEdit {
    border: 1px solid #cccccc;
    border-radius: 3px;
    padding: 4px 8px;
}

/* 复选框 */
QCheckBox {
    spacing: 8px;
}

/* 组合框 */
QComboBox {
    border: 1px solid #aaaaaa;
    padding: 2px 8px;
}

/* 滚动条 */
QScrollBar:vertical {
    width: 10px;
    background: #f0f0f0;
}
```

---

## 4. 类选择器（Class Selector）

使用 `.ClassName` 语法，**仅匹配精确类型**，不包含子类。

```css
/* 仅匹配 QPushButton，不匹配 MyButton */
.QPushButton {
    background-color: #2ecc71;
}
```

### 类型选择器 vs 类选择器对比

```cpp
class PrimaryButton : public QPushButton { Q_OBJECT };
class DangerButton : public QPushButton { Q_OBJECT };
```

```css
/* 类型选择器：匹配 QPushButton、PrimaryButton、DangerButton */
QPushButton {
    padding: 8px 20px;
}

/* 类选择器：仅匹配 QPushButton 本身 */
.QPushButton {
    background-color: gray;
}
```

> **实际应用场景**：当你定义了多个子类按钮并分别设置样式时，用 `.QPushButton` 可以避免基类样式"泄漏"到子类。

---

## 5. ID 选择器（ID Selector）

通过控件的 `objectName` 属性进行匹配，语法为 `#objectName`。

```cpp
// C++ 中设置 objectName
QPushButton *btn = new QPushButton("确认");
btn->setObjectName("confirmBtn");

QLineEdit *edit = new QLineEdit();
edit->setObjectName("searchInput");
```

```css
/* 精确匹配 objectName 为 confirmBtn 的控件 */
#confirmBtn {
    background-color: #27ae60;
    color: white;
    font-weight: bold;
    min-width: 100px;
}

/* 匹配搜索输入框 */
#searchInput {
    border: 2px solid #3498db;
    border-radius: 15px;
    padding: 5px 15px;
}
```

### 组合使用：类型 + ID

```css
/* 更精确：只匹配 objectName 为 confirmBtn 的 QPushButton */
QPushButton#confirmBtn {
    background-color: #27ae60;
}
```

> **最佳实践**：为关键控件设置有意义的 `objectName`，方便用 ID 选择器精确定位，也有利于自动化测试。

---

## 6. 属性选择器（Property Selector）

根据 Qt 属性（Q_PROPERTY 或动态属性）的值来匹配控件。

### 6.1 基本语法

```css
/* 匹配 flat 属性为 true 的 QPushButton */
QPushButton[flat="true"] {
    border: none;
    background: transparent;
}
```

### 6.2 使用动态属性

动态属性是 QSS 中非常强大的技巧，通过 `setProperty()` 设置自定义属性，实现样式的动态切换。

```cpp
// C++ 中设置动态属性
QPushButton *btn = new QPushButton("操作");
btn->setProperty("btnType", "primary");

QPushButton *btn2 = new QPushButton("删除");
btn2->setProperty("btnType", "danger");

QLabel *label = new QLabel("错误信息");
label->setProperty("level", "error");
```

```css
/* 按属性值分别设置样式 */
QPushButton[btnType="primary"] {
    background-color: #3498db;
    color: white;
}

QPushButton[btnType="danger"] {
    background-color: #e74c3c;
    color: white;
}

QPushButton[btnType="success"] {
    background-color: #2ecc71;
    color: white;
}

QLabel[level="error"] {
    color: #e74c3c;
    font-weight: bold;
}

QLabel[level="warning"] {
    color: #f39c12;
}
```

### 6.3 动态切换样式

```cpp
// 运行时更改属性后需要刷新样式
btn->setProperty("btnType", "danger");
btn->style()->unpolish(btn);  // 移除旧样式
btn->style()->polish(btn);    // 应用新样式
// 或者更简便的方式：
btn->setStyleSheet(btn->styleSheet());
```

### 6.4 多属性组合

```css
/* 同时满足多个属性条件 */
QPushButton[flat="true"][btnType="primary"] {
    color: #3498db;
    background: transparent;
    border: none;
}
```

> **注意事项**：
> - 属性值用双引号包裹，且比较的是字符串形式
> - 属性选择器**不支持**通配符匹配
> - 动态属性更改后必须手动触发样式刷新

---

## 7. 后代选择器（Descendant Selector）

匹配某个父控件**任意层级**内的后代控件，用空格分隔。

```css
/* QGroupBox 内部所有层级的 QLabel */
QGroupBox QLabel {
    color: #2c3e50;
    font-size: 13px;
}

/* QTabWidget 内部所有 QPushButton */
QTabWidget QPushButton {
    min-height: 30px;
}
```

### 实际示例：设置对话框内的控件样式

```css
/* 对话框内的所有标签 */
QDialog QLabel {
    color: #555555;
}

/* 对话框内的所有输入框 */
QDialog QLineEdit {
    background-color: #fafafa;
    border: 1px solid #dddddd;
}

/* 对话框内的所有按钮 */
QDialog QPushButton {
    min-width: 80px;
    min-height: 32px;
}
```

### 多级后代

```css
/* QMainWindow → QDockWidget → QTreeView 内部的样式 */
QMainWindow QDockWidget QTreeView {
    background-color: #f5f5f5;
    border: none;
}
```

---

## 8. 子选择器（Child Selector）

使用 `>` 仅匹配**直接子级**控件，不递归到更深层级。

```css
/* 仅匹配 QFrame 直接子级的 QPushButton，不匹配更深层嵌套的 */
QFrame > QPushButton {
    margin: 5px;
}
```

### 后代选择器 vs 子选择器对比

```
QDialog
├── QVBoxLayout
│   ├── QLabel          ← "提示信息"
│   ├── QFrame
│   │   └── QLabel      ← "嵌套标签"
│   └── QPushButton     ← "确定"
```

```css
/* 后代选择器：匹配两个 QLabel */
QDialog QLabel {
    color: blue;
}

/* 子选择器：仅匹配 QDialog 直接包含的 QLabel */
/* 注意：在上述布局中，QLabel 的实际 parent 是 QFrame 或 QVBoxLayout 对应的 QWidget */
QDialog > QLabel {
    color: red;
}
```

> **重要提醒**：Qt 中布局管理器（QLayout）不是 QWidget，子选择器的父子关系基于 **QWidget 的 parent-child 层次**，而非视觉上的布局层次。

---

## 9. 选择器组合与复合使用

### 9.1 逗号分组（多个选择器共享同一规则）

```css
/* 多个控件共享样式 */
QPushButton, QToolButton, QComboBox {
    border: 1px solid #bbbbbb;
    border-radius: 3px;
    padding: 4px 12px;
    background-color: #ffffff;
}
```

### 9.2 复合选择器（叠加多种条件）

```css
/* 类型 + ID + 属性组合 */
QPushButton#submitBtn[enabled="true"] {
    background-color: #3498db;
    color: white;
}

/* 后代 + 类型 + 属性 */
QDialog QPushButton[btnType="primary"] {
    font-weight: bold;
}
```

### 9.3 完整主题示例

```css
/* === 全局基础 === */
QWidget {
    font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
    font-size: 13px;
    color: #333333;
}

/* === 按钮系列 === */
QPushButton {
    background-color: #ecf0f1;
    border: 1px solid #bdc3c7;
    border-radius: 4px;
    padding: 6px 20px;
    min-height: 28px;
}

QPushButton:hover {
    background-color: #d5dbdb;
}

QPushButton:pressed {
    background-color: #aeb6bf;
}

QPushButton[btnType="primary"] {
    background-color: #3498db;
    border-color: #2980b9;
    color: white;
}

QPushButton[btnType="primary"]:hover {
    background-color: #2980b9;
}

QPushButton[btnType="primary"]:pressed {
    background-color: #1f6dad;
}

QPushButton:disabled {
    background-color: #f0f0f0;
    color: #aaaaaa;
    border-color: #dddddd;
}

/* === 输入框 === */
QLineEdit, QTextEdit, QPlainTextEdit {
    border: 1px solid #cccccc;
    border-radius: 3px;
    padding: 4px 8px;
    background-color: white;
    selection-background-color: #3498db;
    selection-color: white;
}

QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
    border-color: #3498db;
}

QLineEdit:read-only {
    background-color: #f5f5f5;
}

/* === 侧边栏导航 === */
#sidebar QToolButton {
    border: none;
    border-radius: 0px;
    padding: 10px 20px;
    text-align: left;
    background: transparent;
}

#sidebar QToolButton:hover {
    background-color: rgba(52, 152, 219, 0.1);
}

#sidebar QToolButton:checked {
    background-color: rgba(52, 152, 219, 0.2);
    border-left: 3px solid #3498db;
}
```

---

## 10. 选择器优先级（Specificity）

QSS 的优先级规则与 CSS 类似，当多条规则冲突时，按**特异性**（Specificity）决定优先级。

### 优先级从高到低

| 优先级 | 选择器类型 | 示例 |
|---|---|---|
| 最高 | 内联样式（setStyleSheet） | `widget->setStyleSheet("color: red;")` |
| 高 | ID 选择器 | `#myBtn { }` |
| 中 | 属性选择器 | `QPushButton[flat="true"] { }` |
| 中 | 类/类型选择器 | `QPushButton { }` / `.QPushButton { }` |
| 低 | 通用选择器 | `* { }` |

### 特异性计算规则

特异性用 (a, b, c) 三元组计算：
- **a**：ID 选择器个数
- **b**：属性选择器 + 类选择器个数
- **c**：类型选择器 + 通用选择器个数

```css
QPushButton { }                          /* (0, 0, 1) */
QPushButton[flat="true"] { }             /* (0, 1, 1) */
#dialog QPushButton { }                  /* (1, 0, 1) */
#dialog QPushButton[flat="true"] { }     /* (1, 1, 1) */
```

### 特异性相同时

当特异性相同时，后定义的规则覆盖先定义的：

```css
QPushButton { color: red; }    /* 先定义 */
QPushButton { color: blue; }   /* 后定义 → 生效 */
```

### 层级覆盖关系

```css
/* 父控件直接设置的样式 vs 选择器匹配的样式 */
/* setStyleSheet 直接设置在控件上的样式优先级最高 */
```

```cpp
// 全局样式表
qApp->setStyleSheet("QPushButton { color: blue; }");

// 控件级样式表（优先级更高）
btn->setStyleSheet("color: red;");  // 最终 btn 文字为红色
```

---

## 11. 伪状态选择器（Pseudo-States）

伪状态用单冒号 `:` 表示，描述控件的交互状态。**伪状态是选择器的一部分**，不是独立的选择器类型。

### 11.1 通用伪状态

| 伪状态 | 说明 |
|---|---|
| `:hover` | 鼠标悬停 |
| `:pressed` | 鼠标按下 |
| `:focus` | 获取焦点 |
| `:disabled` | 控件禁用 |
| `:enabled` | 控件启用 |
| `:checked` | 选中状态（QCheckBox、QRadioButton、QToolButton） |
| `:unchecked` | 未选中 |
| `:indeterminate` | 半选状态（QCheckBox 三态） |
| `:on` | 等价于 `:checked` |
| `:off` | 等价于 `:unchecked` |
| `:read-only` | 只读状态 |
| `:editable` | QComboBox 可编辑时 |

### 11.2 专用伪状态

| 伪状态 | 适用控件 | 说明 |
|---|---|---|
| `:open` | QComboBox, QToolButton(menu) | 弹出菜单打开 |
| `:closed` | QComboBox, QToolButton(menu) | 弹出菜单关闭 |
| `:has-children` | QTreeView item | 有子节点 |
| `:!has-children` | QTreeView item | 无子节点 |
| `:alternate` | QAbstractItemView | 交替行 |
| `:first` | QTabBar tab | 第一个标签页 |
| `:last` | QTabBar tab | 最后一个标签页 |
| `:middle` | QTabBar tab | 中间标签页 |
| `:only-one` | QTabBar tab | 唯一标签页 |
| `:selected` | QTabBar tab, QMenu item | 选中项 |
| `:horizontal` | QScrollBar, QSlider | 水平方向 |
| `:vertical` | QScrollBar, QSlider | 垂直方向 |
| `:top`, `:bottom`, `:left`, `:right` | QTabWidget, QTabBar | 标签栏位置 |
| `:maximized` | QMdiSubWindow | 最大化 |
| `:minimized` | QMdiSubWindow | 最小化 |
| `:flat` | QPushButton | flat 模式 |

### 11.3 伪状态取反

用 `!` 前缀表示取反：

```css
/* 非悬停状态 */
QPushButton:!hover {
    background-color: #ecf0f1;
}

/* 非禁用状态 */
QLineEdit:!read-only {
    background-color: white;
}
```

### 11.4 伪状态组合

多个伪状态可以链式组合，表示同时满足：

```css
/* 悬停 + 选中 */
QCheckBox:hover:checked {
    color: #2980b9;
}

/* 鼠标按下 + 未选中 */
QRadioButton:pressed:!checked {
    color: #7f8c8d;
}

/* QComboBox 展开 + 悬停 */
QComboBox:editable:hover:open {
    border-color: #3498db;
}
```

### 11.5 完整按钮状态机示例

```css
QPushButton {
    background-color: #ecf0f1;
    border: 1px solid #bdc3c7;
    color: #2c3e50;
}

QPushButton:hover {
    background-color: #d5dbdb;
    border-color: #95a5a6;
}

QPushButton:pressed {
    background-color: #aeb6bf;
    border-color: #7f8c8d;
}

QPushButton:disabled {
    background-color: #f7f7f7;
    border-color: #eeeeee;
    color: #bbbbbb;
}

QPushButton:focus {
    border-color: #3498db;
    outline: none;
}

/* checked 用于 checkable 按钮 */
QPushButton:checked {
    background-color: #3498db;
    color: white;
    border-color: #2980b9;
}

QPushButton:checked:hover {
    background-color: #2980b9;
}
```

---

## 12. 子控件选择器（Sub-Controls）

子控件用双冒号 `::` 表示，用于定位复合控件的内部组成部分。

### 12.1 常用子控件

| 子控件 | 适用控件 | 说明 |
|---|---|---|
| `::drop-down` | QComboBox | 下拉箭头区域 |
| `::down-arrow` | QComboBox, QSpinBox, QScrollBar | 向下箭头图标 |
| `::up-arrow` | QSpinBox, QScrollBar | 向上箭头图标 |
| `::indicator` | QCheckBox, QRadioButton, QGroupBox, QMenu, QTreeView | 指示器（勾选框） |
| `::handle` | QScrollBar, QSlider, QSplitter | 拖拽手柄 |
| `::groove` | QSlider, QProgressBar | 滑槽/槽道 |
| `::chunk` | QProgressBar | 进度块 |
| `::tab` | QTabBar | 标签页 |
| `::pane` | QTabWidget | 内容面板 |
| `::tab-bar` | QTabWidget | 标签栏容器 |
| `::title` | QGroupBox, QDockWidget | 标题区域 |
| `::section` | QHeaderView | 表头列 |
| `::item` | QMenu, QMenuBar, QComboBox(弹出列表), QTreeView, QListView, QTableView | 列表项 |
| `::branch` | QTreeView | 分支展开/折叠图标 |
| `::menu-button` | QToolButton | 菜单按钮区域 |
| `::menu-indicator` | QPushButton | 菜单指示器 |
| `::tear` | QTabBar | 撕裂指示器 |
| `::close-button` | QTabBar tab, QDockWidget | 关闭按钮 |
| `::scroller` | QTabBar | 滚动箭头 |
| `::separator` | QMenu, QMainWindow | 分隔线 |
| `::right-arrow` | QMenu item | 子菜单箭头 |
| `::left-arrow` | QScrollBar | 左箭头 |
| `::right-arrow` | QScrollBar | 右箭头 |

### 12.2 QComboBox 子控件示例

```css
/* 主体 */
QComboBox {
    border: 1px solid #cccccc;
    border-radius: 3px;
    padding: 4px 8px;
    padding-right: 20px;  /* 为箭头留空间 */
    background: white;
}

/* 下拉按钮区域 */
QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: center right;
    width: 20px;
    border-left: 1px solid #cccccc;
    border-top-right-radius: 3px;
    border-bottom-right-radius: 3px;
}

/* 下拉箭头图标 */
QComboBox::down-arrow {
    image: url(:/icons/arrow_down.png);
    width: 12px;
    height: 12px;
}

/* 展开时箭头旋转 */
QComboBox::down-arrow:on {
    image: url(:/icons/arrow_up.png);
}

/* 弹出列表 */
QComboBox QAbstractItemView {
    border: 1px solid #cccccc;
    selection-background-color: #3498db;
    selection-color: white;
    background-color: white;
    outline: none;
}

/* 弹出列表中的项 */
QComboBox QAbstractItemView::item {
    height: 30px;
    padding-left: 10px;
}

QComboBox QAbstractItemView::item:hover {
    background-color: #ecf0f1;
}
```

### 12.3 QScrollBar 完整定制

```css
/* === 垂直滚动条 === */
QScrollBar:vertical {
    background: #f0f0f0;
    width: 12px;
    margin: 0;
    border-radius: 6px;
}

QScrollBar::handle:vertical {
    background: #c0c0c0;
    min-height: 30px;
    border-radius: 6px;
    margin: 2px;
}

QScrollBar::handle:vertical:hover {
    background: #a0a0a0;
}

QScrollBar::handle:vertical:pressed {
    background: #808080;
}

/* 隐藏上下箭头按钮 */
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical {
    height: 0px;
}

/* 隐藏顶部和底部空白区域 */
QScrollBar::add-page:vertical,
QScrollBar::sub-page:vertical {
    background: none;
}

/* === 水平滚动条 === */
QScrollBar:horizontal {
    background: #f0f0f0;
    height: 12px;
    margin: 0;
    border-radius: 6px;
}

QScrollBar::handle:horizontal {
    background: #c0c0c0;
    min-width: 30px;
    border-radius: 6px;
    margin: 2px;
}
```

### 12.4 QCheckBox / QRadioButton 指示器定制

```css
/* 复选框指示器 */
QCheckBox::indicator {
    width: 18px;
    height: 18px;
    border: 2px solid #bdc3c7;
    border-radius: 3px;
    background: white;
}

QCheckBox::indicator:hover {
    border-color: #3498db;
}

QCheckBox::indicator:checked {
    background-color: #3498db;
    border-color: #3498db;
    image: url(:/icons/check_white.png);
}

QCheckBox::indicator:indeterminate {
    background-color: #3498db;
    border-color: #3498db;
    image: url(:/icons/minus_white.png);
}

QCheckBox::indicator:disabled {
    background-color: #eeeeee;
    border-color: #dddddd;
}

/* 单选按钮指示器 */
QRadioButton::indicator {
    width: 18px;
    height: 18px;
    border: 2px solid #bdc3c7;
    border-radius: 10px;  /* 圆形 */
    background: white;
}

QRadioButton::indicator:checked {
    background-color: #3498db;
    border-color: #3498db;
    /* 内部白点效果需要用图片或 qradialgradient */
    background: qradialgradient(
        cx: 0.5, cy: 0.5, radius: 0.4,
        fx: 0.5, fy: 0.5,
        stop: 0 white,
        stop: 0.4 white,
        stop: 0.5 #3498db,
        stop: 1.0 #3498db
    );
}
```

### 12.5 QSlider 定制

```css
QSlider::groove:horizontal {
    border: none;
    height: 6px;
    background: #ecf0f1;
    border-radius: 3px;
}

QSlider::handle:horizontal {
    background: #3498db;
    border: none;
    width: 16px;
    height: 16px;
    margin: -5px 0;
    border-radius: 8px;
}

QSlider::handle:horizontal:hover {
    background: #2980b9;
    width: 18px;
    height: 18px;
    margin: -6px 0;
    border-radius: 9px;
}

QSlider::sub-page:horizontal {
    background: #3498db;
    border-radius: 3px;
}

QSlider::add-page:horizontal {
    background: #ecf0f1;
    border-radius: 3px;
}
```

### 12.6 子控件 + 伪状态组合

子控件可以与伪状态自由组合：

```css
/* QTabBar 标签页 */
QTabBar::tab {
    padding: 8px 20px;
    margin-right: 2px;
    background: #ecf0f1;
    border: 1px solid #cccccc;
    border-bottom: none;
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
}

QTabBar::tab:selected {
    background: white;
    border-bottom: 2px solid #3498db;
}

QTabBar::tab:hover:!selected {
    background: #d5dbdb;
}

QTabBar::tab:first:selected {
    margin-left: 0;
}

/* 关闭按钮 */
QTabBar::close-button {
    image: url(:/icons/close_gray.png);
    subcontrol-position: right;
}

QTabBar::close-button:hover {
    image: url(:/icons/close_red.png);
}
```

---

## 13. subcontrol-origin 与 subcontrol-position

子控件的位置通过这两个属性精确控制。

### subcontrol-origin

定义子控件的参考坐标系：

| 值 | 说明 |
|---|---|
| `margin` | 外边距外边缘 |
| `border` | 边框外边缘（默认） |
| `padding` | 内边距内边缘 |
| `content` | 内容区域 |

### subcontrol-position

定义子控件在参考区域中的位置，使用方位组合：

```css
QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: center right;  /* 垂直居中、水平靠右 */
}

QSpinBox::up-button {
    subcontrol-origin: border;
    subcontrol-position: top right;  /* 右上角 */
}

QSpinBox::down-button {
    subcontrol-origin: border;
    subcontrol-position: bottom right;  /* 右下角 */
}
```

---

## 14. 选择器调试技巧

### 14.1 使用背景色快速定位

```css
/* 调试时用醒目颜色确认选择器是否生效 */
QFrame#targetFrame {
    background-color: rgba(255, 0, 0, 0.3);  /* 半透明红色 */
    border: 2px dashed red;
}
```

### 14.2 查看控件层级

```cpp
// 打印控件的对象树，确认 parent-child 关系
qDebug() << widget->metaObject()->className()
         << widget->objectName()
         << "parent:" << widget->parentWidget()->objectName();

// 或者打印整棵控件树
QWidget *root = QApplication::activeWindow();
root->dumpObjectTree();
```

### 14.3 确认动态属性是否生效

```cpp
qDebug() << "btnType =" << btn->property("btnType").toString();
```

### 14.4 样式刷新

```cpp
// 方法 1：unpolish + polish
widget->style()->unpolish(widget);
widget->style()->polish(widget);

// 方法 2：重新设置样式表
widget->setStyleSheet(widget->styleSheet());

// 方法 3：对整个应用刷新
qApp->setStyleSheet(qApp->styleSheet());
```

---

## 15. 常见陷阱与最佳实践

### 陷阱 1：继承 QWidget 后样式不生效

自定义控件继承 `QWidget` 后，QSS 不生效。需要重写 `paintEvent`：

```cpp
class CustomWidget : public QWidget {
    Q_OBJECT
protected:
    void paintEvent(QPaintEvent *event) override {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }
};
```

### 陷阱 2：子选择器与布局管理器

```css
/* 这可能不起作用，因为 QLayout 不是 QWidget */
QGroupBox > QLabel { }  /* label 的 parent 可能是中间的 QWidget 容器 */
```

建议使用后代选择器或 ID 选择器替代。

### 陷阱 3：样式表覆盖

```css
/* 子控件的 setStyleSheet 会覆盖父级/全局样式表 */
/* 尽量在统一的全局样式表中管理，避免分散设置 */
```

### 最佳实践

1. **统一管理**：将所有 QSS 放在 `.qss` 文件中，通过 `QFile` 加载
2. **命名规范**：为关键控件设置语义化的 `objectName`
3. **善用动态属性**：用属性选择器替代多个子类
4. **避免通用选择器**：`*` 会影响性能，用类型选择器替代
5. **最小化内联样式**：减少 `setStyleSheet` 在代码中的使用
6. **注意继承关系**：类型选择器会匹配子类，需要时用类选择器 `.ClassName`

```cpp
// 推荐：从文件加载 QSS
QFile file(":/styles/app.qss");
if (file.open(QFile::ReadOnly | QFile::Text)) {
    qApp->setStyleSheet(file.readAll());
    file.close();
}
```

---

## 16. 选择器速查表

```
选择器语法                            匹配目标
────────────────────────────────────────────────────────
*                                    所有控件
QPushButton                          QPushButton 及其子类
.QPushButton                         仅 QPushButton 自身
QPushButton#okBtn                    objectName="okBtn" 的 QPushButton
QPushButton[flat="true"]             flat 属性为 true 的 QPushButton
QDialog QPushButton                  QDialog 内任意层级的 QPushButton
QDialog > QPushButton                QDialog 直接子级 QPushButton
QPushButton, QToolButton             QPushButton 和 QToolButton
QPushButton:hover                    悬停状态的 QPushButton
QPushButton:!enabled                 禁用状态的 QPushButton
QPushButton::menu-indicator          QPushButton 的菜单指示器子控件
QCheckBox::indicator:checked         选中状态的 QCheckBox 指示器
QTabBar::tab:first:selected          选中的第一个标签页
```
