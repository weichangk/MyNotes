`std::shared_ptr` 是 C++11 引入的**智能指针**之一，用来实现**共享式所有权管理（shared ownership）**，核心特点是：**多个指针可以共同拥有同一块堆内存，最后一个释放资源的人负责析构对象**。

------

# 1. 基本概念

```
#include <memory>

std::shared_ptr<int> p1 = std::make_shared<int>(10);
std::shared_ptr<int> p2 = p1;  // 共享所有权
```

此时：

- `p1` 和 `p2` 指向同一对象
- 引用计数 = 2
- 最后一个 `shared_ptr` 被销毁时，对象才释放

------

# 2. 核心机制：引用计数（Reference Counting）

`shared_ptr` 内部维护一个控制块：

```
[对象数据] <---- shared_ptr
   ↑
[控制块]
   ├── 引用计数（use_count）
   ├── weak_count
   └── deleter
```

示例：

```
auto a = std::make_shared<int>(5);
auto b = a;

std::cout << a.use_count(); // 2
```

当：

- `a` 析构 → count = 1
- `b` 析构 → count = 0 → delete 对象

------

# 3. 常用操作

## ① 创建

```
auto p = std::make_shared<int>(10); // 推荐
```

❌ 不推荐：

```
std::shared_ptr<int> p(new int(10));
```

------

## ② 拷贝（增加引用计数）

```
auto p2 = p1;
```

------

## ③ 赋值

```
p2 = p1;
```

------

## ④ 获取原始指针

```
p.get();
```

------

## ⑤ 引用计数

```
p.use_count();
```

------

## ⑥ reset

```
p.reset();        // 释放当前对象
p.reset(new int); // 指向新对象
```

------

# 4. shared_ptr 的优点

### ✔ 自动释放内存（RAII）

避免：

- 内存泄漏
- 忘记 delete

### ✔ 支持共享所有权

适合：

- 多模块共享对象
- 图结构
- 缓存对象

------

# 5. 常见问题（面试高频）

------

## Q1：shared_ptr 为什么可能导致循环引用？

```
struct A {
    std::shared_ptr<B> b;
};

struct B {
    std::shared_ptr<A> a;
};
```

结果：

- A → B（计数 +1）
- B → A（计数 +1）
- 永远不会归零 ❌ 内存泄漏

------

## 解决方案：weak_ptr

```
std::weak_ptr<A> a;
```

------

## Q2：shared_ptr 线程安全吗？

### ✔ 引用计数是线程安全的

- 增减计数是原子操作

### ❌ 对象本身不是线程安全

```
*p = 10; // 需要额外同步
```

------

## Q3：shared_ptr vs unique_ptr

| 特性     | shared_ptr       | unique_ptr |
| -------- | ---------------- | ---------- |
| 所有权   | 共享             | 独占       |
| 拷贝     | 可以             | 不可以     |
| 性能     | 较低（原子计数） | 高         |
| 使用场景 | 共享资源         | 单一拥有   |

------

# 6. 底层结构（面试重点）

```
shared_ptr<T>
 ├── T* ptr
 └── control block
       ├── use_count
       ├── weak_count
       ├── deleter
       └── allocator
```

------

# 7. 适用场景

✔ 对象生命周期不明确
 ✔ 多模块共享
 ✔ 缓存系统
 ✔ GUI 对象管理（部分场景）

------

# 8. 一句话总结（面试用）

> `std::shared_ptr` 是基于引用计数的智能指针，通过共享所有权自动管理堆内存，当最后一个引用释放时自动销毁对象，但需要注意循环引用问题，通常配合 weak_ptr 使用。