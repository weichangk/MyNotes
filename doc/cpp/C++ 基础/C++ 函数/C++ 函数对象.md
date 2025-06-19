“**函数对象**”（Function Object，也叫“仿函数”）是 C++ 非常重要的概念。它是 **“可以像函数一样被调用的对象”**，并且比普通函数更强大：**它能携带状态**，还能与泛型、STL算法完美结合。

> 函数对象 = **定义了 `operator()` 的类对象**，可以像函数那样使用 `对象()` 的形式来调用。

### 示例演示：最简单的函数对象

```cpp
#include <iostream>

struct Adder {
    int operator()(int a, int b) {
        return a + b;
    }
};

int main() {
    Adder add;           // 创建对象
    int result = add(3, 5); // 调用 operator()，像函数一样
    std::cout << result << std::endl;  // 输出 8
}
```

📌 实际等价于：

```cpp
add.operator()(3, 5);
```

### 结构分析

```cpp
struct Adder {
    int operator()(int a, int b) {
        return a + b;
    }
};
```

- `operator()` 是函数调用运算符。
- 定义后，这个对象就可以像函数一样使用。

### 函数对象 vs 普通函数的优点

| 功能                   | 普通函数 | 函数对象 ✅           |
| ---------------------- | -------- | -------------------- |
| 能否像函数一样调用     | ✅ 是     | ✅ 是                 |
| 能否携带状态           | ❌ 否     | ✅ 可以（成员变量）   |
| 能否组合使用           | ❌ 受限   | ✅ 可以作为类模板参数 |
| 性能（可能被内联优化） | ⬇️ 较少   | ✅ 可内联             |
| 与 STL 算法配合使用    | ✅ 是     | ✅ 更好支持           |

### 示例：带状态的函数对象

```cpp
#include <iostream>

struct Multiplier {
    int factor;
    Multiplier(int f) : factor(f) {}

    int operator()(int value) const {
        return value * factor;
    }
};

int main() {
    Multiplier times2(2);
    Multiplier times10(10);

    std::cout << times2(5) << std::endl;   // 输出 10
    std::cout << times10(5) << std::endl;  // 输出 50
}
```

解释：

- 这个函数对象可以记住它的“因子 factor”，实现可配置的乘法行为。
- 函数指针没法做到“记住状态”。

### 函数对象在 STL 中的应用

C++ STL（标准模板库）大量使用了函数对象，举两个例子：

#### 1. `std::sort` 使用函数对象

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

struct Desc {
    bool operator()(int a, int b) const {
        return a > b;  // 降序排序
    }
};

int main() {
    std::vector<int> vec = {3, 1, 4, 2};
    std::sort(vec.begin(), vec.end(), Desc());  // 使用函数对象排序
    for (int n : vec) std::cout << n << " ";    // 输出：4 3 2 1
}
```

等价的 lambda 写法（更简洁）

```cpp
std::sort(vec.begin(), vec.end(), [](int a, int b) { return a > b; });
```

> ✅ Lambda 就是函数对象的语法糖！其实 C++ 编译器会自动生成一个匿名结构体并重载 `operator()`。

### 更复杂的函数对象：可变参数、多种调用方式等

```cpp
struct Logger {
    template<typename... Args>
    void operator()(Args... args) const {
        (std::cout << ... << args) << std::endl;
    }
};

int main() {
    Logger log;
    log("Error: ", 404, ", Not Found");  // 输出：Error: 404, Not Found
}
```

> ✅ 使用函数对象 + 模板技术，写出功能强大的工具类。

### 总结一下：函数对象 Function Object

| 特性             | 描述                                            |
| ---------------- | ----------------------------------------------- |
| 本质             | 重载了 `operator()` 的类或结构体对象            |
| 调用方式         | `对象(参数列表)`，实际调用的是 `operator()`     |
| 能否持有状态     | ✅ 可以有成员变量，记住状态                      |
| 与函数指针区别   | 更强大，更灵活                                  |
| 与 Lambda 的关系 | Lambda 是函数对象的简写方式（语法糖）           |
| STL 典型用途     | `std::sort`, `std::for_each`, `std::function`等 |