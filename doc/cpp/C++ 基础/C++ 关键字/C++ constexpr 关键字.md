# C++ constexpr 关键字

`constexpr` 是 C++11 引入的关键字，表示"**编译期常量表达式**"。它告诉编译器：这个值或这个函数的结果**必须在编译期就能确定**。这是对 `const` 的补充和增强——`const` 只表示"不可修改"，而 `constexpr` 更进一步要求"编译期可求值"。

> 相关文档：[const 关键字](./C++%20const%20关键字.md) | [const 和 #define 对比](./C++%20const%20和%20%23define%20对比.md) | [const 符号表机制](./C++%20const%20符号表机制.md)

---

## 1. constexpr vs const

### 1.1 核心区别

| 特性 | `const` | `constexpr` |
|------|---------|-------------|
| 含义 | 变量**不可修改** | 变量的值**编译期可确定** |
| 是否一定是编译期常量 | ❌ 不一定 | ✅ 一定是 |
| 可用于数组大小 | ⚠️ 有条件（值必须编译期已知） | ✅ 保证可用 |
| 可用于模板参数 | ⚠️ 有条件 | ✅ 保证可用 |
| 可修饰函数 | ❌ 不能 | ✅ 可以 |
| 初始化来源 | 编译期常量 **或** 运行时值 | 只能是编译期常量 |

### 1.2 直观对比

```cpp
// const：只保证"不可修改"，不保证编译期可知
const int a = 10;        // ✅ 编译期已知（碰巧是）
const int b = rand();    // ✅ 合法！但 b 的值运行时才确定

// constexpr：保证"编译期可求值"
constexpr int c = 10;    // ✅ 编译期确定
constexpr int d = rand(); // ❌ 编译错误！rand() 不是编译期可求值的
```

### 1.3 作为数组大小和模板参数

```cpp
const int a = 10;
const int b = rand();
constexpr int c = 10;

int arr1[a];             // ✅ 编译器知道 a = 10（整型 const 常量折叠）
int arr2[b];             // ❌ 编译错误：b 不是编译期常量
int arr3[c];             // ✅ constexpr 保证编译期可知

template <int N>
struct Array { int data[N]; };

Array<a> x;              // ✅ const int 编译期已知时可用
Array<b> y;              // ❌ 编译错误
Array<c> z;              // ✅ constexpr 一定可用
```

> **经验法则：** 如果一个值需要在编译期使用（数组大小、模板参数、`switch case`、`static_assert`），用 `constexpr` 而不是 `const`。

---

## 2. constexpr 变量

### 2.1 基本用法

```cpp
constexpr int MAX_SIZE = 1024;
constexpr double PI = 3.14159265358979;
constexpr char NEWLINE = '\n';
constexpr bool ENABLE_LOG = true;
```

### 2.2 constexpr 变量的要求

```cpp
// ✅ 字面量
constexpr int a = 42;

// ✅ 由其他 constexpr 计算
constexpr int b = a * 2 + 1;

// ✅ constexpr 函数的返回值
constexpr int square(int x) { return x * x; }
constexpr int c = square(5);  // 25

// ❌ 运行时值
int runtime = 10;
constexpr int d = runtime;     // ❌ runtime 不是编译期常量

// ❌ 非 constexpr 函数的返回值
int getValue() { return 42; }
constexpr int e = getValue();  // ❌ getValue 不是 constexpr 函数
```

### 2.3 constexpr 隐含 const

`constexpr` 变量自动具有 `const` 属性：

```cpp
constexpr int MAX = 100;
MAX = 200;  // ❌ 编译错误：constexpr 变量不可修改（隐含 const）

// 等价于
const int MAX = 100;  // 但 constexpr 有更强的编译期保证
```

---

## 3. constexpr 函数

### 3.1 基本概念

`constexpr` 函数是**既能在编译期执行，也能在运行时执行**的函数：

```cpp
constexpr int square(int x) {
    return x * x;
}

// 编译期调用（参数是编译期常量）
constexpr int a = square(5);   // 编译期计算：25
int arr[square(3)];             // 编译期计算：arr[9]

// 运行时调用（参数是运行时值）
int n;
std::cin >> n;
int b = square(n);              // 运行时计算
```

```
┌─────────────────────────────────────────────────────────┐
│           constexpr 函数的双重身份                        │
│                                                         │
│  ┌──────────────────┐    ┌──────────────────┐           │
│  │   编译期调用       │    │   运行时调用       │          │
│  │                   │    │                   │          │
│  │ constexpr int r   │    │ int n = input();  │          │
│  │   = square(5);    │    │ int r = square(n);│          │
│  │                   │    │                   │          │
│  │ → 编译器直接算出   │    │ → 生成普通函数调用 │          │
│  │   25，嵌入代码     │    │   运行时计算       │          │
│  └──────────────────┘    └──────────────────┘           │
└─────────────────────────────────────────────────────────┘
```

### 3.2 C++11 中的限制

C++11 的 `constexpr` 函数限制非常严格：

```cpp
// C++11：函数体只能有一个 return 语句
constexpr int factorial_11(int n) {
    return n <= 1 ? 1 : n * factorial_11(n - 1);  // 只能用三元运算符
}
```

### 3.3 C++14 放宽限制

C++14 大幅放宽了 `constexpr` 函数的限制，允许：
- 多条语句
- 局部变量
- `if/else`、`switch`
- 循环（`for`、`while`）
- 修改局部变量

```cpp
// C++14：可以用正常的控制流
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

constexpr int f5 = factorial(5);  // 编译期计算：120
```

### 3.4 constexpr 函数的规则

```cpp
// ✅ 可以有多个参数
constexpr int add(int a, int b) { return a + b; }

// ✅ 可以递归
constexpr int fib(int n) {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}

// ❌ 不能有 static 局部变量（C++11~C++20）
constexpr int bad() {
    static int x = 0;  // ❌ 编译错误
    return x;
}

// ❌ 不能调用非 constexpr 函数
constexpr int bad2() {
    return rand();  // ❌ rand() 不是 constexpr
}

// ❌ 不能有 try-catch（C++20 前）
constexpr int bad3() {
    try { return 1; }  // ❌ C++20 前不允许
    catch (...) { return 0; }
}
```

### 3.5 实用示例

```cpp
// 编译期计算字符串长度
constexpr size_t strLen(const char* s) {
    size_t len = 0;
    while (s[len] != '\0') ++len;
    return len;
}

constexpr size_t len = strLen("Hello");  // 编译期计算：5
static_assert(strLen("World") == 5, "长度应为 5");

// 编译期计算幂
constexpr double power(double base, int exp) {
    double result = 1.0;
    for (int i = 0; i < exp; ++i)
        result *= base;
    return result;
}

constexpr double val = power(2.0, 10);  // 1024.0
```

---

## 4. constexpr 与类

### 4.1 constexpr 构造函数

类的构造函数可以声明为 `constexpr`，使得对象可以在编译期创建：

```cpp
struct Point {
    int x, y;

    constexpr Point(int x, int y) : x(x), y(y) {}

    constexpr int distSq() const {
        return x * x + y * y;
    }
};

// 编译期创建对象
constexpr Point origin(0, 0);
constexpr Point p(3, 4);

// 编译期调用成员函数
constexpr int dist = p.distSq();  // 25
static_assert(p.distSq() == 25, "3² + 4² = 25");

// 编译期数组
constexpr Point points[] = { {1, 2}, {3, 4}, {5, 6} };
static_assert(points[1].x == 3, "");
```

### 4.2 constexpr 构造函数的要求

```cpp
struct Valid {
    int a, b;
    
    // ✅ 成员初始化列表 + 空函数体（C++11）
    constexpr Valid(int a, int b) : a(a), b(b) {}
    
    // ✅ C++14 起可以有函数体
    constexpr Valid(int val) : a(val), b(val * 2) {
        // 可以有语句，但不能有 virtual / try-catch
    }
};

struct Invalid {
    int a;
    virtual void func() {}         // ❌ 有虚函数的类不能有 constexpr 构造（C++20 前）
    // constexpr Invalid() : a(0) {}  // 编译错误
};
```

### 4.3 实际应用：编译期配置

```cpp
struct Color {
    uint8_t r, g, b, a;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}
};

// 编译期定义颜色常量——零运行时开销
constexpr Color RED   {255, 0,   0  };
constexpr Color GREEN {0,   255, 0  };
constexpr Color BLUE  {0,   0,   255};
constexpr Color WHITE {255, 255, 255};

// 编译期混合颜色
constexpr Color blend(Color a, Color b) {
    return Color(
        (a.r + b.r) / 2,
        (a.g + b.g) / 2,
        (a.b + b.b) / 2
    );
}

constexpr Color YELLOW = blend(RED, GREEN);
static_assert(YELLOW.r == 127, "");
```

---

## 5. constexpr if（C++17）

C++17 引入了 `if constexpr`，在编译期根据条件选择分支，**未选中的分支不会被编译**：

### 5.1 基本用法

```cpp
template <typename T>
auto getValue(T t) {
    if constexpr (std::is_integral_v<T>) {
        return t * 2;           // 整型：乘以 2
    } else if constexpr (std::is_floating_point_v<T>) {
        return t + 0.5;         // 浮点型：加 0.5
    } else {
        return t;               // 其他类型：原样返回
    }
}

auto a = getValue(10);    // 20（走 integral 分支）
auto b = getValue(3.14);  // 3.64（走 floating_point 分支）
```

### 5.2 vs 普通 if 的区别

```cpp
template <typename T>
void print(T value) {
    // 普通 if：两个分支都必须能编译
    if (std::is_integral_v<T>) {
        std::cout << value % 2;     // ❌ 如果 T 是 string，% 编译错误
    } else {
        std::cout << value.size();  // ❌ 如果 T 是 int，.size() 编译错误
    }

    // if constexpr：未选中的分支不编译
    if constexpr (std::is_integral_v<T>) {
        std::cout << value % 2;     // ✅ T 不是整型时这段代码不存在
    } else {
        std::cout << value.size();  // ✅ T 是整型时这段代码不存在
    }
}
```

### 5.3 替代 SFINAE 和 enable_if

`if constexpr` 大大简化了模板编程中的条件逻辑：

```cpp
// C++11/14 SFINAE 写法（复杂）
template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
process(T val) { return val * 2; }

template <typename T>
typename std::enable_if<!std::is_integral<T>::value, T>::type
process(T val) { return val; }

// C++17 if constexpr 写法（简洁）
template <typename T>
T process(T val) {
    if constexpr (std::is_integral_v<T>) {
        return val * 2;
    } else {
        return val;
    }
}
```

---

## 6. C++20 constexpr 增强

### 6.1 constexpr virtual（C++20）

```cpp
struct Base {
    constexpr virtual int value() const { return 1; }
    constexpr virtual ~Base() = default;
};

struct Derived : Base {
    constexpr int value() const override { return 2; }
};

constexpr Derived d;
constexpr const Base& b = d;
static_assert(b.value() == 2, "编译期多态");
```

### 6.2 constexpr new/delete（C++20）

```cpp
constexpr int compute() {
    int* p = new int(42);    // ✅ C++20 允许 constexpr new
    int val = *p;
    delete p;                // 必须在同一个 constexpr 求值中 delete
    return val;
}

static_assert(compute() == 42, "");
```

### 6.3 constexpr std::vector / std::string（C++20）

```cpp
constexpr int sumVector() {
    std::vector<int> v = {1, 2, 3, 4, 5};  // ✅ C++20
    int sum = 0;
    for (int x : v) sum += x;
    return sum;
}

static_assert(sumVector() == 15, "");
```

### 6.4 consteval 和 constinit（C++20）

C++20 还引入了两个相关关键字：

```cpp
// consteval：强制只能在编译期调用（立即函数）
consteval int compileTimeOnly(int x) {
    return x * x;
}

constexpr int a = compileTimeOnly(5);  // ✅ 编译期调用
int n = 5;
// int b = compileTimeOnly(n);         // ❌ 编译错误：不能运行时调用

// constinit：保证变量在编译期初始化，但运行时可修改
constinit int global = 42;             // ✅ 编译期初始化
// 后续可以修改 global
```

对比三者：

| 关键字 | 变量 | 函数 | 编译期 | 运行时 |
|--------|------|------|--------|--------|
| `constexpr` | ✅ | ✅ | ✅ 可在编译期求值 | ✅ 也可运行时求值 |
| `consteval` | ❌ | ✅ | ✅ **必须**编译期求值 | ❌ 不能运行时调用 |
| `constinit` | ✅ | ❌ | ✅ **必须**编译期初始化 | ✅ 运行时可修改 |

---

## 7. constexpr 各版本演进总结

### 7.1 版本对比

| 特性 | C++11 | C++14 | C++17 | C++20 |
|------|-------|-------|-------|-------|
| constexpr 变量 | ✅ | ✅ | ✅ | ✅ |
| constexpr 函数（单 return） | ✅ | ✅ | ✅ | ✅ |
| constexpr 函数（多语句/循环） | ❌ | ✅ | ✅ | ✅ |
| constexpr lambda | ❌ | ❌ | ✅ | ✅ |
| if constexpr | ❌ | ❌ | ✅ | ✅ |
| constexpr virtual | ❌ | ❌ | ❌ | ✅ |
| constexpr new/delete | ❌ | ❌ | ❌ | ✅ |
| constexpr std::vector/string | ❌ | ❌ | ❌ | ✅ |
| constexpr try-catch | ❌ | ❌ | ❌ | ✅ |
| consteval | ❌ | ❌ | ❌ | ✅ |
| constinit | ❌ | ❌ | ❌ | ✅ |

### 7.2 演进趋势

```
C++11          C++14          C++17          C++20
──────         ──────         ──────         ──────
基础 constexpr  放宽函数限制   if constexpr   几乎无限制
单行 return    多语句/循环    编译期分支      virtual / new
简单构造函数    修改局部变量   constexpr       consteval
                             lambda         constinit
                                            STL 容器
```

---

## 8. 实际项目中的应用

### 8.1 编译期查找表

```cpp
constexpr int hexCharToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// 编译期生成查找表
constexpr auto makeHexTable() {
    struct Table { int data[128]{}; };
    Table t{};
    for (int i = 0; i < 128; ++i)
        t.data[i] = hexCharToInt(static_cast<char>(i));
    return t;
}

constexpr auto HEX_TABLE = makeHexTable();

// 运行时 O(1) 查找
int value = HEX_TABLE.data['A'];  // 10
```

### 8.2 编译期字符串处理

```cpp
constexpr bool startsWith(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str++ != *prefix++) return false;
    }
    return true;
}

static_assert(startsWith("Hello World", "Hello"), "");
static_assert(!startsWith("Hello", "World"), "");

constexpr size_t countChar(const char* str, char c) {
    size_t count = 0;
    while (*str) {
        if (*str == c) ++count;
        ++str;
    }
    return count;
}

static_assert(countChar("banana", 'a') == 3, "");
```

### 8.3 类型安全的单位系统

```cpp
struct Meters {
    double value;
    constexpr explicit Meters(double v) : value(v) {}
};

struct Seconds {
    double value;
    constexpr explicit Seconds(double v) : value(v) {}
};

struct MetersPerSecond {
    double value;
    constexpr explicit MetersPerSecond(double v) : value(v) {}
};

constexpr MetersPerSecond operator/(Meters m, Seconds s) {
    return MetersPerSecond(m.value / s.value);
}

constexpr auto speed = Meters(100.0) / Seconds(9.58);
// 编译期计算速度，且类型安全：不能把 Meters + Seconds
```

### 8.4 编译期配置校验

```cpp
struct Config {
    int maxThreads;
    int bufferSize;
    int timeoutMs;

    constexpr Config(int threads, int buffer, int timeout)
        : maxThreads(threads), bufferSize(buffer), timeoutMs(timeout) {}

    constexpr bool isValid() const {
        return maxThreads > 0 && maxThreads <= 64
            && bufferSize >= 1024
            && timeoutMs > 0;
    }
};

constexpr Config APP_CONFIG(8, 4096, 3000);
static_assert(APP_CONFIG.isValid(), "配置参数非法！请检查 APP_CONFIG");

// 如果配置有误，编译时就报错，不用等到运行时
```

---

## 9. 常见错误与注意事项

### 9.1 constexpr 函数不保证编译期执行

```cpp
constexpr int square(int x) { return x * x; }

// 编译期执行（参数是编译期常量）
constexpr int a = square(5);    // ✅ 编译期

// 运行时执行（参数是运行时值）
int n;
std::cin >> n;
int b = square(n);              // ✅ 合法，但运行时计算

// 如果你想强制编译期执行，用 consteval（C++20）
consteval int mustCompileTime(int x) { return x * x; }
```

### 9.2 不要过度使用 constexpr

```cpp
// ❌ 过度：简单赋值不需要 constexpr
constexpr int x = 1 + 1;  // 编译器本来就会优化

// ✅ 有意义的使用：需要编译期保证的场景
constexpr int BUFFER_SIZE = 1024;
char buffer[BUFFER_SIZE];           // 需要编译期常量

template <int N>
struct FixedArray { int data[N]; };
FixedArray<BUFFER_SIZE> arr;        // 需要编译期常量
```

### 9.3 constexpr 与引用

```cpp
// constexpr 引用必须绑定到具有静态存储期的对象
static int global = 42;
constexpr int& ref = global;       // ✅ 全局/静态变量

int local = 10;
// constexpr int& ref2 = local;   // ❌ 局部变量没有静态存储期
```

### 9.4 调试 constexpr 函数

constexpr 函数在编译期执行时**无法用调试器设断点**。调试技巧：

```cpp
// 去掉 constexpr，改为普通函数调试
// constexpr int compute(int x) {
int compute(int x) {
    // 现在可以设断点调试
    int result = x * x;
    return result;
}

// 调试完成后再加回 constexpr
```

---

## 10. 总结

### 速查表

| 用法 | 语法 | 说明 |
|------|------|------|
| constexpr 变量 | `constexpr int N = 10;` | 编译期常量 |
| constexpr 函数 | `constexpr int f(int x) {...}` | 可编译期/运行时调用 |
| constexpr 构造 | `constexpr Point(int x, int y)` | 编译期创建对象 |
| if constexpr | `if constexpr (cond) {...}` | 编译期条件分支（C++17） |
| consteval | `consteval int f(int x) {...}` | 只能编译期调用（C++20） |
| constinit | `constinit int g = 42;` | 编译期初始化（C++20） |

### 什么时候用 constexpr

```
┌─────────────────────────────────────────────────────────┐
│                何时使用 constexpr？                       │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ✅ 用 constexpr 的场景：                                │
│     • 数组大小、模板参数                                  │
│     • static_assert 中的条件                             │
│     • 编译期查找表 / 预计算                               │
│     • switch-case 的值                                   │
│     • 类型安全的常量定义                                  │
│     • 零运行时开销的配置/常量                              │
│                                                         │
│  ❌ 不需要 constexpr 的场景：                             │
│     • 值在运行时才确定 → 用 const                         │
│     • 简单的字面量赋值（编译器本来就优化）                  │
│     • 复杂的 I/O 或系统调用（无法编译期执行）              │
│                                                         │
│  选择指南：                                               │
│     const      → "这个值不会改变"                         │
│     constexpr  → "这个值编译期就能算出来"                  │
│     consteval  → "这个函数只能在编译期调用"（C++20）       │
│     constinit  → "这个变量编译期初始化，运行时可改"（C++20）│
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

*最后更新时间：2026年3月5日*
