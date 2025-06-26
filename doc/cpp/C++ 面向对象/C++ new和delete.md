在 C++ 中，`new` 和 `delete` 是动态内存管理的核心操作符，它们分别用于动态分配和释放内存。虽然语法简单，但其背后的机制却非常复杂。

## 基础概念

### `new` 操作符

```cpp
int* p = new int(10);
```

- 分配一块足够容纳 `int` 的内存；
- 调用构造函数（如果是类对象）；
- 返回指向该内存的指针。

### 2. `delete` 操作符

```cpp
delete p;
```

- 调用析构函数（如果是类对象）；
- 释放内存。

## 底层实现机制

### `new` 实际上做了两件事：

```cpp
T* p = new T(args);
```

等价于：

```cpp
void* mem = operator new(sizeof(T)); // 调用全局 operator new（底层调用 malloc 或更高级分配器）
T* p = new (mem) T(args);            // 在那块内存上构造对象，称 placement new
```

### `delete` 过程：

```cpp
delete p;
```

等价于：

```cpp
p->~T();                     // 显式调用析构函数
operator delete(p);         // 调用全局 operator delete，底层释放内存
```

## 自定义 `new` 和 `delete`

类可以重载 `new` 和 `delete`，实现更精细的内存管理：

```cpp
class MyClass {
public:
    void* operator new(size_t size) {
        std::cout << "MyClass::new " << size << " bytes\n";
        return malloc(size);
    }
    void operator delete(void* ptr) {
        std::cout << "MyClass::delete\n";
        free(ptr);
    }
};
```

## 数组形式：`new[]` 和 `delete[]`

```cpp
int* arr = new int[10];
delete[] arr;
```

注意事项：

- `new[]` 会额外存储数组元素个数（例如前 4 字节）；
- `delete[]` 会依次调用析构函数；
- 不要混用 `new[]` 和 `delete`（未定义行为）。

## 内存泄漏与悬挂指针

### 内存泄漏

```cpp
int* p = new int(5);
// 忘记 delete p; => 内存泄漏
```

### 悬挂指针

```cpp
int* p = new int(5);
delete p;
// p 现在是悬挂指针
```

> 解决：将 `p` 置为 `nullptr`，或使用智能指针（推荐）

## placement new 的使用

Placement new 是 C++ 中一种特殊形式的 `new` 操作符，用于**在已有的内存上构造对象**，**而不是分配新内存**。

在预分配内存上构造对象（如内存池）

```cpp
char buffer[sizeof(MyClass)];
MyClass* p = new (buffer) MyClass(10, 20);  // 在 buffer 上构造对象
p->~MyClass();                       // 需要手动析构，不能用 delete，因为 delete 会试图释放内存，而内存不是 new 分配的！
```

内存池优化

```cpp
void* pool = malloc(1024);  // 分配一大块内存
for (int i = 0; i < 10; ++i) {
    void* addr = (char*)pool + i * sizeof(MyClass);
    MyClass* p = new (addr) MyClass();
}
```

## 异常安全性

如果构造函数抛异常，`operator new` 分配的内存会自动释放：

```cpp
cpp复制编辑struct A {
    A() {
        throw std::runtime_error("Oops");
    }
};

try {
    A* p = new A;
} catch (...) {
    // 不会内存泄漏
}
```

## 和 malloc/free 的区别

| 特性              | `new/delete`      | `malloc/free`     |
| ----------------- | ----------------- | ----------------- |
| 类型安全          | ✅（返回正确类型） | ❌（返回 void*）   |
| 构造/析构函数调用 | ✅                 | ❌                 |
| 可重载            | ✅                 | ❌                 |
| 对象初始化        | ✅                 | ❌（需手动初始化） |

## 智能指针替代方案

现代 C++ 推荐使用：

- `std::unique_ptr`（唯一所有权）
- `std::shared_ptr`（共享所有权）
- 避免手动调用 `new` 和 `delete`

```cpp
auto ptr = std::make_unique<MyClass>();
```

## 调试建议

- 使用 Valgrind / AddressSanitizer 检测泄漏
- 开启编译器警告和运行时检查
- 使用智能指针简化内存管理