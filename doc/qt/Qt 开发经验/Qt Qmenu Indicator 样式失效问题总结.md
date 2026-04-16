# Qt QMenu::indicator 样式失效问题总结

## 一、问题现象

在使用 QSS 自定义菜单选中图标（`QMenu::indicator`）时：

* 某些菜单可以正常显示自定义图标
* 某些菜单始终显示系统默认样式

即使 QSS 写法一致，也会出现“部分生效、部分失效”的情况。

---

## 二、核心原因（本质）

问题的根本在于：

> `QAction` 是否属于 **exclusive（单选）** 或 **non-exclusive（多选）** 状态，直接影响 QSS 是否能命中。

---

## 三、Qt 中 indicator 的两种模式

### 1. non-exclusive（多选 / checkbox）

```cpp
QAction* act = new QAction(menu);
act->setCheckable(true);
```

特点：

* 可以多个同时选中
* 表现为 ✔ checkbox
* QSS 状态：`:non-exclusive`

---

### 2. exclusive（单选 / radio）

```cpp
QActionGroup* group = new QActionGroup(this);
act->setCheckable(true);
group->addAction(act);
```

默认行为：

```cpp
group->setExclusive(true); // 默认就是 true
```

特点：

* 只能选中一个
* 表现为 ● radio
* QSS 状态：`:exclusive`

---

## 四、问题触发条件

### ❌ 错误写法

```css
#DCCustomMenu::indicator:non-exclusive:checked {
    image: url(...);
}
```

当 QAction 实际属于 `exclusive` 时：

👉 **QSS 完全不会命中**
👉 导致使用系统默认 indicator

---

## 五、为什么会“看起来一部分生效”？

| 场景              | 是否使用 QActionGroup | 状态            | QSS 是否命中 |
| --------------- | ----------------- | ------------- | -------- |
| 普通 menu         | 否                 | non-exclusive | ✅ 生效     |
| 使用 QActionGroup | 是                 | exclusive     | ❌ 失效     |

---

## 六、正确解决方案

### ✅ 方案 1（推荐）：不区分状态

```css
#DCCustomMenu::indicator {
    width: 16px;
    height: 16px;
}

#DCCustomMenu::indicator:checked {
    image: url(:/icons/checked.png);
}
```

优点：

* 简单稳定
* 不依赖 Qt 内部状态

---

### ✅ 方案 2：同时支持两种模式

```css
/* 单选（radio） */
#DCCustomMenu::indicator:exclusive:checked {
    image: url(:/icons/checked.png);
}

/* 多选（checkbox） */
#DCCustomMenu::indicator:non-exclusive:checked {
    image: url(:/icons/checked.png);
}
```

---

## 七、补充知识点

### 1. QActionGroup 的隐式行为

* 默认 `exclusive = true`
* 会自动改变 indicator 类型（checkbox → radio）

---

### 2. indicator 与 right-arrow 区别

| 元素            | 作用      |
| ------------- | ------- |
| `indicator`   | 勾选/单选图标 |
| `right-arrow` | 子菜单箭头   |

---

### 3. 子菜单不会完全继承样式

* 子菜单是独立窗口
* 有时需要单独 `setStyleSheet`

---

### 4. 自定义 QMenu（如 DCMenu）注意点

如果重写了：

* `paintEvent`
* `drawPrimitive`
* `QStyle`

可能会导致 QSS 完全失效

---

## 八、最佳实践（推荐写法）

```css
#DCCustomMenu::indicator {
    width: 16px;
    height: 16px;
}

#DCCustomMenu::indicator:checked {
    image: url(:/FSCommonLib/Resources/light/icon16_menu_checked_n.png);
}
```

---

## 九、一句话总结

> QMenu::indicator 样式失效，本质是 QSS 选择器与 QAction 的 exclusive / non-exclusive 状态不匹配。

---

## 十、排查 Checklist

* [ ] QAction 是否 setCheckable
* [ ] 是否使用 QActionGroup
* [ ] 是否误用了 `:non-exclusive`
* [ ] QSS 是否命中（objectName / 类名）
* [ ] 是否被自定义样式覆盖

---

这个问题在 Qt 中非常常见，本质是“状态驱动的样式匹配问题”，理解后基本可以一次性避免类似坑。
