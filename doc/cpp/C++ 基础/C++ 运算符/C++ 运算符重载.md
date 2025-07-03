C++ 运算符重载（Operator Overloading）是指为自定义类型定义已有运算符（如 `+`, `==`, `[]`, `()` 等）的操作，使得这些运算符可以用于自定义类型，就像它们用于内建类型一样。

## 为什么需要运算符重载

自定义类型（如类或结构体）默认不支持运算符操作。例如：

```c++
MyVector a(1, 2);
MyVector b(3, 4);
MyVector c = a + b; // 如果不重载 +，这行将报错
```

如果你想让 `+` 用于 `MyVector`，就需要重载运算符 `+`。

## 基本语法

```c++
返回类型 operator运算符(参数列表);
```

你可以在类的成员函数中重载，也可以用友元函数或普通函数重载。

## 重载分类

| 类型     | 举例               | 是否必须成员函数 |
| -------- | ------------------ | ---------------- |
| 成员函数 | `operator+`        | 否               |
| 友元函数 | `friend operator+` | 否               |
| 全局函数 | `operator<<`       | 否               |

## 常见运算符重载详解

### 重载二元运算符（如 `+`, `-`, `*`, `/`）

示例：重载 `+` 运算符

```c++
class MyVector {
public:
    int x, y;

    MyVector(int x = 0, int y = 0) : x(x), y(y) {}

    MyVector operator+(const MyVector& rhs) const {
        return MyVector(x + rhs.x, y + rhs.y);
    }
};
```

使用：

```c++
MyVector a(1, 2), b(3, 4);
MyVector c = a + b; // 调用重载的 operator+
```

### 重载一元运算符（如 `++`, `--`, `-`, `!`）

示例：前置 ++

```c++
class Counter {
    int value;
public:
    Counter(int v = 0) : value(v) {}

    // 前置 ++
    Counter& operator++() {
        ++value;
        return *this;
    }

    // 后置 ++
    Counter operator++(int) {
        Counter temp = *this;
        ++value;
        return temp;
    }
};
```

### 重载关系运算符（`==`, `!=`, `<`, `>` 等）

```cpp
class Point {
    int x, y;
public:
    Point(int x, int y): x(x), y(y) {}

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};
```

### 重载输入输出运算符（`<<`, `>>`）【必须用友元或全局函数】

```cpp
#include <iostream>
class Point {
    int x, y;
public:
    Point(int x = 0, int y = 0): x(x), y(y) {}

    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        os << "(" << p.x << ", " << p.y << ")";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Point& p) {
        is >> p.x >> p.y;
        return is;
    }
};
```

### 重载下标运算符 `[]`

```cpp
class Array {
    int data[10];
public:
    int& operator[](int index) {
        return data[index];
    }
};
```

### 重载函数调用运算符 `()`（仿函数）

```cpp
class Adder {
    int base;
public:
    Adder(int b): base(b) {}
    int operator()(int x) {
        return base + x;
    }
};
```

使用：

```
Adder add5(5);
std::cout << add5(3); // 输出 8
```

## 运算符重载限制（不能重载的运算符）

| 不能重载的运算符         |
| ------------------------ |
| `::` 作用域解析符        |
| `.`  成员访问符          |
| `.*` 成员指针访问符      |
| `sizeof`                 |
| `typeid`                 |
| `alignof`                |
| `static_cast` 等类型转换 |

## 运算符重载技巧与建议

1. **使用 `const` 正确性**：如 `const T&` 参数传递、`const` 成员函数。
2. **保持语义一致性**：`a + b` 要有加法语义，不要滥用。
3. **注意深拷贝问题**：涉及资源的类要管理内存（例如重载 `=` 时）。
4. **返回引用 vs 值**：
   - 修改类内数据返回引用（如 `operator[]`）
   - 新对象应返回值（如 `operator+`）

## 示例：复数类完整运算符重载

```cpp
#include <iostream>

class Complex {
    double real, imag;

public:
    Complex(double r = 0, double i = 0): real(r), imag(i) {}

    Complex operator+(const Complex& other) const {
        return Complex(real + other.real, imag + other.imag);
    }

    Complex operator*(const Complex& other) const {
        return Complex(real * other.real - imag * other.imag,
                       real * other.imag + imag * other.real);
    }

    friend std::ostream& operator<<(std::ostream& os, const Complex& c) {
        os << c.real << " + " << c.imag << "i";
        return os;
    }
};
```

## 总结

| 运算符      | 典型用法          | 重载建议方式      |
| ----------- | ----------------- | ----------------- |
| `+ - * /`   | 类之间的加减乘除  | 成员/友元函数皆可 |
| `== != < >` | 比较操作          | 成员或友元函数    |
| `<< >>`     | 流操作符          | 友元函数          |
| `[]`        | 容器类下标        | 成员函数          |
| `()`        | 函数对象/回调机制 | 成员函数          |
| `=`         | 赋值操作符        | 必须小心管理资源  |

## 逻辑与 `&&` / 逻辑或 `||` 运算符重载的陷阱

不能实现“短路求值”

- 内建的 `a && b` 会先判断 `a` 是否为 `false`，若为 `false`，则不会求值 `b`（**短路行为**）
-  重载 `operator&&`/`operator||` 时，**两边的表达式一定都会被计算**，**失去短路特性**

例子：

```cpp
if (a && b) { ... }  // 原生操作符：若 a 为 false，b 不会被求值（安全）
if (a.operator&&(b)) { ... }  // 运算符重载：a 和 b 都会被求值
```

### 语法：如何重载逻辑与/或运算符

```cpp
class BoolLike {
    bool value;
public:
    BoolLike(bool v) : value(v) {}

    // 重载逻辑与
    BoolLike operator&&(const BoolLike& rhs) const {
        std::cout << "Evaluating &&\n";
        return BoolLike(value && rhs.value);
    }

    // 重载逻辑或
    BoolLike operator||(const BoolLike& rhs) const {
        std::cout << "Evaluating ||\n";
        return BoolLike(value || rhs.value);
    }

    bool get() const { return value; }
};
```

示例：

```cpp
BoolLike a(true);
BoolLike b(false);
BoolLike c = a && b;  // 调用 operator&&，两边都求值
```

输出：

```cpp
Evaluating &&
```

### 短路行为无法模拟的原因

运算符重载函数的参数是值传递（或引用），即：

```cpp
BoolLike operator&&(const BoolLike& rhs) const;
```

此时，`rhs` 已经被求值传入函数中，因此：无法做到“判断左边为 false 后直接跳过右边”的行为。

## 重载 `operator bool()`

重载 `operator bool()` 是 C++ 中一种非常实用的技术，主要用于让**自定义类型在布尔上下文中表现得像布尔值一样**，比如用于 `if`、`while`、逻辑判断等场景。

### `operator bool()` 的基本作用

让类对象能在布尔上下文中使用，比如：

```cpp
if (obj) { ... }
while (obj) { ... }
```

默认情况下，自定义类不能这样使用，除非重载 `operator bool()`。

### 常见使用场景

#### 判断对象是否有效（常见于智能指针）

```cpp
class SmartPointer {
    void* ptr;
public:
    SmartPointer(void* p = nullptr) : ptr(p) {}

    operator bool() const {
        return ptr != nullptr;
    }
};

SmartPointer sp(new int);
if (sp) {
    std::cout << "有效指针\n";
}
```

> 类似于 `std::shared_ptr`、`std::unique_ptr` 的使用方式。

#### 状态检测类（文件、连接、初始化等）

```cpp
class Connection {
    bool connected;
public:
    Connection(bool flag = false) : connected(flag) {}

    operator bool() const {
        return connected;
    }
};

Connection conn(true);
if (conn) {
    std::cout << "连接成功\n";
}
```

#### 操作结果类（包装是否成功）

```cpp
class Result {
    bool success;
public:
    Result(bool ok) : success(ok) {}

    operator bool() const {
        return success;
    }
};

Result res = doSomething();
if (res) {
    std::cout << "成功\n";
} else {
    std::cout << "失败\n";
}
```

> 可以进一步加上 `getError()` 接口表示失败信息。

#### 流类判断

仿照 `std::istream` 的语义：

```cpp
class MyStream {
    bool good;
public:
    MyStream(bool g = true) : good(g) {}

    operator bool() const {
        return good;
    }
};
MyStream s;
if (s) {
    std::cout << "流状态良好\n";
}
```

#### 类似于 Optional 的容器类（如 Maybe、Option）

```
template<typename T>
class Optional {
    bool hasValue;
    T value;
public:
    Optional() : hasValue(false) {}
    Optional(T val) : hasValue(true), value(val) {}

    operator bool() const { return hasValue; }

    T& get() { return value; }
};
Optional<int> opt(42);
if (opt) {
    std::cout << "有值：" << opt.get() << "\n";
}
```

### 注意事项：防止隐式转换错误（explicit vs 隐式）

在 C++11 之后，你可以用 `explicit operator bool()` 来避免非布尔上下文中的误用：

```cpp
explicit operator bool() const { return isValid(); }
```

会在 `if (obj)` 生效
不允许自动转为 `int` 或用于三目运算 `int x = obj ? 1 : 2;`（会报错）

实现建议

| 建议                       | 说明                                     |
| -------------------------- | ---------------------------------------- |
| 标记为 `const`             | `operator bool() const` 表示不会修改对象 |
| 用 `explicit` 限制隐式转换 | 避免意外转化为 `int` 或其他类型          |
| 保持语义清晰               | 表示“对象是否可用/有效/存在”等           |