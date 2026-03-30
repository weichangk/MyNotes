# QSS 基础语法

QSS（Qt Style Sheets）是 Qt 提供的一套基于 CSS 2.1 语法的样式机制，用于自定义控件外观。它将 UI 表现与业务逻辑分离，支持运行时动态换肤。

---

## 1. QSS 规则结构

一条 QSS 规则由**选择器**和**声明块**组成：

```
选择器 {
    属性: 值;
    属性: 值;
}
```

```css
QPushButton {
    background-color: #3498db;
    color: white;
    border-radius: 4px;
    padding: 6px 16px;
}
```

- **选择器**：指定规则作用的控件（详见 QSS 选择器文档）
- **声明块**：花括号 `{}` 内的一组属性-值对
- 每条声明以分号 `;` 结尾（最后一条可省略，但建议保留）
- 属性名不区分大小写，值可能区分

---

## 2. 设置样式表的方式

### 2.1 代码中设置

```cpp
// 方式 1：对单个控件设置
QPushButton *btn = new QPushButton("确认");
btn->setStyleSheet("background-color: #3498db; color: white;");

// 方式 2：对容器设置（影响所有子控件）
QWidget *panel = new QWidget;
panel->setStyleSheet("QPushButton { color: white; } QLabel { color: #333; }");

// 方式 3：对整个应用设置（全局样式）
qApp->setStyleSheet("QToolTip { color: white; background: #333; border: 1px solid #555; }");
```

### 2.2 从文件加载（推荐）

```cpp
// 从 qrc 资源文件加载
QFile file(":/styles/app.qss");
if (file.open(QFile::ReadOnly | QFile::Text)) {
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);
    file.close();
}
```

```cpp
// 从本地文件加载（方便开发调试，无需重新编译）
QFile file("styles/app.qss");
if (file.open(QFile::ReadOnly | QFile::Text)) {
    qApp->setStyleSheet(file.readAll());
    file.close();
}
```

### 2.3 Qt Designer 中设置

在 Qt Designer 中右键控件 → "Change styleSheet..."，可直接编辑样式表并实时预览。

### 2.4 样式表作用域与层叠

```
qApp->setStyleSheet(...)       → 全局，优先级最低
  parentWidget->setStyleSheet(...)  → 容器级
    widget->setStyleSheet(...)        → 控件级，优先级最高
```

当多个层级存在冲突规则时，**更具体的作用域优先**。控件自身的 `setStyleSheet` 覆盖父级和全局样式。

---

## 3. 注释

QSS 仅支持 `/* */` 块注释，**不支持** `//` 行注释：

```css
/* 这是合法注释 */
QPushButton {
    /* 主色调按钮 */
    background-color: #3498db;
    color: white;  /* 文字颜色 */
}

/* 以下是错误的写法：
// 这不是合法注释，会导致解析错误
*/
```

---

## 4. 颜色值

### 4.1 颜色表示方式

```css
QLabel {
    /* 命名颜色 */
    color: red;
    color: darkblue;
    color: transparent;

    /* 十六进制 */
    color: #ff0000;         /* 6 位 */
    color: #f00;            /* 3 位简写 */

    /* RGB 函数 */
    color: rgb(255, 0, 0);

    /* RGBA（带透明度，0~255） */
    color: rgba(255, 0, 0, 128);    /* 半透明红色 */

    /* HSV 函数 */
    color: hsv(0, 255, 255);        /* 纯红 */
    color: hsva(0, 255, 255, 128);  /* 半透明纯红 */

    /* HSL 函数 */
    color: hsl(0, 100%, 50%);       /* 纯红 */
    color: hsla(0, 100%, 50%, 50%); /* 半透明纯红 */
}
```

### 4.2 常用命名颜色

Qt 支持 SVG 1.0 中定义的所有命名颜色：

```
white, black, red, green, blue, yellow, cyan, magenta,
gray, darkGray, lightGray, transparent,
darkRed, darkGreen, darkBlue, darkCyan, darkMagenta, darkYellow
```

### 4.3 颜色应用属性

```css
QWidget {
    color: #333333;                     /* 前景色（文字） */
    background-color: #ffffff;          /* 背景色 */
    border-color: #cccccc;              /* 边框颜色 */
    selection-color: white;             /* 选中文字颜色 */
    selection-background-color: #3498db;/* 选中背景色 */
    alternate-background-color: #f5f5f5;/* 交替行背景（QTreeView 等） */
    gridline-color: #e0e0e0;           /* 网格线颜色（QTableView） */
}
```

---

## 5. 渐变（Gradient）

QSS 支持三种渐变类型，语法与 CSS 有所不同。

### 5.1 线性渐变（qlineargradient）

```css
QPushButton {
    background: qlineargradient(
        x1: 0, y1: 0,        /* 起点（左上角） */
        x2: 0, y2: 1,        /* 终点（左下角），即从上到下 */
        stop: 0 #3498db,     /* 起始颜色 */
        stop: 0.5 #2980b9,   /* 中间颜色 */
        stop: 1 #1f6dad      /* 结束颜色 */
    );
}
```

坐标系说明：
- `x1, y1`：起点坐标，取值 0~1（相对于控件区域）
- `x2, y2`：终点坐标，取值 0~1
- `(0,0)→(0,1)` 从上到下，`(0,0)→(1,0)` 从左到右，`(0,0)→(1,1)` 对角线

```css
/* 从左到右的渐变 */
QProgressBar::chunk {
    background: qlineargradient(
        x1: 0, y1: 0, x2: 1, y2: 0,
        stop: 0 #2ecc71,
        stop: 1 #27ae60
    );
}

/* 对角线渐变 */
QFrame#banner {
    background: qlineargradient(
        x1: 0, y1: 0, x2: 1, y2: 1,
        stop: 0 #667eea,
        stop: 1 #764ba2
    );
}
```

### 5.2 径向渐变（qradialgradient）

```css
QRadioButton::indicator:checked {
    background: qradialgradient(
        cx: 0.5, cy: 0.5,   /* 圆心 */
        radius: 0.5,         /* 半径 */
        fx: 0.5, fy: 0.5,   /* 焦点 */
        stop: 0 white,       /* 中心颜色 */
        stop: 0.4 white,
        stop: 0.5 #3498db,
        stop: 1.0 #3498db   /* 边缘颜色 */
    );
}
```

### 5.3 锥形渐变（qconicalgradient）

```css
QDial {
    background: qconicalgradient(
        cx: 0.5, cy: 0.5,   /* 圆心 */
        angle: 0,            /* 起始角度（度） */
        stop: 0 red,
        stop: 0.17 yellow,
        stop: 0.33 green,
        stop: 0.50 cyan,
        stop: 0.67 blue,
        stop: 0.83 magenta,
        stop: 1.0 red
    );
}
```

---

## 6. 字体属性

```css
QWidget {
    /* 字体族 —— 多个备选用逗号分隔 */
    font-family: "Microsoft YaHei", "Segoe UI", sans-serif;

    /* 字号 */
    font-size: 14px;       /* 像素 */
    font-size: 10pt;       /* 磅 */

    /* 粗细 */
    font-weight: bold;     /* 或 normal, light, 100~900 */

    /* 风格 */
    font-style: italic;    /* 或 normal, oblique */

    /* 简写形式 */
    font: bold italic 14px "Microsoft YaHei";
}
```

### font 简写语法

```
font: [style] [weight] size family;
```

```css
QLabel#title {
    font: bold 18px "Microsoft YaHei";
}

QLabel#subtitle {
    font: italic 13px "Segoe UI";
}
```

> **注意**：QSS 中不支持 `font-variant` 和 `line-height`。

---

## 7. 盒模型（Box Model）

QSS 的盒模型与 CSS 一致，由外到内分为四层：

```
┌─────────────────────────────── margin ──┐
│ ┌─────────────────────────── border ──┐ │
│ │ ┌─────────────────────── padding ─┐ │ │
│ │ │                                 │ │ │
│ │ │         content 内容            │ │ │
│ │ │                                 │ │ │
│ │ └─────────────────────────────────┘ │ │
│ └─────────────────────────────────────┘ │
└─────────────────────────────────────────┘
```

### 7.1 margin（外边距）

控件与外部其他元素之间的距离：

```css
QPushButton {
    margin: 5px;                /* 四边相同 */
    margin: 5px 10px;           /* 上下 5px，左右 10px */
    margin: 5px 10px 5px 10px;  /* 上 右 下 左 */
    margin-top: 5px;            /* 单独设置 */
    margin-right: 10px;
    margin-bottom: 5px;
    margin-left: 10px;
}
```

### 7.2 border（边框）

```css
QPushButton {
    /* 简写形式 */
    border: 1px solid #cccccc;

    /* 分开设置 */
    border-width: 1px;
    border-style: solid;       /* solid, dashed, dotted, double, groove, ridge, inset, outset, none */
    border-color: #cccccc;

    /* 单边设置 */
    border-top: 2px solid #3498db;
    border-left: none;
    border-right: 1px dashed #aaa;
    border-bottom: 1px solid #cccccc;

    /* 单边分属性设置 */
    border-top-width: 2px;
    border-top-style: solid;
    border-top-color: #3498db;

    /* 圆角 */
    border-radius: 4px;                 /* 四角相同 */
    border-top-left-radius: 8px;        /* 单角设置 */
    border-top-right-radius: 8px;
    border-bottom-left-radius: 0px;
    border-bottom-right-radius: 0px;
}
```

### 7.3 padding（内边距）

内容与边框之间的距离：

```css
QPushButton {
    padding: 6px 16px;          /* 上下 6px，左右 16px */
    padding: 8px;               /* 四边相同 */
    padding-top: 10px;          /* 单独设置 */
    padding-left: 20px;
}
```

### 7.4 盒模型完整示例

```css
QGroupBox {
    margin: 10px;                /* 与外部间距 */
    border: 2px solid #bdc3c7;  /* 边框 */
    border-radius: 6px;         /* 圆角 */
    padding: 15px 10px;         /* 内容与边框间距 */
    margin-top: 20px;           /* 为标题腾出空间 */
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    padding: 0 8px;
    color: #2c3e50;
    font-weight: bold;
}
```

---

## 8. 背景属性

### 8.1 背景色

```css
QWidget {
    background-color: #f5f5f5;
    background-color: transparent;       /* 透明 */
    background-color: rgba(0, 0, 0, 50); /* 半透明黑 */
}
```

### 8.2 背景图片

```css
QPushButton#settingsBtn {
    /* 基本用法 */
    background-image: url(:/icons/settings.png);
    background-repeat: no-repeat;        /* repeat, repeat-x, repeat-y, no-repeat */
    background-position: center;         /* top, bottom, left, right, center 或组合 */

    /* 也可用 border-image 来拉伸图片填满控件 */
    border-image: url(:/images/btn_bg.png);
}
```

### 8.3 background 简写

```css
QFrame#header {
    background: #2c3e50 url(:/images/pattern.png) repeat-x top left;
}
```

### 8.4 border-image（九宫格拉伸）

`border-image` 将图片分为 9 个区域进行智能拉伸，适合制作可缩放按钮和面板背景：

```css
QPushButton {
    /* border-image: url(path) top right bottom left; */
    border-image: url(:/images/button.png) 4 4 4 4 stretch stretch;
    border-width: 4px;
}
```

四个数值定义切割边距（像素），将图片划分为九宫格：
- 四角保持原样不拉伸
- 四边按方向拉伸
- 中心区域双向拉伸

```
┌────┬──────────┬────┐
│ 角 │   边     │ 角 │
├────┼──────────┼────┤
│    │          │    │
│ 边 │   中心   │ 边 │
│    │          │    │
├────┼──────────┼────┤
│ 角 │   边     │ 角 │
└────┴──────────┴────┘
```

---

## 9. 图片与图标

### 9.1 image 属性

`image` 属性用于替换子控件的绘制内容（如箭头、指示器）：

```css
QComboBox::down-arrow {
    image: url(:/icons/arrow_down.png);
    width: 12px;
    height: 12px;
}

QCheckBox::indicator:checked {
    image: url(:/icons/check.png);
}

QTreeView::branch:open:has-children {
    image: url(:/icons/branch_open.png);
}
```

### 9.2 icon 相关属性

```css
QPushButton {
    /* QSS 中设置图标 */
    qproperty-icon: url(:/icons/save.png);
    qproperty-iconSize: 16px 16px;
}
```

### 9.3 URL 路径格式

```css
/* Qt 资源系统（推荐） */
image: url(:/icons/arrow.png);

/* 相对路径（相对于应用工作目录） */
image: url(icons/arrow.png);

/* 绝对路径（不推荐，不可移植） */
image: url(C:/app/icons/arrow.png);
```

> **最佳实践**：始终使用 Qt 资源系统 `:/` 前缀，确保跨平台和打包后路径正确。

---

## 10. 尺寸属性

```css
QPushButton {
    /* 固定尺寸 */
    width: 100px;
    height: 36px;

    /* 最小/最大尺寸（更常用） */
    min-width: 80px;
    min-height: 32px;
    max-width: 200px;
    max-height: 50px;
}
```

### 支持的长度单位

| 单位 | 说明 |
|---|---|
| `px` | 像素（默认，可省略） |
| `pt` | 磅（1pt = 1/72 英寸） |
| `em` | 相对于当前字体大小 |
| `ex` | 相对于字体 x 高度 |

```css
QLabel {
    font-size: 14px;
    padding: 0.5em;    /* = 7px */
    margin: 2pt;
}
```

> **注意**：QSS 不支持 `%`、`vh`、`vw` 等 CSS3 单位。建议统一使用 `px`。

---

## 11. 文本属性

```css
QLabel {
    /* 文本对齐（仅部分控件支持） */
    qproperty-alignment: AlignCenter;     /* AlignLeft, AlignRight, AlignHCenter, AlignTop, AlignBottom, AlignVCenter */

    /* 文本装饰（不直接支持 text-decoration） */
    /* 可通过 Qt 代码或富文本实现下划线、删除线 */

    /* 文本缩进 */
    text-indent: 20px;     /* 首行缩进（仅 QTextEdit 等支持） */
}

QLineEdit {
    /* 占位文字颜色 */
    /* 需要通过伪元素或代码设置 */
    color: #333333;
}
```

### 文本省略

QSS 本身不支持 `text-overflow`，需在代码中配合 `QFontMetrics::elidedText` 使用。

---

## 12. 通过 QSS 访问 Qt 属性（qproperty）

QSS 可以通过 `qproperty-<name>` 语法设置控件的 Q_PROPERTY 属性，这是 QSS 独有的强大能力。

```css
/* 设置 QLabel 的文本 */
QLabel#welcomeLabel {
    qproperty-text: "欢迎使用";
}

/* 设置 QLabel 对齐方式 */
QLabel {
    qproperty-alignment: 'AlignCenter';
}

/* 设置 QAbstractButton 的图标 */
QPushButton#saveBtn {
    qproperty-icon: url(:/icons/save.png);
    qproperty-iconSize: 16px 16px;
}

/* 设置 QFrame 的阴影和形状 */
QFrame#separator {
    qproperty-frameShape: HLine;
    qproperty-frameShadow: Sunken;
}

/* 设置间距 */
QCheckBox {
    qproperty-spacing: 8;
}

/* 设置 QSpinBox 范围 */
QSpinBox#ageInput {
    qproperty-minimum: 0;
    qproperty-maximum: 150;
}
```

> **限制**：`qproperty-` 只在样式表首次应用时生效，后续修改不会触发属性变更信号。

---

## 13. 样式表中的特殊值

### 13.1 none

```css
QPushButton {
    border: none;          /* 无边框 */
    background: none;      /* 无背景 */
    outline: none;         /* 无焦点虚线框 */
}
```

### 13.2 transparent

```css
QToolButton {
    background-color: transparent;  /* 全透明背景 */
    border: 1px solid transparent;  /* 透明边框（保留空间，hover 时切换为可见） */
}

QToolButton:hover {
    border: 1px solid #3498db;      /* 悬停时显示边框 */
}
```

### 13.3 inherit

QSS **不支持** CSS 的 `inherit` 关键字。子控件的属性不会自动继承父控件的 QSS 设置（`color` 和 `font` 在部分场景例外）。

---

## 14. 样式表加载与热重载

### 14.1 基本加载

```cpp
void loadStyleSheet(const QString &path) {
    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }
}
```

### 14.2 开发期热重载

通过文件监视实现样式的实时修改预览，无需重启应用：

```cpp
#include <QFileSystemWatcher>

// 监视 QSS 文件变化，自动重新加载
QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
watcher->addPath("styles/app.qss");

connect(watcher, &QFileSystemWatcher::fileChanged, this, [](const QString &path) {
    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }
    qDebug() << "样式表已重新加载:" << path;
});
```

### 14.3 多主题切换

```cpp
void ThemeManager::switchTheme(const QString &themeName) {
    QString path = QString(":/styles/%1.qss").arg(themeName);
    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }
}

// 调用
themeManager->switchTheme("dark");
themeManager->switchTheme("light");
```

---

## 15. 样式刷新机制

当动态属性或控件状态改变后，需要手动触发样式刷新：

```cpp
// 方法 1：unpolish + polish（推荐）
widget->style()->unpolish(widget);
widget->style()->polish(widget);

// 方法 2：重新设置样式表
widget->setStyleSheet(widget->styleSheet());

// 方法 3：全局刷新（开销较大）
qApp->setStyleSheet(qApp->styleSheet());

// 方法 4：更新控件（触发重绘）
widget->update();
```

### 动态属性 + 样式刷新实战

```cpp
// 设置按钮状态属性
btn->setProperty("state", "loading");
btn->style()->unpolish(btn);
btn->style()->polish(btn);

// 对应 QSS
QPushButton[state="loading"] {
    background-color: #95a5a6;
    color: #ecf0f1;
}

QPushButton[state="success"] {
    background-color: #27ae60;
    color: white;
}

QPushButton[state="error"] {
    background-color: #e74c3c;
    color: white;
}
```

---

## 16. QSS 与 QPalette 的关系

- QSS 设置后会 **覆盖** QPalette 的颜色设置
- 如果控件设置了 QSS，`QPalette::color()` 仍返回原始值，实际渲染使用 QSS 的值
- 想要移除 QSS 恢复 QPalette 效果：`widget->setStyleSheet("")`

```cpp
// QPalette 设置（会被 QSS 覆盖）
QPalette pal = btn->palette();
pal.setColor(QPalette::Button, Qt::red);
btn->setPalette(pal);

// 如果同时存在 QSS，QPalette 设置无效
btn->setStyleSheet("background-color: blue;");  // 最终为蓝色
```

---

## 17. 常用属性速查表

### 背景与颜色

| 属性 | 说明 | 示例值 |
|---|---|---|
| `color` | 前景色（文字） | `#333`, `white` |
| `background-color` | 背景色 | `#fff`, `transparent` |
| `background` | 背景简写 | `#fff url(bg.png) no-repeat` |
| `background-image` | 背景图 | `url(:/img/bg.png)` |
| `background-repeat` | 背景重复 | `no-repeat`, `repeat-x` |
| `background-position` | 背景位置 | `center`, `top left` |
| `background-attachment` | 背景附着 | `scroll`, `fixed` |
| `selection-color` | 选中文字色 | `white` |
| `selection-background-color` | 选中背景色 | `#3498db` |
| `alternate-background-color` | 交替行背景 | `#f5f5f5` |
| `gridline-color` | 表格网格线色 | `#e0e0e0` |

### 边框

| 属性 | 说明 | 示例值 |
|---|---|---|
| `border` | 边框简写 | `1px solid #ccc` |
| `border-width` | 边框宽度 | `1px` |
| `border-style` | 边框样式 | `solid`, `dashed`, `dotted`, `none` |
| `border-color` | 边框颜色 | `#ccc` |
| `border-radius` | 圆角半径 | `4px` |
| `border-image` | 九宫格图 | `url(bg.png) 4 4 4 4 stretch` |
| `border-top` | 上边框 | `2px solid #3498db` |
| `outline` | 焦点轮廓 | `none` |

### 字体

| 属性 | 说明 | 示例值 |
|---|---|---|
| `font` | 字体简写 | `bold 14px "Segoe UI"` |
| `font-family` | 字体族 | `"Microsoft YaHei"` |
| `font-size` | 字号 | `14px`, `10pt` |
| `font-weight` | 粗细 | `bold`, `normal`, `600` |
| `font-style` | 风格 | `italic`, `normal` |

### 间距与尺寸

| 属性 | 说明 | 示例值 |
|---|---|---|
| `margin` | 外边距 | `5px`, `5px 10px` |
| `padding` | 内边距 | `6px 16px` |
| `width` | 宽度 | `100px` |
| `height` | 高度 | `36px` |
| `min-width` | 最小宽度 | `80px` |
| `min-height` | 最小高度 | `32px` |
| `max-width` | 最大宽度 | `200px` |
| `max-height` | 最大高度 | `50px` |
| `spacing` | 控件间距 | `8px` |

### 图片与图标

| 属性 | 说明 | 示例值 |
|---|---|---|
| `image` | 子控件图标 | `url(:/icons/arrow.png)` |
| `image-position` | 图标位置 | `left center` |
| `icon-size` | 图标大小 | `16px 16px` |

### 定位（子控件）

| 属性 | 说明 | 示例值 |
|---|---|---|
| `subcontrol-origin` | 参考坐标系 | `padding`, `border`, `content` |
| `subcontrol-position` | 子控件位置 | `center right`, `top left` |
| `position` | 定位模式 | `relative`, `absolute` |
| `top`, `right`, `bottom`, `left` | 偏移值 | `2px` |

### 其他

| 属性 | 说明 | 示例值 |
|---|---|---|
| `opacity` | 透明度（仅 QToolTip）| `200`（0~255） |
| `text-align` | 文本对齐 | `left`, `center` |
| `text-decoration` | 文本装饰 | `underline`, `none` |
| `show-decoration-selected` | 选中装饰 | `1` |
| `-qt-style-features` | 样式特性 | `background-color` |

---

## 18. 完整示例：暗色主题

```css
/* ===== 全局基础 ===== */
QWidget {
    font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
    font-size: 13px;
    color: #d4d4d4;
    background-color: #1e1e1e;
}

/* ===== 按钮 ===== */
QPushButton {
    background-color: #3c3c3c;
    border: 1px solid #555555;
    border-radius: 4px;
    padding: 6px 20px;
    min-height: 28px;
    color: #d4d4d4;
}

QPushButton:hover {
    background-color: #505050;
    border-color: #777777;
}

QPushButton:pressed {
    background-color: #2a2a2a;
}

QPushButton:disabled {
    background-color: #2a2a2a;
    color: #666666;
    border-color: #3c3c3c;
}

QPushButton[btnType="primary"] {
    background-color: #0e639c;
    border-color: #1177bb;
    color: white;
}

QPushButton[btnType="primary"]:hover {
    background-color: #1177bb;
}

/* ===== 输入框 ===== */
QLineEdit, QTextEdit, QPlainTextEdit {
    background-color: #2d2d2d;
    border: 1px solid #3c3c3c;
    border-radius: 3px;
    padding: 4px 8px;
    color: #d4d4d4;
    selection-background-color: #264f78;
    selection-color: #d4d4d4;
}

QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
    border-color: #0e639c;
}

/* ===== 列表 / 树 ===== */
QListView, QTreeView, QTableView {
    background-color: #1e1e1e;
    alternate-background-color: #252525;
    border: 1px solid #3c3c3c;
    outline: none;
}

QListView::item:hover, QTreeView::item:hover {
    background-color: #2a2d2e;
}

QListView::item:selected, QTreeView::item:selected {
    background-color: #094771;
    color: white;
}

/* ===== 表头 ===== */
QHeaderView::section {
    background-color: #2d2d2d;
    border: none;
    border-right: 1px solid #3c3c3c;
    border-bottom: 1px solid #3c3c3c;
    padding: 4px 8px;
    color: #d4d4d4;
}

/* ===== 滚动条 ===== */
QScrollBar:vertical {
    background: transparent;
    width: 10px;
}

QScrollBar::handle:vertical {
    background: #424242;
    min-height: 30px;
    border-radius: 5px;
}

QScrollBar::handle:vertical:hover {
    background: #555555;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}

/* ===== 标签页 ===== */
QTabWidget::pane {
    border: 1px solid #3c3c3c;
    background-color: #1e1e1e;
}

QTabBar::tab {
    background-color: #2d2d2d;
    border: none;
    padding: 8px 20px;
    color: #888888;
}

QTabBar::tab:selected {
    background-color: #1e1e1e;
    color: #d4d4d4;
    border-bottom: 2px solid #0e639c;
}

QTabBar::tab:hover:!selected {
    color: #bbbbbb;
}

/* ===== 菜单 ===== */
QMenuBar {
    background-color: #2d2d2d;
    border-bottom: 1px solid #3c3c3c;
}

QMenuBar::item:selected {
    background-color: #3c3c3c;
}

QMenu {
    background-color: #2d2d2d;
    border: 1px solid #3c3c3c;
    padding: 4px 0;
}

QMenu::item {
    padding: 6px 30px 6px 20px;
}

QMenu::item:selected {
    background-color: #094771;
}

QMenu::separator {
    height: 1px;
    background: #3c3c3c;
    margin: 4px 10px;
}

/* ===== 工具提示 ===== */
QToolTip {
    background-color: #2d2d2d;
    border: 1px solid #555555;
    color: #d4d4d4;
    padding: 4px 8px;
}

/* ===== 进度条 ===== */
QProgressBar {
    background-color: #3c3c3c;
    border: none;
    border-radius: 4px;
    text-align: center;
    color: #d4d4d4;
    height: 8px;
}

QProgressBar::chunk {
    background-color: #0e639c;
    border-radius: 4px;
}
```

---

## 19. 常见问题与注意事项

### Q1：QSS 不支持哪些 CSS 特性？

| 不支持的特性 | 替代方案 |
|---|---|
| `float`, `position: fixed` | Qt 布局管理器 |
| `display`, `visibility` | `QWidget::setVisible()` |
| `z-index` | `QWidget::raise()` / `lower()` |
| `box-shadow` | `QGraphicsDropShadowEffect` |
| `transform` | `QGraphicsRotation` 等 |
| `transition` / `animation` | `QPropertyAnimation` |
| `@media` 媒体查询 | 代码中判断后切换样式表 |
| `calc()` | 代码中计算后设置 |
| `var()` CSS 变量 | 代码预处理替换 |
| `inherit` | 手动设置 |
| `%` 百分比单位 | 布局管理器 + 固定像素 |

### Q2：QSS 设置后控件大小异常？

设置 `border` 或 `padding` 后控件可能变大。盒模型会增加控件实际尺寸：

```
实际宽度 = margin-left + border-left + padding-left + width + padding-right + border-right + margin-right
```

使用 `min-width` / `min-height` 代替 `width` / `height` 可保持弹性。

### Q3：自定义 QWidget 子类样式不生效？

必须重写 `paintEvent`：

```cpp
void CustomWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
```

### Q4：样式表中路径不生效？

- 使用 `/` 而非 `\` 作为路径分隔符
- 优先使用 Qt 资源系统 `url(:/path/to/file)`
- 确保资源文件已在 `.qrc` 中注册
