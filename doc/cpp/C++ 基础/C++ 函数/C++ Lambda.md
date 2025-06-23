Lambda 是现代 C++ 的重要特性之一，从 C++11 引入之后，它成为 **回调、函数对象、并发、STL 算法调用** 的重要利器。

## 什么是 Lambda 表达式？

> Lambda 是一种**匿名函数对象（函数对象的语法糖）**，可以在函数内部快速定义、传递和使用函数。

你可以理解为：

```cpp
[](参数列表) -> 返回类型 { 函数体 }
```

------

## Lambda 的完整语法结构

```
[capture](parameters) -> return_type {
    function_body
};
```

各部分说明：

| 部分             | 说明                               |
| ---------------- | ---------------------------------- |
| `[capture]`      | 捕获外部变量的方式（值、引用、等） |
| `(parameters)`   | 参数列表，如同函数                 |
| `-> return_type` | 返回类型（可省略，编译器自动推导） |
| `{}`             | 函数体，支持任意语句               |

### 示例：最简单的 Lambda 表达式

```cpp
#include <iostream>

int main() {
    auto f = []() {
        std::cout << "Hello from Lambda!" << std::endl;
    };
    f();  // 调用
}
```

------

### 示例：带参数和返回值的 Lambda

```cpp
auto add = [](int a, int b) -> int {
    return a + b;
};

std::cout << add(2, 3);  // 输出 5
```

`-> int` 可以省略，编译器会自动推导。

------

### 示例：使用 STL + Lambda 进行排序

```cpp
#include <vector>
#include <algorithm>
#include <iostream>

int main() {
    std::vector<int> v = {3, 1, 4, 2};

    std::sort(v.begin(), v.end(), [](int a, int b) {
        return a > b;  // 降序排序
    });

    for (int x : v) std::cout << x << " ";  // 输出：4 3 2 1
}
```

## 捕获列表 [capture]

这是 Lambda 表达式最重要的部分之一 —— **如何访问外部变量**：

1. `[=]`：捕获外部变量 **值拷贝**

```
int x = 10;
auto f = [=]() { std::cout << x << std::endl; };
```

2. `[&]`：捕获外部变量 **引用**

```
int x = 10;
auto f = [&]() { x += 5; };  // 修改 x
f();
std::cout << x;  // 输出 15
```

3. `[x]`：只捕获 `x` 的值

4. `[&x]`：只捕获 `x` 的引用

5. `[=, &y]`：值捕获其他变量，`y` 用引用捕获

### 示例：值捕获 vs 引用捕获

```cpp
int a = 5, b = 10;

auto f1 = [=]() { std::cout << a << std::endl; };   // 值捕获，不可修改
auto f2 = [&]() { b += 10; };  // 引用捕获，可修改

f1();           // 输出 5
f2();
std::cout << b; // 输出 20
```

------

### 示例：捕获并返回 Lambda

```cpp
auto make_multiplier(int factor) {
    return [factor](int value) {
        return value * factor;
    };
}

int main() {
    auto times2 = make_multiplier(2);
    std::cout << times2(5);  // 输出 10
}
```

> Lambda 其实就是函数对象，所以可以返回！

## Lambda 与函数对象（Function Object）比较

```cpp
struct Adder {
    int operator()(int a, int b) { return a + b; }
};

auto add = [](int a, int b) { return a + b; };
```

| 项目         | 函数对象         | Lambda 表达式 |
| ------------ | ---------------- | ------------- |
| 写法         | 类 + operator()  | 一行          |
| 可读性       | 一般             | ✅ 更高        |
| 捕获外部变量 | 手动添加成员变量 | ✅ 自动捕获    |
| 与 STL 结合  | ✅ 非常好         | ✅ 非常好      |



------

## Lambda 与 `std::function`

```cpp
#include <functional>

std::function<int(int, int)> op = [](int a, int b) {
    return a * b;
};

std::cout << op(3, 4);  // 输出 12
```

`std::function` 可以接收任何可调用对象（函数、函数对象、Lambda）。

------

## 可变 Lambda（mutable）

默认 Lambda 捕获的值是只读的。如果你想修改值捕获的变量副本，需要加 `mutable`：

```cpp
int x = 10;
auto f = [=]() mutable {
    x += 5;  // 修改的是副本
    std::cout << x << std::endl;  // 输出 15
};
f();
std::cout << x << std::endl;  // 输出 10，外部变量没变
```

------

## Lambda 表达式的底层原理（类对象）

下面的 Lambda：

```
auto f = [](int a, int b) { return a + b; };
```

等价于编译器生成的代码：

```cpp
struct __LambdaAdd {
    int operator()(int a, int b) const {
        return a + b;
    }
};

__LambdaAdd f;
```

------

## C++14/17/20 中更高级用法

C++14：自动参数类型

```cpp
auto add = [](auto a, auto b) {
    return a + b;
};
add(1, 2);       // int
add(1.1, 2.2);   // double
```

C++20：捕获 this by value

```cpp
[this]() { doSomething(); };  // 捕获 this 指针
[=*this]() { doSomething(); };  // 捕获 this 的副本（C++20）
```

------

## 总结对照表

| 用法        | 示例                        | 说明                       |
| ----------- | --------------------------- | -------------------------- |
| 基本形式    | `[]() {}`                   | 最简单的 Lambda            |
| 参数/返回值 | `[](int a) -> int {}`       | 明确指定参数和返回值       |
| 值捕获      | `[=]() {}`                  | 捕获外部变量副本           |
| 引用捕获    | `[&]() {}`                  | 修改外部变量               |
| mutable     | `[=]() mutable {}`          | 允许修改副本               |
| 返回 Lambda | `return [factor](...) {}`   | 常见于工厂函数、策略模式等 |
| 与 STL 结合 | `std::sort(..., [](a,b){})` | 函数回调、排序、查找       |