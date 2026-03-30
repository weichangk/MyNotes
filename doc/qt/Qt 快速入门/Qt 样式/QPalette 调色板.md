# QPalette 调色板

QPalette 是 Qt 中管理控件颜色方案的核心类，定义了控件在各种状态下各部位的绘制颜色。它是 Qt 样式系统的底层基础，比 QSS 更接近渲染层，性能开销也更小。

---

## 1. QPalette 核心概念

QPalette 通过 **ColorGroup（颜色组）** × **ColorRole（颜色角色）** 的二维矩阵管理颜色：

```
               Active      Inactive     Disabled
Window         #f0f0f0     #f0f0f0      #f0f0f0
WindowText     #000000     #000000      #787878
Base           #ffffff     #ffffff      #f0f0f0
Text           #000000     #000000      #787878
Button         #f0f0f0     #f0f0f0      #f0f0f0
ButtonText     #000000     #000000      #787878
Highlight      #0078d7     #e0e0e0      #0078d7
...
```

每个颜色条目由一个 `QBrush` 对象表示（支持纯色、渐变、贴图）。

---

## 2. ColorGroup — 颜色组

ColorGroup 描述控件当前所处的交互状态：

| 颜色组 | 枚举值 | 说明 |
|---|---|---|
| `Active` | `QPalette::Active` | 控件所在窗口处于前台激活状态 |
| `Inactive` | `QPalette::Inactive` | 控件所在窗口处于后台非激活状态 |
| `Disabled` | `QPalette::Disabled` | 控件被禁用（`setEnabled(false)`） |
| `Normal` | `QPalette::Normal` | Active 的别名 |
| `Current` | `QPalette::Current` | 运行时自动解析为当前实际状态 |

### 状态切换示例

```cpp
QPushButton *btn = new QPushButton("测试");

// 窗口在前台 → Active 组生效
// 切换到其他窗口 → Inactive 组生效
// 禁用按钮 → Disabled 组生效
btn->setEnabled(false);
```

> **Active vs Inactive**：Windows 上差异明显（标题栏变灰），macOS 上很多控件的 Active 和 Inactive 外观相同。Qt 默认会将 Inactive 组的颜色复制为 Active 组的值。

---

## 3. ColorRole — 颜色角色

ColorRole 定义控件的哪个**视觉部位**使用该颜色。这是 QPalette 最核心的部分。

### 3.1 基础角色

| 角色 | 枚举值 | 用途 | 典型控件 |
|---|---|---|---|
| `Window` | `QPalette::Window` | 通用背景色 | QWidget、QFrame、QDialog |
| `WindowText` | `QPalette::WindowText` | 通用前景色（文字） | QLabel、QGroupBox 标题 |
| `Base` | `QPalette::Base` | 输入/列表控件背景 | QLineEdit、QTextEdit、QListView |
| `Text` | `QPalette::Text` | 输入/列表控件文字 | QLineEdit、QTextEdit、QListView |
| `AlternateBase` | `QPalette::AlternateBase` | 交替行背景色 | QTreeView、QTableView |
| `ToolTipBase` | `QPalette::ToolTipBase` | 工具提示背景 | QToolTip |
| `ToolTipText` | `QPalette::ToolTipText` | 工具提示文字 | QToolTip |
| `PlaceholderText` | `QPalette::PlaceholderText` | 占位文字颜色（Qt 5.12+） | QLineEdit、QTextEdit |

### 3.2 按钮角色

| 角色 | 枚举值 | 用途 |
|---|---|---|
| `Button` | `QPalette::Button` | 按钮背景色 |
| `ButtonText` | `QPalette::ButtonText` | 按钮文字色 |

### 3.3 选中/高亮角色

| 角色 | 枚举值 | 用途 |
|---|---|---|
| `Highlight` | `QPalette::Highlight` | 选中项背景色 |
| `HighlightedText` | `QPalette::HighlightedText` | 选中项文字色 |

### 3.4 链接角色

| 角色 | 枚举值 | 用途 |
|---|---|---|
| `Link` | `QPalette::Link` | 未访问超链接色 |
| `LinkVisited` | `QPalette::LinkVisited` | 已访问超链接色 |

### 3.5 3D 效果角色

用于绘制控件的立体浮雕效果（如按钮凸起、面板凹陷）：

| 角色 | 枚举值 | 用途 |
|---|---|---|
| `Light` | `QPalette::Light` | 比 Button 更亮，用于3D高光（左上边缘） |
| `Midlight` | `QPalette::Midlight` | Button 和 Light 之间的中间色 |
| `Mid` | `QPalette::Mid` | Button 和 Dark 之间的中间色 |
| `Dark` | `QPalette::Dark` | 比 Button 更暗，用于3D阴影（右下边缘） |
| `Shadow` | `QPalette::Shadow` | 非常暗的颜色，用于投影 |

### 3.6 3D 浮雕效果示意

```
Light ─────────────────┐
│ Midlight ──────────┐ │
│ │                  │ │
│ │    Button 面     │ Mid
│ │                  │ │
│ └────────── Dark ──┘ │
└────────────── Shadow ┘
```

```cpp
// 手动绘制 3D 凸起效果
void MyWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    // 左上高光
    p.setPen(palette().color(QPalette::Light));
    p.drawLine(0, 0, width(), 0);       // 上边
    p.drawLine(0, 0, 0, height());      // 左边
    // 右下阴影
    p.setPen(palette().color(QPalette::Dark));
    p.drawLine(width()-1, 0, width()-1, height()-1);  // 右边
    p.drawLine(0, height()-1, width()-1, height()-1);  // 下边
}
```

---

## 4. 颜色角色分布图

以一个典型窗口为例，展示各 ColorRole 的作用区域：

```
┌─────────────────────────────────────────────────────┐
│               Window (背景)                          │
│                                                     │
│   WindowText: "表单标签"                              │
│   ┌─────────────────────────────┐                   │
│   │ Base (输入框背景)             │                   │
│   │ Text: "输入的文字"            │                   │
│   │ PlaceholderText: "请输入..."  │                   │
│   │ Highlight: [选中的文字背景]    │                   │
│   │ HighlightedText: [选中文字]   │                   │
│   └─────────────────────────────┘                   │
│                                                     │
│   ┌──────────────┐                                  │
│   │ Button (背景) │                                  │
│   │ ButtonText    │                                  │
│   └──────────────┘                                  │
│                                                     │
│   ┌─────────────────────────────┐                   │
│   │ Base (列表背景)              │                   │
│   │ Text: 第一行                 │                   │
│   │ AlternateBase: 第二行(交替)  │                   │
│   │ Highlight: [选中行]          │                   │
│   └─────────────────────────────┘                   │
└─────────────────────────────────────────────────────┘
```

---

## 5. 基本操作

### 5.1 获取调色板

```cpp
// 获取控件当前调色板
QPalette pal = widget->palette();

// 获取应用默认调色板
QPalette appPal = QApplication::palette();

// 获取特定控件类的默认调色板
QPalette btnPal = QApplication::palette("QPushButton");
```

### 5.2 读取颜色

```cpp
QPalette pal = widget->palette();

// 方式 1：指定 ColorGroup + ColorRole → 返回 QBrush
QBrush brush = pal.brush(QPalette::Active, QPalette::Button);

// 方式 2：指定 ColorGroup + ColorRole → 返回 QColor
QColor color = pal.color(QPalette::Active, QPalette::WindowText);

// 方式 3：省略 ColorGroup → 使用 Current 组（自动解析当前状态）
QColor textColor = pal.color(QPalette::Text);

// 便捷方法
QBrush windowBrush = pal.window();          // 等价于 brush(Current, Window)
QBrush textBrush   = pal.text();            // 等价于 brush(Current, Text)
QBrush baseBrush   = pal.base();            // 等价于 brush(Current, Base)
QBrush buttonBrush = pal.button();          // 等价于 brush(Current, Button)
QBrush highlightBrush = pal.highlight();    // 等价于 brush(Current, Highlight)
```

### 5.3 设置颜色

```cpp
QPalette pal = widget->palette();

// 方式 1：设置所有 ColorGroup 的某个 Role
pal.setColor(QPalette::WindowText, QColor("#333333"));

// 方式 2：设置指定 ColorGroup 的某个 Role
pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#999999"));

// 方式 3：使用 QBrush（支持渐变和贴图）
QLinearGradient gradient(0, 0, 0, 1);
gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
gradient.setColorAt(0, QColor("#3498db"));
gradient.setColorAt(1, QColor("#2980b9"));
pal.setBrush(QPalette::Button, QBrush(gradient));

// 应用调色板
widget->setPalette(pal);
```

### 5.4 应用到全局

```cpp
// 设置整个应用的调色板
QApplication app(argc, argv);

QPalette pal = app.palette();
pal.setColor(QPalette::Window, QColor("#f5f5f5"));
pal.setColor(QPalette::WindowText, QColor("#333333"));
app.setPalette(pal);
```

---

## 6. 实战：自定义暗色调色板

```cpp
QPalette createDarkPalette() {
    QPalette pal;

    // ----- 基础背景与前景 -----
    pal.setColor(QPalette::Window,          QColor(30, 30, 30));
    pal.setColor(QPalette::WindowText,      QColor(212, 212, 212));
    pal.setColor(QPalette::Base,            QColor(45, 45, 45));
    pal.setColor(QPalette::AlternateBase,   QColor(53, 53, 53));
    pal.setColor(QPalette::Text,            QColor(212, 212, 212));
    pal.setColor(QPalette::PlaceholderText, QColor(128, 128, 128));

    // ----- 按钮 -----
    pal.setColor(QPalette::Button,          QColor(60, 60, 60));
    pal.setColor(QPalette::ButtonText,      QColor(212, 212, 212));

    // ----- 选中/高亮 -----
    pal.setColor(QPalette::Highlight,       QColor(0, 120, 215));
    pal.setColor(QPalette::HighlightedText, QColor(255, 255, 255));

    // ----- 3D 效果 -----
    pal.setColor(QPalette::Light,           QColor(80, 80, 80));
    pal.setColor(QPalette::Midlight,        QColor(70, 70, 70));
    pal.setColor(QPalette::Mid,             QColor(50, 50, 50));
    pal.setColor(QPalette::Dark,            QColor(35, 35, 35));
    pal.setColor(QPalette::Shadow,          QColor(20, 20, 20));

    // ----- 工具提示 -----
    pal.setColor(QPalette::ToolTipBase,     QColor(45, 45, 45));
    pal.setColor(QPalette::ToolTipText,     QColor(212, 212, 212));

    // ----- 链接 -----
    pal.setColor(QPalette::Link,            QColor(42, 130, 218));
    pal.setColor(QPalette::LinkVisited,     QColor(150, 100, 200));

    // ----- 禁用状态单独设置 -----
    pal.setColor(QPalette::Disabled, QPalette::WindowText,  QColor(120, 120, 120));
    pal.setColor(QPalette::Disabled, QPalette::Text,        QColor(120, 120, 120));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText,  QColor(120, 120, 120));
    pal.setColor(QPalette::Disabled, QPalette::Base,        QColor(40, 40, 40));
    pal.setColor(QPalette::Disabled, QPalette::Button,      QColor(45, 45, 45));
    pal.setColor(QPalette::Disabled, QPalette::Highlight,   QColor(80, 80, 80));

    return pal;
}

// 使用
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");  // Fusion 风格对 QPalette 响应最好
    app.setPalette(createDarkPalette());
    // ...
}
```

---

## 7. 实战：自定义亮色调色板

```cpp
QPalette createLightPalette() {
    QPalette pal;

    pal.setColor(QPalette::Window,          QColor(245, 245, 245));
    pal.setColor(QPalette::WindowText,      QColor(51, 51, 51));
    pal.setColor(QPalette::Base,            QColor(255, 255, 255));
    pal.setColor(QPalette::AlternateBase,   QColor(245, 245, 245));
    pal.setColor(QPalette::Text,            QColor(51, 51, 51));
    pal.setColor(QPalette::PlaceholderText, QColor(160, 160, 160));

    pal.setColor(QPalette::Button,          QColor(240, 240, 240));
    pal.setColor(QPalette::ButtonText,      QColor(51, 51, 51));

    pal.setColor(QPalette::Highlight,       QColor(0, 120, 215));
    pal.setColor(QPalette::HighlightedText, QColor(255, 255, 255));

    pal.setColor(QPalette::Light,           QColor(255, 255, 255));
    pal.setColor(QPalette::Midlight,        QColor(230, 230, 230));
    pal.setColor(QPalette::Mid,             QColor(200, 200, 200));
    pal.setColor(QPalette::Dark,            QColor(160, 160, 160));
    pal.setColor(QPalette::Shadow,          QColor(105, 105, 105));

    pal.setColor(QPalette::ToolTipBase,     QColor(255, 255, 225));
    pal.setColor(QPalette::ToolTipText,     QColor(0, 0, 0));

    pal.setColor(QPalette::Link,            QColor(0, 102, 204));
    pal.setColor(QPalette::LinkVisited,     QColor(128, 0, 128));

    pal.setColor(QPalette::Disabled, QPalette::WindowText,  QColor(170, 170, 170));
    pal.setColor(QPalette::Disabled, QPalette::Text,        QColor(170, 170, 170));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText,  QColor(170, 170, 170));
    pal.setColor(QPalette::Disabled, QPalette::Highlight,   QColor(200, 200, 200));

    return pal;
}
```

---

## 8. QPalette 与 QStyle 的关系

### 8.1 不同 QStyle 对 QPalette 的支持程度

| QStyle | QPalette 支持 | 说明 |
|---|---|---|
| **Fusion** | ★★★★★ | 完全尊重 QPalette，自定义调色板首选 |
| **Windows** | ★★★★☆ | 基本尊重，少数细节使用系统颜色 |
| **WindowsVista** | ★★☆☆☆ | 大量使用系统原生绘制，QPalette 被部分忽略 |
| **macOS** | ★★☆☆☆ | 原生渲染优先，QPalette 影响有限 |

> **关键结论**：要充分利用 QPalette，必须使用 **Fusion** 风格。

```cpp
// 设置为 Fusion 风格（跨平台一致的外观）
QApplication::setStyle("Fusion");
```

### 8.2 QStyle 如何使用 QPalette

QStyle 在绘制控件时，从 `QStyleOption` 中获取调色板：

```cpp
// QStyle 内部绘制按钮的伪代码
void QFusionStyle::drawControl(CE_PushButton, const QStyleOption *opt, QPainter *p) {
    const QPalette &pal = opt->palette;

    if (opt->state & QStyle::State_Enabled) {
        if (opt->state & QStyle::State_MouseOver) {
            p->fillRect(rect, pal.button().color().lighter(110));
        } else {
            p->fillRect(rect, pal.button());
        }
        p->setPen(pal.buttonText().color());
    } else {
        p->fillRect(rect, pal.button());
        p->setPen(pal.color(QPalette::Disabled, QPalette::ButtonText));
    }
    p->drawText(rect, text);
}
```

---

## 9. QPalette 的传播与继承

### 9.1 自动传播

Qt 中 QPalette 沿控件树**自动向下传播**。子控件默认继承父控件的调色板。

```cpp
QWidget *parent = new QWidget;

QPalette pal = parent->palette();
pal.setColor(QPalette::WindowText, Qt::red);
parent->setPalette(pal);

QLabel *child = new QLabel("我是子控件", parent);
// child 自动继承 parent 的红色 WindowText，无需额外设置
```

### 9.2 resolve 机制

QPalette 内部通过 `resolveMask` 跟踪哪些角色被"显式设置"过。只有显式设置的角色会覆盖继承值：

```cpp
QPalette pal;
pal.setColor(QPalette::WindowText, Qt::red);
// 此时 resolveMask 中只有 WindowText 被标记

widget->setPalette(pal);
// widget 的 WindowText 变为红色
// 其他未标记的角色仍从父级或应用默认值继承
```

### 9.3 完全替换 vs 部分修改

```cpp
// ❌ 创建新 QPalette 并只设置一项 → 其他角色丢失可能导致显示异常
QPalette pal;
pal.setColor(QPalette::WindowText, Qt::red);
widget->setPalette(pal);  // 只有 WindowText 被标记修改，其他继承默认值

// ✅ 基于现有调色板修改 → 保留所有原始值
QPalette pal = widget->palette();
pal.setColor(QPalette::WindowText, Qt::red);
widget->setPalette(pal);
```

---

## 10. QPalette 与 QSS 的关系

### 10.1 QSS 覆盖 QPalette

**QSS 的优先级高于 QPalette**。一旦控件设置了 QSS，相关的 QPalette 角色将被忽略：

```cpp
btn->setPalette(redPalette);                        // 设置红色调色板
btn->setStyleSheet("color: blue;");                 // QSS 生效，文字为蓝色
qDebug() << btn->palette().color(QPalette::ButtonText);  // 仍返回红色，但实际渲染为蓝色
```

### 10.2 QSS 中引用 QPalette 颜色

QSS 可以通过 `palette()` 函数引用调色板颜色：

```css
QLabel {
    color: palette(window-text);        /* 引用 WindowText 角色 */
    background-color: palette(base);    /* 引用 Base 角色 */
    border: 1px solid palette(mid);     /* 引用 Mid 角色 */
}

QPushButton {
    background-color: palette(button);
    color: palette(button-text);
}

QPushButton:hover {
    background-color: palette(light);
}
```

QSS `palette()` 函数支持的角色名：

| QSS 名称 | 对应 ColorRole |
|---|---|
| `window` | `Window` |
| `window-text` | `WindowText` |
| `base` | `Base` |
| `text` | `Text` |
| `alternate-base` | `AlternateBase` |
| `button` | `Button` |
| `button-text` | `ButtonText` |
| `bright-text` | `BrightText` |
| `light` | `Light` |
| `midlight` | `Midlight` |
| `mid` | `Mid` |
| `dark` | `Dark` |
| `shadow` | `Shadow` |
| `highlight` | `Highlight` |
| `highlighted-text` | `HighlightedText` |
| `link` | `Link` |
| `link-visited` | `LinkVisited` |

### 10.3 最佳实践：QSS + QPalette 配合

```cpp
// 1. 用 QPalette 定义颜色方案
app.setPalette(createDarkPalette());

// 2. QSS 中引用 palette() 而非硬编码颜色
app.setStyleSheet(R"(
    QPushButton {
        background-color: palette(button);
        color: palette(button-text);
        border: 1px solid palette(mid);
        border-radius: 4px;
        padding: 6px 16px;
    }
    QPushButton:hover {
        background-color: palette(midlight);
    }
    QPushButton:pressed {
        background-color: palette(dark);
    }
)");

// 3. 切换主题时只需更换 QPalette，QSS 无需修改
void switchTheme(bool dark) {
    if (dark)
        qApp->setPalette(createDarkPalette());
    else
        qApp->setPalette(createLightPalette());
    // QSS 中的 palette() 会自动引用新调色板
}
```

---

## 11. 在自定义绘制中使用 QPalette

### 11.1 paintEvent 中读取调色板

```cpp
void CustomCard::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QPalette &pal = palette();

    // 背景
    painter.setBrush(pal.window());
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);

    // 边框
    painter.setPen(QPen(pal.mid(), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);

    // 标题文字
    painter.setPen(pal.color(QPalette::WindowText));
    painter.setFont(QFont(font().family(), 14, QFont::Bold));
    painter.drawText(QRect(16, 12, width() - 32, 30), Qt::AlignLeft, m_title);

    // 描述文字（使用更浅的颜色）
    painter.setPen(pal.color(QPalette::Mid));
    painter.setFont(QFont(font().family(), 11));
    painter.drawText(QRect(16, 44, width() - 32, 60), Qt::AlignLeft | Qt::TextWordWrap, m_desc);
}
```

### 11.2 QStyledItemDelegate 中使用调色板

```cpp
void MyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const {
    const QPalette &pal = option.palette;

    // 选中状态
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, pal.highlight());
        painter->setPen(pal.highlightedText().color());
    }
    // 鼠标悬停
    else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, pal.midlight());
        painter->setPen(pal.text().color());
    }
    // 正常状态
    else {
        // 交替行背景
        if (index.row() % 2 == 1)
            painter->fillRect(option.rect, pal.alternateBase());
        else
            painter->fillRect(option.rect, pal.base());
        painter->setPen(pal.text().color());
    }

    QString text = index.data(Qt::DisplayRole).toString();
    painter->drawText(option.rect.adjusted(10, 0, -10, 0), Qt::AlignVCenter, text);
}
```

---

## 12. 使用 QBrush 实现渐变和贴图

QPalette 不限于纯色，支持 QBrush 的所有能力：

### 12.1 渐变背景

```cpp
QPalette pal = widget->palette();

// 线性渐变作为窗口背景
QLinearGradient gradient(0, 0, 0, 1);
gradient.setCoordinateMode(QGradient::ObjectBoundingMode);  // 重要：使用相对坐标
gradient.setColorAt(0.0, QColor(44, 62, 80));
gradient.setColorAt(1.0, QColor(52, 73, 94));
pal.setBrush(QPalette::Window, QBrush(gradient));

pal.setColor(QPalette::WindowText, Qt::white);
widget->setPalette(pal);
widget->setAutoFillBackground(true);  // 必须开启自动填充
```

### 12.2 图案/贴图背景

```cpp
QPalette pal = widget->palette();
QPixmap pattern(":/images/pattern.png");
pal.setBrush(QPalette::Window, QBrush(pattern));
widget->setPalette(pal);
widget->setAutoFillBackground(true);
```

> **关键提醒**：使用 `setBrush` 设置背景时，控件必须调用 `setAutoFillBackground(true)`，否则背景不会自动绘制。

---

## 13. 调色板序列化与持久化

### 13.1 保存到 QSettings

```cpp
void savePalette(const QPalette &pal, QSettings &settings) {
    settings.beginGroup("Palette");
    auto saveRole = [&](QPalette::ColorRole role, const QString &name) {
        settings.setValue(name + "_active",   pal.color(QPalette::Active,   role).name());
        settings.setValue(name + "_inactive", pal.color(QPalette::Inactive, role).name());
        settings.setValue(name + "_disabled", pal.color(QPalette::Disabled, role).name());
    };
    saveRole(QPalette::Window,        "Window");
    saveRole(QPalette::WindowText,    "WindowText");
    saveRole(QPalette::Base,          "Base");
    saveRole(QPalette::Text,          "Text");
    saveRole(QPalette::Button,        "Button");
    saveRole(QPalette::ButtonText,    "ButtonText");
    saveRole(QPalette::Highlight,     "Highlight");
    saveRole(QPalette::HighlightedText, "HighlightedText");
    settings.endGroup();
}
```

### 13.2 从 QSettings 加载

```cpp
QPalette loadPalette(QSettings &settings) {
    QPalette pal;
    settings.beginGroup("Palette");
    auto loadRole = [&](QPalette::ColorRole role, const QString &name) {
        pal.setColor(QPalette::Active,   role, QColor(settings.value(name + "_active").toString()));
        pal.setColor(QPalette::Inactive, role, QColor(settings.value(name + "_inactive").toString()));
        pal.setColor(QPalette::Disabled, role, QColor(settings.value(name + "_disabled").toString()));
    };
    loadRole(QPalette::Window,        "Window");
    loadRole(QPalette::WindowText,    "WindowText");
    loadRole(QPalette::Base,          "Base");
    loadRole(QPalette::Text,          "Text");
    loadRole(QPalette::Button,        "Button");
    loadRole(QPalette::ButtonText,    "ButtonText");
    loadRole(QPalette::Highlight,     "Highlight");
    loadRole(QPalette::HighlightedText, "HighlightedText");
    settings.endGroup();
    return pal;
}
```

### 13.3 使用 QDataStream 序列化

```cpp
// 保存
QByteArray data;
QDataStream out(&data, QIODevice::WriteOnly);
out << palette;

// 加载
QPalette pal;
QDataStream in(data);
in >> pal;
```

---

## 14. 主题管理器完整示例

```cpp
class ThemeManager : public QObject {
    Q_OBJECT
public:
    enum Theme { Light, Dark, System };

    void setTheme(Theme theme) {
        m_theme = theme;
        applyTheme();
    }

    Theme currentTheme() const { return m_theme; }

signals:
    void themeChanged(Theme theme);

private:
    void applyTheme() {
        QApplication::setStyle("Fusion");

        switch (m_theme) {
        case Dark:
            qApp->setPalette(createDarkPalette());
            break;
        case Light:
            qApp->setPalette(createLightPalette());
            break;
        case System:
            qApp->setPalette(QApplication::style()->standardPalette());
            break;
        }

        // 强制所有控件刷新
        for (QWidget *w : QApplication::allWidgets()) {
            w->update();
        }

        emit themeChanged(m_theme);
    }

    QPalette createDarkPalette() {
        QPalette pal;
        pal.setColor(QPalette::Window,          QColor(30, 30, 30));
        pal.setColor(QPalette::WindowText,      QColor(212, 212, 212));
        pal.setColor(QPalette::Base,            QColor(45, 45, 45));
        pal.setColor(QPalette::AlternateBase,   QColor(53, 53, 53));
        pal.setColor(QPalette::Text,            QColor(212, 212, 212));
        pal.setColor(QPalette::Button,          QColor(60, 60, 60));
        pal.setColor(QPalette::ButtonText,      QColor(212, 212, 212));
        pal.setColor(QPalette::Highlight,       QColor(0, 120, 215));
        pal.setColor(QPalette::HighlightedText, Qt::white);
        pal.setColor(QPalette::Link,            QColor(42, 130, 218));

        pal.setColor(QPalette::Disabled, QPalette::Text,       QColor(120, 120, 120));
        pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 120));
        pal.setColor(QPalette::Disabled, QPalette::Highlight,  QColor(80, 80, 80));
        return pal;
    }

    QPalette createLightPalette() {
        QPalette pal;
        pal.setColor(QPalette::Window,          QColor(245, 245, 245));
        pal.setColor(QPalette::WindowText,      QColor(51, 51, 51));
        pal.setColor(QPalette::Base,            Qt::white);
        pal.setColor(QPalette::AlternateBase,   QColor(245, 245, 245));
        pal.setColor(QPalette::Text,            QColor(51, 51, 51));
        pal.setColor(QPalette::Button,          QColor(240, 240, 240));
        pal.setColor(QPalette::ButtonText,      QColor(51, 51, 51));
        pal.setColor(QPalette::Highlight,       QColor(0, 120, 215));
        pal.setColor(QPalette::HighlightedText, Qt::white);
        pal.setColor(QPalette::Link,            QColor(0, 102, 204));

        pal.setColor(QPalette::Disabled, QPalette::Text,       QColor(170, 170, 170));
        pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(170, 170, 170));
        return pal;
    }

    Theme m_theme = System;
};
```

---

## 15. 跟随系统主题变化

### Qt 5.15+

```cpp
// 检测系统是否为暗色模式（Windows 10+）
#ifdef Q_OS_WIN
#include <QSettings>
bool isSystemDarkMode() {
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat);
    return settings.value("AppsUseLightTheme", 1).toInt() == 0;
}
#endif
```

### Qt 6.5+ 原生支持

```cpp
// Qt 6.5 引入了 QStyleHints::colorScheme()
QStyleHints *hints = QApplication::styleHints();
Qt::ColorScheme scheme = hints->colorScheme();

if (scheme == Qt::ColorScheme::Dark) {
    qApp->setPalette(createDarkPalette());
} else {
    qApp->setPalette(createLightPalette());
}

// 监听系统主题变化
connect(hints, &QStyleHints::colorSchemeChanged, this, [](Qt::ColorScheme scheme) {
    if (scheme == Qt::ColorScheme::Dark)
        qApp->setPalette(createDarkPalette());
    else
        qApp->setPalette(createLightPalette());
});
```

---

## 16. 调试 QPalette

### 16.1 打印调色板内容

```cpp
void dumpPalette(const QPalette &pal) {
    auto dump = [&](QPalette::ColorGroup group, const QString &groupName) {
        qDebug() << "=====" << groupName << "=====";
        auto print = [&](QPalette::ColorRole role, const QString &name) {
            qDebug().noquote() << QString("  %-20s %s").arg(name, pal.color(group, role).name());
        };
        print(QPalette::Window,          "Window");
        print(QPalette::WindowText,      "WindowText");
        print(QPalette::Base,            "Base");
        print(QPalette::Text,            "Text");
        print(QPalette::Button,          "Button");
        print(QPalette::ButtonText,      "ButtonText");
        print(QPalette::Highlight,       "Highlight");
        print(QPalette::HighlightedText, "HighlightedText");
        print(QPalette::Light,           "Light");
        print(QPalette::Dark,            "Dark");
        print(QPalette::Mid,             "Mid");
        print(QPalette::Shadow,          "Shadow");
    };
    dump(QPalette::Active,   "Active");
    dump(QPalette::Inactive, "Inactive");
    dump(QPalette::Disabled, "Disabled");
}
```

### 16.2 可视化调色板查看器

```cpp
// 简易的调色板可视化控件
class PaletteViewer : public QWidget {
public:
    PaletteViewer(QWidget *parent = nullptr) : QWidget(parent) {
        setMinimumSize(400, 300);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        const QPalette &pal = palette();
        int y = 10;
        int h = 20;

        auto drawRole = [&](QPalette::ColorRole role, const QString &name) {
            QColor c = pal.color(role);
            p.fillRect(10, y, 30, h, c);
            p.setPen(pal.windowText().color());
            p.drawRect(10, y, 30, h);
            p.drawText(50, y + 15, QString("%1: %2").arg(name, c.name()));
            y += h + 4;
        };

        drawRole(QPalette::Window,          "Window");
        drawRole(QPalette::WindowText,      "WindowText");
        drawRole(QPalette::Base,            "Base");
        drawRole(QPalette::Text,            "Text");
        drawRole(QPalette::Button,          "Button");
        drawRole(QPalette::ButtonText,      "ButtonText");
        drawRole(QPalette::Highlight,       "Highlight");
        drawRole(QPalette::HighlightedText, "HighlightedText");
        drawRole(QPalette::Light,           "Light");
        drawRole(QPalette::Midlight,        "Midlight");
        drawRole(QPalette::Mid,             "Mid");
        drawRole(QPalette::Dark,            "Dark");
        drawRole(QPalette::Shadow,          "Shadow");
    }
};
```

---

## 17. 常见问题与注意事项

### Q1：setPalette 后控件没有变化？

**原因 1**：未使用 Fusion 风格。WindowsVista/macOS 风格忽略大部分 QPalette 设置。
```cpp
QApplication::setStyle("Fusion");  // 必须设置
```

**原因 2**：被 QSS 覆盖。检查是否有 `setStyleSheet` 设置了相关属性。

**原因 3**：控件未开启 `autoFillBackground`。对于非窗口控件的背景色：
```cpp
widget->setAutoFillBackground(true);
```

### Q2：只想改某个控件的某个颜色，如何避免影响其他角色？

基于现有调色板修改，而非创建全新的：
```cpp
QPalette pal = widget->palette();  // 先获取当前的
pal.setColor(QPalette::WindowText, Qt::red);  // 只改需要的
widget->setPalette(pal);
```

### Q3：QPalette 和 QSS 应该选哪个？

| 场景 | 推荐方案 |
|---|---|
| 全局颜色方案/暗色模式 | QPalette + Fusion 风格 |
| 精细控制单个控件外观（圆角、阴影、图片） | QSS |
| 自定义绘制（paintEvent） | QPalette |
| 需要运行时频繁切换外观 | QSS + palette() 函数引用 |
| 需要最佳性能 | QPalette（无解析开销） |
| 快速原型/设计稿还原 | QSS（上手快） |

### Q4：Inactive 和 Active 组颜色想设置成一样？

```cpp
// 默认情况下 Qt 会自动将 Inactive 设为与 Active 相同
// 如果需要显式确保一致：
for (int role = 0; role < QPalette::NColorRoles; ++role) {
    pal.setColor(QPalette::Inactive,
                 static_cast<QPalette::ColorRole>(role),
                 pal.color(QPalette::Active, static_cast<QPalette::ColorRole>(role)));
}
```

---

## 18. ColorRole 速查表

```
角色                   典型用途
───────────────────────────────────────────────
Window                 QWidget/QFrame/QDialog 背景
WindowText             QLabel/QGroupBox 标题文字
Base                   QLineEdit/QTextEdit/QListView 背景
Text                   QLineEdit/QTextEdit/QListView 文字
AlternateBase          QTreeView/QTableView 交替行背景
PlaceholderText        QLineEdit/QTextEdit 占位符文字
Button                 QPushButton/QToolButton 背景
ButtonText             QPushButton/QToolButton 文字
Highlight              选中项背景
HighlightedText        选中项文字
Link                   超链接
LinkVisited            已访问超链接
ToolTipBase            QToolTip 背景
ToolTipText            QToolTip 文字
BrightText             与 Dark 背景搭配的亮色文字
Light                  3D高光（比Button亮）
Midlight               Button与Light之间
Mid                    Button与Dark之间
Dark                   3D阴影（比Button暗）
Shadow                 3D投影（最暗）
```
