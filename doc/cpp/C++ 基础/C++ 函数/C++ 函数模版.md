# C++ 函数模板

## 目录
- [1. 函数模板基础](#1-函数模板基础)
- [2. 模板参数推导](#2-模板参数推导)
- [3. 显式模板实参](#3-显式模板实参)
- [4. 模板参数类型](#4-模板参数类型)
- [5. 函数模板特化](#5-函数模板特化)
- [6. 函数模板重载](#6-函数模板重载)
- [7. 可变参数模板](#7-可变参数模板)
- [8. SFINAE 与类型萃取](#8-sfinae-与类型萃取)
- [9. 完美转发](#9-完美转发)
- [10. 编译期计算](#10-编译期计算)
- [11. 最佳实践与常见陷阱](#11-最佳实践与常见陷阱)

---

## 1. 函数模板基础

### 1.1 什么是函数模板

函数模板是C++中实现泛型编程的核心机制之一。它允许我们编写类型无关的通用代码，由编译器根据实际使用的类型自动生成具体的函数实例。

**基本语法：**
```cpp
template <typename T>
T max(T a, T b) {
    return a > b ? a : b;
}

// 使用
int main() {
    int i = max(3, 7);           // T = int
    double d = max(3.14, 2.71);  // T = double
    std::string s = max(std::string("hello"), std::string("world")); // T = std::string
}
```

### 1.2 模板的实例化

**两种实例化方式：**

1. **隐式实例化**：编译器根据函数调用自动推导类型
```cpp
template <typename T>
void print(T value) {
    std::cout << value << std::endl;
}

print(42);        // 隐式实例化 print<int>
print("hello");   // 隐式实例化 print<const char*>
```

2. **显式实例化**：强制编译器生成特定类型的模板实例
```cpp
template <typename T>
T add(T a, T b) {
    return a + b;
}

// 显式实例化声明
template int add<int>(int, int);
template double add<double>(double, double);
```

### 1.3 模板定义的位置

**重要原则**：函数模板的定义通常放在头文件中。

```cpp
// math_utils.h
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

template <typename T>
T multiply(T a, T b) {
    return a * b;  // 定义必须在头文件中
}

#endif
```

**原因**：编译器需要看到完整的模板定义才能实例化，否则会产生链接错误。

---

## 2. 模板参数推导

### 2.1 基本推导规则

编译器通过函数实参推导模板类型参数：

```cpp
template <typename T>
void func(T param);

int x = 42;
const int cx = x;
const int& rx = x;

func(x);   // T = int,        param 类型为 int
func(cx);  // T = int,        param 类型为 int (忽略顶层const)
func(rx);  // T = int,        param 类型为 int (引用和顶层const被忽略)
```

### 2.2 引用参数的推导

```cpp
template <typename T>
void func1(T& param);  // 非const引用

template <typename T>
void func2(const T& param);  // const引用

int x = 42;
const int cx = x;
const int& rx = x;

func1(x);   // T = int,       param 类型为 int&
func1(cx);  // T = const int, param 类型为 const int&
func1(rx);  // T = const int, param 类型为 const int& (引用被忽略)

func2(x);   // T = int,       param 类型为 const int&
func2(cx);  // T = int,       param 类型为 const int&
func2(rx);  // T = int,       param 类型为 const int&
```

### 2.3 指针参数的推导

```cpp
template <typename T>
void func(T* param);

int x = 42;
const int* px = &x;

func(&x);   // T = int,       param 类型为 int*
func(px);   // T = const int, param 类型为 const int*
```

### 2.4 万能引用与引用折叠

```cpp
template <typename T>
void func(T&& param);  // 万能引用(Universal Reference)

int x = 42;
const int cx = x;

func(x);           // x是左值，T = int&,       param 类型为 int&
func(cx);          // cx是左值，T = const int&, param 类型为 const int&
func(42);          // 42是右值，T = int,       param 类型为 int&&
func(std::move(x)); // 右值引用，T = int,       param 类型为 int&&
```

**引用折叠规则**：
- `T& &`   → `T&`
- `T& &&`  → `T&`
- `T&& &`  → `T&`
- `T&& &&` → `T&&`

### 2.5 数组和函数的推导

```cpp
template <typename T>
void func1(T param);  // 按值传递

template <typename T>
void func2(T& param);  // 按引用传递

const char name[] = "Hello";
void someFunc(int, double);

func1(name);     // T = const char*,  数组退化为指针
func2(name);     // T = const char[6], 保留数组类型

func1(someFunc); // T = void(*)(int, double), 函数退化为函数指针
func2(someFunc); // T = void(int, double),    保留函数类型
```

### 2.6 推导失败的情况

```cpp
template <typename T>
void func(T a, T b);

func(1, 2.5);  // 错误！无法推导T：第一个参数推导为int，第二个为double

// 解决方案1：显式指定类型
func<double>(1, 2.5);

// 解决方案2：使用两个类型参数
template <typename T1, typename T2>
void func2(T1 a, T2 b);

func2(1, 2.5);  // OK: T1 = int, T2 = double
```

---

## 3. 显式模板实参

### 3.1 显式指定类型

```cpp
template <typename T>
T convert(const std::string& str) {
    // 转换逻辑
}

// 无法从参数推导T，必须显式指定
int i = convert<int>("42");
double d = convert<double>("3.14");
```

### 3.2 部分显式指定

```cpp
template <typename T1, typename T2, typename T3>
T1 func(T2 a, T3 b);

// 只显式指定返回类型
auto result = func<long>(3, 4.5);  // T1 = long, T2 = int, T3 = double
```

**规则**：显式指定的模板参数必须从左到右依次匹配。

### 3.3 尾置返回类型（C++11）

```cpp
template <typename T1, typename T2>
auto add(T1 a, T2 b) -> decltype(a + b) {
    return a + b;
}

auto result = add(3, 4.5);  // 返回类型为 double
```

### 3.4 返回类型推导（C++14）

```cpp
template <typename T1, typename T2>
auto add(T1 a, T2 b) {
    return a + b;  // 编译器自动推导返回类型
}

template <typename T>
decltype(auto) getElement(T& container, size_t index) {
    return container[index];  // 保留引用语义
}
```

---

## 4. 模板参数类型

### 4.1 类型参数

最常见的模板参数：

```cpp
template <typename T>  // 推荐使用 typename
void func(T param);

template <class T>     // 等价于 typename（旧式写法）
void func2(T param);
```

### 4.2 非类型参数

接受编译期常量作为模板参数：

```cpp
template <typename T, size_t N>
class Array {
    T data[N];
public:
    size_t size() const { return N; }
};

Array<int, 10> arr;  // 创建包含10个int的数组

template <int N>
int factorial() {
    return N * factorial<N - 1>();
}

template <>
int factorial<0>() {
    return 1;
}

int result = factorial<5>();  // 编译期计算：120
```

**非类型参数的限制**：
- 必须是整型、枚举、指针、引用或 `std::nullptr_t`
- C++20 开始支持浮点类型和字面类型

### 4.3 模板模板参数

模板本身作为模板参数：

```cpp
template <typename T, template <typename> class Container>
class Stack {
    Container<T> data;
public:
    void push(const T& value) {
        data.push_back(value);
    }
};

// 使用
Stack<int, std::vector> intStack;
Stack<std::string, std::list> stringStack;
```

### 4.4 默认模板参数

```cpp
template <typename T, typename Allocator = std::allocator<T>>
class Vector {
    // 实现
};

Vector<int> v1;                           // 使用默认分配器
Vector<int, MyAllocator<int>> v2;         // 使用自定义分配器

template <typename T = int>
void print(T value = T()) {
    std::cout << value << std::endl;
}

print();        // T = int, value = 0
print(3.14);    // T = double, value = 3.14
```

---

## 5. 函数模板特化

### 5.1 全特化

为特定类型提供完全定制的实现：

```cpp
// 通用模板
template <typename T>
void process(T value) {
    std::cout << "Generic: " << value << std::endl;
}

// 针对 const char* 的全特化
template <>
void process<const char*>(const char* value) {
    std::cout << "String: " << value << std::endl;
}

// 针对 bool 的全特化
template <>
void process<bool>(bool value) {
    std::cout << "Boolean: " << (value ? "true" : "false") << std::endl;
}

process(42);         // 调用通用版本
process("hello");    // 调用 const char* 特化版本
process(true);       // 调用 bool 特化版本
```

### 5.2 函数模板不支持偏特化

**注意**：函数模板不支持偏特化，只能全特化。

```cpp
// 错误示例（函数模板不支持偏特化）
template <typename T>
void func(T value);

template <typename T>
void func<T*>(T* value);  // 编译错误！

// 解决方案：使用重载
template <typename T>
void func(T value) {
    std::cout << "General version" << std::endl;
}

template <typename T>
void func(T* value) {  // 这是重载，不是偏特化
    std::cout << "Pointer version" << std::endl;
}
```

### 5.3 特化的匹配规则

当有多个匹配时，编译器选择最特化的版本：

```cpp
template <typename T>
void func(T value) { std::cout << "1: Generic" << std::endl; }

template <typename T>
void func(T* value) { std::cout << "2: Pointer" << std::endl; }

template <typename T>
void func(const T* value) { std::cout << "3: Const Pointer" << std::endl; }

template <>
void func<int>(int value) { std::cout << "4: int specialization" << std::endl; }

int x = 42;
const int cx = 42;
int* px = &x;
const int* pcx = &cx;

func(x);    // 调用 4
func(px);   // 调用 2
func(pcx);  // 调用 3
```

---

## 6. 函数模板重载

### 6.1 重载规则

函数模板可以与其他模板或非模板函数重载：

```cpp
// 1. 通用模板
template <typename T>
void print(T value) {
    std::cout << "Generic: " << value << std::endl;
}

// 2. 针对指针的重载
template <typename T>
void print(T* value) {
    std::cout << "Pointer: " << *value << std::endl;
}

// 3. 非模板重载
void print(int value) {
    std::cout << "Int: " << value << std::endl;
}

int x = 42;
double d = 3.14;
int* px = &x;

print(x);    // 调用非模板版本（完全匹配优先）
print(d);    // 调用通用模板
print(px);   // 调用指针模板重载
```

### 6.2 重载解析优先级

1. **非模板函数**（完全匹配）
2. **特化的函数模板**
3. **普通函数模板**

```cpp
template <typename T>
std::string type_name(T) { return "template"; }

template <>
std::string type_name<int>(int) { return "template<int>"; }

std::string type_name(int) { return "non-template"; }

std::cout << type_name(42) << std::endl;     // 输出: non-template
std::cout << type_name(3.14) << std::endl;   // 输出: template
```

### 6.3 重载与参数数量

```cpp
template <typename T>
T max(T a, T b) {
    return a > b ? a : b;
}

template <typename T>
T max(T a, T b, T c) {
    return max(max(a, b), c);
}

template <typename T>
T max(std::initializer_list<T> list) {
    return *std::max_element(list.begin(), list.end());
}

auto m1 = max(3, 7);              // 调用两参数版本
auto m2 = max(3, 7, 5);           // 调用三参数版本
auto m3 = max({1, 9, 3, 5, 2});   // 调用 initializer_list 版本
```

---

## 7. 可变参数模板

### 7.1 基本语法

可变参数模板允许接受任意数量的模板参数：

```cpp
template <typename... Args>
void print(Args... args) {
    // args 是参数包
}

print(1, 2.5, "hello", 'c');  // Args = int, double, const char*, char
```

### 7.2 参数包展开

**递归展开**：
```cpp
// 递归终止条件
void print() {
    std::cout << std::endl;
}

// 递归模板
template <typename T, typename... Args>
void print(T first, Args... rest) {
    std::cout << first << " ";
    print(rest...);  // 递归调用
}

print(1, 2.5, "hello");  // 输出: 1 2.5 hello
```

**折叠表达式（C++17）**：
```cpp
template <typename... Args>
auto sum(Args... args) {
    return (args + ...);  // 右折叠
}

template <typename... Args>
void print(Args... args) {
    ((std::cout << args << " "), ...);  // 逗号折叠
    std::cout << std::endl;
}

auto s = sum(1, 2, 3, 4, 5);  // s = 15
print(1, 2.5, "hello");       // 输出: 1 2.5 hello
```

### 7.3 参数包大小

```cpp
template <typename... Args>
void func(Args... args) {
    constexpr size_t count = sizeof...(Args);  // 参数包大小
    std::cout << "参数数量: " << count << std::endl;
}

func(1, 2, 3);  // 输出: 参数数量: 3
```

### 7.4 完美转发与可变参数

```cpp
template <typename... Args>
void wrapper(Args&&... args) {
    // 完美转发所有参数
    actual_function(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

auto ptr = make_unique<std::string>(10, 'a');  // 创建包含10个'a'的字符串
```

### 7.5 参数包索引访问

```cpp
template <size_t Index, typename T, typename... Args>
decltype(auto) get_nth_element(T&& first, Args&&... rest) {
    if constexpr (Index == 0) {
        return std::forward<T>(first);
    } else {
        return get_nth_element<Index - 1>(std::forward<Args>(rest)...);
    }
}

auto value = get_nth_element<2>(1, 2.5, "hello", 'c');  // value = "hello"
```

---

## 8. SFINAE 与类型萃取

### 8.1 SFINAE 原理

**SFINAE**（Substitution Failure Is Not An Error）：替换失败不是错误。

当模板参数替换失败时，编译器不会报错，而是从重载集中移除该候选。

```cpp
// 仅对整数类型启用
template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
process(T value) {
    return value * 2;
}

// 仅对浮点类型启用
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
process(T value) {
    return value * 0.5;
}

auto i = process(10);    // 调用整数版本，返回 20
auto d = process(10.0);  // 调用浮点版本，返回 5.0
```

### 8.2 std::enable_if 的使用

**作为返回类型**：
```cpp
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
is_positive(T value) {
    return value > 0;
}
```

**作为模板参数**：
```cpp
template <typename T, 
          typename = typename std::enable_if<std::is_integral<T>::value>::type>
void func(T value) {
    // 只接受整数类型
}
```

**作为函数参数（C++14简化）**：
```cpp
template <typename T>
std::enable_if_t<std::is_integral_v<T>, void>  // C++14/17 简化语法
func(T value) {
    // 实现
}
```

### 8.3 constexpr if（C++17）

C++17引入的编译期条件分支，替代SFINAE的简洁方案：

```cpp
template <typename T>
auto process(T value) {
    if constexpr (std::is_integral_v<T>) {
        return value * 2;
    } else if constexpr (std::is_floating_point_v<T>) {
        return value * 0.5;
    } else {
        return value;
    }
}
```

### 8.4 类型萃取示例

```cpp
// 检测类型是否有 size() 成员函数
template <typename T, typename = void>
struct has_size : std::false_type {};

template <typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> 
    : std::true_type {};

// 使用
template <typename T>
void print_size(const T& container) {
    if constexpr (has_size<T>::value) {
        std::cout << "Size: " << container.size() << std::endl;
    } else {
        std::cout << "No size() method" << std::endl;
    }
}
```

### 8.5 Concepts（C++20）

C++20引入的concepts提供了更清晰的约束语法：

```cpp
template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

template <typename T>
concept HasSize = requires(T t) {
    { t.size() } -> std::convertible_to<size_t>;
};

// 使用 concept 约束模板
template <Numeric T>
T double_value(T value) {
    return value * 2;
}

template <HasSize T>
void print_size(const T& container) {
    std::cout << "Size: " << container.size() << std::endl;
}
```

---

## 9. 完美转发

### 9.1 问题背景

在编写泛型包装函数时，我们希望保持参数的原始属性（左值/右值、const等）：

```cpp
// 不完美的转发
template <typename T>
void wrapper(T param) {
    func(param);  // param 总是左值，无法转发右值
}
```

### 9.2 完美转发实现

使用万能引用和 `std::forward`：

```cpp
template <typename T>
void wrapper(T&& param) {
    func(std::forward<T>(param));  // 完美转发
}

// std::forward 的简化实现
template <typename T>
T&& forward(typename std::remove_reference<T>::type& param) {
    return static_cast<T&&>(param);
}
```

### 9.3 std::forward 的工作原理

```cpp
int x = 42;

wrapper(x);           // T = int&,  param 为 int&,  forward<int&>  返回 int&
wrapper(42);          // T = int,   param 为 int&&, forward<int>   返回 int&&
wrapper(std::move(x)); // T = int,   param 为 int&&, forward<int>   返回 int&&
```

### 9.4 实际应用场景

**工厂函数**：
```cpp
template <typename T, typename... Args>
std::shared_ptr<T> make_shared(Args&&... args) {
    return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
}

class MyClass {
public:
    MyClass(int a, const std::string& b) { }
};

auto ptr = make_shared<MyClass>(42, "hello");
```

**包装器模式**：
```cpp
template <typename Func, typename... Args>
auto measure_time(Func&& func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // 完美转发函数和参数
    auto result = std::forward<Func>(func)(std::forward<Args>(args)...);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "执行时间: " << duration.count() << "ms" << std::endl;
    return result;
}
```

### 9.5 常见陷阱

**多次转发**：
```cpp
template <typename T>
void bad_wrapper(T&& param) {
    func1(std::forward<T>(param));
    func2(std::forward<T>(param));  // 危险！param 可能已被移动
}

// 正确做法
template <typename T>
void good_wrapper(T&& param) {
    func1(param);  // 使用左值
    func2(std::forward<T>(param));  // 最后一次使用时转发
}
```

---

## 10. 编译期计算

### 10.1 constexpr 函数

C++11 引入 constexpr，允许在编译期执行函数：

```cpp
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

constexpr int result = factorial(5);  // 编译期计算
int arr[factorial(4)];  // 数组大小必须是编译期常量
```

### 10.2 编译期类型计算

**类型选择**：
```cpp
template <bool Condition, typename T, typename F>
struct conditional {
    using type = T;
};

template <typename T, typename F>
struct conditional<false, T, F> {
    using type = F;
};

// C++11 标准库提供
using result_t = std::conditional<true, int, double>::type;  // int
```

**编译期斐波那契**：
```cpp
template <int N>
struct Fibonacci {
    static constexpr int value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value;
};

template <>
struct Fibonacci<0> {
    static constexpr int value = 0;
};

template <>
struct Fibonacci<1> {
    static constexpr int value = 1;
};

constexpr int fib10 = Fibonacci<10>::value;  // 55
```

### 10.3 编译期循环（C++17）

使用折叠表达式和 if constexpr：

```cpp
template <size_t... Indices>
void print_indices(std::index_sequence<Indices...>) {
    ((std::cout << Indices << " "), ...);
    std::cout << std::endl;
}

print_indices(std::make_index_sequence<5>{});  // 输出: 0 1 2 3 4

// 编译期遍历 tuple
template <typename Tuple, size_t... Indices>
void print_tuple_impl(const Tuple& t, std::index_sequence<Indices...>) {
    ((std::cout << std::get<Indices>(t) << " "), ...);
}

template <typename... Args>
void print_tuple(const std::tuple<Args...>& t) {
    print_tuple_impl(t, std::index_sequence_for<Args...>{});
    std::cout << std::endl;
}

print_tuple(std::make_tuple(1, 2.5, "hello"));  // 输出: 1 2.5 hello
```

### 10.4 编译期字符串处理

```cpp
template <size_t N>
struct CompileTimeString {
    char data[N];
    
    constexpr CompileTimeString(const char (&str)[N]) {
        for (size_t i = 0; i < N; ++i) {
            data[i] = str[i];
        }
    }
    
    constexpr size_t length() const {
        return N - 1;
    }
};

constexpr CompileTimeString str("Hello");
static_assert(str.length() == 5);
```

---

## 11. 最佳实践与常见陷阱

### 11.1 最佳实践

**1. 优先使用 typename 而非 class**
```cpp
template <typename T>  // 推荐
void func(T param);

template <class T>     // 不推荐（旧式）
void func(T param);
```

**2. 模板定义放在头文件**
```cpp
// utils.h
template <typename T>
T max(T a, T b) {
    return a > b ? a : b;  // 定义在头文件中
}
```

**3. 使用 std::decay 去除引用和cv限定符**
```cpp
template <typename T>
void func(T param) {
    using bare_type = std::decay_t<T>;  // 去除引用、const、volatile
    // 使用 bare_type
}
```

**4. 使用 decltype(auto) 保留表达式类型**
```cpp
template <typename Container, typename Index>
decltype(auto) get_element(Container&& c, Index i) {
    return std::forward<Container>(c)[i];  // 保留引用语义
}
```

**5. 避免模板代码膨胀**
```cpp
// 不好：每个类型都生成一份代码
template <typename T>
void process(const std::vector<T>& vec) {
    // 大量代码...
}

// 好：提取通用部分到非模板函数
void process_impl(const void* data, size_t size, size_t element_size);

template <typename T>
void process(const std::vector<T>& vec) {
    process_impl(vec.data(), vec.size(), sizeof(T));
}
```

### 11.2 常见陷阱

**陷阱1：模板参数依赖查找**
```cpp
template <typename T>
class Derived : public Base<T> {
public:
    void func() {
        // 错误：编译器找不到 base_member
        base_member = 0;
        
        // 正确写法：
        this->base_member = 0;
        // 或者
        Base<T>::base_member = 0;
    }
};
```

**陷阱2：typename 关键字的必要性**
```cpp
template <typename T>
class MyClass {
public:
    // 错误：T::value_type 是否是类型？
    void func(T::value_type value);
    
    // 正确：使用 typename 声明这是类型
    void func(typename T::value_type value);
};
```

**陷阱3：模板的两阶段查找**
```cpp
void func(int) { std::cout << "int" << std::endl; }

template <typename T>
void call_func(T value) {
    func(value);  // 第一阶段：查找非依赖名称
}

void func(double) { std::cout << "double" << std::endl; }

call_func(3.14);  // 输出: int（令人惊讶！）
```

**陷阱4：std::forward 误用**
```cpp
template <typename T>
void bad(T&& param) {
    // 错误：param 是左值，即使绑定到右值
    func(param);  // 总是传递左值
}

template <typename T>
void good(T&& param) {
    // 正确：使用 std::forward 保持值类别
    func(std::forward<T>(param));
}
```

**陷阱5：模板特化的顺序**
```cpp
// 错误的顺序
template <>
void func<int>(int value) { }  // 特化必须在声明之后

template <typename T>
void func(T value);

// 正确的顺序
template <typename T>
void func(T value);

template <>
void func<int>(int value) { }
```

### 11.3 调试技巧

**1. 使用 static_assert 进行编译期检查**
```cpp
template <typename T>
void func(T value) {
    static_assert(std::is_arithmetic<T>::value, "T must be arithmetic type");
}
```

**2. 类型输出技巧**
```cpp
template <typename T>
class TypeDisplayer;  // 只声明，不定义

template <typename T>
void show_type(T&&) {
    TypeDisplayer<T> dummy;  // 编译错误会显示 T 的实际类型
}

// 使用
int x = 42;
show_type(x);  // 编译错误会显示 T = int&
```

**3. 使用编译器内置函数**
```cpp
#include <cxxabi.h>

template <typename T>
std::string type_name() {
    int status;
    char* demangled = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
    std::string result(demangled);
    free(demangled);
    return result;
}

std::cout << type_name<decltype(x)>() << std::endl;
```

---

## 总结

### 关键要点

1. **函数模板提供类型无关的通用编程能力**
   - 编译器根据实参推导类型
   - 模板定义通常放在头文件中

2. **模板参数推导规则复杂但有规律**
   - 值传递忽略引用和顶层const
   - 引用传递保留const属性
   - 万能引用结合引用折叠实现完美转发

3. **SFINAE 和 type traits 实现编译期类型检查**
   - C++17 的 constexpr if 提供更简洁的语法
   - C++20 的 concepts 是未来方向

4. **可变参数模板处理任意数量参数**
   - 递归展开和折叠表达式两种方式
   - 结合完美转发实现通用包装器

5. **编译期计算提升性能**
   - constexpr 函数在编译期执行
   - 模板元编程实现复杂的编译期计算

### 学习路径建议

1. **基础阶段**：掌握基本语法和参数推导
2. **进阶阶段**：学习SFINAE、完美转发、可变参数模板
3. **高级阶段**：深入模板元编程、concepts、编译期计算
4. **实践阶段**：阅读STL源码，编写通用库

### 参考资源

- 《C++ Templates: The Complete Guide》（第2版）
- 《Effective Modern C++》by Scott Meyers
- 《C++ Template Metaprogramming》by David Abrahams
- cppreference.com - 最权威的在线参考
- C++ Core Guidelines - 最佳实践指南

---

**更新日期**：2026年1月5日