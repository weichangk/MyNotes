C++ 中函数参数相关知识点，包括传递方式、默认参数、引用种类、右值引用、可变参数模板等。

## 参数传递方式详解

### 1. **值传递（Pass by Value）**

```cpp
void change(int a) {
    a = 100;
}

int main() {
    int x = 10;
    change(x);
    std::cout << x << std::endl; // 输出 10，值没有被改变
}
```

> ❗ 实参复制一份到函数内部，外部不受影响。

------

### 2. **引用传递（Pass by Reference）**

```cpp
void change(int& a) {
    a = 100;
}

int main() {
    int x = 10;
    change(x);
    std::cout << x << std::endl; // 输出 100，值被改变
}
```

> 📌 `int&` 是原变量的别名，修改会反映到原变量上。

------

### 3. **常量引用（Pass by const Reference）**

```cpp
void print(const std::string& str) {
    std::cout << str << std::endl;
}

int main() {
    std::string name = "Alice";
    print(name);
}
```

> ✅ 用于传递大对象，避免复制，同时保护数据不被修改。

------

### 4. **指针传递（Pass by Pointer）**

```cpp
void change(int* p) {
    *p = 100;
}

int main() {
    int x = 10;
    change(&x);
    std::cout << x << std::endl; // 输出 100
}
```

> 🧠 用指针时要注意空指针判断和内存管理。

------

### 5. **右值引用（C++11 起）**

```cpp
void set(std::string&& s) {
    std::cout << "Moved string: " << s << std::endl;
}

int main() {
    set("hello");  // 临时对象，调用右值引用版本
}
```

> ✅ 用于**移动语义**，避免资源的复制（如拷贝构造、深拷贝）。

## 默认参数

示例：

```cpp
void greet(std::string name = "Guest") {
    std::cout << "Hello, " << name << std::endl;
}

int main() {
    greet();            // Hello, Guest
    greet("Alice");     // Hello, Alice
}
```

> ⚠️ 默认参数必须从右向左连续定义，例如：

```cpp
// 正确：
void foo(int a, int b = 2, int c = 3);

// 错误：
void foo(int a = 1, int b, int c = 3); // ❌ b 没有默认值
```

## 函数重载（Overload）

```cpp
void show(int a) {
    std::cout << "int: " << a << std::endl;
}

void show(double a) {
    std::cout << "double: " << a << std::endl;
}

void show(const std::string& a) {
    std::cout << "string: " << a << std::endl;
}

int main() {
    show(1);           // 调用 int
    show(3.14);        // 调用 double
    show("hello");     // 调用 string
}
```

> ⚠️ **函数重载与默认参数可能冲突：**

```cpp
void test(int a, int b = 10);
void test(int a);  // ❌ 歧义，调用 test(5) 编译报错
```

## const 修饰的不同方式

### `const int&`

```cpp
void print(const int& a) {
    // 不能修改 a
}
```

### `const int*` vs `int* const`

```cpp
void read(const int* p) {
    // *p 不可修改，p 可变
}

void write(int* const p) {
    // *p 可修改，p 不可修改（地址固定）
}

void readOnly(const int* const p) {
    // *p 和 p 都不可修改
}
```

## 可变参数函数（两种方式）

### 1. C 风格 `...`

```cpp
#include <cstdarg>

int sum(int count, ...) {
    va_list args;
    va_start(args, count);
    int total = 0;
    for (int i = 0; i < count; ++i) {
        total += va_arg(args, int);
    }
    va_end(args);
    return total;
}

int main() {
    std::cout << sum(3, 1, 2, 3);  // 输出 6
}
```

------

### 2. C++11 可变参数模板

```cpp
#include <iostream>

void print() {}  // 递归终止

template<typename T, typename... Args>
void print(T first, Args... rest) {
    std::cout << first << " ";
    print(rest...);
}

int main() {
    print(1, "hello", 3.14); // 输出：1 hello 3.14
}
```

## 函数指针与函数对象

### 函数指针：

```cpp
void hello(int a) {
    std::cout << "a = " << a << std::endl;
}

void call(void (*func)(int)) {
    func(42);
}

int main() {
    call(hello);// 是传递函数本身的地址，call函数拿到hello函数地址再执行hello(42)
}
```

### 函数对象（Functor）：

```cpp
struct Printer {
    void operator()(int a) {
        std::cout << "Printer: " << a << std::endl;
    }
};

int main() {
    Printer p;
    p(10);  // 调用 operator()
}
```

### Lambda 表达式传参

```cpp
auto lambda = [](int x) { std::cout << x << std::endl; };

void run(std::function<void(int)> f) {
    f(100);
}

int main() {
    run(lambda);
}
```

## 总结对照表

| 方式       | 写法                             | 特点               |
| ---------- | -------------------------------- | ------------------ |
| 值传递     | `void foo(int x)`                | 复制，外部不变     |
| 引用传递   | `void foo(int& x)`               | 直接修改原值       |
| const 引用 | `void foo(const int& x)`         | 防止修改，避免拷贝 |
| 指针传递   | `void foo(int* p)`               | 手动管理指针       |
| 右值引用   | `void foo(std::string&& s)`      | 支持移动语义       |
| 默认参数   | `void foo(int x = 1)`            | 可省略某些参数     |
| 可变参数   | `template<typename... T>`        | 不定个数参数       |
| 函数重载   | `void foo(int), foo(double)`     | 同名不同参数       |
| 函数对象   | `struct F { void operator()();}` | 类似函数的对象     |
| Lambda     | `[](int x){}`                    | 匿名函数，简洁     |