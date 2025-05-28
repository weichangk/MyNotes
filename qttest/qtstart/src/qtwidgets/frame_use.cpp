#include "frame_use.h"

#include <QEventLoop>
#include <QFrame>
#include <QLabel>
#include <QMetaEnum>

void frame_use() {
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

    for (int shapeIndex = 0; shapeIndex < shapeList.count() ; shapeIndex++) {
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