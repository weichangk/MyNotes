# C++ static_cast
## 1. 概述

`static_cast` 是 C++ 四种命名转换运算符中**使用频率最高**的一种，用于在编译期执行类型转换。它涵盖了大部分"合理的"类型转换需求，同时在编译期提供类型检查，拒绝明显不合法的转换。

```cpp
static_cast<目标类型>(表达式)
```

**核心特点**：
- 编译期完成，**零运行时开销**
- 仅允许存在合理转换路径的类型之间转换
- 不能移除 `const`/`volatile`（需要 `const_cast`）
- 不能在无关指针/引用类型之间转换（需要 `reinterpret_cast`）
- 不进行运行时类型检查（区别于 `dynamic_cast`）

---

## 2. static_cast 合法的转换场景

### 2.1 数值类型之间的显式转换

这是最常见且最安全的用途——在内置算术类型之间转换：

```cpp
int i = 42;
double d = static_cast<double>(i);    // int → double，无精度丢失
std::cout << d;  // 42.0

double pi = 3.14159;
int truncated = static_cast<int>(pi);  // double → int，截断小数部分
std::cout << truncated;  // 3

long long big = 100000;
int small = static_cast<int>(big);     // 窄化转换，可能溢出但意图明确

char c = 'A';
int ascii = static_cast<int>(c);       // char → int，值为 65

unsigned int u = static_cast<unsigned int>(-1);  // 有符号 → 无符号
std::cout << u;  // 4294967295（UINT_MAX，二进制补码）
```

> **实践意义**：虽然隐式转换也能完成 `int → double`，但在可能丢失精度的转换中（如 `double → int`），显式使用 `static_cast` 可以消除编译器的窄化警告，并向代码读者表明这是有意为之。

### 2.2 继承体系中的向上转型（Upcast）

将派生类指针/引用转换为基类——这是安全的，编译器会正确调整指针偏移：

```cpp
class Base {
public:
    virtual ~Base() = default;
    void hello() { std::cout << "Base\n"; }
};

class Derived : public Base {
public:
    void world() { std::cout << "Derived\n"; }
};

Derived d;
Base* bp = static_cast<Base*>(&d);    // 向上转型（其实隐式转换就行）
bp->hello();  // "Base"

Base& br = static_cast<Base&>(d);     // 引用的向上转型
br.hello();   // "Base"
```

> 向上转型通常**不需要**显式使用 `static_cast`，因为隐式转换就能完成。但在模板代码或需要明确意图时，显式写出更清晰。

### 2.3 继承体系中的向下转型（Downcast）

将基类指针/引用转换为派生类——**程序员必须保证实际类型正确**：

```cpp
class Animal {
public:
    virtual ~Animal() = default;
    virtual void speak() = 0;
};

class Dog : public Animal {
public:
    void speak() override { std::cout << "Woof!\n"; }
    void fetch() { std::cout << "Fetching ball!\n"; }
};

class Cat : public Animal {
public:
    void speak() override { std::cout << "Meow!\n"; }
};

Animal* animal = new Dog();

// 程序员确信 animal 指向 Dog 对象
Dog* dog = static_cast<Dog*>(animal);
dog->speak();  // "Woof!"
dog->fetch();  // "Fetching ball!"

// ⚠️ 危险：如果实际类型不是 Dog
Animal* realCat = new Cat();
Dog* wrongDog = static_cast<Dog*>(realCat);  // 编译通过！
wrongDog->fetch();  // 未定义行为！realCat 并不是 Dog
// static_cast 不做运行时检查，这就是它比 dynamic_cast 快但更危险的原因

delete animal;
delete realCat;
```

**向下转型的安全原则**：

```cpp
// 当不确定类型时，优先用 dynamic_cast：
Animal* unknown = getAnimalFromSomewhere();
Dog* maybeDog = dynamic_cast<Dog*>(unknown);
if (maybeDog) {
    maybeDog->fetch();  // 安全：确认是 Dog
} else {
    // 不是 Dog，安全处理
}

// 当确定类型时（如刚刚创建/类型标签已验证），用 static_cast 更高效：
enum class AnimalType { Dog, Cat };
struct TaggedAnimal { AnimalType type; Animal* ptr; };

TaggedAnimal ta = { AnimalType::Dog, new Dog() };
if (ta.type == AnimalType::Dog) {
    Dog* dog = static_cast<Dog*>(ta.ptr);  // 类型已通过标签验证，安全
    dog->fetch();
}
```

### 2.4 void* 与具体类型指针之间的转换

`static_cast` 可以在 `void*` 和任意对象指针之间转换：

```cpp
int value = 42;
void* vp = static_cast<void*>(&value);    // int* → void*（隐式也行）
int* ip = static_cast<int*>(vp);          // void* → int*（必须显式）
std::cout << *ip;  // 42

// 常见场景：C 回调中的 void* 用户数据
struct CallbackData {
    std::string name;
    int count;
};

// C 风格的回调接口
void c_callback(void* userData) {
    auto* data = static_cast<CallbackData*>(userData);  // 从 void* 恢复类型
    std::cout << data->name << ": " << data->count << "\n";
}

CallbackData cbd{"Counter", 10};
c_callback(&cbd);  // "Counter: 10"

// ⚠️ 注意：程序员必须保证 void* 确实指向正确的类型
// 如果传入错误类型的指针，static_cast 不会检测到
```

### 2.5 枚举与整数之间的转换

```cpp
// 传统枚举
enum Color { Red = 0, Green = 1, Blue = 2 };
int colorVal = static_cast<int>(Green);         // 枚举 → int，值为 1
Color c = static_cast<Color>(2);                 // int → 枚举，值为 Blue

// 强类型枚举（C++11 scoped enum）
enum class Direction : uint8_t { North = 0, South = 1, East = 2, West = 3 };

// scoped enum 不能隐式转换为整数，必须用 static_cast
uint8_t dirVal = static_cast<uint8_t>(Direction::East);  // 值为 2
Direction dir = static_cast<Direction>(3);                 // 值为 West

// 实用场景：用于位运算
enum class Permission : unsigned {
    Read    = 1 << 0,
    Write   = 1 << 1,
    Execute = 1 << 2
};

// scoped enum 重载运算符需要 static_cast
Permission combined = static_cast<Permission>(
    static_cast<unsigned>(Permission::Read) |
    static_cast<unsigned>(Permission::Write)
);

bool canRead = (static_cast<unsigned>(combined) &
                static_cast<unsigned>(Permission::Read)) != 0;  // true
```

### 2.6 用户定义的转换（构造函数 / 转换运算符）

`static_cast` 可以触发**单参数构造函数**和**转换运算符**：

```cpp
class Meters {
    double value_;
public:
    // explicit 单参数构造函数
    explicit Meters(double v) : value_(v) {}
    double get() const { return value_; }
};

double raw = 3.5;
// Meters m = raw;                    // 编译错误！构造函数是 explicit 的
Meters m = static_cast<Meters>(raw);  // OK！static_cast 可以调用 explicit 构造函数
std::cout << m.get();  // 3.5

// 转换运算符
class Fraction {
    int num_, den_;
public:
    Fraction(int n, int d) : num_(n), den_(d) {}
    explicit operator double() const { return static_cast<double>(num_) / den_; }
};

Fraction f(3, 4);
// double val = f;                        // 编译错误！转换运算符是 explicit 的
double val = static_cast<double>(f);      // OK！调用 explicit operator double()
std::cout << val;  // 0.75
```

### 2.7 左值到右值 / 数组到指针 / 函数到指针的转换

```cpp
int arr[5] = {1, 2, 3, 4, 5};
int* p = static_cast<int*>(arr);  // 数组退化为指针（隐式也行）

void foo() {}
auto fp = static_cast<void(*)()>(foo);  // 函数退化为函数指针

int x = 42;
int&& rr = static_cast<int&&>(x);  // 左值转为右值引用，等价于 std::move(x)
```

### 2.8 std::move 和 std::forward 的本质

`std::move` 和 `std::forward` 的底层实现就是 `static_cast`：

```cpp
// std::move 的简化实现
template<typename T>
constexpr std::remove_reference_t<T>&& my_move(T&& t) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(t);
}

// std::forward 的简化实现
template<typename T>
constexpr T&& my_forward(std::remove_reference_t<T>& t) noexcept {
    return static_cast<T&&>(t);
}

// 使用示例
std::string s = "Hello";
std::string s2 = my_move(s);       // s 被移动，s2 = "Hello"
std::cout << s.empty();  // true（s 已被移走）

// 在完美转发中
template<typename... Args>
auto make_unique_wrapper(Args&&... args) {
    return std::make_unique<std::string>(std::forward<Args>(args)...);
}
```

---

## 3. static_cast 不能做的事

### 3.1 不能移除 const / volatile

```cpp
const int ci = 100;
// int* p = static_cast<int*>(&ci);  // 编译错误！不能去掉 const
int* p = const_cast<int*>(&ci);       // 需要 const_cast

volatile int vi = 200;
// int* q = static_cast<int*>(&vi);  // 编译错误！不能去掉 volatile
```

### 3.2 不能在无关类型的指针/引用间转换

```cpp
class Dog {};
class Car {};

Dog dog;
// Car* car = static_cast<Car*>(&dog);  // 编译错误！Dog 和 Car 无继承关系

int i = 42;
// float* fp = static_cast<float*>(&i);  // 编译错误！int* 和 float* 不相关
float* fp = reinterpret_cast<float*>(&i); // 需要 reinterpret_cast（通常也不该这么做）
```

### 3.3 不能在整数与指针之间转换

```cpp
int addr = 0x7FFE0000;
// int* p = static_cast<int*>(addr);       // 编译错误！
int* p = reinterpret_cast<int*>(addr);      // 需要 reinterpret_cast

int* q = new int(42);
// int val = static_cast<int>(q);           // 编译错误！
auto val = reinterpret_cast<uintptr_t>(q);  // 需要 reinterpret_cast
```

### 3.4 不能进行运行时多态检查

```cpp
class Base { public: virtual ~Base() = default; };
class Derived : public Base {};
class Other : public Base {};

Base* bp = new Other();

// static_cast 不检查实际类型，盲目转换
Derived* dp = static_cast<Derived*>(bp);  // 编译通过，但结果是未定义行为

// dynamic_cast 会检查实际类型
Derived* dp2 = dynamic_cast<Derived*>(bp);
std::cout << (dp2 == nullptr);  // true，安全地检测到类型不匹配

delete bp;
```

---

## 4. 多重继承中的 static_cast

在多重继承场景下，`static_cast` 能正确调整指针偏移（这是它优于 `reinterpret_cast` 和 C 风格转换的关键优势之一）：

```cpp
class Engine {
public:
    int horsepower = 200;
    virtual ~Engine() = default;
};

class Radio {
public:
    int channels = 100;
    virtual ~Radio() = default;
};

class Car : public Engine, public Radio {
public:
    int seats = 4;
};

Car car;
std::cout << "Car addr:    " << &car << "\n";

Engine* ep = static_cast<Engine*>(&car);
std::cout << "Engine addr: " << ep << "\n";  // 通常 == &car

Radio* rp = static_cast<Radio*>(&car);
std::cout << "Radio addr:  " << rp << "\n";  // 通常 != &car，偏移了 sizeof(Engine)

// 从 Radio* 转回 Car*，static_cast 正确调整偏移
Car* cp = static_cast<Car*>(rp);
std::cout << "Car addr:    " << cp << "\n";  // == &car，正确恢复

// 如果用 reinterpret_cast 或 C 风格转换从 Radio* 转 Car*：
Car* wrong = reinterpret_cast<Car*>(rp);
std::cout << "Wrong addr:  " << wrong << "\n";  // != &car！指针未调整！
// 访问 wrong->horsepower 是未定义行为
```

**内存布局示意**：

```
Car 对象的内存布局：
┌─────────────────────────┐ ← &car 和 Engine* 指向这里
│  Engine::vptr            │
│  Engine::horsepower      │
├─────────────────────────┤ ← Radio* 指向这里（偏移后）
│  Radio::vptr             │
│  Radio::channels         │
├─────────────────────────┤
│  Car::seats              │
└─────────────────────────┘
```

---

## 5. 虚基类的限制

`static_cast` **不能**用于涉及虚继承的向下转型，必须使用 `dynamic_cast`：

```cpp
class Base {
public:
    int x = 10;
    virtual ~Base() = default;
};

class Left : virtual public Base {
public:
    int l = 20;
};

class Right : virtual public Base {
public:
    int r = 30;
};

class Diamond : public Left, public Right {
public:
    int d = 40;
};

Diamond diamond;
Base* bp = &diamond;  // 隐式向上转型

// ❌ 编译错误：不能通过 static_cast 从虚基类向下转型
// Diamond* dp = static_cast<Diamond*>(bp);

// ✅ 必须使用 dynamic_cast
Diamond* dp = dynamic_cast<Diamond*>(bp);
if (dp) {
    std::cout << dp->d;  // 40
}
```

> **原因**：虚基类的位置在对象中不是编译期固定的，需要通过 vptr/虚基类表在运行时查找，因此编译期的 `static_cast` 无法完成转换。

---

## 6. static_cast 与隐式转换的关系

`static_cast` 可以执行**任何隐式转换的逆操作**（在合理范围内），以及触发 `explicit` 标记的转换：

```cpp
// 隐式转换：int → double
int i = 10;
double d = i;  // 隐式

// 逆操作：double → int（窄化，需要显式）
double d2 = 3.14;
// int j = d2;                        // 编译警告（窄化转换）
int j = static_cast<int>(d2);          // 无警告，意图明确

// 隐式转换：Derived* → Base*
class Base {};
class Derived : public Base {};
Derived dObj;
Base* bp = &dObj;  // 隐式

// 逆操作：Base* → Derived*（需要显式）
Derived* dp = static_cast<Derived*>(bp);  // 必须 static_cast

// 触发 explicit 转换
class Widget {
public:
    explicit Widget(int size) {}
    explicit operator bool() const { return true; }
};
// Widget w = 10;                      // 编译错误！explicit
Widget w = static_cast<Widget>(10);     // OK
// bool b = w;                         // 编译错误！explicit
bool b = static_cast<bool>(w);         // OK

// 注意：在 if/while/for 等布尔上下文中，explicit operator bool 会被隐式调用
if (w) {  // OK！这是语言的特殊规定
    std::cout << "Widget is truthy\n";
}
```

---

## 7. 编译期求值中的 static_cast（constexpr）

`static_cast` 完全支持编译期求值：

```cpp
constexpr double PI = 3.14159265358979;

// 编译期完成的转换
constexpr int INT_PI = static_cast<int>(PI);  // 3
static_assert(INT_PI == 3, "PI truncated to 3");

// 编译期枚举转换
enum class Weekday { Mon = 1, Tue, Wed, Thu, Fri, Sat, Sun };
constexpr int dayNum = static_cast<int>(Weekday::Fri);  // 5
static_assert(dayNum == 5);

// 在 constexpr 函数中使用
constexpr int safeCharToInt(char c) {
    return static_cast<int>(c);
}
static_assert(safeCharToInt('A') == 65);
```

---

## 8. 实际工程中的经典用法

### 8.1 消除编译器隐式转换警告

```cpp
void draw(int x, int y, int width, int height);

double calcX = 100.7, calcY = 200.3;
double calcW = 50.5, calcH = 30.9;

// 不用 static_cast：编译器产生 4 个 "possible loss of data" 警告
// draw(calcX, calcY, calcW, calcH);

// 使用 static_cast：无警告，且明确表达了截断意图
draw(static_cast<int>(calcX), static_cast<int>(calcY),
     static_cast<int>(calcW), static_cast<int>(calcH));
```

### 8.2 CRTP（奇异递归模板模式）中的向下转型

```cpp
template <typename Derived>
class Countable {
    static inline int count_ = 0;
public:
    Countable() { ++count_; }
    ~Countable() { --count_; }
    static int count() { return count_; }
    
    // CRTP：编译期确定的向下转型
    Derived& self() {
        return static_cast<Derived&>(*this);
    }
    const Derived& self() const {
        return static_cast<const Derived&>(*this);
    }
};

class Widget : public Countable<Widget> {
public:
    void doWork() { std::cout << "Working! Total widgets: " << count() << "\n"; }
};

Widget w1, w2;
std::cout << Widget::count();  // 2
```

### 8.3 类型安全的 ID/句柄封装

```cpp
template <typename Tag>
class StrongId {
    int value_;
public:
    explicit StrongId(int v) : value_(v) {}
    int raw() const { return value_; }
};

struct UserTag {};
struct OrderTag {};
using UserId = StrongId<UserTag>;
using OrderId = StrongId<OrderTag>;

UserId uid = static_cast<UserId>(1001);
OrderId oid = static_cast<OrderId>(2001);
// uid = oid;  // 编译错误！类型不同

// 从原始 int 构造时用 static_cast，意图明确
void processUser(UserId id) {
    std::cout << "User: " << id.raw() << "\n";
}
processUser(uid);
```

### 8.4 标准库中的 static_cast 使用

```cpp
#include <vector>
#include <cstddef>

std::vector<int> vec = {1, 2, 3, 4, 5};

// size() 返回 size_t（无符号），循环变量是 int（有符号）
// 需要 static_cast 消除符号不匹配警告
for (int i = 0; i < static_cast<int>(vec.size()); ++i) {
    std::cout << vec[i] << " ";
}

// 或者更好的做法：使用 ssize()（C++20）
// for (auto i = 0; i < std::ssize(vec); ++i) { ... }

// shared_ptr 的 static_pointer_cast
class Base { public: virtual ~Base() = default; };
class Derived : public Base { public: int x = 42; };

std::shared_ptr<Derived> dp = std::make_shared<Derived>();
std::shared_ptr<Base> bp = dp;  // 隐式向上转型

// 对 shared_ptr 做向下转型，保持引用计数
std::shared_ptr<Derived> dp2 = std::static_pointer_cast<Derived>(bp);
std::cout << dp2->x;       // 42
std::cout << dp2.use_count();  // 3（dp、bp、dp2 共享）
```

---

## 9. static_cast vs 其他转换的选择

| 场景 | 推荐转换 | 原因 |
|------|---------|------|
| 数值类型转换（int↔double） | `static_cast` | 编译期检查，零开销 |
| 继承向上转型 | 隐式转换或 `static_cast` | 总是安全的 |
| 继承向下转型（类型已知） | `static_cast` | 零运行时开销 |
| 继承向下转型（类型未知） | `dynamic_cast` | RTTI 运行时检查 |
| 去掉 const | `const_cast` | `static_cast` 做不到 |
| 不相关类型指针互转 | `reinterpret_cast` | `static_cast` 做不到 |
| 指针↔整数 | `reinterpret_cast` | `static_cast` 做不到 |
| void* ↔ 具体类型指针 | `static_cast` | 标准推荐方式 |
| 枚举 ↔ 整数 | `static_cast` | 标准推荐方式 |
| 触发 explicit 构造/转换 | `static_cast` | 唯一合法途径 |

---

## 10. 常见陷阱与注意事项

### 10.1 向下转型的未定义行为

```cpp
class Base { public: virtual ~Base() = default; };
class Derived : public Base { public: int data = 999; };

Base b;  // 注意：b 是一个 Base 对象，不是 Derived
Derived* dp = static_cast<Derived*>(&b);  // 编译通过！
std::cout << dp->data;  // 未定义行为！b 内部根本没有 Derived::data

// 规则：static_cast 向下转型时，被转换的指针/引用必须真正指向目标类型的对象
```

### 10.2 窄化转换可能导致数据丢失

```cpp
int64_t big = 0x1'0000'0000LL;  // 4294967296，超出 int 范围
int small = static_cast<int>(big);  // 值被截断！
std::cout << small;  // 0（实现定义行为）

// 使用 GSL 的 narrow 进行安全窄化
// int safe = gsl::narrow<int>(big);  // 抛出 gsl::narrowing_error
```

### 10.3 浮点转整数的精度问题

```cpp
float f = 16777217.0f;  // 超出 float 精确表示范围
int i = static_cast<int>(f);
std::cout << i;  // 可能是 16777216（精度丢失）

double huge = 1e18;
int overflow = static_cast<int>(huge);  // 溢出！未定义行为
```

### 10.4 不要把 static_cast 当万能药

```cpp
// ❌ 错误：试图绕过类型系统
struct A { int x; };
struct B { double y; };
A a{42};
// B* bp = static_cast<B*>(static_cast<void*>(&a));  // 两步转换规避了检查！
// 这虽然能编译，但本质上和 reinterpret_cast 一样危险

// ✅ 如果真的需要重新解释内存，用 reinterpret_cast 明确表达意图
// （但应该尽量避免这种需求）
```

---

## 11. 总结

`static_cast` 是 C++ 类型转换的**主力军**，核心优势在于：

1. **编译期类型检查**：拒绝明显不合法的转换（如无关类型指针互转）
2. **零运行时开销**：所有工作在编译期完成
3. **意图清晰**：告诉代码读者"这里进行了一次经过考虑的类型转换"
4. **正确处理多重继承**：自动调整指针偏移

**使用原则**：

> - **能用 `static_cast` 就不要用 C 风格转换**
> - **向下转型时，如果不能 100% 确定实际类型，用 `dynamic_cast`**
> - **需要去 const 用 `const_cast`，需要重新解释位模式用 `reinterpret_cast`**
> - **`static_cast` 是默认选择——只有当它不能满足需求时，才考虑其他转换**

> **Scott Meyers (Effective C++ Item 27)**：尽量少做类型转换。如果必须转换，优先使用 C++ 命名转换。`static_cast` 对应大多数"安全"的转换场景，当你写下它时，编译器会帮你验证转换的合理性。