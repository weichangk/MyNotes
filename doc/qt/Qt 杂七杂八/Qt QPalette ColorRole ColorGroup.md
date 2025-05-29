QPalette 在 Qt 的样式自定义中起着非常核心的作用，尤其在不使用 QSS（StyleSheet）而采用更“原生”的方式开发控件时，QPalette 是非常重要的工具
| 特点                       | 说明                                                                  |
| ------------------------ | ------------------------------------------------------------------- |
| **统一管理颜色角色**          | 每个控件可以通过角色（如 WindowText、Base、Highlight）分别设置和获取对应的颜色。                |
| **根据状态区分颜色组**         | QPalette 支持 `Active`、`Inactive` 和 `Disabled` 三个状态，让控件在不同状态下有不同颜色表现。 |
| **控件默认使用角色绘制自身**      | 很多控件默认通过角色（比如 `Text`、`Window`）来渲染内容和边框。                             |
| **比 QSS 更灵活地支持自定义绘制** | QSS 是“黑盒式”皮肤化，而 QPalette 与 `paintEvent` 配合可以实现更多细致控制。               |
| **支持系统主题和样式联动**       | 比如切换暗色主题时，系统会更新 QPalette，控件样式自动适配。                                  |


## ColorRole
Qt 中 QPalette::ColorRole 的枚举定义，这些枚举项是 Qt 控制控件颜色（文字、背景、边框等）使用的角色（Role）标识符

### 作用解释
它用于 QPalette::setColor() 来设置不同角色的颜色，例如：
```cpp
QPalette palette = widget->palette();
palette.setColor(QPalette::WindowText, Qt::red);  // 设置前景色/文本色
widget->setPalette(palette);
```

### 角色说明

| ColorRole                    | 用途说明                                    |
| ---------------------------- | --------------------------------------- |
| `WindowText`                 | 用于控件前景颜色（如 QFrame 的 `Plain` 边框）         |
| `Window`                     | 控件背景色                                   |
| `Base`                       | 文本编辑框等区域的背景色                            |
| `Text`                       | 正文文本颜色（如 QLineEdit）                     |
| `Button`                     | 按钮背景色                                   |
| `ButtonText`                 | 按钮上的文字颜色                                |
| `Light`, `Mid`, `Dark`       | 在 `Raised`/`Sunken` 边框中用于绘制阴影（亮边、中线、暗边） |
| `Shadow`                     | 一般用于阴影色                                 |
| `Highlight`                  | 高亮背景色（比如选中文字）                           |
| `HighlightedText`            | 高亮文字的颜色                                 |
| `ToolTipBase`, `ToolTipText` | 提示框背景/文字色                               |
| `PlaceholderText`            | 占位文本的颜色（比如 QLineEdit 的提示文本）             |

用于指定控件哪个部分使用哪种颜色：
- 文本：WindowText, Text, ButtonText, HighlightedText
- 背景：Window, Base, AlternateBase, Button
- 边框与阴影：Light, Dark, Mid, Shadow
- 交互高亮：Highlight, Link, VisitedLink


### 应用场景

- 控件文本和背景色设置
    ```cpp
    QPalette p = widget->palette();
    p.setColor(QPalette::Text, Qt::red);
    p.setColor(QPalette::Base, Qt::black);
    widget->setPalette(p);
    ```
- QFrame 的边框色控制（Plain 模式）
    ```cpp
    frame->setFrameShadow(QFrame::Plain);
    frame->setLineWidth(2);
    p.setColor(QPalette::WindowText, Qt::blue);
    frame->setForegroundRole(QPalette::WindowText);
    ```
- 高亮色设置（比如选中项背景）
    ```cpp
    QPalette p = app.palette();
    p.setColor(QPalette::Highlight, QColor("#FF4081"));        // 粉红高亮
    p.setColor(QPalette::HighlightedText, Qt::white);          // 高亮文字
    app.setPalette(p);
    ```
- 禁用状态颜色控制（Disabled group）
    ```cpp
    QPalette p = widget->palette();
    p.setColor(QPalette::Disabled, QPalette::Text, Qt::gray);  // 禁用时的文字
    widget->setPalette(p);
    ```


## ColorGroup
### 作用解释
QPalette 支持 Active、Inactive 和 Disabled 三个状态，让控件在不同状态下有不同颜色表现

### 角色说明
Qt 中的 ColorGroup 表示控件当前所处的状态，对应三种：
| 枚举值                  | 含义              |
| -------------------- | --------------- |
| `QPalette::Active`   | 控件所在窗口为活动窗口时使用  |
| `QPalette::Inactive` | 控件所在窗口为非活动窗口时使用 |
| `QPalette::Disabled` | 控件被禁用时使用        |

### 应用场景
可以使用 setColor(ColorGroup, ColorRole, QColor) 精确设置某种状态下的颜色。

- 设置不同状态下的文字颜色
    ```cpp
    QPalette palette;

    // 活动状态下的颜色
    palette.setColor(QPalette::Active, QPalette::WindowText, Qt::black);

    // 非活动状态下颜色（例如，窗口不在前台时）
    palette.setColor(QPalette::Inactive, QPalette::WindowText, Qt::darkGray);

    // 禁用状态下颜色（例如，控件 setEnabled(false)）
    palette.setColor(QPalette::Disabled, QPalette::WindowText, Qt::gray);

    label->setPalette(palette);
    ```

- 按钮不同状态下的颜色区分
    ```cpp
    QPushButton* button = new QPushButton("测试按钮");

    QPalette p = button->palette();
    p.setColor(QPalette::Active, QPalette::Button, Qt::blue);          // 活动时按钮背景蓝色
    p.setColor(QPalette::Inactive, QPalette::Button, Qt::gray);        // 非活动窗口时变灰
    p.setColor(QPalette::Disabled, QPalette::Button, Qt::darkGray);    // 禁用时更暗

    button->setPalette(p);
    button->setAutoFillBackground(true);
    ```

- 查看当前状态
    ```cpp
    // 判断某个控件是否是活动窗口的一部分
    if (widget->palette().currentColorGroup() == QPalette::Active) {
        qDebug() << "当前是活动状态";
    }
    ```

## QPalette 配色模板封装
### 封装类
定义：PaletteTemplate
```cpp
#pragma once

#include <QPalette>
#include <QColor>
#include <QMap>
#include <QWidget>

// 配色模板结构体
struct ColorScheme {
    QColor active;
    QColor inactive;
    QColor disabled;
};

class PaletteTemplate {
public:
    PaletteTemplate() = default;

    // 设置某个角色的配色方案
    void setRoleScheme(QPalette::ColorRole role, const ColorScheme& scheme) {
        colorSchemes[role] = scheme;
    }

    // 应用到控件
    void applyTo(QWidget* widget) const {
        QPalette palette = widget->palette();

        for (auto it = colorSchemes.begin(); it != colorSchemes.end(); ++it) {
            QPalette::ColorRole role = it.key();
            const ColorScheme& scheme = it.value();
            palette.setColor(QPalette::Active, role, scheme.active);
            palette.setColor(QPalette::Inactive, role, scheme.inactive);
            palette.setColor(QPalette::Disabled, role, scheme.disabled);
        }

        widget->setPalette(palette);
        widget->setAutoFillBackground(true);
    }

private:
    QMap<QPalette::ColorRole, ColorScheme> colorSchemes;
};
```
### 用法示例
统一设置按钮配色
```cpp
ColorScheme blueBtn {
    QColor("#007bff"),   // active
    QColor("#6c757d"),   // inactive
    QColor("#cccccc")    // disabled
};

ColorScheme textWhite {
    QColor("#ffffff"),
    QColor("#eeeeee"),
    QColor("#bbbbbb")
};

QPushButton* button = new QPushButton("配色示例");

PaletteTemplate pt;
pt.setRoleScheme(QPalette::Button, blueBtn);
pt.setRoleScheme(QPalette::ButtonText, textWhite);

pt.applyTo(button);
```

## QPalette 使用 JSON 配色模板封装
### JSON 配置格式示例
palette_theme.json
```json
{
  "Window": {
    "Active": "#ffffff",
    "Inactive": "#f0f0f0",
    "Disabled": "#dcdcdc"
  },
  "WindowText": {
    "Active": "#000000",
    "Inactive": "#333333",
    "Disabled": "#999999"
  },
  "Base": {
    "Active": "#ffffff",
    "Inactive": "#f5f5f5",
    "Disabled": "#eaeaea"
  },
  "Text": {
    "Active": "#000000",
    "Inactive": "#555555",
    "Disabled": "#aaaaaa"
  },
  "Button": {
    "Active": "#007bff",
    "Inactive": "#6c757d",
    "Disabled": "#cccccc"
  },
  "ButtonText": {
    "Active": "#ffffff",
    "Inactive": "#eeeeee",
    "Disabled": "#bbbbbb"
  },
  "Highlight": {
    "Active": "#3399ff",
    "Inactive": "#cccccc",
    "Disabled": "#dddddd"
  },
  "HighlightedText": {
    "Active": "#ffffff",
    "Inactive": "#666666",
    "Disabled": "#999999"
  }
}
```

### 封装类
支持从 JSON 读取并应用到 QWidget
```cpp
#pragma once

#include <QPalette>
#include <QWidget>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QColor>
#include <QDebug>

class JsonPaletteLoader {
public:
    static bool setPaletteFromJson(const QString& path, QWidget* widget) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open palette file:" << path;
            return false;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (!doc.isObject()) {
            qWarning() << "Invalid JSON format in palette file.";
            return false;
        }

        QPalette palette = widget->palette();
        QJsonObject root = doc.object();

        for (auto it = root.begin(); it != root.end(); ++it) {
            QString roleStr = it.key();
            QPalette::ColorRole role = stringToColorRole(roleStr);
            if (role == QPalette::NoRole)
                continue;

            QJsonObject roleColors = it.value().toObject();
            for (const QString& groupStr : roleColors.keys()) {
                QColor color(roleColors[groupStr].toString());
                QPalette::ColorGroup group = stringToColorGroup(groupStr);
                if (group != QPalette::NColorGroups)
                    palette.setColor(group, role, color);
            }
        }

        widget->setPalette(palette);
        widget->setAutoFillBackground(true);
        return true;
    }

    static bool applyAppTheme(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open theme file:" << path;
            return false;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (!doc.isObject()) {
            qWarning() << "Invalid theme JSON format.";
            return false;
        }

        QJsonObject root = doc.object();
        QPalette palette;

        for (const QString& roleKey : root.keys()) {
            QPalette::ColorRole role = stringToColorRole(roleKey);
            if (role == QPalette::NoRole)
                continue;

            QJsonObject groupColors = root[roleKey].toObject();
            for (const QString& groupKey : groupColors.keys()) {
                QPalette::ColorGroup group = stringToColorGroup(groupKey);
                if (group == QPalette::NColorGroups)
                    continue;

                QColor color(groupColors[groupKey].toString());
                palette.setColor(group, role, color);
            }
        }

        qApp->setPalette(palette);
        return true;
    }

private:
    static QPalette::ColorRole stringToColorRole(const QString& name) {
        static const QMap<QString, QPalette::ColorRole> roleMap = {
            {"WindowText", QPalette::WindowText}, {"Button", QPalette::Button},
            {"Light", QPalette::Light}, {"Midlight", QPalette::Midlight},
            {"Dark", QPalette::Dark}, {"Mid", QPalette::Mid},
            {"Text", QPalette::Text}, {"BrightText", QPalette::BrightText},
            {"ButtonText", QPalette::ButtonText}, {"Base", QPalette::Base},
            {"Window", QPalette::Window}, {"Shadow", QPalette::Shadow},
            {"Highlight", QPalette::Highlight}, {"HighlightedText", QPalette::HighlightedText},
            {"Link", QPalette::Link}, {"LinkVisited", QPalette::LinkVisited},
            {"AlternateBase", QPalette::AlternateBase}, {"ToolTipBase", QPalette::ToolTipBase},
            {"ToolTipText", QPalette::ToolTipText}, {"PlaceholderText", QPalette::PlaceholderText}
        };
        return roleMap.value(name, QPalette::NoRole);
    }

    static QPalette::ColorGroup stringToColorGroup(const QString& name) {
        static QMap<QString, QPalette::ColorGroup> groupMap = {
            {"Active", QPalette::Active},
            {"Inactive", QPalette::Inactive},
            {"Disabled", QPalette::Disabled}
        };
        return groupMap.value(name, QPalette::NColorGroups);
    }
};
```

### 用法示例
```cpp
QWidget* widget = new QPushButton("使用 JSON 配色");
JsonPaletteLoader::setPaletteFromJson(":/themes/palette_theme.json", widget);
```

支持动态主题切换
```cpp
void switchTheme(const QString& name) {
    if (name == "dark")
        JsonPaletteLoader::setPaletteFromJson(":/themes/dark_theme.json", this);
    else
        JsonPaletteLoader::setPaletteFromJson(":/themes/light_theme.json", this);
}
```

应用全局主题色
```cpp
JsonPaletteLoader::applyAppTheme(":/themes/light_theme.json");
```

## QPalette 和 QPainter QSS
### 三者作用范围与差异

| 项目                    | 作用范围      | 主要控制内容               | 是否影响其他方式                                         |
| --------------------- | --------- | -------------------- | ------------------------------------------------ |
| **QPalette**          | Qt 控件配色系统 | 控件默认状态下的颜色（背景、文字、边框） | 被 `QSS` 覆盖，影响 `paintEvent` 的默认 `QStyleOption` 颜色 |
| **QSS（StyleSheet）**   | 精准样式控制    | 边框、背景、字体、圆角等视觉样式     | 覆盖 `QPalette`，不影响手动绘制                            |
| **自定义绘制（paintEvent）** | 自绘控件      | 完全控制绘制逻辑             | 不受 QPalette、QSS 影响，除非你主动读取它们                     |

### 优先级顺序
从高到低排序
- 自定义绘制（paintEvent）：你画啥就是啥，其他无效，最高优先级。
- QSS（Qt StyleSheet）：能覆盖大多数控件的默认样式和 QPalette 的颜色设置。
- QPalette：默认控件在未设置 QSS 或未自绘时采用的颜色系统。