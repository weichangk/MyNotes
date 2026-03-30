# QPainter 基础

## 概述

`QPainter` 是 Qt 2D 绘图系统的核心类，提供了在**绘图设备（QPaintDevice）**上绘制图形、文本、图像的统一 API。它是 Qt 整个可视化体系的基石——所有 `QWidget` 的外观、自定义控件的绘制、离屏渲染、PDF 生成、SVG 导出都依赖 `QPainter`。

### 架构关系

```
QPainter（画笔/绘图指令）
    │
    ▼ 绘制到
QPaintDevice（绘图设备）
    ├── QWidget      — 屏幕控件
    ├── QPixmap      — 离屏像素图（依赖显示系统，适合屏幕显示）
    ├── QImage       — 离屏像素图（纯内存，可跨线程使用）
    ├── QBitmap      — 单色位图（QPixmap 的子类，仅 0/1）
    ├── QPicture     — 录制/回放绘图指令序列
    ├── QPrinter     — 打印机 / PDF 输出
    ├── QSvgGenerator— SVG 文件输出
    └── QOpenGLPaintDevice — OpenGL 表面
```

---

## 1. QPainter 的基本使用流程

### 1.1 在 Widget 中绑定

所有 `QWidget` 的绘制都发生在 `paintEvent()` 中：

```cpp
class MyWidget : public QWidget
{
    Q_OBJECT
protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this); // 以当前 widget 为绘图设备
        // 绑定成功后即可绑定
        painter.drawText(rect(), Qt::AlignCenter, "Hello QPainter!");
    }
};
```

> **关键规则**：`QPainter` 只能在 `paintEvent()` 内部对 `QWidget` 进行绑定。在其他地方创建会触发警告：`QPainter::begin: Widget painting can only begin as a result of a paintEvent`。

### 1.2 begin() / end() 显式写法

```cpp
void MyWidget::paintEvent(QPaintEvent *) {
    QPainter painter;
    painter.begin(this); // 显式绑定
    painter.drawRect(10, 10, 100, 80);
    painter.end();       // 显式结束（也可省略，析构时自动调用）
}
```

### 1.3 在离屏设备上绘制

```cpp
// 在 QPixmap 上绘制（不需要在 paintEvent 中）
QPixmap pixmap(400, 300);
pixmap.fill(Qt::white);

QPainter painter(&pixmap);
painter.setPen(Qt::blue);
painter.drawEllipse(50, 50, 300, 200);
painter.end();

pixmap.save("output.png");
```

```cpp
// 在 QImage 上绘制（可在非 GUI 线程中使用）
QImage image(800, 600, QImage::Format_ARGB32);
image.fill(Qt::transparent);

QPainter painter(&image);
painter.setRenderHint(QPainter::Antialiasing);
painter.setBrush(QColor(255, 100, 50, 180));
painter.drawRoundedRect(50, 50, 700, 500, 20, 20);
painter.end();

image.save("output.png");
```

### 1.4 触发重绘

```cpp
// 在需要重绘时调用（不要直接调用 paintEvent）
update();          // 异步重绘（推荐：Qt 会合并多次调用）
repaint();         // 立即同步重绘（慎用：可能导致性能问题）
update(QRect(0, 0, 100, 100)); // 只重绘指定区域
```

---

## 2. 基本图形绘制

### 2.1 直线

```cpp
void MyWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    // 单条直线
    painter.drawLine(10, 10, 200, 150);
    painter.drawLine(QPoint(10, 50), QPoint(200, 50));
    painter.drawLine(QLineF(10.5, 80.5, 200.5, 80.5)); // 浮点精度

    // 多条直线
    QVector<QLineF> lines = {
        {10, 100, 100, 100},
        {10, 120, 100, 120},
        {10, 140, 100, 140}
    };
    painter.drawLines(lines);
}
```

### 2.2 矩形

```cpp
QPainter painter(this);
painter.setPen(QPen(Qt::black, 2));
painter.setBrush(QColor(100, 150, 200));

// 普通矩形
painter.drawRect(10, 10, 150, 100);
painter.drawRect(QRect(10, 10, 150, 100));

// 圆角矩形
painter.drawRoundedRect(10, 130, 150, 100, 15, 15);
// 参数：x, y, w, h, xRadius, yRadius

// 多个矩形
QVector<QRect> rects = {{10, 10, 50, 50}, {70, 10, 50, 50}, {130, 10, 50, 50}};
painter.drawRects(rects);
```

### 2.3 椭圆与圆

```cpp
QPainter painter(this);

// 椭圆（由外接矩形定义）
painter.drawEllipse(10, 10, 200, 120);
painter.drawEllipse(QRectF(10.0, 10.0, 200.0, 120.0));

// 圆（宽高相等的椭圆）
painter.drawEllipse(QPoint(150, 150), 80, 80); // 圆心 + 半径x + 半径y

// 精确圆
painter.drawEllipse(QPointF(150.5, 150.5), 80.0, 80.0);
```

### 2.4 弧线、扇形、弦

Qt 中角度单位是 **1/16 度**（sixteenth of a degree）。

```cpp
QPainter painter(this);
QRect rect(30, 30, 200, 150); // 外接矩形

int startAngle = 30 * 16;     // 起始角 30°
int spanAngle = 120 * 16;     // 跨越角 120°

// 弧线（仅轮廓）
painter.drawArc(rect, startAngle, spanAngle);

// 扇形（填充 + 连接圆心的两条半径）
painter.drawPie(rect, startAngle, spanAngle);

// 弦（填充 + 连接弧线两端的弦）
painter.drawChord(rect, startAngle, spanAngle);
```

```
        弧线 Arc              扇形 Pie              弦 Chord
       ╭─────╮              ╭─────╮              ╭─────╮
      ╱       ╲            ╱ ░░░░░ ╲            ╱░░░░░░░╲
     ╱         ╲          ╱░░░░░░░░░╲          ╱░░░░░░░░░╲
                          ╲░░░░░░░╱            ╲░░░░░░░░░╱
                           ╲░░░╱                ─────────
                            ╲╱
```

### 2.5 多边形

```cpp
QPainter painter(this);
painter.setRenderHint(QPainter::Antialiasing);

// 三角形
QPolygon triangle;
triangle << QPoint(100, 10) << QPoint(10, 180) << QPoint(190, 180);
painter.drawPolygon(triangle);

// 五角星
QPolygonF star;
const int points = 5;
const double outerR = 80, innerR = 35;
for (int i = 0; i < points * 2; ++i) {
    double r = (i % 2 == 0) ? outerR : innerR;
    double angle = M_PI / 2 + i * M_PI / points;
    star << QPointF(100 + r * cos(angle), 100 - r * sin(angle));
}
painter.setBrush(Qt::yellow);
painter.drawPolygon(star);

// 折线（不闭合的多边形）
QPolygon polyline;
polyline << QPoint(10, 80) << QPoint(50, 10) << QPoint(90, 60)
         << QPoint(130, 30) << QPoint(170, 90);
painter.drawPolyline(polyline);
```

### 2.6 点

```cpp
QPainter painter(this);
painter.setPen(QPen(Qt::red, 3)); // 点的大小由画笔宽度决定

// 单个点
painter.drawPoint(100, 100);

// 多个点
QVector<QPointF> points;
for (int i = 0; i < 360; ++i) {
    double rad = i * M_PI / 180.0;
    points << QPointF(200 + 100 * cos(rad), 200 + 100 * sin(rad));
}
painter.drawPoints(points.data(), points.size());
```

---

## 3. 文本绘制

### 3.1 基本文本

```cpp
QPainter painter(this);
painter.setFont(QFont("Microsoft YaHei", 16));
painter.setPen(Qt::black);

// 在指定点绘制（文本基线左端对齐到该点）
painter.drawText(20, 40, "Hello, Qt!");

// 在矩形内绘制（支持对齐和自动换行）
QRect textRect(20, 60, 300, 200);
painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap,
                 "这是一段较长的文本，会在矩形内自动换行并居中显示。");

// 绘制边框辅助查看
painter.setPen(Qt::gray);
painter.drawRect(textRect);
```

### 3.2 文本度量（QFontMetrics）

```cpp
QPainter painter(this);
QFont font("Consolas", 14);
painter.setFont(font);

QFontMetrics fm(font);

QString text = "Hello, QPainter!";
int textWidth = fm.horizontalAdvance(text); // 文本像素宽度
int textHeight = fm.height();                // 行高
int ascent = fm.ascent();                    // 基线到顶部
int descent = fm.descent();                  // 基线到底部

// 居中绘制
int x = (width() - textWidth) / 2;
int y = (height() + ascent - descent) / 2;
painter.drawText(x, y, text);

// 文本省略
QString longText = "这是一段非常非常长的文本需要省略显示";
QString elided = fm.elidedText(longText, Qt::ElideRight, 200);
painter.drawText(20, 100, elided); // "这是一段非常非常长..."
```

### 3.3 富文本绘制

使用 `QTextDocument` 绘制 HTML：

```cpp
void MyWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    QTextDocument doc;
    doc.setHtml("<h2 style='color:navy'>标题</h2>"
                "<p>这是<b>加粗</b>和<i>斜体</i>文本，"
                "还有<span style='color:red'>红色</span>字。</p>");
    doc.setTextWidth(300);

    painter.translate(20, 20); // 移动到绘制起点
    doc.drawContents(&painter);
}
```

---

## 4. 渲染提示（Render Hints）

渲染提示控制绘制质量与性能的平衡：

```cpp
QPainter painter(this);

// 抗锯齿（图形边缘平滑）
painter.setRenderHint(QPainter::Antialiasing, true);

// 文本抗锯齿
painter.setRenderHint(QPainter::TextAntialiasing, true);

// 平滑图像缩放（双线性过滤）
painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

// 一次设置多个
painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
```

### 4.1 抗锯齿效果对比

```
关闭抗锯齿 (Aliased)          开启抗锯齿 (Antialiased)
┌─────────────────┐           ┌─────────────────┐
│ ██              │           │ ▓█              │
│   ██            │           │  ░▓█            │
│     ██          │           │    ░▓█          │
│       ██        │           │      ░▓█        │
│         ██      │           │        ░▓█      │
└─────────────────┘           └─────────────────┘
  边缘锯齿明显                  边缘渐变平滑
```

### 4.2 各提示的性能影响

| 提示 | 效果 | 性能开销 | 建议 |
|---|---|---|---|
| `Antialiasing` | 图形抗锯齿 | 中等 | 自定义控件推荐开启 |
| `TextAntialiasing` | 文本抗锯齿 | 低 | 默认开启 |
| `SmoothPixmapTransform` | 图像平滑缩放 | 较高 | 需要时开启 |
| `HighQualityAntialiasing` | 更高质量抗锯齿（已弃用） | 高 | 用 Antialiasing 替代 |

---

## 5. 画笔（QPen）

### 5.1 基本属性

```cpp
QPen pen;
pen.setColor(QColor(50, 100, 200));  // 颜色
pen.setWidth(3);                      // 宽度（像素）
pen.setStyle(Qt::DashLine);           // 线型
pen.setCapStyle(Qt::RoundCap);        // 端点样式
pen.setJoinStyle(Qt::RoundJoin);      // 连接样式

QPainter painter(this);
painter.setPen(pen);
painter.drawRect(20, 20, 200, 150);
```

### 5.2 线型

```cpp
pen.setStyle(Qt::SolidLine);     // ───────────  实线
pen.setStyle(Qt::DashLine);      // ── ── ── ── 虚线
pen.setStyle(Qt::DotLine);       // · · · · · ·  点线
pen.setStyle(Qt::DashDotLine);   // ─·─·─·─·─·  点划线
pen.setStyle(Qt::DashDotDotLine);// ─··─··─··─   双点划线
pen.setStyle(Qt::NoPen);         //              无线条

// 自定义虚线模式（以画笔宽度为单位）
pen.setStyle(Qt::CustomDashLine);
pen.setDashPattern({8, 4, 2, 4}); // 长线8 空4 短线2 空4 循环
```

### 5.3 端点样式（Cap Style）

```
FlatCap          SquareCap        RoundCap
  │                ┃                ┃
  │              ──╂──            ──╂──
──┘              ──╂──            ──╂──
                   ┃                ┃
端点平齐            端点延伸半宽       端点半圆
```

### 5.4 连接样式（Join Style）

```
BevelJoin         MiterJoin         RoundJoin
    ╱                ╱                ╱
   ╱                ╱                ╱
  ╱──              ╱                ╱
 ╱  ──            ╱──              ╱──
斜切               尖角              圆角
```

### 5.5 无画笔（只有填充）

```cpp
painter.setPen(Qt::NoPen);       // 不画边框
painter.setBrush(Qt::green);
painter.drawEllipse(50, 50, 100, 100); // 只有填充的圆
```

---

## 6. 画刷（QBrush）

### 6.1 纯色填充

```cpp
QBrush brush(QColor(100, 200, 100));
painter.setBrush(brush);

// 或直接
painter.setBrush(Qt::blue);
painter.setBrush(QColor(255, 128, 0, 180)); // 半透明橙色
```

### 6.2 图案填充

```cpp
painter.setBrush(QBrush(Qt::red, Qt::DiagCrossPattern));
painter.drawRect(20, 20, 200, 150);
```

常用图案：

| 图案 | 枚举 | 效果 |
|---|---|---|
| 实心 | `Qt::SolidPattern` | █████ |
| 水平线 | `Qt::HorPattern` | ───── |
| 垂直线 | `Qt::VerPattern` | │││││ |
| 交叉线 | `Qt::CrossPattern` | ┼┼┼┼┼ |
| 正斜线 | `Qt::BDiagPattern` | ╲╲╲╲╲ |
| 反斜线 | `Qt::FDiagPattern` | ╱╱╱╱╱ |
| 斜交叉 | `Qt::DiagCrossPattern` | ╳╳╳╳╳ |
| 无填充 | `Qt::NoBrush` | 透明 |

### 6.3 渐变填充

```cpp
// 线性渐变
QLinearGradient linearGrad(0, 0, 300, 0);
linearGrad.setColorAt(0, QColor(255, 0, 0));     // 左端红色
linearGrad.setColorAt(0.5, QColor(255, 255, 0));  // 中间黄色
linearGrad.setColorAt(1, QColor(0, 200, 0));      // 右端绿色
painter.setBrush(linearGrad);
painter.drawRect(20, 20, 300, 80);

// 径向渐变
QRadialGradient radialGrad(150, 150, 100); // 圆心 (150,150), 半径 100
radialGrad.setColorAt(0, QColor(255, 255, 255));   // 中心白色
radialGrad.setColorAt(1, QColor(0, 0, 180));       // 边缘深蓝
painter.setBrush(radialGrad);
painter.drawEllipse(50, 50, 200, 200);

// 锥形渐变（角度渐变）
QConicalGradient conicalGrad(150, 150, 0); // 圆心 (150,150), 起始角 0°
conicalGrad.setColorAt(0, Qt::red);
conicalGrad.setColorAt(0.25, Qt::yellow);
conicalGrad.setColorAt(0.5, Qt::green);
conicalGrad.setColorAt(0.75, Qt::cyan);
conicalGrad.setColorAt(1, Qt::red);
painter.setBrush(conicalGrad);
painter.drawEllipse(50, 50, 200, 200);
```

### 6.4 渐变扩展模式

```cpp
QLinearGradient grad(0, 0, 100, 0);
grad.setColorAt(0, Qt::red);
grad.setColorAt(1, Qt::blue);

grad.setSpread(QGradient::PadSpread);     // 超出范围用末端颜色填充（默认）
grad.setSpread(QGradient::RepeatSpread);  // 重复渐变
grad.setSpread(QGradient::ReflectSpread); // 镜像重复
```

### 6.5 纹理填充

```cpp
QPixmap texture(":/textures/pattern.png");
QBrush textureBrush(texture);
painter.setBrush(textureBrush);
painter.drawRect(0, 0, 400, 300);
```

---

## 7. QPainterPath —— 复杂路径

### 7.1 概念

`QPainterPath` 用于描述复杂的 2D 图形路径，支持直线、曲线、子路径的组合。定义一次后可多次绘制和复用。

### 7.2 基本用法

```cpp
QPainter painter(this);
painter.setRenderHint(QPainter::Antialiasing);

QPainterPath path;
path.moveTo(50, 200);           // 移动到起点
path.lineTo(150, 20);           // 直线到
path.lineTo(250, 200);          // 直线到
path.closeSubpath();             // 闭合当前子路径（连回起点）

painter.setPen(QPen(Qt::black, 2));
painter.setBrush(QColor(100, 180, 255, 128));
painter.drawPath(path);
```

### 7.3 曲线

```cpp
QPainterPath path;
path.moveTo(20, 200);

// 二次贝塞尔曲线（控制点 + 终点）
path.quadTo(QPointF(100, 20), QPointF(180, 200));

// 三次贝塞尔曲线（两个控制点 + 终点）
path.cubicTo(QPointF(220, 20), QPointF(300, 20), QPointF(380, 200));

painter.drawPath(path);
```

### 7.4 内置图形

```cpp
QPainterPath path;

// 添加矩形
path.addRect(10, 10, 100, 80);

// 添加圆角矩形
path.addRoundedRect(130, 10, 100, 80, 10, 10);

// 添加椭圆
path.addEllipse(250, 10, 100, 80);

// 添加文本
QFont font("Arial", 48, QFont::Bold);
path.addText(10, 200, font, "Qt");

painter.setBrush(Qt::darkCyan);
painter.drawPath(path);
```

### 7.5 路径的布尔运算

```cpp
QPainterPath circle1, circle2;
circle1.addEllipse(50, 50, 150, 150);
circle2.addEllipse(120, 50, 150, 150);

// 并集
QPainterPath united = circle1.united(circle2);

// 交集
QPainterPath intersected = circle1.intersected(circle2);

// 差集（circle1 - circle2）
QPainterPath subtracted = circle1.subtracted(circle2);

painter.setBrush(QColor(255, 200, 100, 180));
painter.drawPath(intersected);
```

### 7.6 路径的碰撞检测

```cpp
QPainterPath shape;
shape.addEllipse(50, 50, 200, 200);

QPointF testPoint(150, 150);
if (shape.contains(testPoint)) {
    qDebug() << "点在图形内部";
}

QPainterPath otherShape;
otherShape.addRect(100, 100, 200, 200);
if (shape.intersects(otherShape)) {
    qDebug() << "两个图形相交";
}
```

---

## 8. 状态保存与恢复

### 8.1 save() / restore() 栈机制

`QPainter` 内部维护一个**状态栈**，`save()` 将当前状态压栈，`restore()` 弹出并恢复。

保存的状态包括：画笔、画刷、字体、变换矩阵、裁剪区域、渲染提示、合成模式等。

```cpp
QPainter painter(this);

// 保存初始状态
painter.save();

// 修改状态
painter.setPen(Qt::red);
painter.setBrush(Qt::yellow);
painter.translate(100, 100);
painter.rotate(45);
painter.drawRect(-30, -30, 60, 60); // 红边黄底旋转方块

// 恢复初始状态（画笔、画刷、变换全部恢复）
painter.restore();

painter.drawRect(10, 10, 60, 60); // 用初始默认状态绘制
```

### 8.2 嵌套保存

```cpp
painter.save();         // 保存状态 A
    painter.translate(100, 0);
    painter.save();     // 保存状态 B（包含 translate）
        painter.rotate(30);
        painter.drawRect(0, 0, 50, 50);
    painter.restore();  // 恢复到 B（有 translate，无 rotate）
    painter.drawRect(0, 0, 50, 50);
painter.restore();      // 恢复到 A（无 translate，无 rotate）
```

> **重要**：`save()` 和 `restore()` 必须严格配对。不匹配会导致状态混乱或绘制错误。

---

## 9. 坐标变换预览

`QPainter` 支持四种基本变换（详见《坐标系统与变换》）：

```cpp
QPainter painter(this);
painter.setRenderHint(QPainter::Antialiasing);

// 平移
painter.translate(200, 150);

// 旋转（以当前原点为中心）
painter.rotate(30); // 顺时针 30°

// 缩放
painter.scale(1.5, 1.5);

// 斜切
painter.shear(0.3, 0);

painter.drawRect(-50, -30, 100, 60);
```

### 9.1 变换的组合顺序

变换是**从右到左**（矩阵右乘）作用的，代码中**后写的先生效**：

```cpp
// 效果：先以原点旋转45°，再平移到(200,150)
painter.translate(200, 150);
painter.rotate(45);
// 等价于：物体先旋转，再整体搬移到(200,150)

// 如果顺序反过来：
painter.rotate(45);
painter.translate(200, 150);
// 效果：先平移，再以原点旋转（物体绕原点公转）
```

---

## 10. 裁剪（Clipping）

### 10.1 矩形裁剪

```cpp
QPainter painter(this);

// 设置裁剪区域：只有此区域内的绘制可见
painter.setClipRect(QRect(50, 50, 300, 200));

// 超出裁剪区域的部分不会被渲染
painter.setBrush(Qt::blue);
painter.drawEllipse(0, 0, 400, 400); // 只显示裁剪区域内的部分
```

### 10.2 路径裁剪（任意形状）

```cpp
QPainterPath clipPath;
clipPath.addEllipse(100, 50, 200, 200);
painter.setClipPath(clipPath);

// 所有后续绘制都被限制在椭圆内
QImage photo("portrait.jpg");
painter.drawImage(rect(), photo); // 图片被裁剪为椭圆形
```

### 10.3 裁剪操作

```cpp
// 替换裁剪区域（默认）
painter.setClipRect(rectA, Qt::ReplaceClip);

// 与现有裁剪区域取交集
painter.setClipRect(rectB, Qt::IntersectClip);
```

---

## 11. 合成模式（Composition Mode）

控制新绘制的像素如何与已有像素混合：

```cpp
QPainter painter(this);

// 绘制底层
painter.setBrush(Qt::blue);
painter.drawEllipse(50, 50, 150, 150);

// 设置合成模式
painter.setCompositionMode(QPainter::CompositionMode_Multiply);

// 绘制上层（颜色与底层混合）
painter.setBrush(Qt::red);
painter.drawEllipse(120, 50, 150, 150);
```

### 11.1 常用合成模式

| 模式 | 效果 |
|---|---|
| `SourceOver` | 默认——新内容覆盖在旧内容上，带 Alpha 混合 |
| `DestinationOver` | 旧内容在新内容上方 |
| `Clear` | 清除（目标区域变透明） |
| `Source` | 完全替换 |
| `Multiply` | 颜色相乘（变暗） |
| `Screen` | 颜色相加（变亮） |
| `Overlay` | 叠加（对比度增强） |
| `Difference` | 颜色差值 |
| `SourceIn` | 仅在重叠区域显示新内容 |
| `DestinationIn` | 仅在重叠区域显示旧内容 |

> **注意**：部分合成模式（如 `Multiply`、`Screen`）仅在 `QImage` 上有效，在 `QWidget` 直接绘制时可能不支持。

---

## 12. 实战示例

### 12.1 自定义圆形进度条

```cpp
class CircularProgress : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
public:
    explicit CircularProgress(QWidget *parent = nullptr)
        : QWidget(parent) {
        setFixedSize(150, 150);
    }

    int value() const { return m_value; }
    void setValue(int val) {
        m_value = qBound(0, val, 100);
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int side = qMin(width(), height());
        QRectF outerRect(10, 10, side - 20, side - 20);

        // 1. 背景圆环
        QPen bgPen(QColor(230, 230, 230), 8);
        bgPen.setCapStyle(Qt::RoundCap);
        painter.setPen(bgPen);
        painter.drawArc(outerRect, 0, 360 * 16);

        // 2. 进度圆弧
        QPen fgPen(QColor(60, 150, 255), 8);
        fgPen.setCapStyle(Qt::RoundCap);
        painter.setPen(fgPen);

        int startAngle = 90 * 16;  // 12 点钟方向
        int spanAngle = -m_value * 360 * 16 / 100; // 顺时针
        painter.drawArc(outerRect, startAngle, spanAngle);

        // 3. 中间百分比文字
        painter.setPen(QColor(60, 60, 60));
        painter.setFont(QFont("Arial", side / 5, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter,
                         QString::number(m_value) + "%");
    }

private:
    int m_value = 0;
};
```

### 12.2 自定义按钮

```cpp
class FancyButton : public QWidget
{
    Q_OBJECT
public:
    explicit FancyButton(const QString &text, QWidget *parent = nullptr)
        : QWidget(parent), m_text(text) {
        setFixedSize(160, 48);
        setCursor(Qt::PointingHandCursor);
    }

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QRectF r = rect().adjusted(1, 1, -1, -1);

        // 背景渐变
        QLinearGradient grad(0, 0, 0, height());
        if (m_pressed) {
            grad.setColorAt(0, QColor(30, 100, 200));
            grad.setColorAt(1, QColor(20, 80, 170));
        } else if (m_hovered) {
            grad.setColorAt(0, QColor(70, 160, 255));
            grad.setColorAt(1, QColor(50, 130, 240));
        } else {
            grad.setColorAt(0, QColor(55, 140, 250));
            grad.setColorAt(1, QColor(40, 115, 220));
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(grad);
        painter.drawRoundedRect(r, 8, 8);

        // 文字
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 11, QFont::Medium));
        painter.drawText(r, Qt::AlignCenter, m_text);
    }

    void enterEvent(QEvent *) override { m_hovered = true; update(); }
    void leaveEvent(QEvent *) override { m_hovered = false; update(); }
    void mousePressEvent(QMouseEvent *) override { m_pressed = true; update(); }
    void mouseReleaseEvent(QMouseEvent *e) override {
        m_pressed = false;
        update();
        if (rect().contains(e->pos()))
            emit clicked();
    }

private:
    QString m_text;
    bool m_hovered = false;
    bool m_pressed = false;
};
```

### 12.3 绘制数据折线图

```cpp
class LineChart : public QWidget
{
    Q_OBJECT
public:
    void setData(const QVector<double> &data) {
        m_data = data;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        if (m_data.isEmpty()) return;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), Qt::white);

        const int padding = 40;
        QRectF plotArea(padding, padding,
                        width() - 2 * padding, height() - 2 * padding);

        // 数据范围
        double minVal = *std::min_element(m_data.begin(), m_data.end());
        double maxVal = *std::max_element(m_data.begin(), m_data.end());
        if (qFuzzyCompare(minVal, maxVal)) maxVal = minVal + 1;

        // 坐标轴
        painter.setPen(QPen(Qt::gray, 1));
        painter.drawLine(plotArea.bottomLeft(), plotArea.topLeft());     // Y 轴
        painter.drawLine(plotArea.bottomLeft(), plotArea.bottomRight()); // X 轴

        // 数据点 → 屏幕坐标
        auto toScreen = [&](int i, double val) -> QPointF {
            double x = plotArea.left() + i * plotArea.width() / (m_data.size() - 1);
            double y = plotArea.bottom() - (val - minVal) / (maxVal - minVal) * plotArea.height();
            return {x, y};
        };

        // 填充区域
        QPainterPath fillPath;
        fillPath.moveTo(toScreen(0, minVal));
        for (int i = 0; i < m_data.size(); ++i)
            fillPath.lineTo(toScreen(i, m_data[i]));
        fillPath.lineTo(toScreen(m_data.size() - 1, minVal));
        fillPath.closeSubpath();

        QLinearGradient fillGrad(0, plotArea.top(), 0, plotArea.bottom());
        fillGrad.setColorAt(0, QColor(60, 150, 255, 80));
        fillGrad.setColorAt(1, QColor(60, 150, 255, 10));
        painter.setPen(Qt::NoPen);
        painter.setBrush(fillGrad);
        painter.drawPath(fillPath);

        // 折线
        QPainterPath linePath;
        linePath.moveTo(toScreen(0, m_data[0]));
        for (int i = 1; i < m_data.size(); ++i)
            linePath.lineTo(toScreen(i, m_data[i]));

        painter.setPen(QPen(QColor(60, 150, 255), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(linePath);

        // 数据点
        painter.setPen(QPen(QColor(60, 150, 255), 2));
        painter.setBrush(Qt::white);
        for (int i = 0; i < m_data.size(); ++i) {
            QPointF pt = toScreen(i, m_data[i]);
            painter.drawEllipse(pt, 4, 4);
        }

        // Y 轴刻度
        painter.setPen(Qt::darkGray);
        painter.setFont(QFont("Arial", 9));
        for (int i = 0; i <= 5; ++i) {
            double val = minVal + (maxVal - minVal) * i / 5;
            double y = plotArea.bottom() - plotArea.height() * i / 5;
            painter.drawText(QRectF(0, y - 10, padding - 5, 20),
                             Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(val, 'f', 1));
        }
    }

private:
    QVector<double> m_data;
};

// 使用
LineChart *chart = new LineChart;
chart->setData({3.2, 5.1, 4.8, 7.3, 6.5, 8.2, 7.9, 9.1, 8.6, 10.0});
```

---

## 13. 性能优化建议

### 13.1 减少重绘区域

```cpp
// ❌ 整个控件重绘
update();

// ✅ 只重绘变化区域
update(dirtyRect);

// 在 paintEvent 中利用 event->rect() 跳过无需重绘的区域
void paintEvent(QPaintEvent *event) override {
    QRect updateRect = event->rect();
    // 只绘制与 updateRect 相交的内容
}
```

### 13.2 离屏缓存

```cpp
class CachedWidget : public QWidget
{
protected:
    void paintEvent(QPaintEvent *) override {
        if (m_cache.isNull() || m_dirty) {
            // 重新渲染到缓存
            m_cache = QPixmap(size() * devicePixelRatioF());
            m_cache.setDevicePixelRatio(devicePixelRatioF());
            m_cache.fill(Qt::transparent);

            QPainter cachePainter(&m_cache);
            cachePainter.setRenderHint(QPainter::Antialiasing);
            drawComplexContent(&cachePainter);
            cachePainter.end();

            m_dirty = false;
        }

        // 直接绘制缓存图
        QPainter painter(this);
        painter.drawPixmap(0, 0, m_cache);

        // 在缓存之上绘制动态内容（如光标、选中效果）
        drawDynamicOverlay(&painter);
    }

private:
    QPixmap m_cache;
    bool m_dirty = true;
};
```

### 13.3 其他优化要点

| 技巧 | 说明 |
|---|---|
| 避免在 paintEvent 中创建对象 | `QFont`、`QPen`、`QBrush` 等应作为成员变量或局部 static |
| 使用 `QPixmap` 缓存静态内容 | 复杂图形只渲染一次，后续直接 `drawPixmap` |
| 合理使用抗锯齿 | 仅在需要时开启，大量直线/矩形可关闭 |
| 使用 `drawPixmap` 而非 `drawImage` | `QPixmap` 针对屏幕显示优化，速度更快 |
| 避免频繁调用 `repaint()` | 使用 `update()`，Qt 会自动合并重绘请求 |
| 利用 `QPainter::clipRegion()` | 跳过不可见区域的绘制 |

---

## 14. 常见陷阱

### 14.1 在 paintEvent 外使用 QPainter

```cpp
// ❌ 运行时警告
void MyWidget::onButtonClicked() {
    QPainter painter(this); // Widget 的 QPainter 只能在 paintEvent 中创建
    painter.drawLine(0, 0, 100, 100);
}

// ✅ 标记数据，触发重绘
void MyWidget::onButtonClicked() {
    m_drawLine = true;
    update(); // 触发 paintEvent
}
```

### 14.2 忘记 end() 或 save/restore 不匹配

```cpp
// ❌ 同时激活两个 QPainter 到同一设备
QPainter p1(this);
QPainter p2(this); // 警告：设备已被另一个 QPainter 使用

// ❌ save/restore 不匹配
painter.save();
painter.save();
painter.restore(); // 只恢复了一层，少了一次 restore
```

### 14.3 高 DPI 适配

```cpp
void MyWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    // 获取设备像素比（高 DPI 屏幕通常为 1.25、1.5、2.0 等）
    qreal dpr = devicePixelRatioF();

    // 绘制时使用逻辑像素（Qt 自动处理物理像素映射）
    painter.drawRect(10, 10, 100, 100); // 逻辑 100x100

    // 离屏绘制时需要手动处理
    QPixmap pixmap(QSize(200, 200) * dpr);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);
    QPainter pm(&pixmap);
    pm.drawEllipse(0, 0, 200, 200); // 使用逻辑坐标
    pm.end();

    painter.drawPixmap(0, 0, pixmap);
}
```

---

## 15. QPainter API 速查

### 15.1 图形绘制

| 方法 | 说明 |
|---|---|
| `drawPoint()` / `drawPoints()` | 画点 |
| `drawLine()` / `drawLines()` | 画直线 |
| `drawRect()` / `drawRects()` | 画矩形 |
| `drawRoundedRect()` | 画圆角矩形 |
| `drawEllipse()` | 画椭圆/圆 |
| `drawArc()` | 画弧线 |
| `drawPie()` | 画扇形 |
| `drawChord()` | 画弦 |
| `drawPolygon()` | 画多边形（闭合） |
| `drawPolyline()` | 画折线（不闭合） |
| `drawPath()` | 画路径 |
| `fillRect()` | 填充矩形（无边框） |
| `eraseRect()` | 擦除矩形（用背景色填充） |
| `fillPath()` | 填充路径 |
| `strokePath()` | 描边路径 |

### 15.2 文本与图像

| 方法 | 说明 |
|---|---|
| `drawText()` | 绘制文本 |
| `drawPixmap()` | 绘制 QPixmap |
| `drawImage()` | 绘制 QImage |
| `drawTiledPixmap()` | 平铺绘制 QPixmap |
| `drawPicture()` | 回放 QPicture |

### 15.3 状态控制

| 方法 | 说明 |
|---|---|
| `save()` / `restore()` | 状态压/弹栈 |
| `setPen()` / `pen()` | 设置/获取画笔 |
| `setBrush()` / `brush()` | 设置/获取画刷 |
| `setFont()` / `font()` | 设置/获取字体 |
| `setOpacity()` / `opacity()` | 设置/获取全局透明度（0.0~1.0） |
| `setRenderHint()` | 设置渲染提示 |
| `setClipRect()` / `setClipPath()` | 设置裁剪 |
| `setCompositionMode()` | 设置合成模式 |

### 15.4 坐标变换

| 方法 | 说明 |
|---|---|
| `translate(dx, dy)` | 平移 |
| `rotate(angle)` | 旋转（度） |
| `scale(sx, sy)` | 缩放 |
| `shear(sh, sv)` | 斜切 |
| `setTransform()` / `transform()` | 设置/获取变换矩阵 |
| `resetTransform()` | 重置变换 |

---

## 16. 总结

| 要点 | 说明 |
|---|---|
| **绑定设备** | `QPainter` 必须绑定到 `QPaintDevice` 才能工作 |
| **Widget 绑定** | 只能在 `paintEvent()` 中对 `QWidget` 创建 `QPainter` |
| **触发重绘** | 使用 `update()`（异步合并），避免 `repaint()` |
| **抗锯齿** | `setRenderHint(QPainter::Antialiasing)` 提升图形质量 |
| **画笔 + 画刷** | `QPen` 控制轮廓，`QBrush` 控制填充，`Qt::NoPen`/`Qt::NoBrush` 禁用 |
| **QPainterPath** | 复杂图形用路径描述，支持布尔运算和碰撞检测 |
| **状态管理** | `save()`/`restore()` 严格配对，保护变换和样式状态 |
| **性能** | 离屏缓存静态内容、限制重绘区域、避免 paintEvent 中创建对象 |
| **高 DPI** | 离屏 QPixmap 需乘 `devicePixelRatioF()` 并设置 `setDevicePixelRatio` |
