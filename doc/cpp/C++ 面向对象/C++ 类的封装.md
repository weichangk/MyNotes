C++ 中的类封装（Encapsulation）是面向对象编程的三大特性之一（封装、继承、多态），主要目的是**隐藏内部实现细节，对外提供统一接口**，增强代码的**可维护性、安全性和可重用性**。

### 封装的基本概念

封装 = **数据 + 方法**

- 将数据（成员变量）和操作数据的方法（成员函数）放在一个类中；
- 通过访问权限控制外部对类中成员的访问方式。

### 访问权限控制

| 关键字      | 含义                                               |
| ----------- | -------------------------------------------------- |
| `private`   | 私有成员，只能被类的成员函数或友元访问（默认权限） |
| `protected` | 受保护成员，子类也能访问                           |
| `public`    | 公有成员，类外可以访问                             |

### 封装的示例

在 C++ 中，为了提高代码的**可读性**和**模块化管理能力**，通常会将类的声明（接口）和实现分开：

- **类的声明**：写在 `.h`（或 `.hpp`）头文件中。
- **类的实现**：写在 `.cpp` 源文件中。

`Person.h` —— 类的声明

```cpp
#ifndef PERSON_H
#define PERSON_H

#include <string>

class Person {
private:
    std::string name;
    int age;

public:
    Person(const std::string& name, int age);
    
    void setName(const std::string& name);
    void setAge(int age);
    
    std::string getName() const;
    int getAge() const;

    void introduce() const;
};

#endif // PERSON_H
```

`Person.cpp` —— 类的实现

```cpp
#include "Person.h"
#include <iostream>

Person::Person(const std::string& name, int age) {
    this->name = name;
    setAge(age);  // 使用封装的方法
}

void Person::setName(const std::string& name) {
    this->name = name;
}

void Person::setAge(int age) {
    if (age >= 0 && age <= 150)
        this->age = age;
    else
        this->age = 0;
}

std::string Person::getName() const {
    return name;
}

int Person::getAge() const {
    return age;
}

void Person::introduce() const {
    std::cout << "I am " << name << ", " << age << " years old." << std::endl;
}
```

#### `main.cpp` —— 使用类

```cpp
#include "Person.h"

int main() {
    Person p("Alice", 30);
    p.introduce();

    p.setAge(200);  // 无效，age 会被置为 0
    p.introduce();

    return 0;
}
```

### 封装的优点

1. **隐藏内部实现**，防止外部直接修改对象状态，破坏程序逻辑；
2. **接口统一**，便于维护和修改；
3. **增强安全性**，可以做合法性校验（如 setAge）；
4. **代码结构清晰**，职责明确。

### 类封装技巧

- 成员变量一律设为 `private`；
- 对外提供 `public` 的 getter/setter 方法；
- 使用 `const` 成员函数保证访问器不修改对象；
- 使用构造函数初始化成员，避免未初始化的状态；
- 用友元（`friend`）慎重打破封装。

