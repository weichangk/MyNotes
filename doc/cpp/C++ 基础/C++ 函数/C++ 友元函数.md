C++ 中的 **友元函数（friend function）** 是一种特殊的函数，它被允许访问某个类的 **私有成员（private）** 和 **保护成员（protected）**，即使它本身不是这个类的成员函数。友元机制是 C++ 提供的一种 **打破封装** 的方式，用来支持某些特殊场景下的设计需求。

## 为什么需要友元函数？

我们知道 C++ 的类具有封装性，类的私有成员只能被该类的成员函数访问。但在某些情况下，有些非成员函数确实需要访问类的内部实现。例如：

- 重载运算符时，例如 `operator<<` 通常需要访问私有数据。
- 有些辅助函数（如数学函数）需要访问类内部成员但又不适合作为成员函数。
- 两个类之间互相访问私有成员时（互为友元）。

## 友元函数的定义与声明

声明方式

```cpp
class MyClass {
private:
    int m_data;

public:
    MyClass(int val) : m_data(val) {}

    // 声明 friend 函数
    friend void printData(const MyClass& obj);
};
```

定义方式

友元函数虽然在类内声明，但它**不是类的成员函数**，所以定义时不用加类作用域：

```cpp
void printData(const MyClass& obj) {
    std::cout << "Data: " << obj.m_data << std::endl;
}
```

## 友元函数的特性总结

| 特性     | 说明                                                       |
| -------- | ---------------------------------------------------------- |
| 访问权限 | 友元函数可以访问类的私有（private）和保护（protected）成员 |
| 所属     | 友元函数不是类的成员                                       |
| 作用域   | 不能使用类的作用域运算符定义（如 `MyClass::printData`）    |
| 调用方式 | 与普通函数一样使用，不需要对象调用                         |
| 继承     | 不受继承影响，不能继承友元关系                             |

## 友元函数的使用场景

场景一：输出重载

```cpp
#include <iostream>
class Point {
private:
    int x, y;

public:
    Point(int a, int b) : x(a), y(b) {}

    friend std::ostream& operator<<(std::ostream& os, const Point& p);
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}
```

场景二：非成员函数需要访问私有成员

```cpp
class Box {
private:
    double width;

public:
    Box(double w) : width(w) {}

    friend double getWidth(const Box& b); // 让外部函数访问私有 width
};

double getWidth(const Box& b) {
    return b.width;
}
```

场景三：友元成员函数

```cpp
class A;

class B {
public:
    void accessA(const A& a);
};

class A {
private:
    int secret = 42;

    friend void B::accessA(const A&); // 只让 B::accessA 访问 A 的私有数据
};

void B::accessA(const A& a) {
    std::cout << "Access A's secret: " << a.secret << std::endl;
}
```

## 友元函数的注意事项

1. 友元函数破坏了类的封装性，要谨慎使用。
2. 友元关系是**单向的**：A 声明 B 为友元，B 能访问 A，但 A 不能访问 B。
3. 友元关系**不可传递**：A 是 B 的朋友，B 是 C 的朋友，不代表 A 能访问 C。
4. 友元函数可以是全局函数、类的成员函数、类、模板函数等。