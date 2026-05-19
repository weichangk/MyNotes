`weak_ptr` 是 C++11 引入的**弱引用智能指针**，用于配合 `shared_ptr` 使用，**解决循环引用问题**。

------

# 一、为什么需要 weak_ptr？

先看问题：

```
class B;

class A {
public:
    std::shared_ptr<B> bptr;
    ~A() { std::cout << "A destroyed\n"; }
};

class B {
public:
    std::shared_ptr<A> aptr;
    ~B() { std::cout << "B destroyed\n"; }
};
int main() {
    auto a = std::make_shared<A>();
    auto b = std::make_shared<B>();

    a->bptr = b;
    b->aptr = a;
}
```

------

## 问题：循环引用（Circular Reference）

内存关系：

```
a ---> B
^      |
|      v
A <--- b
```

- `a` 持有 `B`
- `b` 持有 `A`
- 两边引用计数都不为 0
- 离开作用域后：
  - `a` 和 `b` 本身销毁
  - 但对象内部还有引用
  - 导致析构函数永远不执行

结果：

```
内存泄漏
```

------

# 二、weak_ptr 的作用

`weak_ptr`：

- 不增加引用计数
- 不拥有对象
- 只是观察对象（observer）

改造：

```
class B;

class A {
public:
    std::shared_ptr<B> bptr;
};

class B {
public:
    std::weak_ptr<A> aptr;
};
```

现在：

```
shared_ptr: 强引用
weak_ptr  : 弱引用
```

弱引用不会让计数 +1。

------

# 三、weak_ptr 的核心特点

| 特性                     | 说明             |
| ------------------------ | ---------------- |
| 不管理对象生命周期       | 不负责释放       |
| 不增加引用计数           | 不影响对象销毁   |
| 可检测对象是否存在       | 防止悬空         |
| 必须配合 shared_ptr 使用 | 不能单独管理对象 |

------

# 四、weak_ptr 常用操作

------

## 1. 创建 weak_ptr

```
std::shared_ptr<int> sp = std::make_shared<int>(100);

std::weak_ptr<int> wp = sp;
```

------

## 2. use_count()

查看强引用数量：

```
std::cout << wp.use_count();
```

注意：

```
只统计 shared_ptr
不统计 weak_ptr
```

------

## 3. expired()

判断对象是否已释放：

```
if (wp.expired()) {
    std::cout << "对象已销毁";
}
```

------

## 4. lock()（最重要）

把 weak_ptr 临时提升为 shared_ptr：

```
if (auto sp = wp.lock()) {
    std::cout << *sp;
}
```

如果对象已释放：

```
wp.lock() == nullptr
```

这是线程安全的。

------

# 五、为什么不能直接使用 weak_ptr？

因为：

```
weak_ptr 不拥有对象
```

所以不能直接：

```
*wp;        // 错
wp->xxx;    // 错
```

必须：

```
auto sp = wp.lock();
```

因为对象可能已经被释放。

------

# 六、典型使用场景

------

## 1. 解决循环引用（最经典）

```
class Parent;
class Child;

class Parent {
public:
    std::shared_ptr<Child> child;
};

class Child {
public:
    std::weak_ptr<Parent> parent;
};
```

这是面试高频。

------

## 2. 观察者模式

GUI、Qt、事件系统常见：

```
Observer 不应该决定 Subject 生命周期
```

因此：

```
weak_ptr
```

适合观察对象。

------

## 3. 缓存系统

缓存里不希望对象永久存活：

```
std::weak_ptr<Texture>
```

对象没人用时自动释放。

------

# 七、shared_ptr 与 weak_ptr 关系

控制块（control block）：

```
shared_ptr:
    强引用计数 strong_count

weak_ptr:
    弱引用计数 weak_count
```

对象真正释放条件：

```
strong_count == 0
```

控制块真正释放条件：

```
strong_count == 0
&&
weak_count == 0
```

因此：

```
对象可能已经销毁
但 weak_ptr 还能知道对象曾经存在
```

------

# 八、面试高频问题

------

## 问：weak_ptr 如何解决循环引用？

答：

```
循环引用本质是 shared_ptr 相互持有，
导致引用计数永远不为 0。

weak_ptr 不增加引用计数，
因此不会阻止对象释放。
```

------

## 问：weak_ptr 为什么不能直接访问对象？

答：

```
因为 weak_ptr 不拥有对象，
对象可能已经销毁。

因此必须 lock() 获取 shared_ptr，
确认对象仍然存在。
```

------

## 问：lock() 为什么线程安全？

答：

```
lock() 会原子地检查对象是否存在，
并安全增加引用计数。
```

避免：

```
检查后对象被其他线程释放
```

的问题。

------

# 九、weak_ptr vs shared_ptr

| 对比           | shared_ptr | weak_ptr |
| -------------- | ---------- | -------- |
| 拥有对象       | 是         | 否       |
| 增加引用计数   | 是         | 否       |
| 控制生命周期   | 是         | 否       |
| 可直接访问对象 | 是         | 否       |
| 解决循环引用   | 否         | 是       |

------

# 十、推荐记忆（面试版）

一句话：

```
weak_ptr 是 shared_ptr 的弱引用，
不增加引用计数，
用于解决循环引用和安全观察对象。
```

核心 API：

```
expired()
lock()
use_count()
```

核心思想：

```
只观察，不拥有。
```