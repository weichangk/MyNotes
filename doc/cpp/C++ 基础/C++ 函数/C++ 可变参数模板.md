**详细讲解 C++ 的可变参数模板（Variadic Templates）** —— 这是 C++11 引入的高级模板特性，用来处理**参数个数不固定的函数、类或结构体**。

## 什么是可变参数模板？

> 可变参数模板 = 模板中可以接收**任意多个参数类型或值**。

在 C 语言中我们用 `...` 和 `va_list` 做类似事，但类型信息丢失，不安全。C++ 可变参数模板是 **类型安全 + 编译期处理** 的解决方案。

## 基本语法

```cpp
template<typename... Args>
void func(Args... args);
```

- `Args...` 是一个 **模板参数包**（template parameter pack）
- `args...` 是一个 **函数参数包**（function parameter pack）

## 示例：最简单的可变模板函数（什么都不做）

```cpp
template<typename... Args>
void dummy(Args... args) {
    // 什么也不做
}
```

可以这样调用：

```cpp
dummy();               // 零个参数
dummy(1);              // 一个参数
dummy(1, "hello", 3.14);  // 多个参数
```

## 示例：递归打印参数（经典模式）

```cpp
#include <iostream>

// 终止函数
void print() {
    std::cout << "(end)" << std::endl;
}

// 递归展开
template<typename T, typename... Rest>
void print(T first, Rest... rest) {
    std::cout << first << " ";
    print(rest...);  // 递归展开参数包
}

int main() {
    print(1, "hello", 3.14, 'A');
    // 输出：1 hello 3.14 A (end)
}
```

解释：展开参数包（parameter pack）

模板展开是靠**递归函数调用**实现的：

```cpp
print(1, 2, 3)
→ print(1, 2, 3)
→ print(2, 3)
→ print(3)
→ print()
```

每次调用处理一个参数，剩下的交给下一轮处理，直到没有参数为止。

## 示例：函数参数计数器

```cpp
template<typename... Args>
void countArgs(Args... args) {
    std::cout << "参数个数：" << sizeof...(Args) << std::endl;
}
```

使用：

```cpp
countArgs();                    // 输出 0
countArgs(1, 2, 3);             // 输出 3
countArgs("a", 1.2, 'x', true); // 输出 4
```

## 示例：C++17 折叠表达式（推荐写法）

```cpp
#include <iostream>

template<typename... Args>
void printAll(Args... args) {
    (std::cout << ... << args) << std::endl;
}

int main() {
    printAll("Sum = ", 1, ", ", 2.5, ", ", 'C'); 
}
```

> `(std::cout << ... << args)` 是 C++17 的**折叠表达式（Fold Expression）**，自动展开为：

```cpp
(((std::cout << args1) << args2) << args3) ...
```

## 示例：编写一个安全的 `sum` 函数

方法 1：递归加法

```cpp
template<typename T>
T sum(T val) {
    return val;
}

template<typename T, typename... Args>
T sum(T first, Args... rest) {
    return first + sum(rest...);
}
```

使用：

```cpp
std::cout << sum(1, 2, 3, 4);  // 输出 10
```

方法 2（推荐）：C++17 折叠表达式

```cpp
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);  // 从左向右折叠
}
```

## 示例：类模板的可变参数构造函数

```
#include <iostream>

class Logger {
public:
    template<typename... Args>
    Logger(Args... args) {
        log(args...);
    }

    template<typename... Args>
    void log(Args... args) {
        (std::cout << ... << args) << std::endl;
    }
};

int main() {
    Logger l("Log:", 123, ",", 4.56);
}
```

## 示例：初始化 list 展开

```cpp
template<typename... Args>
void init(Args... args) {
    std::initializer_list<int>{(std::cout << args << " ", 0)...};
    std::cout << std::endl;
}
```

等价于：

```cpp
(init(1, 2, 3)); // 输出：1 2 3
```

> `(表达式, 0)...` 是个技巧：执行表达式但返回 0，从而填入初始化列表。

## 示例：可变参数模板与 `std::forward`（完美转发）

```cpp
template<typename... Args>
void call(Args&&... args) {
    func(std::forward<Args>(args)...);
}
```

这叫做**完美转发**，用于避免不必要的拷贝，保持参数的左值/右值特性。

## 应用场景总结

| 应用场景         | 用法示例                         |
| ---------------- | -------------------------------- |
| 参数个数不定函数 | `print(...)`, `sum(...)`         |
| 类型转换/封装    | `std::make_tuple(...)`           |
| 组合构造函数     | `Logger(...)`, `Widget(...)`     |
| 转发函数         | `std::forward<Args>(args)...`    |
| 与折叠表达式结合 | `(args + ...)`, `(f(args), ...)` |

## 总结

> C++ 可变参数模板 是一种强大、类型安全的方式，支持你编写任意参数个数的泛型代码，替代旧的 `...`，配合递归、折叠表达式、`std::forward`、`initializer_list` 等可以做很多高级用法。