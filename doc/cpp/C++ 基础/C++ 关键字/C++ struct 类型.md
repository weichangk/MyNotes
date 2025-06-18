在 C++ 中，`struct`（结构体）是一种用户自定义的数据类型，用于将多个不同类型的数据组合在一起。和 `class` 类似，`struct` 也可以包含成员变量、成员函数、构造函数、析构函数、访问控制符等。

### 一、基本语法

```cpp
struct Person {
    std::string name;
    int age;
};
```

你可以这样使用：

```cpp
Person p;
p.name = "Tom";
p.age = 25;
```

### 二、struct 与 class 的区别

| 特性             | struct   | class     |
| ---------------- | -------- | --------- |
| 默认访问权限     | `public` | `private` |
| 继承默认访问权限 | `public` | `private` |

```cpp
struct A {
    int x; // 默认 public
};

class B {
    int x; // 默认 private
};
```

但从功能上来说，两者几乎可以互换，很多现代 C++ 开发者将 `struct` 用于只包含数据的数据结构（即 POD 类型）。

### 三、struct 的进阶用法

#### 1. 带构造函数

```cpp
struct Point {
    int x, y;
    Point(int x_, int y_) : x(x_), y(y_) {}
};
```

#### 2. 成员函数

```cpp
struct Rectangle {
    int width, height;
    int area() const {
        return width * height;
    }
};
```

#### 3. 嵌套使用

```cpp
struct Address {
    std::string city;
    std::string street;
};

struct Person {
    std::string name;
    int age;
    Address addr;
};
```

### 四、初始化方式

```cpp
Person p1 = {"Alice", 30};  // 聚合初始化
Person p2{"Bob", 25};       // C++11 列表初始化
```

### 五、作为函数参数或返回值

```cpp
void printPerson(const Person& p);
Person createPerson();
```

### 六、简单了解什么是POD 类型？

POD 类型，全称是 **Plain Old Data**（“普通旧数据”），是 C++ 中的一类特殊类型，具有以下特性：

POD 类型 = 既是 “普通的类类型（Plain Old Class），也是 “标准布局类型（Standard Layout）。

POD 类型本质上是类似于 C 语言中的 `struct` 类型：

- **数据成员是基本类型或其他 POD**
- **没有用户定义的构造函数、析构函数、拷贝/移动构造函数、赋值运算符**
- **没有虚函数**
- **没有继承（或者只从另一个 POD 类型继承）**

这样类型的数据结构可以在内存中按字节复制（如 `memcpy`），通常用于与 C 接口兼容、底层数据传输、文件或网络二进制读写等。

POD 类型的优点和用途：

- **与 C 语言兼容**：比如用于 `extern "C"` 的接口
- **二进制兼容/序列化方便**：可以直接 `memcpy`，也可以写入文件或网络
- **更高的性能优化**：因为没有构造/析构/虚函数等复杂行为

如果你做嵌入式编程、与 C 库交互、或者性能敏感的内存操作时，理解和使用 POD 类型非常重要。