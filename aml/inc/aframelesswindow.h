/*
 * @Author: weick 
 * @Date: 2024-05-20 07:52:31 
 * @Last Modified by: weick
 * @Last Modified time: 2024-05-20 08:03:55
 */

#pragma once
#include "aml_global.h"
#include <QQuickWindow>

class AML_EXPORT AFramelessWindow : public QQuickWindow {
    Q_OBJECT

    Q_PROPERTY(bool resizable READ resizable WRITE setResizable NOTIFY resizableChanged)

public:
    explicit AFramelessWindow(QWindow *parent = nullptr);

    bool resizable() const;
    void setResizable(bool resizable);

signals:
    void resizableChanged();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    bool m_resizable = true;
    QRect m_moveArea = {8, 8, width() - 16, 35};
};