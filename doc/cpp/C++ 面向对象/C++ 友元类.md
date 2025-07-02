C++ 中的 **友元类（friend class）** 是一种特殊的类，它可以访问另一个类的 **私有（`private`）** 和 **保护（`protected`）** 成员，哪怕它并不是该类的成员。

## 什么是友元类？

当一个类 A 声明另一个类 B 为“友元类”时，**B 可以访问 A 的私有成员和保护成员**，就像是它自己的成员一样。
 这打破了类与类之间的访问权限限制，常用于紧密合作的类之间。

## 友元类的基本语法

```cpp
class A {
    friend class B;  // B 是 A 的友元类
private:
    int data = 42;
};

class B {
public:
    void showA(A& a) {
        std::cout << a.data << std::endl;  // 可以访问 A 的私有成员
    }
};
```

## 友元类的特点

| 特性     | 说明                                                      |
| -------- | --------------------------------------------------------- |
| 单向关系 | A 把 B 设为友元类，B 能访问 A，但 A 不能访问 B 的私有成员 |
| 不是成员 | B 虽能访问 A 的私有成员，但并不是 A 的成员类              |
| 非传递   | 如果 A 是 B 的友元，B 是 C 的友元，不代表 A 是 C 的友元   |
| 提前声明 | 被设为友元的类，必须先被声明（通常用前向声明）            |

## 场景 1：**控制访问**

当一个辅助类需要频繁访问主类的私有成员时，可以设置其为友元类，避免写大量 getter/setter。

示例：`MatrixHelper` 类访问 `Matrix` 私有数据

```cpp
#include <iostream>
#include <vector>

class Matrix;  // 前向声明

class MatrixHelper {
public:
    static void printMatrix(const Matrix& m);
};

class Matrix {
    friend class MatrixHelper;  // MatrixHelper 是友元类
private:
    std::vector<std::vector<int>> data;

public:
    Matrix(int rows, int cols, int value) {
        data.resize(rows, std::vector<int>(cols, value));
    }
};

void MatrixHelper::printMatrix(const Matrix& m) {
    for (const auto& row : m.data) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    Matrix mat(2, 3, 7);
    MatrixHelper::printMatrix(mat);
    return 0;
}
```

## 场景 2：**桥接模式（Bridge Pattern）**

桥接模式让抽象和实现分离。接口类和实现类分离后，接口类可以将实现类设为友元，方便操作私有成员。

示例：接口 `Device` 与实现 `ConcreteDevice`

```cpp
#include <iostream>
#include <string>

class ConcreteDevice;

class Device {
protected:
    ConcreteDevice* impl;

public:
    Device(ConcreteDevice* impl) : impl(impl) {}
    void turnOn();  // 调用实现
};

class ConcreteDevice {
    friend class Device;  // 允许 Device 访问实现的私有成员
private:
    std::string status = "off";

    void internalTurnOn() {
        status = "on";
    }

    void printStatus() {
        std::cout << "Device status: " << status << std::endl;
    }
};

void Device::turnOn() {
    impl->internalTurnOn();
    impl->printStatus();
}

int main() {
    ConcreteDevice realDevice;
    Device d(&realDevice);
    d.turnOn();
    return 0;
}
```

## 场景 3：**工具类操作主类的内部细节**

工具类可以被设置为主类的友元，用于封装对主类内部的操作逻辑，比如调试、序列化等。

示例：`FileUtils` 工具类读取 `File` 私有字段

```cpp
#include <iostream>
#include <string>

class File;  // 前向声明

class FileUtils {
public:
    static void debugPrint(const File& file);
};

class File {
    friend class FileUtils;  // FileUtils 是友元类
private:
    std::string path = "/user/home/file.txt";
    int size = 1024;

public:
    File() = default;
};

void FileUtils::debugPrint(const File& file) {
    std::cout << "File path: " << file.path << ", size: " << file.size << " bytes" << std::endl;
}

int main() {
    File f;
    FileUtils::debugPrint(f);
    return 0;
}
```

## 场景 4：**调试器/监视器类访问被调试类**

常用于游戏开发、系统调试等场景。

```cpp
#include <iostream>
#include <string>

class GameCharacter;  // 前向声明

class Debugger {
public:
    static void dumpState(const GameCharacter& gc);
};

class GameCharacter {
    friend class Debugger;  // Debugger 是友元类
private:
    std::string name = "Hero";
    int health = 100;
    int mana = 50;
};

void Debugger::dumpState(const GameCharacter& gc) {
    std::cout << "Name: " << gc.name
              << ", Health: " << gc.health
              << ", Mana: " << gc.mana << std::endl;
}

int main() {
    GameCharacter c;
    Debugger::dumpState(c);
    return 0;
}
```

## 场景 5：**对象池等设计模式的内部控制类**

对象池管理类作为资源类的友元，可以访问其构造/析构控制细节。

```cpp
#include <iostream>

class ObjectPool;

class Resource {
    friend class ObjectPool;  // 只让对象池控制它的创建和销毁
private:
    Resource() { std::cout << "Resource created\n"; }
    ~Resource() { std::cout << "Resource destroyed\n"; }

public:
    void use() { std::cout << "Using resource\n"; }
};

class ObjectPool {
public:
    static Resource* create() {
        return new Resource();
    }

    static void destroy(Resource* r) {
        delete r;
    }
};

int main() {
    Resource* r = ObjectPool::create();
    r->use();
    ObjectPool::destroy(r);
    return 0;
}
```

## 总结：友元类的使用时机

| 场景                       | 是否适合用 friend class |
| -------------------------- | ----------------------- |
| 助手类需要频繁访问私有数据 | ✅ 是                    |
| 抽象与实现分离（桥接）     | ✅ 是                    |
| 工具类进行调试/序列化      | ✅ 是                    |
| 代码风格要求严格封装       | ❌ 建议用 getter/setter  |

## 复杂案例：友元类 + 模板

为什么模板类需要 friend？

有些模板类需要与其他类协作，而这些协作类又需要访问模板类的私有成员。例如：

- 工具类需要访问模板类内部数据；
- 不同类型模板实例需要互相访问；
- 模板类的某些函数专门配合其他类使用；
- 为某个类型的模板实例设置 friend。

### 模板 + friend 的组合形式多样

下面列出各种友元模板组合形式，我们通过实战一个个讲解：

| 类型                        | 示例                                           |
| --------------------------- | ---------------------------------------------- |
| 1. 普通类是模板类的友元     | `friend class Helper;`                         |
| 2. 模板类是模板类的友元     | `friend class Helper<T>;`                      |
| 3. 模板类的所有实例都是友元 | `template <typename U> friend class Helper;`   |
| 4. 模板函数是友元函数       | `friend void helperFunc<>(const MyClass<T>&);` |
| 5. 具体实例是友元           | `friend class Helper<int>;`                    |

### 示例 1：**一个普通类作为模板类的友元**

```cpp
#include <iostream>

class Inspector;  // 前向声明

template <typename T>
class Box {
    friend class Inspector;  // 所有 Box<T> 都允许 Inspector 访问

private:
    T data;
public:
    Box(T val) : data(val) {}
};

class Inspector {
public:
    template <typename T>
    static void printBox(const Box<T>& b) {
        std::cout << "Box data = " << b.data << std::endl;
    }
};

int main() {
    Box<int> b(123);
    Inspector::printBox(b);
    return 0;
}
```

说明：

- `Inspector` 是普通类；
- 它可以访问所有 `Box<T>` 的私有成员；
- 适合调试器、工具类、日志系统。

### 示例 2：**另一个模板类是友元**

```cpp
#include <iostream>

template <typename T>
class B;  // 前向声明

template <typename T>
class A {
    friend class B<T>;  // B<T> 是 A<T> 的朋友

private:
    T value = 100;
};

template <typename T>
class B {
public:
    void accessA(const A<T>& a) {
        std::cout << "Access A<T>'s value = " << a.value << std::endl;
    }
};

int main() {
    A<int> a;
    B<int> b;
    b.accessA(a);
    return 0;
}
```

说明：

- `A<T>` 和 `B<T>` 是“配对模板”；
- `B<int>` 只能访问 `A<int>`，不能访问 `A<float>`。

### 示例 3：**让所有 B<U> 模板实例访问 A<T>**

```cpp
#include <iostream>

template <typename T>
class A;

template <typename U>
class B {
public:
    void show(const A<U>& a);
};

template <typename T>
class A {
    template <typename U>
    friend class B;  // 所有 B<U> 都能访问所有 A<T>

private:
    T secret = 777;
};

template <typename U>
void B<U>::show(const A<U>& a) {
    std::cout << "Accessed A<U>'s secret: " << a.secret << std::endl;
}

int main() {
    A<int> a;
    B<int> b;
    b.show(a);
    return 0;
}
```

说明：

- `template <typename U> friend class B;`：表示 **所有 B<U> 都是 A 的朋友**；
- 常用于库开发，如 `std::hash<T>`、`std::allocator<T>` 访问用户定义的模板。

### 示例 4：**模板函数作为模板类的友元函数**

```cpp
#include <iostream>

template <typename T>
class Box;

template <typename T>
void printBox(const Box<T>& b);  // 模板函数前置声明

template <typename T>
class Box {
    friend void printBox<T>(const Box<T>&);  // 具体函数模板是 friend

private:
    T value = 42;
};

template <typename T>
void printBox(const Box<T>& b) {
    std::cout << "Box contains: " << b.value << std::endl;
}

int main() {
    Box<int> b;
    printBox(b);
    return 0;
}
```

说明：

- 友元函数必须**提前声明**，否则编译器不能识别；
- `friend void printBox<T>(...)` 必须使用 `<T>`，不能省略；
- 否则编译器会认为是非模板版本。

### 示例 5：**只让某个类型实例成为友元**

```cpp
#include <iostream>

template <typename T>
class OnlyInt;  // 前置声明

template <typename T>
class Target {
    friend class OnlyInt<int>;  // 只有 OnlyInt<int> 是朋友

private:
    T data = 99;
};

template <typename T>
class OnlyInt {
public:
    void access(const Target<T>& t) {
        std::cout << "OnlyInt<T> sees: " << t.data << std::endl;
    }
};

int main() {
    Target<int> t;
    OnlyInt<int> oi;
    oi.access(t);  // ✅ ok

    // OnlyInt<double> od;
    // od.access(Target<int>());  // ❌ 编译错误（没有权限）
    return 0;
}
```