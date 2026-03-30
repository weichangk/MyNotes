# QGraphicsView 框架

## 概述

**Graphics View 框架**是 Qt 提供的基于 Item 的 2D 图形场景管理系统，专为处理大量图形项、支持交互操作（选择/拖拽/缩放）和高效渲染而设计。它由三个核心类组成：

```
┌─────────────────────────────────────────────────┐
│  QGraphicsView（视图）                            │
│  ┌─────────────────────────────────────────────┐ │
│  │  QGraphicsScene（场景）                       │ │
│  │  ┌──────┐  ┌──────┐  ┌──────────────────┐  │ │
│  │  │ Item │  │ Item │  │ QGraphicsItem    │  │ │
│  │  │  A   │  │  B   │  │ （图形项）         │  │ │
│  │  └──────┘  └──────┘  └──────────────────┘  │ │
│  └─────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────┘
```

| 类 | 角色 | 类比 |
|---|---|---|
| `QGraphicsScene` | 场景——数据容器 | 画布/世界 |
| `QGraphicsView` | 视图——可视化窗口 | 摄像机/观察窗 |
| `QGraphicsItem` | 图形项——场景中的元素 | 画布上的对象 |

**与直接 QPainter 绑定的对比**：

| 特性 | QPainter 直接绘制 | Graphics View 框架 |
|---|---|---|
| 管理方式 | 自行管理所有绘制逻辑 | 框架管理图形项的绘制/事件/碰撞 |
| 交互 | 自行实现命中测试 | 内建选择、拖拽、焦点管理 |
| 性能 | 简单场景高效 | BSP 索引，大量图形项时高效 |
| 适用场景 | 自定义控件绘制 | 图形编辑器、流程图、地图、游戏 |

---

## 1. QGraphicsScene —— 场景

### 1.1 创建与基本操作

```cpp
// 创建场景，指定逻辑坐标范围
QGraphicsScene *scene = new QGraphicsScene(this);
scene->setSceneRect(-500, -500, 1000, 1000); // x, y, w, h

// 不设置 sceneRect 时，框架自动根据所有 item 的包围盒计算
// 显式设置可避免添加/移除 item 时反复计算

// 设置背景
scene->setBackgroundBrush(QColor(240, 240, 240));
scene->setBackgroundBrush(QBrush(QPixmap(":/bg/grid.png")));

// 设置前景（覆盖在所有 item 之上）
scene->setForegroundBrush(QColor(0, 0, 0, 30)); // 半透明遮罩
```

### 1.2 添加图形项

```cpp
// 方式 1：使用便捷方法（返回对应的 Item 指针）
QGraphicsRectItem *rect = scene->addRect(0, 0, 100, 60,
                                         QPen(Qt::blue, 2),
                                         QBrush(Qt::cyan));

QGraphicsEllipseItem *ellipse = scene->addEllipse(-30, -30, 60, 60,
                                                   QPen(Qt::red),
                                                   QBrush(Qt::yellow));

QGraphicsTextItem *text = scene->addText("Hello", QFont("Arial", 16));

QGraphicsLineItem *line = scene->addLine(0, 0, 200, 100, QPen(Qt::green, 3));

QGraphicsPixmapItem *pixItem = scene->addPixmap(QPixmap(":/icons/star.png"));

QGraphicsPathItem *pathItem = scene->addPath(myPath, QPen(Qt::black), QBrush(Qt::gray));

// 方式 2：先创建 Item，再添加到场景
auto *item = new QGraphicsRectItem(0, 0, 80, 50);
item->setBrush(Qt::magenta);
scene->addItem(item);
```

### 1.3 查找与遍历

```cpp
// 获取所有 item
QList<QGraphicsItem *> allItems = scene->items();

// 按区域查找（默认 BSP 索引加速）
QList<QGraphicsItem *> inRect = scene->items(QRectF(0, 0, 200, 200));

// 按点查找（命中测试）
QList<QGraphicsItem *> atPoint = scene->items(QPointF(100, 50));

// 获取选中的 item
QList<QGraphicsItem *> selected = scene->selectedItems();

// 获取获得焦点的 item
QGraphicsItem *focused = scene->focusItem();

// 获取指定位置最上层的 item
QGraphicsItem *topItem = scene->itemAt(QPointF(100, 50), QTransform());

// 按 Z 值排序（前到后）
QList<QGraphicsItem *> sorted = scene->items(Qt::AscendingOrder);
```

### 1.4 删除

```cpp
// 从场景中移除（不销毁对象，调用者负责 delete）
scene->removeItem(item);
delete item;

// 清空场景（销毁所有 item）
scene->clear();
```

### 1.5 BSP 索引

场景默认使用 **BSP 树（Binary Space Partitioning）** 对图形项进行空间索引，使碰撞检测和区域查询在大量 item 时依然高效：

```cpp
// 索引方式
scene->setItemIndexMethod(QGraphicsScene::BspTreeIndex); // 默认，适合大量静态/半静态项
scene->setItemIndexMethod(QGraphicsScene::NoIndex);       // 无索引，适合频繁移动的少量项

// BSP 树深度（默认 0 = 自动，通常够用）
scene->setBspTreeDepth(6); // 手动设置
```

| 索引方式 | 适用场景 |
|---|---|
| `BspTreeIndex` | item 数量大、位置相对稳定 |
| `NoIndex` | item 数量少或频繁大量移动 |

### 1.6 场景信号

```cpp
// 选择变化
connect(scene, &QGraphicsScene::selectionChanged, this, [scene]() {
    auto selected = scene->selectedItems();
    qDebug() << "选中了" << selected.size() << "个 item";
});

// 焦点变化
connect(scene, &QGraphicsScene::focusItemChanged,
        this, [](QGraphicsItem *newFocus, QGraphicsItem *oldFocus, Qt::FocusReason reason) {
    Q_UNUSED(reason)
    qDebug() << "焦点从" << oldFocus << "转到" << newFocus;
});

// 场景矩形变化
connect(scene, &QGraphicsScene::sceneRectChanged, this, [](const QRectF &rect) {
    qDebug() << "场景范围变化:" << rect;
});
```

---

## 2. QGraphicsView —— 视图

### 2.1 创建与关联场景

```cpp
QGraphicsView *view = new QGraphicsView(scene, this);

// 或先创建再关联
QGraphicsView *view2 = new QGraphicsView(this);
view2->setScene(scene);

// 一个场景可以关联多个视图（多视角预览）
QGraphicsView *minimap = new QGraphicsView(scene, this);
minimap->setFixedSize(200, 150);
minimap->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
```

### 2.2 渲染设置

```cpp
// 抗锯齿
view->setRenderHint(QPainter::Antialiasing);
view->setRenderHint(QPainter::SmoothPixmapTransform);
view->setRenderHint(QPainter::TextAntialiasing);

// 视口更新模式
view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);  // 默认，智能判断
view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);   // 全部重绘（半透明场景必需）
view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);// 最小更新
view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
view->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);     // 手动触发

// 缓存背景
view->setCacheMode(QGraphicsView::CacheBackground); // 背景不变时缓存
view->setCacheMode(QGraphicsView::CacheNone);       // 无缓存（默认）

// OpenGL 视口（硬件加速）
view->setViewport(new QOpenGLWidget());
```

**更新模式选择**：

| 模式 | 场景 |
|---|---|
| `SmartViewportUpdate` | 通用，框架自动判断重绘区域 |
| `FullViewportUpdate` | 场景有半透明/重叠元素时，避免残影 |
| `MinimalViewportUpdate` | 少量 item 变化时最快 |
| `NoViewportUpdate` | 完全自主控制刷新时机 |

### 2.3 滚动与适配

```cpp
// 自动适配场景内容
view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

// 居中显示某个 item
view->centerOn(item);
view->centerOn(100, 200); // 或指定场景坐标

// 确保某个 item 可见
view->ensureVisible(item);
view->ensureVisible(QRectF(80, 80, 200, 200));

// 滚动条策略
view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

// 拖拽滚动模式
view->setDragMode(QGraphicsView::ScrollHandDrag); // 手型拖拽滚动
view->setDragMode(QGraphicsView::RubberBandDrag); // 矩形框选
view->setDragMode(QGraphicsView::NoDrag);          // 无拖拽
```

### 2.4 视图变换（缩放/旋转）

```cpp
// 缩放
view->scale(1.5, 1.5);     // 放大 1.5 倍
view->scale(0.5, 0.5);     // 缩小

// 旋转
view->rotate(30);           // 顺时针旋转 30°

// 重置变换
view->resetTransform();

// 获取/设置完整变换矩阵
QTransform t = view->transform();
view->setTransform(t);

// 鼠标滚轮缩放（常见实现）
class ZoomableView : public QGraphicsView {
protected:
    void wheelEvent(QWheelEvent *event) override {
        const double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;

        // 以鼠标位置为缩放中心
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        scale(factor, factor);
        setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    }
};
```

### 2.5 坐标转换

Graphics View 框架有**三套坐标系**，视图提供了它们之间的转换：

```
Item 坐标 ──mapToScene()──► Scene 坐标 ──view.mapFromScene()──► View/Widget 坐标
Item 坐标 ◄──mapFromScene()── Scene 坐标 ◄──view.mapToScene()── View/Widget 坐标
```

```cpp
// View 像素坐标 → Scene 坐标
QPointF scenePos = view->mapToScene(event->pos());

// Scene 坐标 → View 像素坐标
QPoint viewPos = view->mapFromScene(QPointF(100, 200));

// Item 坐标 → Scene 坐标
QPointF scenePos2 = item->mapToScene(0, 0);

// Scene 坐标 → Item 坐标（本地）
QPointF localPos = item->mapFromScene(scenePos2);

// Item A 坐标 → Item B 坐标
QPointF posInB = itemA->mapToItem(itemB, QPointF(10, 20));

// 获取可见的场景矩形
QRectF visibleArea = view->mapToScene(view->viewport()->rect()).boundingRect();
```

### 2.6 自定义背景与前景

```cpp
class MyView : public QGraphicsView {
protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override {
        // 绘制网格背景
        painter->fillRect(rect, QColor(45, 45, 48));

        QPen gridPen(QColor(60, 60, 65), 0);
        painter->setPen(gridPen);

        qreal gridSize = 20;
        qreal left = int(rect.left()) - (int(rect.left()) % int(gridSize));
        qreal top = int(rect.top()) - (int(rect.top()) % int(gridSize));

        for (qreal x = left; x < rect.right(); x += gridSize)
            painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
        for (qreal y = top; y < rect.bottom(); y += gridSize)
            painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }

    void drawForeground(QPainter *painter, const QRectF &rect) override {
        Q_UNUSED(rect)
        // 绘制十字准心
        painter->setPen(QPen(QColor(255, 80, 80, 100), 0));
        painter->drawLine(QPointF(-10000, 0), QPointF(10000, 0));
        painter->drawLine(QPointF(0, -10000), QPointF(0, 10000));
    }
};
```

---

## 3. QGraphicsItem —— 图形项

### 3.1 内建图形项

| 类 | 形状 |
|---|---|
| `QGraphicsRectItem` | 矩形 |
| `QGraphicsEllipseItem` | 椭圆/圆 |
| `QGraphicsLineItem` | 直线 |
| `QGraphicsPolygonItem` | 多边形 |
| `QGraphicsPathItem` | 路径（任意形状） |
| `QGraphicsPixmapItem` | 图片 |
| `QGraphicsTextItem` | 富文本（可编辑） |
| `QGraphicsSimpleTextItem` | 简单文本（更快） |
| `QGraphicsProxyWidget` | 嵌入 QWidget |
| `QGraphicsItemGroup` | 分组 |

```cpp
// 矩形
auto *rect = new QGraphicsRectItem(0, 0, 120, 80);
rect->setPen(QPen(Qt::darkBlue, 2));
rect->setBrush(QColor(100, 150, 255, 180));
rect->setPos(50, 30);

// 椭圆
auto *ellipse = new QGraphicsEllipseItem(-40, -40, 80, 80);
ellipse->setBrush(Qt::yellow);
ellipse->setStartAngle(30 * 16);    // 1/16 度
ellipse->setSpanAngle(270 * 16);

// 多边形
QPolygonF triangle;
triangle << QPointF(0, -50) << QPointF(50, 40) << QPointF(-50, 40);
auto *poly = new QGraphicsPolygonItem(triangle);
poly->setBrush(QColor(255, 100, 100));

// 路径
QPainterPath path;
path.moveTo(0, 0);
path.cubicTo(30, -60, 70, -60, 100, 0);
path.cubicTo(70, 60, 30, 60, 0, 0);
auto *pathItem = new QGraphicsPathItem(path);

// 图片
auto *pixItem = new QGraphicsPixmapItem(QPixmap(":/images/star.png"));
pixItem->setTransformationMode(Qt::SmoothTransformation);
pixItem->setOffset(-pixItem->pixmap().width() / 2.0,
                   -pixItem->pixmap().height() / 2.0); // 居中原点

// 富文本（可编辑）
auto *textItem = new QGraphicsTextItem("Hello <b>World</b>");
textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
textItem->setDefaultTextColor(Qt::white);
textItem->setFont(QFont("Microsoft YaHei", 14));

// 简单文本（不可编辑，绘制更快）
auto *simpleText = new QGraphicsSimpleTextItem("Fast Text");
simpleText->setBrush(Qt::white);
```

### 3.2 通用属性

```cpp
QGraphicsRectItem *item = scene->addRect(0, 0, 100, 60);

// ── 位置 ──
item->setPos(200, 150);              // 在场景中的位置（父坐标系）
QPointF pos = item->pos();
QPointF scenePos = item->scenePos();  // 场景绝对坐标

// ── 变换 ──
item->setRotation(45);               // 旋转（度）
item->setScale(1.5);                 // 缩放
item->setTransformOriginPoint(50, 30); // 变换中心（本地坐标）
item->setTransform(QTransform().shear(0.3, 0));

// ── Z 值（层叠顺序）──
item->setZValue(10);                  // 值大的在上面
qreal z = item->zValue();

// ── 可见性 / 启用状态 ──
item->setVisible(false);
item->setEnabled(false);              // 禁用后不接收事件
item->setOpacity(0.7);               // 透明度 0.0~1.0

// ── 标识 ──
item->setData(0, QVariant("myRect")); // 存储自定义数据（key-value）
QVariant val = item->data(0);

// ── 工具提示 / 光标 ──
item->setToolTip("这是一个矩形");
item->setCursor(Qt::PointingHandCursor);
```

### 3.3 交互标志（Flags）

```cpp
// 可选中
item->setFlag(QGraphicsItem::ItemIsSelectable);

// 可拖拽移动
item->setFlag(QGraphicsItem::ItemIsMovable);

// 可获得焦点
item->setFlag(QGraphicsItem::ItemIsFocusable);

// 位置变化时发送通知
item->setFlag(QGraphicsItem::ItemSendsGeometryChanges);

// 忽略 view 的变换（始终保持固定大小，如标注/工具图标）
item->setFlag(QGraphicsItem::ItemIgnoresTransformations);

// 裁剪子 item 到自身边界内
item->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

// 常见组合
item->setFlags(QGraphicsItem::ItemIsSelectable |
               QGraphicsItem::ItemIsMovable |
               QGraphicsItem::ItemSendsGeometryChanges);
```

### 3.4 碰撞检测

```cpp
// item 是否与另一个 item 碰撞
bool collides = itemA->collidesWithItem(itemB);

// item 是否与路径/形状碰撞
QPainterPath testPath;
testPath.addRect(0, 0, 100, 100);
bool collidesPath = item->collidesWithPath(testPath);

// 获取与某个 item 碰撞的所有 item
QList<QGraphicsItem *> colliders = item->collidingItems();

// 碰撞形状（默认用 boundingRect，可重写 shape() 精确匹配）
QPainterPath itemShape = item->shape();
QRectF bounds = item->boundingRect();
```

### 3.5 父子关系与分组

```cpp
// 设置父子关系
auto *parent = new QGraphicsRectItem(0, 0, 200, 150);
auto *child = new QGraphicsEllipseItem(20, 20, 50, 50, parent);
// child 的坐标相对于 parent
// child 随 parent 移动/变换

// 获取父/子
QGraphicsItem *p = child->parentItem();
QList<QGraphicsItem *> children = parent->childItems();

// 分组
QGraphicsItemGroup *group = scene->createItemGroup({item1, item2, item3});
group->setFlag(QGraphicsItem::ItemIsMovable);
group->setPos(100, 100);

// 取消分组
scene->destroyItemGroup(group);
// item1, item2, item3 回到原来的父（场景顶层）
```

---

## 4. 自定义 QGraphicsItem

### 4.1 最小自定义 Item

必须重写 `boundingRect()` 和 `paint()`：

```cpp
class CircleItem : public QGraphicsItem
{
public:
    CircleItem(qreal radius, QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent), m_radius(radius) {}

    // 必须重写：返回 item 的包围盒（本地坐标）
    QRectF boundingRect() const override {
        qreal penWidth = 2;
        return QRectF(-m_radius - penWidth / 2, -m_radius - penWidth / 2,
                      m_radius * 2 + penWidth, m_radius * 2 + penWidth);
    }

    // 必须重写：绘制 item
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override {
        Q_UNUSED(widget)

        painter->setRenderHint(QPainter::Antialiasing);

        // 选中时高亮
        if (option->state & QStyle::State_Selected) {
            painter->setPen(QPen(Qt::yellow, 2, Qt::DashLine));
        } else {
            painter->setPen(QPen(Qt::darkBlue, 2));
        }

        painter->setBrush(m_color);
        painter->drawEllipse(QPointF(0, 0), m_radius, m_radius);
    }

    void setColor(const QColor &color) {
        m_color = color;
        update(); // 触发重绘
    }

private:
    qreal m_radius;
    QColor m_color = QColor(100, 180, 255);
};
```

### 4.2 精确碰撞形状

```cpp
class CircleItem : public QGraphicsItem {
    // ...

    // 重写 shape() 返回精确轮廓（用于碰撞检测和点击命中）
    QPainterPath shape() const override {
        QPainterPath path;
        path.addEllipse(QPointF(0, 0), m_radius, m_radius);
        return path;
    }
};
```

> 默认 `shape()` 返回 `boundingRect()` 的矩形路径。对于非矩形 item，重写 `shape()` 可以实现精确点击命中和碰撞检测。

### 4.3 处理事件

```cpp
class InteractiveBox : public QGraphicsRectItem
{
public:
    InteractiveBox(qreal w, qreal h) : QGraphicsRectItem(0, 0, w, h) {
        setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges |
                 ItemIsFocusable);
        setAcceptHoverEvents(true);
    }

protected:
    // 鼠标按下
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            m_pressColor = brush().color();
            setBrush(QColor(255, 200, 100));
        }
        QGraphicsRectItem::mousePressEvent(event); // 保留拖拽功能
    }

    // 鼠标释放
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        setBrush(m_pressColor);
        QGraphicsRectItem::mouseReleaseEvent(event);
    }

    // 鼠标悬停进入/离开
    void hoverEnterEvent(QGraphicsSceneHoverEvent *) override {
        setPen(QPen(Qt::red, 3));
        setCursor(Qt::PointingHandCursor);
        update();
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *) override {
        setPen(QPen(Qt::black, 1));
        setCursor(Qt::ArrowCursor);
        update();
    }

    // 键盘事件（需要 ItemIsFocusable）
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Delete) {
            scene()->removeItem(this);
            delete this;
            return;
        }
        qreal step = 5;
        switch (event->key()) {
        case Qt::Key_Left:  moveBy(-step, 0); break;
        case Qt::Key_Right: moveBy(step, 0);  break;
        case Qt::Key_Up:    moveBy(0, -step);  break;
        case Qt::Key_Down:  moveBy(0, step);   break;
        }
    }

    // 几何变化通知（ItemSendsGeometryChanges）
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
        if (change == ItemPositionChange && scene()) {
            // 限制在场景范围内
            QPointF newPos = value.toPointF();
            QRectF sceneRect = scene()->sceneRect();
            newPos.setX(qBound(sceneRect.left(), newPos.x(),
                               sceneRect.right() - rect().width()));
            newPos.setY(qBound(sceneRect.top(), newPos.y(),
                               sceneRect.bottom() - rect().height()));
            return newPos;
        }
        return QGraphicsRectItem::itemChange(change, value);
    }

private:
    QColor m_pressColor;
};
```

### 4.4 itemChange 常用通知

| Change 枚举值 | 触发时机 |
|---|---|
| `ItemPositionChange` | 位置即将变化（可修改返回值拦截） |
| `ItemPositionHasChanged` | 位置已变化 |
| `ItemRotationChange` | 旋转即将变化 |
| `ItemScaleChange` | 缩放即将变化 |
| `ItemSelectedChange` | 选中状态即将变化 |
| `ItemSelectedHasChanged` | 选中状态已变化 |
| `ItemVisibleChange` | 可见性即将变化 |
| `ItemParentChange` | 父 item 即将变化 |
| `ItemZValueChange` | Z 值即将变化 |

---

## 5. 嵌入 QWidget —— QGraphicsProxyWidget

将常规 QWidget 嵌入到 Graphics View 场景中，使其拥有 item 的所有特性（缩放/旋转/透明度）：

```cpp
QGraphicsScene *scene = new QGraphicsScene;

// 方式 1：addWidget 便捷方法
QPushButton *button = new QPushButton("Click Me");
QGraphicsProxyWidget *proxy = scene->addWidget(button);
proxy->setPos(50, 50);
proxy->setRotation(15);
proxy->setScale(1.5);

// 方式 2：手动创建
QGraphicsProxyWidget *proxy2 = new QGraphicsProxyWidget;
QSlider *slider = new QSlider(Qt::Horizontal);
slider->setRange(0, 100);
proxy2->setWidget(slider);
proxy2->setPos(200, 100);
scene->addItem(proxy2);

// 嵌入复杂控件
QCalendarWidget *calendar = new QCalendarWidget;
QGraphicsProxyWidget *calProxy = scene->addWidget(calendar);
calProxy->setPos(-200, -100);
calProxy->setOpacity(0.9);

// 信号槽仍然正常工作
connect(button, &QPushButton::clicked, []() {
    qDebug() << "按钮被点击";
});
```

> **注意**：嵌入的 QWidget 旋转/缩放时可能影响外观（如字体变形），适合简单控件，不适合动态内容很密集的控件。

---

## 6. 动画

### 6.1 QGraphicsItemAnimation（旧方式，了解即可）

```cpp
// 基于 QTimeLine 的简单动画（Qt 5 中已不推荐）
QTimeLine *timeline = new QTimeLine(2000);
timeline->setFrameRange(0, 100);

QGraphicsItemAnimation *anim = new QGraphicsItemAnimation;
anim->setItem(item);
anim->setTimeLine(timeline);
anim->setPosAt(0.0, QPointF(0, 0));
anim->setPosAt(0.5, QPointF(200, 100));
anim->setPosAt(1.0, QPointF(400, 0));
anim->setRotationAt(1.0, 360);

timeline->start();
```

### 6.2 QPropertyAnimation（推荐）

`QGraphicsObject`（继承自 `QGraphicsItem` + `QObject`）支持 Qt 属性动画系统：

```cpp
class AnimatedCircle : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius)

public:
    AnimatedCircle(QGraphicsItem *parent = nullptr) : QGraphicsObject(parent) {}

    QColor color() const { return m_color; }
    void setColor(const QColor &c) { m_color = c; update(); }

    qreal radius() const { return m_radius; }
    void setRadius(qreal r) { prepareGeometryChange(); m_radius = r; update(); }

    QRectF boundingRect() const override {
        return QRectF(-m_radius, -m_radius, m_radius * 2, m_radius * 2);
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override {
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(Qt::NoPen);
        p->setBrush(m_color);
        p->drawEllipse(QPointF(0, 0), m_radius, m_radius);
    }

private:
    QColor m_color = QColor(60, 150, 255);
    qreal m_radius = 30;
};

// 使用动画
auto *circle = new AnimatedCircle;
scene->addItem(circle);

// 位置动画
auto *posAnim = new QPropertyAnimation(circle, "pos");
posAnim->setDuration(2000);
posAnim->setStartValue(QPointF(0, 0));
posAnim->setEndValue(QPointF(300, 200));
posAnim->setEasingCurve(QEasingCurve::OutBounce);

// 颜色动画
auto *colorAnim = new QPropertyAnimation(circle, "color");
colorAnim->setDuration(2000);
colorAnim->setStartValue(QColor(60, 150, 255));
colorAnim->setEndValue(QColor(255, 80, 80));

// 半径动画
auto *radiusAnim = new QPropertyAnimation(circle, "radius");
radiusAnim->setDuration(1000);
radiusAnim->setStartValue(30.0);
radiusAnim->setEndValue(60.0);
radiusAnim->setEasingCurve(QEasingCurve::InOutQuad);

// 并行执行
auto *group = new QParallelAnimationGroup;
group->addAnimation(posAnim);
group->addAnimation(colorAnim);
group->addAnimation(radiusAnim);
group->start();
```

### 6.3 内建可动画属性

`QGraphicsObject` 直接支持以下 Q_PROPERTY 的动画：

| 属性名 | 类型 | 说明 |
|---|---|---|
| `pos` | `QPointF` | 位置 |
| `x` | `qreal` | X 坐标 |
| `y` | `qreal` | Y 坐标 |
| `rotation` | `qreal` | 旋转角度 |
| `scale` | `qreal` | 缩放 |
| `opacity` | `qreal` | 透明度 |

```cpp
// 直接对内建属性做动画（无需自定义子类）
auto *obj = new QGraphicsRectItem(0, 0, 80, 50); // 不行，不是 QObject

// ✅ 用 QGraphicsObject 子类（或用内建的 QGraphicsWidget）
// QGraphicsWidget 也继承了 QGraphicsObject
auto *widget = new QGraphicsWidget;
auto *posAnim = new QPropertyAnimation(widget, "pos");
posAnim->setDuration(1000);
posAnim->setEndValue(QPointF(200, 100));
posAnim->start(QAbstractAnimation::DeleteWhenStopped);
```

---

## 7. 实战示例

### 7.1 简单流程图编辑器

```cpp
// ──── 节点 Item ────
class FlowNode : public QGraphicsRectItem
{
public:
    FlowNode(const QString &label, QGraphicsItem *parent = nullptr)
        : QGraphicsRectItem(0, 0, 140, 60, parent), m_label(label) {
        setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
        setBrush(QColor(70, 130, 220));
        setPen(QPen(QColor(50, 100, 180), 2));
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *w) override {
        // 选中高亮
        if (isSelected()) {
            setPen(QPen(QColor(255, 200, 50), 3));
        } else {
            setPen(QPen(QColor(50, 100, 180), 2));
        }
        QGraphicsRectItem::paint(p, opt, w);

        // 文字
        p->setPen(Qt::white);
        p->setFont(QFont("Microsoft YaHei", 11));
        p->drawText(rect(), Qt::AlignCenter, m_label);
    }

    // 连接点（四个方向中心）
    QPointF connectionPoint(int direction) const {
        QRectF r = sceneBoundingRect();
        switch (direction) {
        case 0: return QPointF(r.center().x(), r.top());     // 上
        case 1: return QPointF(r.right(), r.center().y());   // 右
        case 2: return QPointF(r.center().x(), r.bottom());  // 下
        case 3: return QPointF(r.left(), r.center().y());    // 左
        }
        return r.center();
    }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
        if (change == ItemPositionHasChanged) {
            emit positionChanged();
        }
        return QGraphicsRectItem::itemChange(change, value);
    }

    // 借助 scene 的 signal 转发（简化版使用 QGraphicsObject 更合适）
    void emit positionChanged() {
        // 更新所有连接线
        for (auto *edge : m_edges) edge->adjust();
    }

public:
    QList<class FlowEdge *> m_edges;

private:
    QString m_label;
};

// ──── 连接线 Item ────
class FlowEdge : public QGraphicsLineItem
{
public:
    FlowEdge(FlowNode *from, int fromDir, FlowNode *to, int toDir)
        : m_from(from), m_fromDir(fromDir), m_to(to), m_toDir(toDir) {
        setPen(QPen(QColor(180, 180, 180), 2));
        setZValue(-1);
        from->m_edges.append(this);
        to->m_edges.append(this);
        adjust();
    }

    void adjust() {
        QPointF p1 = m_from->connectionPoint(m_fromDir);
        QPointF p2 = m_to->connectionPoint(m_toDir);
        setLine(QLineF(p1, p2));
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override {
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(pen());
        p->drawLine(line());

        // 箭头
        QLineF l = line();
        double angle = std::atan2(-l.dy(), l.dx());
        QPointF tip = l.p2();
        QPointF arrow1 = tip - QPointF(cos(angle - M_PI / 6) * 12,
                                       -sin(angle - M_PI / 6) * 12);
        QPointF arrow2 = tip - QPointF(cos(angle + M_PI / 6) * 12,
                                       -sin(angle + M_PI / 6) * 12);
        p->setBrush(pen().color());
        p->drawPolygon(QPolygonF({tip, arrow1, arrow2}));
    }

private:
    FlowNode *m_from, *m_to;
    int m_fromDir, m_toDir;
};

// ──── 组装 ────
QGraphicsScene *scene = new QGraphicsScene;
auto *start = new FlowNode("开始");
auto *process = new FlowNode("处理数据");
auto *end = new FlowNode("结束");

start->setPos(0, 0);
process->setPos(0, 120);
end->setPos(0, 240);

scene->addItem(start);
scene->addItem(process);
scene->addItem(end);
scene->addItem(new FlowEdge(start, 2, process, 0));  // 开始→处理（下→上）
scene->addItem(new FlowEdge(process, 2, end, 0));     // 处理→结束

QGraphicsView *view = new QGraphicsView(scene);
view->setRenderHint(QPainter::Antialiasing);
```

### 7.2 可拖拽 + 缩放 + 框选的画布

```cpp
class EditorView : public QGraphicsView
{
    Q_OBJECT
public:
    EditorView(QGraphicsScene *scene, QWidget *parent = nullptr)
        : QGraphicsView(scene, parent) {
        setRenderHint(QPainter::Antialiasing);
        setDragMode(RubberBandDrag);
        setTransformationAnchor(AnchorUnderMouse);
        setResizeAnchor(AnchorUnderMouse);
        setViewportUpdateMode(SmartViewportUpdate);
    }

protected:
    void wheelEvent(QWheelEvent *event) override {
        double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;

        // 限制缩放范围
        QTransform t = transform();
        qreal currentScale = t.m11();
        if ((currentScale > 5.0 && factor > 1) ||
            (currentScale < 0.1 && factor < 1))
            return;

        scale(factor, factor);
    }

    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Space) {
            setDragMode(ScrollHandDrag);    // 空格键切换到拖拽模式
        }
        QGraphicsView::keyPressEvent(event);
    }

    void keyReleaseEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Space) {
            setDragMode(RubberBandDrag);    // 松开恢复框选模式
        }
        QGraphicsView::keyReleaseEvent(event);
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::MiddleButton) {
            // 中键拖拽
            setDragMode(ScrollHandDrag);
            QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton,
                             Qt::LeftButton, event->modifiers());
            QGraphicsView::mousePressEvent(&fake);
            return;
        }
        QGraphicsView::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::MiddleButton) {
            setDragMode(RubberBandDrag);
        }
        QGraphicsView::mouseReleaseEvent(event);
    }
};
```

### 7.3 带手柄的可调整大小 Item

```cpp
class ResizableRect : public QGraphicsRectItem
{
    enum HandlePosition { TopLeft, TopRight, BottomRight, BottomLeft };

public:
    ResizableRect(const QRectF &rect, QGraphicsItem *parent = nullptr)
        : QGraphicsRectItem(rect, parent) {
        setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
        setAcceptHoverEvents(true);
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *w) override {
        QGraphicsRectItem::paint(p, opt, w);

        if (isSelected()) {
            // 绘制 4 个手柄
            p->setPen(Qt::NoPen);
            p->setBrush(QColor(0, 120, 255));
            for (const QRectF &handle : handleRects())
                p->drawRect(handle);
        }
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        m_resizing = false;
        if (isSelected()) {
            QList<QRectF> handles = handleRects();
            for (int i = 0; i < handles.size(); ++i) {
                if (handles[i].contains(event->pos())) {
                    m_resizing = true;
                    m_activeHandle = (HandlePosition)i;
                    m_pressRect = rect();
                    m_pressPos = event->pos();
                    return;
                }
            }
        }
        QGraphicsRectItem::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
        if (m_resizing) {
            QPointF delta = event->pos() - m_pressPos;
            QRectF newRect = m_pressRect;

            switch (m_activeHandle) {
            case TopLeft:     newRect.setTopLeft(newRect.topLeft() + delta); break;
            case TopRight:    newRect.setTopRight(newRect.topRight() + delta); break;
            case BottomRight: newRect.setBottomRight(newRect.bottomRight() + delta); break;
            case BottomLeft:  newRect.setBottomLeft(newRect.bottomLeft() + delta); break;
            }

            if (newRect.width() > 20 && newRect.height() > 20) {
                prepareGeometryChange();
                setRect(newRect.normalized());
            }
            return;
        }
        QGraphicsRectItem::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        m_resizing = false;
        QGraphicsRectItem::mouseReleaseEvent(event);
    }

private:
    QList<QRectF> handleRects() const {
        qreal s = 8;
        QRectF r = rect();
        return {
            QRectF(r.topLeft() - QPointF(s/2, s/2), QSizeF(s, s)),
            QRectF(r.topRight() - QPointF(s/2, s/2), QSizeF(s, s)),
            QRectF(r.bottomRight() - QPointF(s/2, s/2), QSizeF(s, s)),
            QRectF(r.bottomLeft() - QPointF(s/2, s/2), QSizeF(s, s)),
        };
    }

    bool m_resizing = false;
    HandlePosition m_activeHandle = BottomRight;
    QRectF m_pressRect;
    QPointF m_pressPos;
};
```

---

## 8. 性能优化

### 8.1 boundingRect() 准确性

```cpp
// ❌ boundingRect 过大：导致不必要的重绘
QRectF boundingRect() const override {
    return QRectF(-1000, -1000, 2000, 2000); // 太大了！
}

// ✅ 精确的 boundingRect：只包围实际绘制区域
QRectF boundingRect() const override {
    qreal pw = m_penWidth / 2.0;
    return QRectF(-m_radius - pw, -m_radius - pw,
                  m_radius * 2 + pw * 2, m_radius * 2 + pw * 2);
}
```

### 8.2 prepareGeometryChange()

```cpp
// 当 item 的 boundingRect 或 shape 将发生变化时，必须先调用
void setRadius(qreal r) {
    prepareGeometryChange(); // ✅ 通知场景更新索引
    m_radius = r;
    update();
}

// 如果只是外观变化（颜色等），boundingRect 不变，只需 update()
void setColor(const QColor &c) {
    m_color = c;
    update(); // ✅ 触发重绘，无需 prepareGeometryChange
}
```

### 8.3 优化建议

| 优化 | 说明 |
|---|---|
| `boundingRect()` 尽量精确 | 过大会导致大量无效重绘 |
| `shape()` 适度简化 | 用简单形状近似复杂轮廓，减少碰撞计算开销 |
| 批量更新用 `setVisible()` | 不可见 item 不参与绘制和事件 |
| `ItemHasNoContents` | 只作为分组容器、不绘制内容的 item，设置此 flag |
| `setCacheMode(DeviceCoordinateCache)` | 复杂 item 缓存为位图（位置变化不重绘内容） |
| `setCacheMode(ItemCoordinateCache)` | 在 item 坐标系缓存（缩放时重绘，位移旋转不重绘） |
| `setViewportUpdateMode(MinimalViewportUpdate)` | 少量更新时效率最高 |
| 使用 `QGraphicsView::CacheBackground` | 背景复杂但不常变时 |
| `setItemIndexMethod(NoIndex)` | item 数量<200 且频繁运动时 |

```cpp
// 缓存模式示例
item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

// 对于需要缩放的内容（如地图），用 ItemCoordinateCache + logicalCacheSize
item->setCacheMode(QGraphicsItem::ItemCoordinateCache,
                   QSize(200, 200)); // 缓存分辨率
```

---

## 9. 三套坐标系详解

```
┌──────────────────────────────────────────────────────────────┐
│                       Item 坐标系                             │
│  每个 item 有自己的本地坐标系                                    │
│  原点通常在 item 中心或左上角                                    │
│  boundingRect()、shape()、paint() 都用 item 坐标              │
│                                                              │
│  ──── mapToScene() / mapFromScene() ────                     │
│                                                              │
│                       Scene 坐标系                            │
│  全局坐标系，所有 item 共享                                      │
│  item->scenePos() 返回 item 原点在场景中的位置                    │
│  scene->items(QPointF) 等查询用场景坐标                          │
│                                                              │
│  ──── view.mapToScene() / view.mapFromScene() ────           │
│                                                              │
│                       View 坐标系                             │
│  即 QGraphicsView（QWidget）的像素坐标                          │
│  左上角 (0,0)，等同于通常的 widget 坐标系                         │
│  鼠标事件 event->pos() 返回 view 坐标                           │
└──────────────────────────────────────────────────────────────┘
```

完整转换示例：

```cpp
// 场景中的鼠标点击处理
void MyView::mousePressEvent(QMouseEvent *event) {
    // View 坐标 → Scene 坐标
    QPointF scenePos = mapToScene(event->pos());
    qDebug() << "场景坐标:" << scenePos;

    // 获取点击位置的 item
    QGraphicsItem *item = scene()->itemAt(scenePos, transform());
    if (item) {
        // Scene 坐标 → Item 本地坐标
        QPointF localPos = item->mapFromScene(scenePos);
        qDebug() << "在 item 内的本地坐标:" << localPos;
    }

    QGraphicsView::mousePressEvent(event);
}
```

---

## 10. 导出场景为图片

```cpp
void exportSceneToImage(QGraphicsScene *scene, const QString &path, int width)
{
    QRectF sceneRect = scene->sceneRect();
    qreal aspect = sceneRect.height() / sceneRect.width();
    int height = int(width * aspect);

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter, QRectF(0, 0, width, height), sceneRect);
    painter.end();

    image.save(path);
}

// 导出 PDF
void exportSceneToPDF(QGraphicsScene *scene, const QString &path)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPainter painter(&printer);
    scene->render(&painter);
    painter.end();
}

// 导出 SVG
#include <QSvgGenerator>
void exportSceneToSVG(QGraphicsScene *scene, const QString &path)
{
    QSvgGenerator generator;
    generator.setFileName(path);
    generator.setSize(scene->sceneRect().size().toSize());
    generator.setViewBox(scene->sceneRect());

    QPainter painter(&generator);
    scene->render(&painter);
    painter.end();
}
```

---

## 11. API 速查

### 11.1 QGraphicsScene

| 方法 | 说明 |
|---|---|
| `addItem(item)` | 添加 item |
| `addRect/Ellipse/Line/...` | 添加基本图形 |
| `removeItem(item)` | 移除 item（不删除） |
| `clear()` | 清空并删除所有 item |
| `items()` / `items(rect)` | 获取所有/区域内 item |
| `itemAt(pos, transform)` | 获取指定位置最上层 item |
| `selectedItems()` | 获取选中 item |
| `setSceneRect(rect)` | 设置场景范围 |
| `setBackgroundBrush(brush)` | 设置背景 |
| `setForegroundBrush(brush)` | 设置前景 |
| `render(painter, target, source)` | 渲染到 QPainter |
| `setItemIndexMethod(method)` | 设置索引方式 |

### 11.2 QGraphicsView

| 方法 | 说明 |
|---|---|
| `setScene(scene)` | 关联场景 |
| `scale(sx, sy)` | 缩放 |
| `rotate(angle)` | 旋转 |
| `fitInView(rect, mode)` | 自适应区域 |
| `centerOn(item/pos)` | 居中 |
| `ensureVisible(item/rect)` | 确保可见 |
| `mapToScene(viewPos)` | View → Scene 坐标 |
| `mapFromScene(scenePos)` | Scene → View 坐标 |
| `setDragMode(mode)` | 拖拽模式 |
| `setRenderHint(hint)` | 渲染提示 |
| `setViewportUpdateMode(mode)` | 更新模式 |
| `setCacheMode(mode)` | 缓存模式 |
| `setTransformationAnchor(anchor)` | 变换锚点 |

### 11.3 QGraphicsItem

| 方法 | 说明 |
|---|---|
| `setPos(x, y)` | 设置位置 |
| `moveBy(dx, dy)` | 相对移动 |
| `setRotation(angle)` | 旋转 |
| `setScale(factor)` | 缩放 |
| `setZValue(z)` | 层叠顺序 |
| `setOpacity(val)` | 透明度 |
| `setFlag(flag, enabled)` | 设置交互标志 |
| `setData(key, value)` | 存储自定义数据 |
| `boundingRect()` | 包围盒（必须重写） |
| `shape()` | 碰撞形状 |
| `paint()` | 绘制（必须重写） |
| `collidesWithItem()` | 碰撞检测 |
| `mapToScene/mapFromScene` | 坐标转换 |
| `prepareGeometryChange()` | 几何变化前调用 |
| `update()` | 触发重绘 |

---

## 12. 总结

| 要点 | 说明 |
|---|---|
| **三大核心** | Scene（数据容器）、View（可视化窗口）、Item（图形元素） |
| **Scene** | 管理所有 item，BSP 索引加速查询，提供选择/焦点管理 |
| **View** | 支持缩放/旋转/滚动，可多视图共享 Scene |
| **Item** | 支持选择/拖拽/碰撞/父子层级/缓存，自定义须重写 `boundingRect()` + `paint()` |
| **坐标系** | Item 坐标 ⇌ Scene 坐标 ⇌ View 坐标，互转用 `mapTo*` / `mapFrom*` |
| **碰撞** | 重写 `shape()` 实现精确碰撞/点击命中 |
| **几何变化** | boundingRect 将变时必须先调 `prepareGeometryChange()` |
| **动画** | 继承 `QGraphicsObject`，用 `QPropertyAnimation` 驱动属性 |
| **嵌入 Widget** | `QGraphicsProxyWidget` 将 QWidget 嵌入场景 |
| **性能** | 精确 boundingRect + 缓存模式 + 合理索引方式 |
| **导出** | `scene->render()` 可输出到 QImage/QPrinter/QSvgGenerator |
