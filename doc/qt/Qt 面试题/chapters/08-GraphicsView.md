08-GraphicsView.md

# 第八章：Qt Graphics View 框架

[返回目录](../README.md) | [上一章](./07-内存管理.md) | [下一章](./09-模型视图.md)

---

## 8.1 Graphics View 框架的架构是什么？

### 答案

Graphics View 框架采用**三层架构**：

```
场景（Scene）- 管理图元
    ↓
视图（View）- 显示场景
    ↓
图元（Item）- 具体绘制项
```

### 核心类

| 类 | 说明 |
|----|------|
| QGraphicsScene | 场景，管理所有图元 |
| QGraphicsView | 视图，显示场景内容 |
| QGraphicsItem | 图元基类 |

### 示例

```cpp
// 创建场景
QGraphicsScene* scene = new QGraphicsScene();

// 添加图元
scene->addEllipse(0, 0, 100, 100, QPen(Qt::black), QBrush(Qt::red));
scene->addRect(150, 0, 100, 100, QPen(Qt::blue), QBrush(Qt::green));

// 创建视图
QGraphicsView* view = new QGraphicsView(scene);
view->show();
```

---

## 8.2 如何实现自定义 QGraphicsItem？

### 答案

```cpp
class CustomItem : public QGraphicsItem {
public:
    QRectF boundingRect() const override {
        return QRectF(0, 0, 100, 100);
    }
    
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override {
        painter->setBrush(Qt::blue);
        painter->drawEllipse(boundingRect());
    }
};

// 使用
QGraphicsScene* scene = new QGraphicsScene();
scene->addItem(new CustomItem());
```

---

## 8.3 如何优化 Graphics View 的性能？

### 答案

### 1. 使用 boundingRect 和 shape

```cpp
QRectF boundingRect() const override {
    // 返回最小包围矩形
    return QRectF(0, 0, width, height);
}

QPainterPath shape() const override {
    // 返回精确形状（用于碰撞检测）
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}
```

### 2. 使用缓存模式

```cpp
item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
```

### 3. 减少不必要的更新

```cpp
// 局部更新
update(rect);  // 只更新指定区域

// 禁用视图更新期间批量操作
scene->setUpdatesEnabled(false);
// ... 添加多个 item
scene->setUpdatesEnabled(true);
```

---

## 8.4 如何处理 GraphicsItem 的事件？

### 答案

```cpp
class InteractiveItem : public QGraphicsItem {
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            m_dragging = true;
            m_dragStart = event->pos();
        }
    }
    
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override {
        if (m_dragging) {
            QPointF offset = event->pos() - m_dragStart;
            setPos(pos() + offset);
        }
    }
    
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        m_dragging = false;
    }
    
private:
    bool m_dragging = false;
    QPointF m_dragStart;
};
```

---

[上一章：Qt 内存管理](./07-内存管理.md) | [下一章：Qt 模型/视图架构](./09-模型视图.md)
