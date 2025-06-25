在 C++ 中，**构造函数（Constructor）**和**析构函数（Destructor）**是类中的两个特殊成员函数，用于对象的创建和销毁。下面是详细介绍：

## 构造函数（Constructor）

构造函数用于在创建对象时初始化对象的成员变量。

特点：

- 函数名与类名相同；
- 没有返回值（**连 `void` 都不能写**）；
- 可以重载（即可以有多个构造函数）；
- 可以带参数（称为 **有参构造函数**）；
- 可以使用默认参数；
- 可以定义为默认构造函数（无参）；

示例：

```cpp
#include <iostream>
using namespace std;

class Person {
public:
    int age;

    // 默认构造函数
    Person() {
        age = 0;
        cout << "默认构造函数调用" << endl;
    }

    // 有参构造函数
    Person(int a) {
        age = a;
        cout << "有参构造函数调用" << endl;
    }
};
```

## 析构函数（Destructor）

析构函数用于在对象生命周期结束时释放资源，如内存、文件句柄等。

特点：

- 函数名是在类名前加 `~`；
- 没有参数；
- 不能重载；
- 没有返回值；
- 系统会自动调用析构函数，无需手动调用；

示例：

```cpp
class Person {
public:
    Person() {
        cout << "构造函数调用" << endl;
    }

    ~Person() {
        cout << "析构函数调用" << endl;
    }
};
```

使用效果：

```cpp
int main() {
    Person p;  // 创建对象，构造函数自动调用
    return 0;  // main 结束，析构函数自动调用
}
```

## 构造函数初始化列表

**构造函数初始化列表**是 C++ 中构造对象时非常重要的一个语法特性，它不仅可以提升性能，还可以解决一些必须使用初始化列表的特殊情况。

构造函数初始化列表（Constructor Initialization List）是指：在构造函数的函数体前，用冒号 `:` 引出成员变量或基类的初始化语句。

基本语法：

```cpp
class MyClass {
    int x;
    int y;

public:
    MyClass(int a, int b) : x(a), y(b) {  // 初始化列表
        // 构造函数体
    }
};
```

等价于：

```cpp
MyClass(int a, int b) {
    x = a;  // 赋值
    y = b;
}
```

### 为什么使用初始化列表？

**效率更高**

初始化列表是在**对象构造阶段**直接初始化成员变量，而不是先调用默认构造再赋值。

> 构造函数体内是“赋值”，初始化列表是“直接构造”。

举例对比：

```cpp
class A {
    int x;
public:
    A(int val) : x(val) {}        // 直接初始化，推荐 ✅
    // A(int val) { x = val; }    // 先默认初始化，再赋值，性能差 ❌
};
```

**必须初始化的成员**

某些类型**必须**用初始化列表，比如：

const 成员

```cpp
class A {
    const int x;
public:
    A(int val) : x(val) {}  // 必须用初始化列表
};
```

引用成员

```cpp
class A {
    int& ref;
public:
    A(int& r) : ref(r) {}   // 必须用初始化列表
};
```

**成员是没有默认构造函数的类**

```cpp
class B {
public:
    B(int x) {}  // 没有默认构造函数
};

class A {
    B b;
public:
    A(int x) : b(x) {}  // 必须用初始化列表
};
```

**初始化顺序问题**

成员的初始化顺序**不以你写在初始化列表的顺序为准**，而是以**成员在类中定义的顺序为准**。

```cpp
class A {
    int x;
    int y;
public:
    A() : y(2), x(1) {}  // 实际先初始化x再初始化y！⚠️
};
```

**建议：初始化列表中的顺序要和类中成员定义顺序一致。**

### 构造函数初始化列表 VS 构造函数体赋值

| 特性                        | 初始化列表           | 构造函数体赋值     |
| --------------------------- | -------------------- | ------------------ |
| 是否调用默认构造函数        | ❌ 直接构造           | ✅ 默认构造后再赋值 |
| 是否适用于 const / 引用成员 | ✅ 必须用             | ❌ 无法赋值         |
| 性能                        | ✅ 更高效             | ❌ 较低             |
| 成员构造顺序可控性          | ❌ 取决于成员声明顺序 | ❌ 同上             |

## 构造函数委托（Constructor Delegation）**和**继承（base class 构造）

**构造函数委托（Constructor Delegation）\**和\**继承（base class 构造）\**是\**两个不同的概念**，它们解决的问题不一样，虽然语法上都出现在构造函数的“冒号后面”，但用途是**完全不同的**。

### 构造函数委托（C++11）

**本类中的一个构造函数调用另一个构造函数**，用于代码复用。

例子：

```cpp
class Person {
    string name;
    int age;

public:
    Person() : Person("unknown", 0) {  // 委托给有参构造
        cout << "默认构造" << endl;
    }

    Person(string n, int a) : name(n), age(a) {
        cout << "有参构造" << endl;
    }
};
```

**作用：** 避免重复初始化代码。

### 基类构造函数调用（继承时使用）

子类构造函数在初始化列表中调用**基类构造函数**，用于构造继承来的成员。

例子：

```cpp
class Animal {
public:
    Animal(string n) {
        cout << "Animal 构造: " << n << endl;
    }
};

class Dog : public Animal {
public:
    Dog() : Animal("Dog") {  // 初始化基类部分
        cout << "Dog 构造" << endl;
    }
};
```

**作用：** 调用父类构造函数，初始化继承而来的成员。

## Rule of Three（C++98）（三法则）/ Rule of Five（C++11）（五法则）

当你类中包含资源（指针/句柄等），需要手动写：

| 函数名称          | 说明                                     |
| ----------------- | ---------------------------------------- |
| 析构函数          | `~ClassName()`                           |
| 拷贝构造函数      | `ClassName(const ClassName&)`            |
| 拷贝赋值操作符    | `ClassName& operator=(const ClassName&)` |
| ✅ C++11：移动构造 | `ClassName(ClassName&&)`                 |
| ✅ C++11：移动赋值 | `ClassName& operator=(ClassName&&)`      |

### 三法则（Rule of Three）

#### 定义（C++98）

如果一个类**自定义了以下任意一个函数**，那么很可能你也需要自定义另外两个：

| 函数           | 作用                 |
| -------------- | -------------------- |
| 析构函数       | 管理资源释放         |
| 拷贝构造函数   | 构造时复制另一个对象 |
| 拷贝赋值操作符 | 赋值时复制另一个对象 |

#### 为什么？

因为这些操作都涉及“**资源复制/释放**”，如果你只定义一个，**可能导致资源泄漏、浅拷贝、重复释放等严重问题**。

#### 举例：错误的三法则遗漏

```cpp
class Buffer {
    int* data;
    int size;

public:
    Buffer(int s) {
        data = new int[s];
        size = s;
    }

    ~Buffer() {
        delete[] data;
    }

    // 没写拷贝构造和赋值操作符
};

int main() {
    Buffer a(10);
    Buffer b = a;  // ❌ 默认拷贝构造 → 浅拷贝 → 重复释放崩溃
}
```

### 五法则（Rule of Five）

#### 定义（C++11）

在三法则的基础上，C++11 引入了 **右值引用（`&&`）**，为了支持移动语义，三法则扩展成了五法则。

你需要定义：

| 函数名称         | 用途                         |
| ---------------- | ---------------------------- |
| 析构函数         | 释放资源                     |
| 拷贝构造函数     | 拷贝构造                     |
| 拷贝赋值操作符   | 拷贝赋值                     |
| ✅ 移动构造函数   | 将资源转移给新对象（右值）   |
| ✅ 移动赋值操作符 | 将资源转移到已有对象（右值） |

#### 举例：现代类正确实现五法则

```cpp
class Buffer {
    int* data;
    int size;

public:
    Buffer(int s) : size(s), data(new int[s]) {}
    ~Buffer() { delete[] data; }

    Buffer(const Buffer& other) : size(other.size), data(new int[size]) {
        std::copy(other.data, other.data + size, data);
    }

    Buffer& operator=(const Buffer& other) {
        if (this != &other) {
            delete[] data;
            size = other.size;
            data = new int[size];
            std::copy(other.data, other.data + size, data);
        }
        return *this;
    }

    Buffer(Buffer&& other) noexcept : size(other.size), data(other.data) {
        other.data = nullptr;
        other.size = 0;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data;
            size = other.size;
            data = other.data;
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }
};
```

### 什么时候用三法则 / 五法则？

#### 使用时机：类中**管理资源**（如指针、文件、句柄、套接字）时

| 你在类中用了...              | 是否需要三法则/五法则    |
| ---------------------------- | ------------------------ |
| `new` / `delete`             | ✅ 一定要！               |
| `malloc` / `free`            | ✅ 一定要！               |
| 打开文件、句柄等             | ✅ 一定要！               |
| 智能指针 `unique_ptr`        | ✅ 通常禁用拷贝，支持移动 |
| 没有资源（只有 int、string） | ❌ 通常不需要             |

#### =default / =delete 的补充

- `=default`：显式声明默认函数行为；
- `=delete`：禁用拷贝或移动操作；

示例：禁用拷贝，只支持移动

```cpp
class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};
```

#### 判断口诀：

> “你写了析构，看一眼；拷贝构造要跟上。
>  若用移动，要五全；资源类别忘全写。”

使用三法则和五法则的决策树

```cpp
text复制编辑类中是否有资源（如指针、文件等）？
├─ 否 → 不需要三/五法则
└─ 是
    ├─ 只用拷贝 → 三法则
    └─ 还用移动 → 五法则
```

## 拷贝构造函数

**拷贝构造函数（Copy Constructor）\**是 C++ 类设计中非常关键的一部分。它关系到对象的\**复制、资源管理、安全性和性能**。

拷贝构造函数的作用是：**使用一个已有对象来构造一个新的对象**，本质是**对象的“复制”过程**。

### 语法形式

```cpp
ClassName(const ClassName& other);
```

- 参数是 **当前类类型的 const 引用**；
- 不能是值传递（否则会无限递归调用自身）；
- `const` 是为了允许拷贝 `const` 对象；
- 引用是为了避免函数参数传值时再次调用拷贝构造。

### 拷贝构造函数调用时机

| 场景                                 | 是否调用拷贝构造 |
| ------------------------------------ | ---------------- |
| 用对象初始化另一个对象：`A a2 = a1;` | ✅                |
| 将对象作为值传递给函数               | ✅                |
| 函数返回对象时按值接收               | ✅（或移动构造）  |
| STL 容器元素的复制                   | ✅                |

### 举例说明

```cpp
#include <iostream>
using namespace std;

class Person {
public:
    int age;

    Person(int a) : age(a) {
        cout << "构造函数" << endl;
    }

    // 拷贝构造函数
    Person(const Person& other) {
        age = other.age;
        cout << "拷贝构造函数" << endl;
    }
};

int main() {
    Person p1(20);
    Person p2 = p1;  // 触发拷贝构造
}
```

### 编译器自动生成的拷贝构造

如果你不自己写，编译器会生成**默认的拷贝构造函数**，进行 **成员变量的逐个复制（浅拷贝）**。

这在类中只有简单成员变量（如 int、string）时是可以的。

### 需要自定义拷贝构造的情况（资源类）

当类中有**指针成员**（或文件、网络句柄等资源）时，必须写**深拷贝**，否则多个对象指向同一块资源，容易出现：

- 重复释放；
- 改一个对象影响另一个；
- 程序崩溃。

### 深拷贝示例（手动复制指针）：

```cpp
class Buffer {
    int* data;
    int size;

public:
    Buffer(int s) : size(s) {
        data = new int[size];
        cout << "构造" << endl;
    }

    ~Buffer() {
        delete[] data;
        cout << "析构" << endl;
    }

    // 深拷贝构造函数
    Buffer(const Buffer& other) : size(other.size) {
        data = new int[size];
        std::copy(other.data, other.data + size, data);
        cout << "拷贝构造" << endl;
    }
};
```

## 拷贝赋值操作符

拷贝赋值操作符是用来**将一个已有对象的内容赋值给另一个已存在的对象**，用的就是我们熟悉的 `=` 操作符。

### 语法形式

```cpp
ClassName& operator=(const ClassName& other);
```

说明：

- 返回类型是引用：允许连续赋值（如 `a = b = c;`）
- 参数是 `const ClassName&`：避免修改源对象，支持拷贝 const 对象
- 一般都需要加 `if (this != &other)` 判断，避免**自赋值**

### 拷贝赋值什么时候被调用？

| 场景                        | 是否调用拷贝赋值 |
| --------------------------- | ---------------- |
| 对象已存在，再执行 `a = b;` | ✅                |
| STL 容器中的元素赋值        | ✅                |
| 传值返回后再赋值            | ✅                |
| `a = std::move(b);`         | ❌ → 移动赋值     |

### 举例说明：

```cpp
#include <iostream>
using namespace std;

class Person {
public:
    int age;

    Person(int a) : age(a) {}

    // 拷贝赋值操作符
    Person& operator=(const Person& other) {
        cout << "拷贝赋值操作符被调用" << endl;
        if (this != &other) {
            age = other.age;
        }
        return *this;
    }
};

int main() {
    Person p1(18);
    Person p2(20);
    p2 = p1;  // 触发拷贝赋值
}
```

### 编译器提供的默认拷贝赋值行为（浅拷贝）

如果你不写拷贝赋值函数，编译器会自动生成一个**按成员值复制**的默认版本（也叫浅拷贝）：

```cpp
p2.age = p1.age;  // 自动做了这事
```

### 深拷贝版本写法：

```cpp
class Buffer {
    int* data;
    int size;

public:
    Buffer(int s) : size(s) {
        data = new int[size];
    }

    ~Buffer() {
        delete[] data;
    }

    // 拷贝赋值操作符（深拷贝）
    Buffer& operator=(const Buffer& other) {
        if (this != &other) {
            delete[] data;  // 先释放原资源
            size = other.size;
            data = new int[size];
            std::copy(other.data, other.data + size, data);
        }
        return *this;
    }
};
```

### 最佳实践建议

实现时要遵循：

```cpp
1. if (this != &other) 判断自赋值；
2. 释放原有资源；
3. 复制新资源；
4. 返回 *this；
```

C++11推荐方式（带 std::swap），更安全 + 异常安全

```cpp
class A {
    int* ptr;

public:
    A& operator=(A other) {  // 传值（自动拷贝）
        std::swap(ptr, other.ptr);
        return *this;
    }
};
```

这种方式叫做**拷贝并交换（copy and swap）**，安全且异常安全，是现代 C++ 推荐用法。

## 移动构造函数

移动构造函数的作用是：**利用已有对象（通常是临时对象/右值）的资源，快速构造一个新对象，而不是复制。**

它通过**“资源转移”而不是复制资源**来避免不必要的开销。

### 基本语法

```cpp
ClassName(ClassName&& other);
```

- 参数是**右值引用（T&&）**；
- 一般配合 `std::move()` 使用；
- 通常和“资源所有权转移”相关（指针、文件描述符等）；

### 移动构造和拷贝构造的区别

| 比较项       | 拷贝构造函数             | 移动构造函数                   |
| ------------ | ------------------------ | ------------------------------ |
| 参数类型     | `const ClassName&`       | `ClassName&&`（右值引用）      |
| 资源处理方式 | 深拷贝（复制资源）       | 资源转移（直接接管资源）       |
| 性能         | 较慢（可能复制大量数据） | ✅ 高效（零拷贝）               |
| 典型用途     | 复制现有对象             | 接管临时对象或已不再需要的对象 |

### 何时调用移动构造函数？

| 场景                                            | 是否调用移动构造 |
| ----------------------------------------------- | ---------------- |
| 函数返回临时对象                                | ✅                |
| `std::move()` 后进行初始化                      | ✅                |
| STL 容器插入右值对象（如 `vec.emplace_back()`） | ✅                |

### 示例：没有移动构造时（慢）

```cpp
class Buffer {
    int* data;
    int size;

public:
    Buffer(int s) : size(s) {
        data = new int[size];
        cout << "构造" << endl;
    }

    ~Buffer() {
        delete[] data;
        cout << "析构" << endl;
    }

    // 拷贝构造
    Buffer(const Buffer& other) : size(other.size) {
        data = new int[size];
        std::copy(other.data, other.data + size, data);
        cout << "拷贝构造" << endl;
    }
};
```

```cpp
Buffer makeBuffer() {
    Buffer b(10);
    return b;  // 没有移动构造，将调用拷贝构造（性能低）
}
```

### 示例：添加移动构造（性能优化）

```cpp
class Buffer {
    int* data;
    int size;

public:
    Buffer(int s) : size(s) {
        data = new int[size];
        cout << "构造" << endl;
    }

    ~Buffer() {
        delete[] data;
        cout << "析构" << endl;
    }

    // 拷贝构造（深拷贝）
    Buffer(const Buffer& other) : size(other.size) {
        data = new int[size];
        std::copy(other.data, other.data + size, data);
        cout << "拷贝构造" << endl;
    }

    // ✅ 移动构造函数
    Buffer(Buffer&& other) noexcept : data(other.data), size(other.size) {
        other.data = nullptr;  // 防止析构两次
        other.size = 0;
        cout << "移动构造" << endl;
    }
};
```

```cpp
int main() {
    Buffer b1 = makeBuffer();  // ✅ 调用移动构造（更快）
}
```

为什么使用 `noexcept`？

```cpp
Buffer(Buffer&& other) noexcept;
```

- STL 容器如 `std::vector` 在移动元素时 **只会使用 noexcept 的移动构造**；
- 否则会回退到 **拷贝构造函数**，影响性能！

移动构造 vs std::move

- `std::move(obj)`：并不是移动，而是**把左值强制转换为右值**；
- 你写了移动构造，**只有在 `std::move()` 后才可能触发移动行为**；

示例：

```cpp
Buffer a(10);
Buffer b = std::move(a);  // 调用移动构造函数
```

### 推荐的移动构造实现写法

```cpp
ClassName(ClassName&& other) noexcept
    : ptr(other.ptr), size(other.size) {
    other.ptr = nullptr;
    other.size = 0;
}
```

步骤：

1. 把“原对象”的资源指针/句柄转移过来；
2. 把原对象的指针置为 `nullptr`，防止双重释放；
3. 一般加 `noexcept`，以便容器使用；

### 禁用拷贝，仅支持移动

有时你不想类被复制，只能移动：

```cpp
class MyOnlyMovable {
public:
    MyOnlyMovable() = default;
    MyOnlyMovable(MyOnlyMovable&&) = default;

    MyOnlyMovable(const MyOnlyMovable&) = delete;
    MyOnlyMovable& operator=(const MyOnlyMovable&) = delete;
};
```

### 总结：移动构造函数知识点

| 项目                  | 内容                                                        |
| --------------------- | ----------------------------------------------------------- |
| 语法                  | `ClassName(ClassName&& other);`                             |
| 用途                  | 把一个临时对象或右值的资源“转移”到新对象                    |
| 提高性能              | ✅ 避免深拷贝，节省大量资源复制                              |
| 必须处理原对象状态    | 把指针置空等，避免双重释放                                  |
| 与 std::move 配合使用 | `std::move(obj)` 将左值转成右值引用以触发移动构造           |
| 推荐加 noexcept       | 便于 STL 使用，如 `vector::emplace_back()` 优先使用移动构造 |

## 移动赋值操作符

**移动赋值操作符**的作用是：将一个“将亡对象”（右值）的**资源转移**到**已经存在的对象中**，**替换原有资源**，提升性能。

### 基本语法

```cpp
ClassName& operator=(ClassName&& other) noexcept;
```

- `ClassName&&` 是 **右值引用**；
- 返回引用类型，支持链式赋值；
- 推荐加 `noexcept`，以便 STL 容器高效使用。

### 移动赋值操作符的调用时机

| 场景                                         | 是否调用             |
| -------------------------------------------- | -------------------- |
| 对一个已有对象赋值一个右值对象（如临时变量） | ✅ 调用移动赋值操作符 |
| 对一个已有对象赋值一个左值对象               | ❌ 调用拷贝赋值操作符 |
| 对一个已有对象执行 `a = std::move(b)`        | ✅ 调用移动赋值操作符 |

### 举例说明：

```cpp
class Buffer {
    int* data;
    int size;

public:
    Buffer(int s) : size(s) {
        data = new int[size];
        cout << "构造函数" << endl;
    }

    ~Buffer() {
        delete[] data;
        cout << "析构函数" << endl;
    }

    // 移动构造函数
    Buffer(Buffer&& other) noexcept : data(other.data), size(other.size) {
        other.data = nullptr;
        other.size = 0;
        cout << "移动构造" << endl;
    }

    // ✅ 移动赋值操作符
    Buffer& operator=(Buffer&& other) noexcept {
        cout << "移动赋值操作符" << endl;
        if (this != &other) {
            delete[] data;              // 释放当前对象已有资源
            data = other.data;          // 接管资源
            size = other.size;

            other.data = nullptr;       // 防止析构时重复释放
            other.size = 0;
        }
        return *this;
    }
};

int main() {
    Buffer b1(10);
    Buffer b2(5);
    b2 = std::move(b1);  // 调用移动赋值
}
```

### 和拷贝赋值的区别

| 对比项             | 拷贝赋值           | 移动赋值               |
| ------------------ | ------------------ | ---------------------- |
| 参数类型           | `const ClassName&` | `ClassName&&`          |
| 是否复制资源       | ✅ 深拷贝           | ❌ 不复制，直接转移资源 |
| 是否需要释放原资源 | ✅（释放 + 复制）   | ✅（释放 + 接管）       |
| 性能               | 较慢               | ✅ 非常快（零拷贝）     |
| 推荐加 noexcept    | 可选               | ✅ 强烈建议             |

异常安全：推荐 `noexcept`

STL 容器在使用移动赋值时，会检查函数是否标注 `noexcept`，否则**宁愿退回使用拷贝赋值**！

### 现代推荐写法：拷贝并交换（适用于移动赋值）

可以复用已有的移动构造 + `swap`：

```cpp
class Buffer {
    int* data;
    int size;

public:
    // 移动构造
    Buffer(Buffer&& other) noexcept : data(other.data), size(other.size) {
        other.data = nullptr;
        other.size = 0;
    }

    // ✅ 移动赋值（推荐写法）
    Buffer& operator=(Buffer&& other) noexcept {
        swap(data, other.data);
        swap(size, other.size);
        return *this;
    }
};
```

### 禁止拷贝，仅允许移动（唯一资源所有者场景）

有些类只允许“转移”资源，不允许复制：

```cpp
class UniqueFile {
public:
    UniqueFile(const UniqueFile&) = delete;
    UniqueFile& operator=(const UniqueFile&) = delete;

    UniqueFile(UniqueFile&&) = default;
    UniqueFile& operator=(UniqueFile&&) = default;
};
```

### 与 `std::move()` 的关系

- `std::move()`：并不会真的“移动”，它只是**将左值标记为右值**；
- 是否移动，取决于你有没有提供 **移动构造函数 / 移动赋值操作符**。

```cpp
Buffer a(10);
Buffer b(20);
b = std::move(a);  // ❗ 会调用移动赋值
```

### 总结：移动赋值操作符

| 项目            | 内容                                                  |
| --------------- | ----------------------------------------------------- |
| 语法            | `T& operator=(T&& other) noexcept;`                   |
| 场景            | 将一个右值对象的资源转移到已有对象中                  |
| 必须处理        | 原有资源的释放、转移后的对象状态设置                  |
| 推荐 `noexcept` | STL 容器会优先使用 `noexcept` 的移动赋值              |
| 拷贝并交换法    | 通过 swap 简化赋值逻辑，提高异常安全性                |
| Rule of Five    | 若涉及资源管理，需定义移动赋值和其他 4 个特殊成员函数 |