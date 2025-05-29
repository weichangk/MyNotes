#include "frame_use.h"

#include <QEventLoop>
#include <QFrame>
#include <QLabel>
#include <QMetaEnum>
/*
在 Qt 中，QFrame 是一个用于绘制边框和背景的基本界面控件，它继承自 QWidget，主要作用是作为其他控件的容器或用于视觉分隔，提供简单的框架结构

QFrame 并不是一个功能控件（如按钮、标签），而是一个视觉辅助组件，主要提供以下功能：
1. 显示边框或凹槽：可通过 frameShape 和 frameShadow 设置不同的边框风格。
2. 布局容器：可在其内部放置其他控件，作为一个分组区域。
3. 视觉分隔线：通过设置为 HLine 或 VLine，可以作为一条水平/垂直的线来分隔界面区域。

常用属性
1. setFrameShape(QFrame::Shape)设置边框的形状，常用值有：
    - NoFrame：无边框
    - Box：盒子边框
    - Panel：面板样式
    - HLine：水平线
    - VLine：垂直线
    - StyledPanel：风格化面板（常用于分组）

2. setFrameShadow(QFrame::Shadow)设置边框的阴影样式，常用值有：
    - Plain：无阴影
    - Raised：凸起效果
    - Sunken：凹陷效果

3. setLineWidth() / setMidLineWidth()设置边框线的宽度。

在 Qt 中，QFrame 默认的边框颜色是由当前的 样式（Style） 决定的，比如 Fusion、Windows、macOS 等平台样式。要自定义边框颜色，有以下几种常见方法：
1. 方法一：使用 Qt 样式表
2. 方法二：子类化 QFrame 自定义绘制
3. 方法三：直接设置 QPalette

使用 QSS 或 setStyleSheet("QFrame { border: ... }")，QFrame 的 setFrameShape 和 setFrameShadow 设置就会失效，如何同时保留原始立体边框效果并改变颜色，要做到这一点，需要手动绘制边框或使用 QPalette 设置 Panel 风格的颜色
    - 原始立体边框的颜色如何通过 QPalette 设置，具体可查看 ColorRole 属性，不同的 Shape 和 Shadow 属性对不同的 ColorRole 起作用
        - 举例：给 QFrame::Plain 设置颜色
            QFrame *frame = new QFrame(this);
            frame->setFrameShape(QFrame::Box);
            frame->setFrameShadow(QFrame::Plain);
            frame->setLineWidth(2);

            QPalette p = frame->palette();
            p.setColor(QPalette::WindowText, Qt::red);  // 控制边框颜色
            frame->setPalette(p);
            frame->setForegroundRole(QPalette::WindowText);  // 明确指定使用该颜色
            
        - 举例：设置 Raised 或 Sunken 时的颜色
            p.setColor(QPalette::Light, Qt::green);  // 亮边
            p.setColor(QPalette::Dark, Qt::blue);   // 暗边
            p.setColor(QPalette::Mid, Qt::yellow);  // 中间线

    - 彻底自绘 Line + MidLine 改变边框颜色
        void paintEvent(QPaintEvent* event) override {
            QPainter painter(this);
            QRect outer = rect().adjusted(0, 0, -1, -1);

            int lw = lineWidth();
            int mlw = midLineWidth();

            // Line（外框）
            painter.setPen(QPen(lineColor, lw));
            painter.drawRect(outer);

            // MidLine（内框）
            if (mlw > 0) {
                QRect inner = outer.adjusted(lw, lw, -lw, -lw);
                painter.setPen(QPen(midLineColor, mlw));
                painter.drawRect(inner);
            }

总结建议：
- 保留阴影/边框风格 + 修改颜色	            重写 paintEvent()
- 快速设置边框颜色但无阴影	                setStyleSheet()
- 想要平台风格的 Panel/边框效果	            使用 setFrameShape + QPalette
- 想要兼顾美观与灵活                       使用 QStyleOptionFrame 和自绘

*/ void frame_use() {
    FrameUseWidget w;
    w.show();
    QEventLoop loop;
    QObject::connect(&w, &FrameUseWidget::close, &loop, &QEventLoop::quit);
    loop.exec();
}

QString shapeEnumName(QFrame::Shape shape) {
    const QMetaObject &metaObject = QFrame::staticMetaObject;
    int index = metaObject.indexOfEnumerator("Shape");
    QMetaEnum metaEnum = metaObject.enumerator(index);
    return metaEnum.valueToKey(shape);
}

QString shadowEnumName(QFrame::Shadow shadow) {
    const QMetaObject &metaObject = QFrame::staticMetaObject;
    int index = metaObject.indexOfEnumerator("Shadow");
    QMetaEnum metaEnum = metaObject.enumerator(index);
    return metaEnum.valueToKey(shadow);
}

FrameUseWidget::FrameUseWidget(QWidget *parent) :
    QWidget(parent) {
    QList<QFrame::Shape> shapeList = {QFrame::Box, QFrame::Panel, QFrame::StyledPanel, QFrame::HLine, QFrame::VLine, QFrame::WinPanel};
    QList<QFrame::Shadow> shadowList = {QFrame::Plain, QFrame::Raised, QFrame::Sunken};

    int lineWidth = 8;
    int midLineWidth = 4;
    int frameWidth = 240;
    int frameHeight = 120;

    int left = 10, top = 10;

    for (int shapeIndex = 0; shapeIndex < shapeList.count(); shapeIndex++) {
        for (int shadowIndex = 0; shadowIndex < shadowList.count(); shadowIndex++) {
            auto frame = new QFrame(this);
            // 设置边框颜色
            QPalette framePalette = frame->palette();
            framePalette.setColor(QPalette::Dark, Qt::red);    // 影响外框暗边
            framePalette.setColor(QPalette::Light, Qt::green); // 影响外框亮边
            framePalette.setColor(QPalette::Mid, Qt::blue);    // 理论上影响中线（部分平台）
            framePalette.setColor(QPalette::WindowText, Qt::yellow);
            frame->setPalette(framePalette);
            frame->setAutoFillBackground(true);

            auto label = new QLabel(frame);
            QPalette labelPalette = label->palette();
            labelPalette.setColor(QPalette::WindowText, Qt::black);
            label->setPalette(labelPalette);
            label->setAutoFillBackground(true);
            label->setText(QString("QFrame:%1 | QFrame:%2").arg(shapeEnumName(shapeList[shapeIndex])).arg(shadowEnumName(shadowList[shadowIndex])));
            label->move(20, 20);

            frame->setLineWidth(lineWidth);
            frame->setMidLineWidth(midLineWidth);
            frame->setFrameShape(shapeList[shapeIndex]);
            frame->setFrameShadow(shadowList[shadowIndex]);
            // 以上两个函数可以使用setFrameStyle(shapeList[shadowIndex] | shadowList[shadowIndex])代替

            int x = (frameWidth + left) * shadowIndex + left;
            int y = (frameHeight + top) * shapeIndex + top;
            frame->setGeometry(x, y, frameWidth, frameHeight);
        }
    }

    setFixedSize((frameWidth + left) * shadowList.count() + left, (frameHeight + top) * shapeList.count() + top);
}

FrameUseWidget::~FrameUseWidget() {
}