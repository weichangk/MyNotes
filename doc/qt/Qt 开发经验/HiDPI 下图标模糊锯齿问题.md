# HiDPI 下图标模糊/锯齿问题

## 问题现象

在 4K 显示器 + 150% 系统缩放（或其他 HiDPI）环境下，控件图标出现模糊或锯齿，即使图片本身是高分辨率资源也无法改善。

---

## 场景一：QLabel::setPixmap / QPainter::drawPixmap 模糊

### 根本原因

`QPixmap::size()` 返回的是**逻辑像素尺寸**，而屏幕渲染依赖**物理像素尺寸**（逻辑尺寸 × devicePixelRatio）。

常见错误写法：

```cpp
QSize iconSize = QSize(m_pIcon->size()); // 逻辑像素，如 16×16
m_pIcon->setPixmap(pixmap.scaled(iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
```

问题在于：
1. `scaled` 目标是逻辑像素尺寸（如 16×16）
2. Qt 渲染时会把这个 16×16 的 pixmap 拉伸到物理像素区域（如 24×24）
3. 拉伸导致模糊/锯齿

### 解决方案

scaled 到**物理像素尺寸**，并调用 `setDevicePixelRatio` 告知 Qt 正确的像素密度：

```cpp
QSize iconSize = QSize(m_pIcon->size()); // 逻辑像素
qreal dpr = devicePixelRatioF();
QSize physicalSize = iconSize * dpr;     // 物理像素
QPixmap result = (pixmap.size() == physicalSize)
    ? pixmap
    : pixmap.scaled(physicalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
result.setDevicePixelRatio(dpr);         // 关键：告知 Qt 此 pixmap 的像素密度
m_pIcon->setPixmap(result);
```

`QPainter::drawPixmap` 同理（如自绘控件的 paintEvent）：

```cpp
qreal dpr = devicePixelRatioF();
QSize physicalSize = rect().size() * dpr;
QPixmap scaledPix = (pix.size() == physicalSize)
    ? pix
    : pix.scaled(physicalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
scaledPix.setDevicePixelRatio(dpr);
p.drawPixmap(rect(), scaledPix);
```

### 关键点

- `pixmap.size() == physicalSize` 时直接赋值，避免不必要的缩放运算
- `setDevicePixelRatio(dpr)` 是核心修复，Qt 渲染时会以 1:1 物理像素映射，不再拉伸
- 适用于 `QLabel::setPixmap` 和 `QPainter::drawPixmap` 两种场景

---

## 场景二：QSS background-image 模糊

### 根本原因

QSS 的 `background-image` 不会自动适配 HiDPI 缩放，图片会按逻辑像素绘制后拉伸，导致模糊。

```css
/* 错误写法 */
QPushButton#MyButton {
    background-image: url(:/light/icon.svg);
}
```

### 解决方案

将 `background-image` 替换为 `border-image`，`border-image` 会自动拉伸填充整个控件区域，在 HiDPI 下表现更好：

```css
/* 正确写法 */
QPushButton#MyButton {
    border-image: url(:/light/icon.svg);
}
```

### 关键点

- `border-image` 会自动缩放填充控件的完整区域，适合图标型按钮
- `background-image` 默认不拉伸，在 HiDPI 下容易模糊
- 若控件有圆角（`border-radius`），`border-image` 会覆盖 border 样式，需注意同时设置 `border: none`

---

## 适用场景

- 所有通过 `QLabel::setPixmap` 显示图标的自定义控件
- 所有在 `paintEvent` 中使用 `QPainter::drawPixmap` 绘制图标的自绘控件
- 所有使用 QSS `background-image` 显示图标的按钮控件
