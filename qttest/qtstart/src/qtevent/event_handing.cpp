/**
 * @file event_handing.cpp
 * @brief Qt 事件处理机制示例实现
 */

#include "event_handing.h"
#include <QDebug>

namespace EventHanding {

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
 * Qt 事件处理机制流程：
 * 1. 事件首先被发送到当前获得焦点的控件（子控件）
 * 2. 子控件可以选择处理或忽略事件
 * 3. 如果调用 event->ignore()，事件会继续向父控件传播
 * 4. 如果调用 event->accept() 或不调用任何方法，事件被视为已处理，不再传播
 * 
 * 本示例中：
 * - 首先打印日志，证明事件到达 MyLineEdit
 * - 然后调用父类处理函数，执行 QLineEdit 的默认行为（显示输入字符）
 * - 最后调用 ignore()，使事件继续传播到父控件 EventHandingWidget
 */
void MyLineEdit::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "MyLineEdit 键盘按下事件";
    
    // 执行 QLineEdit 类的默认事件处理（如显示输入的字符）
    QLineEdit::keyPressEvent(event);
    
    // 忽略该事件，使其继续向父控件传播
    // 注：如果不调用 ignore()，事件将不会传播到 EventHandingWidget
    event->ignore();
}

/**
 * @brief EventHandingWidget 构造函数
 * @param parent 父窗口指针
 */
EventHandingWidget::EventHandingWidget(QWidget *parent) :
    QWidget(parent) {
    createUi();
}

/**
 * @brief EventHandingWidget 析构函数
 */
EventHandingWidget::~EventHandingWidget() {
}

/**
 * @brief 父窗口的键盘按下事件处理函数
 * @param event 键盘事件对象
 * 
 * 当 MyLineEdit 调用 event->ignore() 后，事件会传播到这里。
 * 此函数打印日志，证明事件成功从子控件传播到父窗口。
 * 
 * 测试方法：
 * 1. 运行程序，在 MyLineEdit 输入框中按任意键
 * 2. 观察控制台输出：先输出 "MyLineEdit 键盘按下事件"
 * 3. 然后输出 "EventHandingWidget 键盘按下事件"
 * 4. 证明事件成功传播
 */
void EventHandingWidget::keyPressEvent(QKeyEvent *event) {
    qDebug() << "EventHandingWidget 键盘按下事件";
    // 注：此处没有调用父类处理函数，也没有 ignore()，事件在此停止传播
}

/**
 * @brief 创建用户界面
 * 
 * 初始化窗口属性和子控件：
 * - 设置窗口标题为 "事件处理机制"
 * - 设置最小窗口大小为 800x600
 * - 创建 MyLineEdit 控件并设置其属性
 */
void EventHandingWidget::createUi() {
    // 设置窗口标题
    setWindowTitle("事件处理机制");
    // 设置窗口最小尺寸
    setMinimumSize(800, 600);

    // 创建自定义输入框控件
    m_pLineEdit = new MyLineEdit(this);
    // 设置占位符文本，提示用户输入
    m_pLineEdit->setPlaceholderText("在此输入内容,观察控制台输出");
    // 设置输入框的位置和大小：x=100, y=100, width=400, height=30
    m_pLineEdit->setGeometry(100, 100, 400, 30);
}

} // namespace EventHanding