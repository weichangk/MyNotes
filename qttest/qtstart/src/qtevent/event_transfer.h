/**
 * @file event_transfer.h
 * @brief Qt 事件传递机制示例
 * 
 * 本文件演示了 Qt 中完整的事件传递流程：
 * 1. 事件过滤器 (eventFilter) - 最先拦截事件
 * 2. event() 函数 - 事件分发中心
 * 3. 具体事件处理函数 (keyPressEvent 等)
 * 4. 事件传播到父控件
 * 
 * 事件传递顺序：
 * eventFilter → event() → keyPressEvent() → 父控件的 keyPressEvent()
 */

#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QKeyEvent>

namespace EventTransfer {

/**
 * @class MyLineEdit
 * @brief 自定义 QLineEdit，演示事件传递流程
 * 
 * 重写 event() 和 keyPressEvent() 来观察事件的完整传递过程
 */
class MyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEdit(QWidget *parent = 0);

    /**
     * @brief 事件分发函数
     * @param event 事件对象
     * @return true 表示事件已处理，false 表示继续传递
     * 
     * 这是 Qt 事件系统的核心函数，所有事件都会先到达这里。
     * event() 函数根据事件类型分发到对应的处理函数（如 keyPressEvent）。
     * 
     * 事件处理流程：
     * 1. eventFilter（如果安装了事件过滤器）
     * 2. event() 函数（本函数）
     * 3. 具体的事件处理函数（如 keyPressEvent）
     */
    bool event(QEvent *event);
    
protected:
    /**
     * @brief 键盘按键按下事件处理函数
     * @param event 键盘事件对象
     * 
     * 在 event() 函数之后被调用。
     * 处理后调用 ignore() 使事件继续传播到父控件。
     */
    void keyPressEvent(QKeyEvent *event);
};

/**
 * @class EventTransferWidget
 * @brief 事件传递示例主窗口
 * 
 * 作为 MyLineEdit 的父窗口，演示：
 * 1. 如何使用事件过滤器拦截子控件的事件
 * 2. 事件如何从子控件传播到父控件
 * 3. 完整的事件传递链路
 */
class EventTransferWidget : public QWidget
{
    Q_OBJECT
public:
    EventTransferWidget(QWidget *parent = nullptr);
    ~EventTransferWidget();

protected:
    /**
     * @brief 事件过滤器函数
     * @param obj 产生事件的对象
     * @param event 事件对象
     * @return true 表示事件已处理不再传递，false 表示继续传递
     * 
     * 这是 Qt 事件传递的第一道关卡，可以在事件到达目标控件之前拦截它。
     * 
     * 使用方法：
     * 1. 调用 obj->installEventFilter(this) 安装过滤器
     * 2. 重写此函数处理事件
     * 
     * 事件处理顺序：
     * eventFilter → MyLineEdit::event() → MyLineEdit::keyPressEvent()
     */
    bool eventFilter(QObject *obj, QEvent *event);
    
    /**
     * @brief 键盘按键按下事件处理函数
     * @param event 键盘事件对象
     * 
     * 当子控件（MyLineEdit）调用 ignore() 后，事件会传播到这里。
     * 用于观察事件从子控件到父控件的传播过程。
     */
    void keyPressEvent(QKeyEvent *event);

private:
    /**
     * @brief 创建用户界面
     * 
     * 初始化窗口属性和子控件，并为子控件安装事件过滤器
     */
    void createUi();

private:
    MyLineEdit *m_pLineEdit = nullptr;  ///< 自定义输入框控件
};

}