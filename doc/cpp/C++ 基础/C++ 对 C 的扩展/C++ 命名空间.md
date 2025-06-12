在 C++ 中，**命名空间（namespace）** 是用来组织代码的一种机制，它可以避免命名冲突，特别是在大型项目或使用多个库时非常有用。

### 🔹 命名空间的基本语法

```cpp
namespace MyNamespace {
    int value = 42;

    void sayHello() {
        std::cout << "Hello from MyNamespace!" << std::endl;
    }
}
```

使用命名空间中的成员有几种方式：

#### ✅ 1. **直接使用完整限定名**

```cpp
MyNamespace::sayHello();
int x = MyNamespace::value;
```

#### ✅ 2. **使用 `using` 声明**

```
using MyNamespace::sayHello;
sayHello();  // 不需要加命名空间前缀
```

#### ✅ 3. **使用 `using namespace` 声明整个命名空间（不推荐在头文件中使用）**

```cpp
using namespace MyNamespace;
sayHello();      // OK
std::cout << value << std::endl;  // OK
```

### 🔹 命名空间的嵌套

```cpp
namespace A {
    namespace B {
        void foo() {
            std::cout << "A::B::foo" << std::endl;
        }
    }
}

// 调用方式
A::B::foo();
```

C++17 起支持嵌套命名空间的简化写法：

```cpp
namespace A::B {
    void foo() {
        std::cout << "A::B::foo" << std::endl;
    }
}
```

### 🔹 匿名命名空间（internal linkage）

```
namespace {
    void helper() {
        std::cout << "This is a local helper function." << std::endl;
    }
}
```

匿名命名空间内的内容只对当前编译单元可见，等价于使用 `static` 修饰的全局变量/函数。

### 🔹 命名空间的实际用途

- 避免函数、变量、类的**重名冲突**
- 组织大型项目的代码结构（如：`core::utils::Logger`）

### 🔹为什么不推荐在头文件中使用命名空间？

不推荐在 **头文件中使用 `using namespace`**（尤其是 `using namespace std;`）的原因主要有以下几点：

#### 🚫 **1. 会造成命名冲突**

头文件是可以被多个 `.cpp` 文件包含的，如果在头文件中使用了：

```cpp
using namespace std;
```

那么每个包含这个头文件的 `.cpp` 文件就都相当于使用了 `using namespace std;`，这样会导致：

- **标准库名字污染了全局命名空间**
- **和用户自定义的标识符冲突**

⚠️ 例如：

```cpp
// 你写了一个函数：
void sort() { /* ... */ }

// 同时包含了一个头文件，里面有 using namespace std;

sort();  // 编译器就分不清是 std::sort 还是你自己的 sort
```

#### 🚫 **2. 降低封装性和可维护性**

头文件本应只提供接口声明，`using namespace` 是具体的实现行为，不该暴露在接口层。

- 这样做破坏了头文件的“纯粹性”
- 使用者没法明确知道哪些名字从哪里来
- 未来要换库或清理依赖时更麻烦

#### 🚫 **3. 引发奇怪的编译错误**

如果两个头文件都用了 `using namespace`，而且引用了相同名字的不同定义，哪怕你的代码本身没错，编译器也可能报错或行为不一致。

#### ✅ 正确的做法

在头文件中：

- **避免使用 `using namespace`**
- **使用具体的命名空间限定符**

```cpp
// 头文件 myheader.h
#include <string>

namespace mylib {
    void print(const std::string& s);  // 使用 std::string 明确指定
}
```

在源文件中（.cpp）：

- 可以使用 `using namespace`，因为作用范围只限于当前文件

```cpp
// 源文件 myheader.cpp
#include "myheader.h"
#include <iostream>

using namespace std;

namespace mylib {
    void print(const string& s) {
        cout << s << endl;
    }
}
```

如果你希望头文件中书写更方便，但又想避免命名污染，可以考虑使用命名空间别名，比如：

```
namespace stdstring = std::string;  // 不推荐，但比 using namespace std; 好一些
```