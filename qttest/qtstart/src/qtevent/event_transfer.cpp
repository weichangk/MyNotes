/**
 * @file event_transfer.cpp
 * @brief Qt 事件传递机制示例实现
 * 
 * 本示例完整展示了 Qt 中一个事件的完整生命周期：
 * 
 * 当在 MyLineEdit 中按下键盘键时，事件传递顺序为：
 * 1. EventTransferWidget::eventFilter()      - 父窗口的事件过滤器拦截
 * 2. MyLineEdit::event()                     - 子控件的事件分发函数
 * 3. MyLineEdit::keyPressEvent()             - 子控件的具体事件处理
 * 4. EventTransferWidget::keyPressEvent()    - 父窗口的事件处理（因为调用了 ignore()）
 */

#include "event_transfer.h"
#include <QDebug>

namespace EventTransfer {

/**
 * @brief MyLineEdit 构造函数
 * @param parent 父控件指针
 */
MyLineEdit::MyLineEdit(QWidget *parent) :
    QLineEdit(parent) {
}

/**
 * @brief 键盘按下事件处理函数
 * @param event 键盘事件对象
 * 
 * 这是事件传递的第三步（在 eventFilter 和 event() 之后）。
 * 
 * 处理流程：
 * 1. 打印日志，证明事件到达 MyLineEdit::keyPressEvent
 * 2. 调用父类处理函数，执行 QLineEdit 的默认行为（显示字符）
 * 3. 调用 ignore()，使事件继续传播到父控件
 */
void MyLineEdit::keyPressEvent(QKeyEvent *event)
{
    qDebug() << tr("MyLineEdit键盘按下事件");
    
    // 执行 QLineEdit 类的默认事件处理
    QLineEdit::keyPressEvent(event);
    
    // 忽略事件，使其继续向父控件传播
    event->ignore();
}

/**
 * @brief 事件分发函数
 * @param event 事件对象
 * @return true 表示事件已处理，false 表示继续传递
 * 
 * 这是事件传递的第二步（在 eventFilter 之后，keyPressEvent 之前）。
 * 
 * event() 函数是 Qt 事件系统的核心，它根据事件类型分发到对应的处理函数。
 * 例如：QEvent::KeyPress → keyPressEvent()
 *       QEvent::MouseButtonPress → mousePressEvent()
 * 
 * 重写此函数可以：
 * 1. 在事件分发给具体处理函数之前进行统一处理
 * 2. 自定义事件的分发逻辑
 * 3. 在不重写每个具体事件处理函数的情况下拦截多种事件
 */
bool MyLineEdit::event(QEvent *event)
{
    // 如果是键盘按下事件，打印日志
    if (event->type() == QEvent::KeyPress)
        qDebug() << tr("MyLineEdit的event()函数");
    
    // 调用父类 event() 函数，它会分发事件到 keyPressEvent() 等处理函数
    return QLineEdit::event(event);
}

/**
 * @brief EventTransferWidget 构造函数
 * @param parent 父窗口指针
 * 
 * 初始化窗口并为 MyLineEdit 安装事件过滤器。
 * installEventFilter(this) 使得 MyLineEdit 的事件在到达 MyLineEdit 之前
 * 先经过 EventTransferWidget::eventFilter() 处理。
 */
EventTransferWidget::EventTransferWidget(QWidget *parent) {
    createUi();
    // 为 MyLineEdit 安装事件过滤器
    // 这样 MyLineEdit 的事件会先经过 EventTransferWidget::eventFilter()
    m_pLineEdit->installEventFilter(this);
}

/**
 * @brief EventTransferWidget 析构函数
 */
EventTransferWidget::~EventTransferWidget() {
}

/**
 * @brief 事件过滤器函数
 * @param obj 产生事件的对象
 * @param event 事件对象
 * @return true 表示事件已处理不再传递，false 表示继续传递
 * 
 * 这是事件传递的第一步，在事件到达目标控件之前被调用。
 * 
 * 使用场景：
 * 1. 监控子控件的事件
 * 2. 在子控件处理之前拦截或修改事件
 * 3. 统一处理多个控件的事件
 * 
 * 注意：
 * - 返回 true 会阻止事件继续传递，目标控件不会收到事件
 * - 返回 false 会让事件继续传递到目标控件
 * - 本示例返回 false，因此事件会继续传递到 MyLineEdit
 */
bool EventTransferWidget::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        qDebug() << tr("EventTransferWidget的eventFilter()函数");
    }
    // 调用父类处理，返回 false 表示事件继续传递
    return QWidget::eventFilter(obj, event);
}

/**
 * @brief 父窗口的键盘按下事件处理函数
 * @param event 键盘事件对象
 * 
 * 这是事件传递的第四步（最后一步）。
 * 
 * 当 MyLineEdit::keyPressEvent() 调用 ignore() 后，事件会向上传播到父控件。
 * 此函数被调用证明了事件成功从子控件传播到父窗口。
 * 
 * 整个事件传递流程总结：
 * 1. EventTransferWidget::eventFilter()   - 事件过滤器拦截
 * 2. MyLineEdit::event()                  - 事件分发中心
 * 3. MyLineEdit::keyPressEvent()          - 子控件处理
 * 4. EventTransferWidget::keyPressEvent() - 父控件处理（当前函数）
 */
void EventTransferWidget::keyPressEvent(QKeyEvent *event) {
    qDebug() << tr("EventTransferWidget键盘按下事件");
    // 执行 QWidget 类的默认事件处理
    QWidget::keyPressEvent(event);
}

/**
 * @brief 创建用户界面
 * 
 * 初始化窗口属性和子控件。
 * 
 * 测试方法：
 * 1. 运行程序
 * 2. 在 MyLineEdit 输入框中按任意键
 * 3. 观察控制台输出，会依次显示：
 *    - "EventTransferWidget的eventFilter()函数"  （第一步）
 *    - "MyLineEdit的event()函数"             （第二步）
 *    - "MyLineEdit键盘按下事件"             （第三步）
 *    - "EventTransferWidget键盘按下事件"    （第四步）
 * 4. 这就是完整的事件传递流程
 */
void EventTransferWidget::createUi() {
    // 设置窗口标题
    setWindowTitle("Qt 事件传递示例");
    // 设置窗口最小尺寸
    setMinimumSize(400, 200);

    // 创建自定义输入框
    m_pLineEdit = new MyLineEdit(this);
    // 设置占位符文本
    m_pLineEdit->setPlaceholderText("请在此输入文本，观察事件传递过程...");
    // 设置输入框的位置和大小
    m_pLineEdit->setGeometry(50, 80, 300, 30);
}

} // namespace EventTransfer