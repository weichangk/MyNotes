# C++ 函数重载 (Function Overloading)

## 目录
- [1. 基本概念](#1-基本概念)
- [2. 重载决议机制](#2-重载决议机制)
- [3. 重载规则详解](#3-重载规则详解)
- [4. 深入理解](#4-深入理解)
- [5. 最佳实践](#5-最佳实践)
- [6. 常见陷阱](#6-常见陷阱)
- [7. 高级主题](#7-高级主题)

---

## 1. 基本概念

### 1.1 什么是函数重载

函数重载(Function Overloading)是 C++ 多态性的一种体现，允许在同一作用域内声明多个同名但参数列表不同的函数。编译器根据调用时提供的实参类型和数量来决定调用哪个函数。

```cpp
// 函数重载示例
void print(int x) {
    std::cout << "整数: " << x << std::endl;
}

void print(double x) {
    std::cout << "浮点数: " << x << std::endl;
}

void print(const std::string& x) {
    std::cout << "字符串: " << x << std::endl;
}

// 使用
print(10);           // 调用 print(int)
print(3.14);         // 调用 print(double)
print("Hello");      // 调用 print(const std::string&)
```

### 1.2 为什么需要函数重载

**好处：**
- **语义清晰**：使用相同的函数名表达相同的操作概念
- **易于使用**：用户不需要记忆不同的函数名
- **类型安全**：编译期确定调用哪个函数
- **代码可维护性**：逻辑相似的操作集中管理

**示例对比：**
```cpp
// 没有重载的 C 风格
int add_int(int a, int b);
double add_double(double a, double b);
float add_float(float a, float b);

// 使用重载的 C++ 风格
int add(int a, int b);
double add(double a, double b);
float add(float a, float b);
```

---

## 2. 重载决议机制

### 2.1 重载决议的三个步骤

编译器在遇到函数调用时，会按以下步骤进行重载决议：

#### **步骤1：候选函数集合 (Candidate Functions)**
找到所有与调用名称相同的函数。

#### **步骤2：可行函数集合 (Viable Functions)**
从候选函数中筛选出参数数量匹配且类型可转换的函数。

#### **步骤3：最佳匹配 (Best Match)**
从可行函数中选择参数匹配度最高的函数。

```cpp
void func(int x);           // #1
void func(double x);        // #2
void func(int x, int y);    // #3

func(10);      // 候选: #1, #2, #3 -> 可行: #1, #2 -> 最佳: #1 (精确匹配)
func(3.14);    // 候选: #1, #2, #3 -> 可行: #1, #2 -> 最佳: #2 (精确匹配)
func(10, 20);  // 候选: #1, #2, #3 -> 可行: #3 -> 最佳: #3 (唯一匹配)
```

### 2.2 匹配等级

编译器按以下优先级顺序进行匹配：

| 优先级 | 匹配类型 | 说明 | 示例 |
|--------|---------|------|------|
| 1 | **精确匹配** | 参数类型完全相同 | `int` → `int` |
| 2 | **平凡转换** | 数组到指针、函数到指针、左值到右值 | `int[]` → `int*` |
| 3 | **提升转换** | `char/short` → `int`，`float` → `double` | `char` → `int` |
| 4 | **标准转换** | 整数转换、浮点转换、指针转换 | `int` → `double` |
| 5 | **用户定义转换** | 通过构造函数或转换运算符 | `int` → `MyClass` |

```cpp
void process(int x);         // #1
void process(double x);      // #2
void process(long x);        // #3

char c = 'A';
process(c);                  // 调用 #1: char 提升为 int (提升转换优于标准转换)

short s = 10;
process(s);                  // 调用 #1: short 提升为 int

float f = 3.14f;
process(f);                  // 调用 #2: float 提升为 double

long l = 100L;
process(l);                  // 调用 #3: 精确匹配
```

### 2.3 二义性问题

当有多个函数匹配程度相同时，会产生编译错误：

```cpp
void func(int x, double y);      // #1
void func(double x, int y);      // #2

func(10, 20);  // ❌ 编译错误: 二义性调用
               // 10 可以转为 double, 20 可以转为 double
               // #1 和 #2 的匹配程度相同

// 解决方法1: 显式类型转换
func(10, 20.0);                  // 调用 #1
func(10.0, 20);                  // 调用 #2

// 解决方法2: 添加精确匹配的重载
void func(int x, int y);         // #3
func(10, 20);                    // 调用 #3 (精确匹配)
```

---

## 3. 重载规则详解

### 3.1 可以构成重载的情况

#### (1) 参数数量不同

```cpp
void draw();
void draw(int x);
void draw(int x, int y);
void draw(int x, int y, int z);
```

#### (2) 参数类型不同

```cpp
void setValue(int val);
void setValue(double val);
void setValue(const std::string& val);
void setValue(bool val);
```

#### (3) 参数顺序不同（类型组合不同）

```cpp
void configure(int id, const std::string& name);
void configure(const std::string& name, int id);
```

#### (4) const 和非 const（针对成员函数）

```cpp
class MyClass {
public:
    void getData();              // 非 const 成员函数
    void getData() const;        // const 成员函数
};

MyClass obj;
const MyClass constObj;

obj.getData();                   // 调用非 const 版本
constObj.getData();              // 调用 const 版本
```

#### (5) 引用限定符（C++11）

```cpp
class Widget {
public:
    void process() &;            // 左值对象调用
    void process() &&;           // 右值对象调用
};

Widget w;
w.process();                     // 调用 & 版本
Widget().process();              // 调用 && 版本
```

### 3.2 不能构成重载的情况

#### (1) ❌ 仅返回值类型不同

```cpp
int getValue();
double getValue();               // ❌ 编译错误: 返回类型不能区分重载
```

**原因**：调用时无法根据返回值确定调用哪个函数：
```cpp
getValue();  // 无法确定应该调用哪个版本
```

#### (2) ❌ 仅 typedef 或类型别名不同

```cpp
typedef int Integer;
using Long = long;

void func(int x);
void func(Integer x);            // ❌ 错误: Integer 就是 int
void func(long y);               // ✅ 正确: long 与 int 是不同类型
void func(Long y);               // ❌ 错误: Long 就是 long (已定义)
```

#### (3) ❌ 仅参数名不同

```cpp
void process(int x);
void process(int y);             // ❌ 错误: 仅参数名不同
```

#### (4) ❌ 仅默认参数不同

```cpp
void setup(int x = 10);
void setup(int x = 20);          // ❌ 错误: 重复定义
```

#### (5) ❌ 数组与指针参数

```cpp
void func(int arr[]);
void func(int* arr);             // ❌ 错误: 数组参数会退化为指针
                                 // 两者本质上是同一种类型
```

#### (6) ❌ 顶层 const 不能构成重载（非引用和指针）

```cpp
void func(int x);
void func(const int x);          // ❌ 错误: 顶层 const 对于传值方式无区别
                                 // 因为传值时会拷贝，const 不影响函数签名

// 但底层 const 可以构成重载
void func(int* p);               // ✅ 
void func(const int* p);         // ✅ 正确: 底层 const 有区别

void func(int& r);               // ✅
void func(const int& r);         // ✅ 正确: 引用的 const 有区别
```

---

## 4. 深入理解

### 4.1 Name Mangling (名称修饰)

C++ 编译器通过名称修饰将函数签名编码到符号名中，以支持函数重载。

```cpp
void func(int);
void func(double);
void func(int, double);
```

编译后的符号可能是：
```
_Z4funci          // void func(int)
_Z4funcd          // void func(double)
_Z4funcid         // void func(int, double)
```

**不同编译器的名称修饰规则不同**，这就是为什么需要 `extern "C"` 来禁用名称修饰以实现 C 兼容：

```cpp
extern "C" {
    void c_function(int x);      // 不进行名称修饰
}

// C++ 函数
void cpp_function(int x);        // 进行名称修饰
```

### 4.2 函数重载与模板的关系

模板函数和非模板函数可以共存，编译器优先选择非模板函数：

```cpp
// 模板版本
template<typename T>
void print(T value) {
    std::cout << "模板: " << value << std::endl;
}

// 特化版本
void print(int value) {
    std::cout << "int 特化: " << value << std::endl;
}

print(3.14);       // 调用模板版本
print(42);         // 调用 int 特化版本 (精确匹配优先)
```

**重载决议顺序：**
1. 非模板函数精确匹配
2. 模板函数特化
3. 模板函数推导
4. 类型转换

```cpp
template<typename T>
void process(T value);                    // #1 模板

void process(int value);                  // #2 非模板

template<>
void process<double>(double value);       // #3 显式特化

process(10);         // 调用 #2 (非模板精确匹配)
process(3.14);       // 调用 #3 (显式特化)
process("hello");    // 调用 #1 (模板推导)
```

### 4.3 函数重载与继承

派生类中的同名函数会**隐藏**基类中的所有重载版本：

```cpp
class Base {
public:
    void func(int x);             // #1
    void func(double x);          // #2
};

class Derived : public Base {
public:
    void func(std::string s);     // #3 - 隐藏了 Base 的所有 func
};

Derived d;
d.func("hello");     // ✅ 调用 #3
d.func(10);          // ❌ 编译错误: Base::func(int) 被隐藏
```

**解决方案1：使用 using 声明**
```cpp
class Derived : public Base {
public:
    using Base::func;             // 将基类的 func 引入派生类作用域
    void func(std::string s);     // #3
};

Derived d;
d.func("hello");     // ✅ 调用 #3
d.func(10);          // ✅ 调用 Base::func(int)
d.func(3.14);        // ✅ 调用 Base::func(double)
```

**解决方案2：显式调用**
```cpp
Derived d;
d.Base::func(10);    // 显式调用基类版本
```

### 4.4 ADL (Argument-Dependent Lookup)

参数依赖查找会在参数类型所在的命名空间中查找函数：

```cpp
namespace MyLib {
    class MyClass {};
    
    void process(MyClass obj) {
        std::cout << "MyLib::process" << std::endl;
    }
}

void process(MyLib::MyClass obj) {
    std::cout << "::process" << std::endl;
}

MyLib::MyClass obj;
process(obj);        // 调用哪个？

// ADL 会同时考虑：
// 1. 全局命名空间的 ::process
// 2. MyLib 命名空间的 MyLib::process（因为参数类型在这里定义）
// 结果: 二义性错误！需要显式指定命名空间
```

---

## 5. 最佳实践

### 5.1 设计原则

#### (1) 语义一致性

所有重载版本应该执行语义相同的操作：

```cpp
// ✅ 好的设计
void print(int x);
void print(double x);
void print(const std::string& x);
// 所有版本都是"打印"操作

// ❌ 不好的设计
void process(int x);        // 计算平方
void process(double x);     // 计算立方根
void process(const std::string& x);  // 转换为大写
// 虽然名字相同，但语义完全不同
```

#### (2) 避免过度重载

```cpp
// ❌ 过度重载，难以理解
void calculate(int a, int b);
void calculate(int a, double b);
void calculate(double a, int b);
void calculate(double a, double b);
void calculate(int a, int b, int c);
void calculate(int a, int b, double c);
// ... 太多了！

// ✅ 更好的设计：使用模板或默认参数
template<typename T1, typename T2>
auto calculate(T1 a, T2 b) {
    return a + b;
}
```

#### (3) 参数转换要明确

```cpp
class String {
public:
    String(const char* str);           // 隐式转换
    explicit String(int capacity);     // 禁止隐式转换
};

void process(const String& s);

process("hello");      // ✅ 隐式转换 const char* -> String
process(100);          // ❌ 错误: explicit 构造函数不能隐式调用
process(String(100));  // ✅ 显式构造
```

### 5.2 性能考虑

#### (1) 避免不必要的拷贝

```cpp
// ❌ 效率低：传值
void process(std::string str);
void process(std::vector<int> vec);

// ✅ 更好：使用 const 引用
void process(const std::string& str);
void process(const std::vector<int>& vec);

// ✅ 现代 C++：使用转发引用和移动语义
void process(std::string&& str);      // 右值引用版本
template<typename T>
void process(T&& value);              // 完美转发
```

#### (2) 小类型传值，大类型传引用

```cpp
// 小类型：传值更高效（避免间接寻址）
void setPosition(int x, int y);
void setColor(uint32_t rgba);

// 大类型：传引用避免拷贝
void setData(const std::vector<int>& data);
void setConfig(const Configuration& cfg);
```

### 5.3 可读性和可维护性

#### (1) 使用默认参数代替简单重载

```cpp
// ❌ 多个重载
void connect(const std::string& host);
void connect(const std::string& host, int port);
void connect(const std::string& host, int port, int timeout);

// ✅ 使用默认参数
void connect(const std::string& host, 
             int port = 80, 
             int timeout = 30);
```

#### (2) 明确的命名

```cpp
// 当参数类型不能清晰表达意图时，使用不同的函数名
// ❌ 不清晰
void load(const std::string& path);      // 从文件加载？
void load(const char* data, size_t len); // 从内存加载？

// ✅ 更清晰
void loadFromFile(const std::string& path);
void loadFromMemory(const char* data, size_t len);
```

---

## 6. 常见陷阱

### 6.1 意外的类型转换

```cpp
void func(int x);
void func(long x);

func(3.14);  // ⚠️ 警告: 调用 func(int)，浮点数被截断
             // 编译器不会选择 func(long)
             // 因为 double -> int 和 double -> long 是同等级的标准转换
```

**解决方案：**
```cpp
void func(int x);
void func(long x);
void func(double x);  // 添加 double 版本

func(3.14);  // 现在调用 func(double)
```

### 6.2 NULL 和 nullptr

```cpp
void func(int x);
void func(char* ptr);

func(NULL);      // ⚠️ 可能调用 func(int)！
                 // NULL 通常定义为 0 或 ((void*)0)
                 
func(nullptr);   // ✅ 调用 func(char*)
                 // C++11 的 nullptr 是正确的空指针
```

### 6.3 数组退化

```cpp
void func(int arr[10]);   // 实际上是 int*
void func(int arr[20]);   // 实际上还是 int*，无法重载
void func(int* arr);      // 错误: 重复定义

// 解决方案：使用引用
void func(int (&arr)[10]);  // 10 元素数组的引用
void func(int (&arr)[20]);  // 20 元素数组的引用 - 可以重载
```

### 6.4 临时对象的生命周期

```cpp
void process(const std::string& str);
void process(std::string&& str);

process("hello");  // 调用哪个？
// "hello" 会创建临时 std::string 对象
// 临时对象是右值，优先匹配 process(std::string&&)
// 如果没有右值引用版本，则匹配 const 左值引用版本
```

### 6.5 模板与重载的交互

```cpp
template<typename T>
void func(T value);           // #1

void func(int value);         // #2

template<typename T>
void func(T* ptr);            // #3

int x = 10;
int* p = &x;

func(x);    // 调用 #2 (非模板精确匹配优先)
func(p);    // 调用 #3 (更特化的模板优先)

func<int>(x);  // 调用 #1 (显式指定模板)
```

---

## 7. 高级主题

### 7.1 SFINAE (Substitution Failure Is Not An Error)

利用 SFINAE 可以根据类型特性选择性地启用重载：

```cpp
#include <type_traits>

// 只对整数类型启用
template<typename T>
typename std::enable_if<std::is_integral<T>::value, void>::type
process(T value) {
    std::cout << "整数处理: " << value << std::endl;
}

// 只对浮点类型启用
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
process(T value) {
    std::cout << "浮点处理: " << value << std::endl;
}

process(42);      // 调用整数版本
process(3.14);    // 调用浮点版本
process("hello"); // 编译错误: 没有匹配的重载
```

**C++17 的 if constexpr 简化：**
```cpp
template<typename T>
void process(T value) {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "整数处理: " << value << std::endl;
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "浮点处理: " << value << std::endl;
    }
}
```

### 7.2 Concepts (C++20)

C++20 的 concepts 提供了更优雅的约束方式：

```cpp
#include <concepts>

// 使用 concept 约束
template<std::integral T>
void process(T value) {
    std::cout << "整数处理: " << value << std::endl;
}

template<std::floating_point T>
void process(T value) {
    std::cout << "浮点处理: " << value << std::endl;
}

// 自定义 concept
template<typename T>
concept Printable = requires(T t) {
    { std::cout << t } -> std::convertible_to<std::ostream&>;
};

template<Printable T>
void print(const T& value) {
    std::cout << value << std::endl;
}
```

### 7.3 重载与 decltype

```cpp
template<typename T1, typename T2>
auto add(T1 a, T2 b) -> decltype(a + b) {
    return a + b;
}

// C++14 的返回类型推导
template<typename T1, typename T2>
auto add(T1 a, T2 b) {
    return a + b;
}

add(1, 2);          // 返回 int
add(1, 2.5);        // 返回 double
add(std::string("Hello, "), "World");  // 返回 std::string
```

### 7.4 重载和完美转发

```cpp
class Logger {
public:
    // 左值引用版本
    template<typename T>
    void log(const T& message) {
        std::cout << "[L-Value] " << message << std::endl;
    }
    
    // 右值引用版本
    template<typename T>
    void log(T&& message) {
        std::cout << "[R-Value] " << message << std::endl;
        // 可以移动 message
    }
};

Logger logger;
std::string msg = "Hello";
logger.log(msg);              // 调用左值版本
logger.log("World");          // 调用右值版本
logger.log(std::string("!")); // 调用右值版本

// 通用引用和完美转发
template<typename... Args>
void forward_log(Args&&... args) {
    logger.log(std::forward<Args>(args)...);
}
```

### 7.5 重载解析中的 Conversion Sequence

```cpp
class MyInt {
    int value;
public:
    MyInt(int v) : value(v) {}
    operator int() const { return value; }
};

void func(int x);           // #1
void func(MyInt x);         // #2

MyInt mi(10);
func(mi);                   // 调用 #2 (精确匹配)
func(20);                   // 调用 #1 (精确匹配)

// 隐式转换链
func(static_cast<short>(5)); // short -> int (提升), 调用 #1

// 用户定义转换
MyInt mi2(10);
int x = mi2;                // MyInt -> int (用户定义转换)
```

---

## 8. 实战案例

### 8.1 构建类型安全的 API

```cpp
class Database {
public:
    // 查询重载 - 不同的参数类型表示不同的查询方式
    
    // 通过 ID 查询
    Result query(int id) {
        return queryById(id);
    }
    
    // 通过名称查询
    Result query(const std::string& name) {
        return queryByName(name);
    }
    
    // 通过复杂条件查询
    Result query(const QueryBuilder& builder) {
        return executeQuery(builder);
    }
    
    // 通过 SQL 直接查询
    Result query(const char* sql, size_t len) {
        return executeRawSQL(sql, len);
    }
    
private:
    Result queryById(int id);
    Result queryByName(const std::string& name);
    Result executeQuery(const QueryBuilder& builder);
    Result executeRawSQL(const char* sql, size_t len);
};

// 使用
Database db;
db.query(12345);                    // ID 查询
db.query("John");                   // 名称查询
db.query(QueryBuilder().where("age > 18")); // 条件查询
db.query("SELECT * FROM users", 21); // 原始 SQL
```

### 8.2 智能容器操作

```cpp
template<typename T>
class SmartVector {
    std::vector<T> data;
    
public:
    // 单个元素添加
    void add(const T& item) {
        data.push_back(item);
    }
    
    // 移动语义版本
    void add(T&& item) {
        data.push_back(std::move(item));
    }
    
    // 批量添加 - 初始化列表
    void add(std::initializer_list<T> items) {
        data.insert(data.end(), items.begin(), items.end());
    }
    
    // 批量添加 - 迭代器范围
    template<typename Iter>
    void add(Iter begin, Iter end) {
        data.insert(data.end(), begin, end);
    }
    
    // 批量添加 - 另一个容器
    void add(const SmartVector<T>& other) {
        data.insert(data.end(), other.data.begin(), other.data.end());
    }
};

// 使用
SmartVector<int> vec;
vec.add(1);                          // 单个元素
vec.add({2, 3, 4});                  // 初始化列表
std::vector<int> v = {5, 6};
vec.add(v.begin(), v.end());         // 迭代器范围
SmartVector<int> vec2;
vec.add(vec2);                       // 另一个容器
```

### 8.3 日志系统

```cpp
class Logger {
public:
    enum Level { DEBUG, INFO, WARNING, ERROR };
    
    // 简单消息
    void log(const std::string& msg) {
        log(INFO, msg);
    }
    
    // 带级别的消息
    void log(Level level, const std::string& msg) {
        output(level, msg);
    }
    
    // 格式化消息（模板参数包）
    template<typename... Args>
    void log(Level level, const std::string& format, Args&&... args) {
        std::string formatted = formatString(format, std::forward<Args>(args)...);
        output(level, formatted);
    }
    
    // 异常日志
    void log(const std::exception& e) {
        log(ERROR, std::string("Exception: ") + e.what());
    }
    
    // 对象日志（需要支持流操作符）
    template<typename T>
    void log(const T& obj) {
        std::ostringstream oss;
        oss << obj;
        log(INFO, oss.str());
    }
    
private:
    void output(Level level, const std::string& msg);
    
    template<typename... Args>
    std::string formatString(const std::string& format, Args&&... args);
};

// 使用
Logger logger;
logger.log("简单消息");
logger.log(Logger::WARNING, "警告消息");
logger.log(Logger::ERROR, "错误代码: %d", 404);
try {
    throw std::runtime_error("出错了");
} catch(const std::exception& e) {
    logger.log(e);  // 异常日志
}
```

---

## 9. 调试技巧

### 9.1 查看调用的重载版本

```cpp
// 技巧1: 使用 static_assert 和 type_traits
template<typename T>
void debug_type(T&&) {
    static_assert(std::is_same_v<T, void>, 
                  "Type is: see compiler error");
}

// 技巧2: 在函数内打印类型信息
#include <typeinfo>
#include <cxxabi.h>

template<typename T>
void show_type(T&& value) {
    int status;
    char* demangled = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    std::cout << "Type: " << demangled << std::endl;
    free(demangled);
}

// 技巧3: 编译器标志
// GCC/Clang: -Woverloaded-virtual
// MSVC: /Wall
```

### 9.2 使用 C++20 的 source_location

```cpp
#include <source_location>

void func(int x, 
          const std::source_location& loc = std::source_location::current()) {
    std::cout << "调用自: " 
              << loc.file_name() << ":" 
              << loc.line() << std::endl;
}

void func(double x,
          const std::source_location& loc = std::source_location::current()) {
    std::cout << "调用自: " 
              << loc.file_name() << ":" 
              << loc.line() << std::endl;
}
```

---

## 10. 总结

### 核心要点

1. **函数重载的本质**：编译期的函数选择机制，通过参数类型和数量区分
2. **重载决议**：候选 → 可行 → 最佳匹配的三步过程
3. **匹配优先级**：精确匹配 > 提升转换 > 标准转换 > 用户定义转换
4. **设计原则**：语义一致、避免二义性、考虑性能和可读性
5. **注意陷阱**：类型转换、NULL/nullptr、数组退化、const 限定

### 与其他特性的关系

| 特性 | 关系 | 说明 |
|------|------|------|
| 模板 | 互补 | 模板提供泛型，重载提供特化 |
| 继承 | 需注意 | 派生类会隐藏基类的重载 |
| 运算符重载 | 特殊形式 | 运算符重载是函数重载的特殊应用 |
| SFINAE | 高级技巧 | 利用替换失败控制重载集合 |
| Concepts | 现代方式 | C++20 提供更清晰的约束语法 |

### 什么时候使用函数重载

**✅ 适合使用：**
- 同一操作需要支持多种参数类型
- 需要类型安全和编译期检查
- 语义相同但实现细节不同
- 构建直观易用的 API

**❌ 避免使用：**
- 操作语义不同（用不同的函数名）
- 参数转换容易造成困惑
- 可以用默认参数简化的情况
- 会导致过多的重载版本

### 进一步学习

- **名称查找规则**：深入理解 ADL、qualified/unqualified lookup
- **模板元编程**：SFINAE、tag dispatch、if constexpr
- **完美转发**：std::forward 和通用引用
- **C++20 Concepts**：现代的类型约束机制
- **重载运算符**：函数重载的特殊应用

---

## 参考资料

- C++ Primer (5th Edition) - Stanley B. Lippman
- Effective C++ (3rd Edition) - Scott Meyers
- C++ Templates: The Complete Guide (2nd Edition) - David Vandevoorde
- C++ 标准文档: [over.match] Overload Resolution
- CppReference: https://en.cppreference.com/w/cpp/language/overload_resolution

---

*最后更新: 2026年1月5日*