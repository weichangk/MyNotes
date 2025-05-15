# QT事件问题

### 鼠标下压移动创建图形
在 eventFilter 里使用 if else if 判断 e->type() 为 MouseMove、MouseButtonPress、MouseButtonRelease 来处理对应逻辑发现无法实时触发 MouseMove
使用 if(MouseMove) if(MouseButtonPress) if(MouseButtonRelease) 也不行

应该直接使用
void mousePressEvent(QMouseEvent *event);
void mouseMoveEvent(QMouseEvent *event);
void mouseReleaseEvent(QMouseEvent *event);
来处理对应事件就好

### 非英文输入状态下实时显示预输入字符解决方案（qt输入框已自带，自定义输入框可能需要自己实现）
非英文输入状态mousePressEvent事件不响应，eventFilter内(QEvent::KeyPress == ev->type())也不响应的，需要在eventFilter内(QEvent::InputMethod== event->type())处理
```
bool ATextEdit::eventFilter(QObject *watched, QEvent *event)
{
    if(QEvent::InputMethod== event->type())
    {
        QInputMethodEvent *keyEvent = dynamic_cast<QInputMethodEvent *>(event);
        QString preeditString = keyEvent->preeditString();//预输入字符
        QString commitString = keyEvent->commitString();//实际输入字符
        qDebug() << "preeditString:" << preeditString << " commitString:" << commitString;
        if(preeditString.size() >= 0)
        {
            //处理显示预输入字符preeditString      
        }
        if(commitString.size() > 0)
        {
            //处理显示实际输入字符commitString
            //按enter时触发实际输入字符有值，预输入字符为空
            //按空格时会将预输入字符替换为实际输入字符
        }
    }
    return QTextEdit::eventFilter(watched, event);
}
```