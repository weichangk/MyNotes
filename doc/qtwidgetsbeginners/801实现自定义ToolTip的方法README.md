# 实现自定义ToolTip的方法

### 自定义 AToolTip


### 方法1：在自定义 APushButton 按钮组件使用 AToolTip
```
installEventFilter(this);

bool APushButton::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == this)
    {
        if(event->type() == QEvent::ToolTip)
        {
            if(!this->toolTip().isEmpty())
            {
                AToolTip::getInstance()->showText(this->toolTip());
                return true;
            }
        }
        else if (event->type() == QEvent::Leave)
        {
            AToolTip::getInstance()->hideText();
        }
    }
    return QPushButton::eventFilter(watched, event);
}
```
这种方法需要使用到自定义 ToolTip 的组件都要实现一遍 ToolTip 的显示隐藏控制，或使用继承已经有实现的基类。

### 方法2：在自定义 AApplication 中使用 AToolTip
```
bool AApplication::notify(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::ToolTip)
	{
		if (QWidget *widget = qobject_cast<QWidget*>(object))
		{
			if (!widget->toolTip().isEmpty())
			{
                AToolTip::getInstance()->showText(widget->toolTip(), static_cast<QHelpEvent*>(event)->globalPos());
                return true;
			}
		}
	}
    else if (event->type() == QEvent::Leave)
    {
        AToolTip::getInstance()->hideText();
    }
    return QApplication::notify(object, event);
}
```
这种方法是在整个应用程序里监听所有的组件的 ToolTip 事件进行处理，不需要对组件实现 ToolTip 的显示隐藏控制，但是在自定义组件时如果组件存在层级关系或多个子组件时也存在一些问题！！！


### 存在问题
Qt::Popup 会造成窗体内部按钮的自定义ToolTip闪退！使用Qt::Tool属性再加个蒙层处理点击外部实现隐藏窗体来实现Qt::Popup窗体


### WA_TranslucentBackground设置透明窗体，但是窗体无法响应鼠标事件了！
解决方法是使用绘制方法来绘制接近透明的颜色
```
QMask::QMask(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

QMask::~QMask()
{
}

void QMask::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(0, 0, 0, 0x01));
    QWidget::paintEvent(event);
}

void QMask::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit sigClicked();
    }
    QWidget::mousePressEvent(event);
}
```