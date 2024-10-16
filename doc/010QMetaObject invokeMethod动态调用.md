静态元对象连接方式通过 QMetaObject::invokeMethod 函数调用对象的槽函数，而不需要显式使用信号和槽的连接。常见用法包括跨线程调用槽函数或动态调用一些槽函数。该方法特别适合在不知道函数签名或参数的情况下进行调用。

---

## 使用 QMetaObject::invokeMethod 的步骤
- 定义槽函数： 槽函数可以是类的普通成员函数，也可以是槽函数。它必须使用 Q_INVOKABLE 宏或声明为槽 slot，以便可以通过 invokeMethod 调用。
- 调用 QMetaObject::invokeMethod： 使用 QMetaObject::invokeMethod 来动态调用指定对象的槽函数。

示例
```
#include <QApplication>
#include <QDebug>
#include <QMetaObject>

class MyClass : public QObject {
    Q_OBJECT
public slots:
    void mySlotWithArg(const QString &msg) {
        qDebug() << "Slot called with argument:" << msg;
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MyClass obj;

    QMetaObject::invokeMethod(&obj, "mySlotWithArg", Qt::DirectConnection,
                              Q_ARG(QString, "Hello, Qt!"));

    return app.exec();
}

```

参数说明：
- 第一个参数：指向接收者对象的指针。
- 第二个参数：要调用的函数名，必须是 QString 类型，函数名需要是 Q_INVOKABLE、slot 或 public 函数。
- 第三个参数：连接类型，通常使用 Qt::DirectConnection（ 同步调用槽函数） 或 Qt::QueuedConnection（异步调用槽函数）。
- 后续参数：使用 Q_ARG 宏传递参数，每个参数必须指定类型和值。

---

## 使用 QMetaObject::invokeMethod 的使用场景
- 跨线程调用：使用 QMetaObject::invokeMethod 可以实现跨线程的槽函数调用，这在信号和槽的机制中是非常重要的。如果直接调用函数，跨线程的调用会导致问题，而通过 invokeMethod 可以指定使用 Qt::QueuedConnection 来实现跨线程的安全调用。

- 动态函数调用：在编译时可能不知道要调用的具体函数名，或者函数的数量和名称是动态的。invokeMethod 允许你在运行时通过字符串指定要调用的函数，这为程序带来了更大的灵活性。

- 插件系统或反射机制：在某些场景下，例如插件系统中，可能你加载的模块或对象是在运行时才知道具体类型和函数，使用 invokeMethod 可以让你在不显式依赖编译时的类型和函数的情况下调用这些对象的函数。

---

## 什么时候使用 QMetaObject::invokeMethod？
- 跨线程调用：线程间通信，需要确保安全的函数调用。
- 动态函数调用：函数名称、参数在运行时决定。
- 反射机制：需要实现类似反射的功能，根据对象的类型和方法名动态调用方法。
- 插件和扩展系统：主程序不知道具体插件接口的实现，只通过名称和接口定义来调用插件的方法。

因此，QMetaObject::invokeMethod 适合那些需要动态、灵活调用函数的场景，而直接调用则更适合在静态类型明确的情况下。