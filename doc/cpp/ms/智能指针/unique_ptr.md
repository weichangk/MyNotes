`std::unique_ptr` 是 C++11 引入的**独占式智能指针**，用于管理动态内存，属于 RAII（资源获取即初始化）的一种典型实现。

------

# 一、核心特性（面试重点）

### 1. 独占所有权（核心）

- 同一时刻 **只能有一个 `unique_ptr` 拥有对象**
- 禁止拷贝，只允许移动

```
std::unique_ptr<int> p1 = std::make_unique<int>(10);

// ❌ 错误：不能拷贝
// std::unique_ptr<int> p2 = p1;

// ✅ 正确：移动所有权
std::unique_ptr<int> p2 = std::move(p1);
```

------

### 2. 自动释放资源（RAII）

- 离开作用域自动 `delete`
- 防止内存泄漏

```
void func() {
    std::unique_ptr<int> p = std::make_unique<int>(5);
} // 自动释放
```

------

### 3. 轻量级（无引用计数）

- 不像 `shared_ptr` 有原子计数
- 性能更高
- 适合“明确唯一所有者”的场景

------

# 二、常见用法

### 1. 管理单对象

```
auto p = std::make_unique<int>(100);
```

------

### 2. 管理数组

```
std::unique_ptr<int[]> arr = std::make_unique<int[]>(10);
arr[0] = 1;
```

------

### 3. 作为函数返回值（推荐）

```
std::unique_ptr<Foo> create() {
    return std::make_unique<Foo>();
}
```

👉 编译器会自动做 RVO / move 优化

------

### 4. 容器中使用（移动语义）

```
std::vector<std::unique_ptr<Foo>> vec;
vec.push_back(std::make_unique<Foo>());
```

------

# 三、为什么不能拷贝？

如果允许拷贝：

```
unique_ptr<int> a = make_unique<int>(5);
unique_ptr<int> b = a; // ❌
```

会导致：

- 两个指针指向同一块内存
- 析构时 double free（重复释放）

👉 所以设计为：

- **禁止拷贝构造**
- **禁止拷贝赋值**
- 只允许 move

------

# 四、和 raw pointer 对比

| 特性         | raw pointer | unique_ptr |
| ------------ | ----------- | ---------- |
| 是否自动释放 | ❌           | ✅          |
| 是否独占     | ❌           | ✅          |
| 是否可拷贝   | ✅           | ❌          |
| 安全性       | 低          | 高         |

------

# 五、和 shared_ptr 对比（高频面试）

| 特性     | unique_ptr     | shared_ptr       |
| -------- | -------------- | ---------------- |
| 所有权   | 独占           | 共享             |
| 引用计数 | ❌              | ✅                |
| 性能     | 高             | 较低             |
| 线程安全 | 不涉及计数     | 引用计数线程安全 |
| 使用场景 | 明确唯一拥有者 | 多处共享         |

------

# 六、定制删除器（高级点）

```
std::unique_ptr<FILE, decltype(&fclose)> fp(fopen("a.txt", "r"), fclose);
```

👉 可以管理任何资源（文件、socket、句柄）

------

# 七、典型使用场景（面试加分）

- 工厂函数返回对象
- RAII 管理资源（文件、锁、句柄）
- 替代裸指针作为成员变量
- PIMPL 模式

------

# 八、一句话总结（面试用）

> `unique_ptr` 是一种轻量级 RAII 智能指针，提供独占式所有权管理，通过移动语义实现所有权转移，避免内存泄漏和重复释放问题。