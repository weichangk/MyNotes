# C++ const 关键字

`const` 是 C++ 中最重要的关键字之一，用于表示**常量性（constness）**——即"不可修改"。它可以修饰变量、指针、引用、函数参数、返回值、成员函数等，是写出安全、健壮 C++ 代码的基础。

> 相关文档：[const 和 #define 对比](./C++%20const%20和%20%23define%20对比.md) | [const 符号表机制](./C++%20const%20符号表机制.md) | [constexpr 关键字](./C++%20constexpr%20关键字.md)

---

## 1. const 修饰变量

### 1.1 基本用法

```cpp
const int MAX_SIZE = 100;
MAX_SIZE = 200;  // ❌ 编译错误：不能修改 const 变量

const double PI = 3.14159265358979;
```

**要点：**
- `const` 变量**必须在声明时初始化**（因为之后不能再赋值）
- 命名惯例：全大写 + 下划线（如 `MAX_SIZE`），或加 `k` 前缀（如 `kMaxSize`）

### 1.2 const vs 非 const

```cpp
int a = 10;
const int b = 20;

a = 30;   // ✅ 普通变量可以修改
b = 30;   // ❌ const 变量不能修改

const int c;  // ❌ 编译错误：const 变量必须初始化
```

### 1.3 const 的本质

`const` 并不是让变量存在只读内存中，而是让编译器**在编译期拒绝对它的修改**。它是一种**编译期约束**：

```cpp
const int a = 10;

// 通过指针强制修改（未定义行为！仅用于理解原理）
int* p = const_cast<int*>(&a);
*p = 20;  // ⚠️ 未定义行为：可能修改成功，也可能不变，也可能崩溃
```

---

## 2. const 修饰指针（重点）

这是 `const` 最容易混淆的用法。核心口诀：**`const` 在 `*` 左边修饰数据，在 `*` 右边修饰指针**。

### 2.1 四种组合

```cpp
int value = 10;
int other = 20;

// ① 指向常量的指针（底层 const）
const int* p1 = &value;     // 也可以写 int const* p1
*p1 = 20;                   // ❌ 不能通过指针修改值
p1 = &other;                // ✅ 可以改变指针指向

// ② 常量指针（顶层 const）
int* const p2 = &value;
*p2 = 20;                   // ✅ 可以通过指针修改值
p2 = &other;                // ❌ 不能改变指针指向

// ③ 指向常量的常量指针
const int* const p3 = &value;
*p3 = 20;                   // ❌ 两个都不能改
p3 = &other;                // ❌

// ④ 普通指针（无 const）
int* p4 = &value;
*p4 = 20;                   // ✅ 都能改
p4 = &other;                // ✅
```

### 2.2 记忆方法

从右往左读，遇到 `*` 读作"指针"，遇到 `const` 读作"常量"：

```
const int* p        →  p is a pointer to const int
                       p 是一个指针，指向 const int
                       "指向常量的指针"

int* const p        →  p is a const pointer to int
                       p 是一个 const 指针，指向 int
                       "常量指针"

const int* const p  →  p is a const pointer to const int
                       "指向常量的常量指针"
```

### 2.3 图解对比

```
  const int* p              int* const p             const int* const p
  ─────────────             ─────────────            ──────────────────
  ┌───┐    ┌───┐            ┌───┐    ┌───┐           ┌───┐    ┌───┐
  │ p │───▶│ 10│            │ p │───▶│ 10│           │ p │───▶│ 10│
  └───┘    └───┘            └───┘    └───┘           └───┘    └───┘
  p 可改✅  *p 不可改❌      p 不可改❌ *p 可改✅      p 不可改❌ *p 不可改❌
```

### 2.4 顶层 const vs 底层 const

| 术语 | 含义 | 示例 |
|------|------|------|
| **顶层 const（top-level）** | 对象本身不可修改 | `int* const p`、`const int a` |
| **底层 const（low-level）** | 所指向/引用的对象不可修改 | `const int* p`、`const int& r` |

这个区别在**拷贝**时非常重要：

```cpp
const int a = 10;
int b = a;           // ✅ 顶层 const 拷贝时被忽略

const int* p1 = &a;
int* p2 = p1;        // ❌ 底层 const 不能忽略！不能丢掉对数据的 const 保护
const int* p3 = p1;  // ✅ 底层 const 匹配
```

---

## 3. const 修饰引用

### 3.1 常量引用（const reference）

```cpp
int a = 10;
const int& ref = a;  // 常量引用

ref = 20;   // ❌ 不能通过 const 引用修改
a = 20;     // ✅ 但原变量仍然可以修改
```

### 3.2 常量引用可以绑定临时值

这是 `const` 引用的一个重要特性：

```cpp
// 普通引用不能绑定临时值
// int& r1 = 42;         // ❌ 编译错误

// const 引用可以！
const int& r2 = 42;      // ✅ 编译器创建临时变量，引用指向它
const std::string& r3 = "hello";  // ✅ 同理
```

### 3.3 函数参数中的 const 引用

这是 C++ 中**最常用的参数传递方式**之一：

```cpp
// ❌ 值传递：拷贝整个 string，性能差
void print(std::string str);

// ❌ 非 const 引用：不能传临时值，调用者担心被修改
void print(std::string& str);

// ✅ const 引用：零拷贝 + 不可修改 + 可接受临时值
void print(const std::string& str);
```

```cpp
void print(const std::string& str) {
    std::cout << str << std::endl;
    // str += "!";  // ❌ 不能修改
}

int main() {
    std::string s = "hello";
    print(s);          // ✅ 传左值
    print("world");    // ✅ 传临时值（右值）
    return 0;
}
```

> **经验法则：** 对于非基本类型（`std::string`、`std::vector`、自定义类等），优先用 `const T&` 传参。对于基本类型（`int`、`double`），直接值传递即可。

---

## 4. const 修饰函数参数

### 4.1 值传递的 const

```cpp
void func(const int x) {
    // x = 20;  // ❌ 不能修改
    std::cout << x << std::endl;
}
```

> 注意：这里 `const` 修饰的是函数内部的**局部副本**，对调用者没有影响。在函数**声明**中通常省略这个 `const`，只在**定义**中使用（如果需要）。

### 4.2 指针参数的 const

```cpp
// 不修改指向的数据
void printArray(const int* arr, int size) {
    for (int i = 0; i < size; ++i)
        std::cout << arr[i] << " ";
    // arr[0] = 0;  // ❌ 不能修改
}

// 不修改指针本身（很少用，因为指针也是值传递）
void process(int* const arr, int size) {
    arr[0] = 100;    // ✅ 可以修改数据
    // arr = nullptr; // ❌ 不能修改指针
}
```

### 4.3 引用参数的 const

```cpp
// 输入参数：const 引用（不修改）
void display(const std::vector<int>& data);

// 输出参数：非 const 引用（需要修改）
void fill(std::vector<int>& data);

// 输入输出参数：非 const 引用
void transform(std::vector<int>& data);
```

---

## 5. const 修饰返回值

### 5.1 返回 const 值

```cpp
// 对基本类型无实际意义（返回的是副本）
const int getValue() { return 42; }

// 对对象类型可以防止返回值被修改
const std::string getName() { return "Alice"; }
// getName() = "Bob";  // ❌ 不能对返回值赋值
```

> ⚠️ 现代 C++ 中，返回 `const` 值会**阻止移动语义**，一般不推荐。

### 5.2 返回 const 引用

```cpp
class Container {
    std::vector<int> m_data;
public:
    // 返回 const 引用：允许外部读取但不能修改
    const std::vector<int>& getData() const {
        return m_data;
    }

    // 返回非 const 引用：允许外部修改
    std::vector<int>& getData() {
        return m_data;
    }
};

int main() {
    Container c;
    const auto& data = c.getData();  // 调用 const 版本
    // data.push_back(1);            // ❌ 不能修改
    return 0;
}
```

### 5.3 返回 const 指针

```cpp
class StringPool {
    std::string m_buffer;
public:
    // 返回只读指针，外部不能修改内部数据
    const char* c_str() const {
        return m_buffer.c_str();
    }
};
```

---

## 6. const 成员函数

### 6.1 基本概念

在成员函数的参数列表后加 `const`，表示该函数**不会修改对象的状态**：

```cpp
class Rectangle {
    int m_width, m_height;
public:
    Rectangle(int w, int h) : m_width(w), m_height(h) {}

    // const 成员函数：承诺不修改任何成员变量
    int area() const {
        // m_width = 0;  // ❌ 不能修改成员
        return m_width * m_height;
    }

    // 非 const 成员函数：可以修改成员变量
    void resize(int w, int h) {
        m_width = w;   // ✅
        m_height = h;  // ✅
    }
};
```

### 6.2 const 成员函数的本质

`const` 修饰的是隐含的 `this` 指针：

```cpp
// 普通成员函数
int area();
// 等价于：int area(Rectangle* const this);
//         this 本身是 const（顶层），但 *this 不是

// const 成员函数
int area() const;
// 等价于：int area(const Rectangle* const this);
//         this 和 *this 都是 const
```

### 6.3 const 对象只能调用 const 成员函数

```cpp
Rectangle r1(10, 20);
const Rectangle r2(30, 40);

r1.area();       // ✅
r1.resize(5, 5); // ✅

r2.area();       // ✅ const 对象可以调用 const 成员函数
r2.resize(5, 5); // ❌ const 对象不能调用非 const 成员函数
```

### 6.4 const 重载

同一个函数可以有 `const` 和非 `const` 两个版本，编译器根据对象是否为 `const` 来选择：

```cpp
class TextBuffer {
    std::string m_text;
public:
    // 非 const 版本：返回可修改的引用
    char& operator[](size_t index) {
        std::cout << "非 const 版本" << std::endl;
        return m_text[index];
    }

    // const 版本：返回不可修改的引用
    const char& operator[](size_t index) const {
        std::cout << "const 版本" << std::endl;
        return m_text[index];
    }
};

int main() {
    TextBuffer buf;
    const TextBuffer& cbuf = buf;

    buf[0] = 'A';       // 调用非 const 版本 ✅
    char c = cbuf[0];   // 调用 const 版本 ✅
    // cbuf[0] = 'B';   // ❌ const 版本返回 const char&

    return 0;
}
```

### 6.5 避免 const/非 const 重载的代码重复

当两个版本逻辑完全一样时，可以用 `const_cast` 消除重复：

```cpp
class Array {
    int* m_data;
    size_t m_size;
public:
    // 先写 const 版本
    const int& at(size_t index) const {
        if (index >= m_size) throw std::out_of_range("越界");
        return m_data[index];
    }

    // 非 const 版本：复用 const 版本的逻辑
    int& at(size_t index) {
        return const_cast<int&>(
            static_cast<const Array&>(*this).at(index)
        );
        // 1. 把 *this 转为 const → 调用 const 版本
        // 2. 用 const_cast 去掉返回值的 const
        // 安全：因为我们知道原始对象不是 const
    }
};
```

---

## 7. mutable 关键字

`mutable` 是 `const` 的"逃生口"——允许在 `const` 成员函数中修改特定成员变量。

### 7.1 典型场景：缓存和计数

```cpp
class Shape {
    std::vector<Point> m_points;
    mutable double m_cachedArea = -1;  // 缓存面积
    mutable int m_accessCount = 0;     // 访问计数

public:
    double area() const {
        m_accessCount++;               // ✅ mutable 允许修改

        if (m_cachedArea < 0) {
            m_cachedArea = calculateArea();  // ✅ 缓存结果
        }
        return m_cachedArea;
    }

private:
    double calculateArea() const { /* 复杂计算 */ return 0; }
};
```

### 7.2 mutable 与互斥锁

多线程中，`const` 成员函数可能需要加锁，但 `mutex::lock()` 不是 const 的：

```cpp
class ThreadSafeCounter {
    int m_count = 0;
    mutable std::mutex m_mutex;  // mutable 使锁可在 const 函数中使用

public:
    int get() const {
        std::lock_guard<std::mutex> lock(m_mutex);  // ✅
        return m_count;
    }

    void increment() {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_count;
    }
};
```

> **原则：** `mutable` 用于那些不影响对象"逻辑状态"的成员（缓存、计数器、互斥锁等）。

---

## 8. const_cast

`const_cast` 用于**添加或移除** `const`/`volatile` 限定符。

### 8.1 移除 const

```cpp
const int a = 10;
int* p = const_cast<int*>(&a);
*p = 20;  // ⚠️ 未定义行为！a 本身是 const

int b = 10;
const int* cp = &b;
int* mp = const_cast<int*>(cp);
*mp = 20;  // ✅ 安全：b 本来就不是 const
```

> **规则：** 只有当原始对象**本身不是 const** 时，`const_cast` 后修改才是安全的。

### 8.2 合法的使用场景

```cpp
// 场景：调用不完善的 C 库（参数没加 const 但实际不修改）
extern "C" void legacy_print(char* str);  // 老 C 函数，参数不是 const

void print(const std::string& s) {
    legacy_print(const_cast<char*>(s.c_str()));  // 安全：legacy_print 不会修改
}
```

---

## 9. const 在类继承中的表现

### 9.1 虚函数的 const

`const` 是函数签名的一部分，`const` 和非 `const` 版本是**不同的函数**：

```cpp
class Base {
public:
    virtual void func() const {
        std::cout << "Base::func() const" << std::endl;
    }
    virtual void func() {
        std::cout << "Base::func()" << std::endl;
    }
};

class Derived : public Base {
public:
    void func() const override {   // 重写 const 版本
        std::cout << "Derived::func() const" << std::endl;
    }
    void func() override {         // 重写非 const 版本
        std::cout << "Derived::func()" << std::endl;
    }
};
```

### 9.2 const 不影响继承权限

```cpp
class Base {
public:
    void modify() { /* ... */ }
    void read() const { /* ... */ }
};

class Derived : public Base {
    // 继承了 modify() 和 read() const
    // const 属性保持不变
};
```

---

## 10. const 与 STL 迭代器

```cpp
#include <vector>

std::vector<int> vec = {1, 2, 3, 4, 5};

// 普通迭代器：可以修改元素
std::vector<int>::iterator it = vec.begin();
*it = 10;  // ✅

// const 迭代器：不能修改元素
std::vector<int>::const_iterator cit = vec.cbegin();
// *cit = 10;  // ❌

// const 修饰迭代器本身（类比 int* const）
const std::vector<int>::iterator cit2 = vec.begin();
*cit2 = 10;     // ✅ 可以修改元素
// ++cit2;      // ❌ 不能移动迭代器

// 范围 for 中的 const
for (const auto& elem : vec) {
    std::cout << elem << " ";  // 只读
    // elem = 0;               // ❌
}
```

| 类型 | 类比指针 | 能修改元素 | 能移动迭代器 |
|------|---------|-----------|-------------|
| `iterator` | `int*` | ✅ | ✅ |
| `const_iterator` | `const int*` | ❌ | ✅ |
| `const iterator` | `int* const` | ✅ | ❌ |

---

## 11. const 的常见陷阱

### 11.1 const 变量必须初始化

```cpp
const int a;        // ❌ 编译错误
const int a = 10;   // ✅
```

### 11.2 不能通过非 const 指针指向 const 数据

```cpp
const int a = 10;
int* p = &a;          // ❌ 编译错误：丢失底层 const
const int* p = &a;    // ✅
```

### 11.3 const 引用延长临时对象生命周期

```cpp
const std::string& ref = std::string("hello");
// 临时对象的生命周期被延长到 ref 的作用域结束
std::cout << ref << std::endl;  // ✅ 安全
```

### 11.4 不要在 assert 里依赖 const

```cpp
const int MAX = 100;
assert(MAX == 100);  // ✅ 但 Release 下 assert 消失
                     // const 本身就是保证，无需 assert
```

---

## 12. 总结

### 速查表

| 用法 | 语法 | 含义 |
|------|------|------|
| 常量变量 | `const int a = 10;` | 变量不可修改 |
| 指向常量的指针 | `const int* p` | 不能通过 `*p` 修改值 |
| 常量指针 | `int* const p` | 指针地址不可改 |
| 指向常量的常量指针 | `const int* const p` | 都不可改 |
| 常量引用 | `const int& r` | 不能通过引用修改 |
| const 参数 | `void f(const T& x)` | 函数不修改参数 |
| const 返回值 | `const T& get() const` | 返回只读引用 |
| const 成员函数 | `int area() const` | 不修改成员变量 |
| mutable | `mutable int count` | 允许在 const 函数中修改 |

### const 正确性（Const Correctness）

`const` 正确性是 C++ 的重要设计原则：

```
┌─────────────────────────────────────────────────────────┐
│                  Const Correctness 原则                  │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  1. 能加 const 就加 const                                │
│     → 让编译器帮你检查错误                                │
│                                                         │
│  2. 非基本类型参数：优先 const T&                         │
│     → 避免拷贝，保护数据                                 │
│                                                         │
│  3. 不修改成员的方法：标记为 const                        │
│     → 使 const 对象也能调用                              │
│                                                         │
│  4. const 从接口开始设计                                 │
│     → 先决定哪些是只读操作                                │
│     → 后期添加 const 往往很痛苦（需要大量级联修改）        │
│                                                         │
│  5. mutable 仅用于不影响逻辑状态的成员                    │
│     → 缓存、计数器、互斥锁                               │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

## 问题与解答

### Q1：`int* const p` 可以指向常量吗？

**答案：❌ 不能。**

```cpp
const int x = 10;
int* const p = &x;  // ❌ 编译错误
```

`int* const p` 的含义是"p 是一个不可改的指针，指向 `int`"。由于 `*p` 是 `int`（非 const），编译器不允许它指向 `const int`——否则就可以通过 `*p` 修改一个本不该修改的值。

---

### Q2：`const int* const p` 可以指向非常量吗？

**答案：✅ 可以。**

```cpp
int x = 10;
const int* const p = &x;  // ✅ 合法
```

你只是告诉编译器"我不会通过 p 去修改 x"，这是一种**更严格的约束**，完全安全。`x` 本身仍然可以通过其他方式修改。

---

### Q3：函数参数有 const，但想在函数内部修改怎么办？

**四种方案：**

```cpp
// 方案 1：去掉 const，值传递（修改的是副本）
void func(int x) {
    x = 10;  // ✅ 仅修改局部副本
}

// 方案 2：复制一份临时变量（推荐）
void func(const int& x) {
    int temp = x;
    temp += 10;  // ✅ 修改副本
}

// 方案 3：const_cast（慎用，仅当原始值非 const 时安全）
void func(const int& x) {
    int& y = const_cast<int&>(x);
    y = 10;  // ⚠️ 如果原始值是 const → 未定义行为！
}

// 方案 4：mutable（用于 const 成员函数中修改特定成员）
class MyClass {
    mutable int counter = 0;
public:
    void tick() const { counter++; }  // ✅
};
```

> **建议：** 优先用方案 1 或 2，方案 3 只在对接老旧 API 时考虑。

---

### Q4：const 成员函数可以调用非 const 成员函数吗？

**答案：❌ 不能。**

```cpp
class Foo {
public:
    void modify() { /* ... */ }
    void read() const {
        // modify();  // ❌ const 成员函数中 this 是 const 的
                      //    不能调用可能修改对象的函数
    }
};
```

但反过来可以——非 const 成员函数可以调用 const 成员函数。

---

*最后更新时间：2026年2月27日*

