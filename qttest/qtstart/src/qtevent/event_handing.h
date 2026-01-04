/**
 * @file event_handing.h
 * @brief Qt 事件处理机制示例
 * 
 * 本文件演示了 Qt 中事件的传递和处理机制：
 * 1. 子控件如何处理事件
 * 2. 事件如何向父控件传播
 * 3. event->ignore() 的作用
 */

#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QKeyEvent>

namespace EventHanding {

/**
 * @class MyLineEdit
 * @brief 自定义 QLineEdit，用于演示事件处理
 * 
 * 重写 keyPressEvent 来观察键盘事件的处理流程
 */
class MyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEdit(QWidget *parent = 0);
    
protected:
    /**
     * @brief 键盘按键按下事件处理函数
     * @param event 键盘事件对象
     * 
     * 处理流程：
     * 1. 打印日志表示事件被此控件接收
     * 2. 调用父类处理函数执行默认行为（输入字符等）
     * 3. ignore() 事件，使其继续向父控件传播
     */
    void keyPressEvent(QKeyEvent *event);
};

/**
 * @class EventHandingWidget
 * @brief 事件处理示例主窗口
 * 
 * 作为 MyLineEdit 的父窗口，用于观察事件的传播机制
 */
class EventHandingWidget : public QWidget {
    Q_OBJECT
public:
    EventHandingWidget(QWidget *parent = nullptr);
    ~EventHandingWidget();

protected:
    /**
     * @brief 键盘按键按下事件处理函数
     * @param event 键盘事件对象
     * 
     * 当子控件（MyLineEdit）调用 ignore() 后，事件会传播到这里
     * 打印日志证明事件确实传播到了父窗口
     */
    void keyPressEvent(QKeyEvent *event);

private:
    /**
     * @brief 创建用户界面
     * 
     * 初始化窗口标题、大小和子控件
     */
    void createUi();

private:
    MyLineEdit *m_pLineEdit = nullptr;  ///< 自定义输入框控件
};

} // namespace