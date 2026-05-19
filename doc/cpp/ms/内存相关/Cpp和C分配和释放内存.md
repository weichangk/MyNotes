C 和 C++ 在“内存分配与释放”上的核心区别，本质是：**C 是函数级控制，C++ 是语言级抽象 + 构造/析构生命周期管理**。

------

# 一、C 语言的内存分配与释放

## 1. 动态内存函数（标准库 `<stdlib.h>`）

### 分配

```
void* malloc(size_t size);
void* calloc(size_t n, size_t size);
void* realloc(void* ptr, size_t new_size);
```

### 释放

```
void free(void* ptr);
```

------

## 2. 特点

### ✔ 1）只负责“内存块”

- 不调用构造函数
- 不初始化对象（malloc）
- 只返回 `void*`

### ✔ 2）类型不安全

```
int* p = (int*)malloc(sizeof(int) * 10);
```

### ✔ 3）必须手动释放

```
free(p);
```

------

## 3. 常见问题

- 内存泄漏（忘记 free）
- 野指针（free 后未置 NULL）
- double free（重复释放）
- 缓冲区溢出

------

# 二、C++ 的内存分配与释放

C++ 在 C 的基础上增加了**对象生命周期管理**。

------

## 1. new / delete

### 分配单对象

```
int* p = new int(10);
delete p;
```

------

### 分配数组

```
int* arr = new int[10];
delete[] arr;
```

------

## 2. new/delete 做了什么？

### new（底层流程）

```
operator new -> 分配原始内存 -> 调用构造函数
```

### delete（底层流程）

```
调用析构函数 -> operator delete -> 释放内存
```

------

## 3. 与 malloc/free 最大区别

| 对比项   | C (malloc/free) | C++ (new/delete)  |
| -------- | --------------- | ----------------- |
| 内存分配 | ✔               | ✔                 |
| 构造函数 | ❌               | ✔                 |
| 析构函数 | ❌               | ✔                 |
| 类型安全 | ❌               | ✔                 |
| 返回类型 | void*           | T*                |
| 可重载   | ❌               | ✔（operator new） |

------

# 三、C++ 的关键增强：对象生命周期

## 1. 构造 + 析构自动绑定

```
class A {
public:
    A()  { std::cout << "ctor\n"; }
    ~A() { std::cout << "dtor\n"; }
};

A* p = new A();  // 自动调用构造
delete p;        // 自动调用析构
```

------

## 2. C 无此能力

```
struct A* p = malloc(sizeof(struct A));
// 没有构造/析构概念
free(p);
```

------

# 四、operator new / malloc 本质区别（面试重点）

## 1. malloc 本质

- libc 实现
- 从堆申请原始内存
- 不可重载

------

## 2. operator new 本质

```
void* operator new(size_t size);
```

- C++ 运行时提供
- 可自定义重载
- 失败抛异常（std::bad_alloc）

------

## 3. 行为对比

| 项目         | malloc    | operator new         |
| ------------ | --------- | -------------------- |
| 失败处理     | 返回 NULL | 抛异常               |
| 可重载       | ❌         | ✔                    |
| 是否调用构造 | ❌         | ❌（new表达式才调用） |

------

# 五、placement new（高级点）

允许“在指定内存上构造对象”：

```
void* buf = malloc(sizeof(A));

A* obj = new (buf) A(); // placement new
```

释放：

```
obj->~A();
free(buf);
```

👉 常用于：

- 内存池
- STL allocator
- 高性能系统

------

# 六、C++ 推荐方式（现代 C++）

## ❌ 不推荐裸 new/delete

```
A* p = new A();
delete p;
```

------

## ✅ 推荐 RAII

### 1. unique_ptr

```
std::unique_ptr<A> p = std::make_unique<A>();
```

### 2. shared_ptr

```
std::shared_ptr<A> p = std::make_shared<A>();
```

------

## 优势

- 自动释放
- 异常安全
- 无泄漏

------

# 七、总结一句话（面试用）

> C 使用 malloc/free 进行“纯内存管理”，不涉及对象生命周期；
>  C++ 使用 new/delete 在分配内存的同时绑定构造与析构，实现 RAII，从而管理对象生命周期。