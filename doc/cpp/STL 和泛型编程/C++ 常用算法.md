# C++ 常用算法

> STL `<algorithm>`、`<numeric>`、`<memory>` 头文件中提供了大量通用算法，它们通过迭代器操作容器，是泛型编程的核心体现。

---

## 目录

- [1. 算法概述](#1-算法概述)
- [2. 非修改序列算法](#2-非修改序列算法)
- [3. 修改序列算法](#3-修改序列算法)
- [4. 排序与相关算法](#4-排序与相关算法)
- [5. 二分查找算法](#5-二分查找算法)
- [6. 集合操作算法](#6-集合操作算法)
- [7. 堆操作算法](#7-堆操作算法)
- [8. 最小/最大算法](#8-最小最大算法)
- [9. 数值算法](#9-数值算法)
- [10. 内存未初始化算法](#10-内存未初始化算法)
- [11. C++17 并行算法](#11-c17-并行算法)
- [12. C++20 Ranges 算法](#12-c20-ranges-算法)
- [13. 实战综合案例](#13-实战综合案例)

---

## 1. 算法概述

### 1.1 算法的设计哲学

STL 算法遵循以下核心设计原则：

- **泛型**：算法通过迭代器与容器解耦，同一算法可作用于不同容器。
- **函数对象 / Lambda**：通过可调用对象定制算法行为。
- **不修改容器结构**：算法不会增删容器的元素（不调用容器的成员函数），只通过迭代器读写元素值。
- **左闭右开区间**：所有算法操作的范围为 `[first, last)`。

### 1.2 头文件

| 头文件 | 内容 |
|---|---|
| `<algorithm>` | 大部分通用算法 |
| `<numeric>` | 数值算法（`accumulate`、`iota` 等） |
| `<memory>` | 未初始化内存算法 |
| `<execution>` | C++17 并行执行策略 |
| `<ranges>` | C++20 Ranges 算法 |

### 1.3 可调用对象

算法中频繁使用的谓词（Predicate）和函数对象：

```cpp
#include <algorithm>
#include <vector>
#include <iostream>
#include <functional>

// 1. 普通函数
bool isEven(int n) { return n % 2 == 0; }

// 2. 函数对象（仿函数）
struct IsGreaterThan {
    int threshold;
    bool operator()(int n) const { return n > threshold; }
};

// 3. Lambda 表达式（最常用）
auto isPositive = [](int n) { return n > 0; };

// 4. std::function
std::function<bool(int)> pred = [](int n) { return n < 100; };

int main() {
    std::vector<int> v = {1, -2, 3, -4, 5, 6};

    // 使用普通函数
    auto count1 = std::count_if(v.begin(), v.end(), isEven);

    // 使用函数对象
    auto count2 = std::count_if(v.begin(), v.end(), IsGreaterThan{3});

    // 使用 Lambda
    auto count3 = std::count_if(v.begin(), v.end(), [](int n) { return n > 0; });

    std::cout << "偶数: " << count1 << ", >3: " << count2 << ", 正数: " << count3 << "\n";
    // 输出: 偶数: 3, >3: 2, 正数: 4
}
```

---

## 2. 非修改序列算法

这类算法只读取元素、不修改序列内容。

### 2.1 for_each — 遍历

对范围内每个元素执行操作，返回函数对象（可用于累计状态）。

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};

    // 基本遍历
    std::for_each(v.begin(), v.end(), [](int n) {
        std::cout << n << " ";
    });
    // 输出: 1 2 3 4 5

    std::cout << "\n";

    // 利用返回值获取状态
    struct Sum {
        int total = 0;
        void operator()(int n) { total += n; }
    };

    Sum result = std::for_each(v.begin(), v.end(), Sum{});
    std::cout << "总和: " << result.total << "\n"; // 输出: 总和: 15

    // C++17: for_each_n 处理前 N 个元素
    std::for_each_n(v.begin(), 3, [](int n) {
        std::cout << n << " ";
    });
    // 输出: 1 2 3
}
```

### 2.2 find / find_if / find_if_not — 查找

```cpp
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>

int main() {
    std::vector<int> v = {10, 20, 30, 40, 50};

    // find: 查找值
    auto it = std::find(v.begin(), v.end(), 30);
    if (it != v.end())
        std::cout << "找到: " << *it << "，位置: " << std::distance(v.begin(), it) << "\n";
    // 输出: 找到: 30，位置: 2

    // find_if: 按条件查找
    auto it2 = std::find_if(v.begin(), v.end(), [](int n) { return n > 25; });
    if (it2 != v.end())
        std::cout << "第一个>25: " << *it2 << "\n"; // 输出: 第一个>25: 30

    // find_if_not: 查找第一个不满足条件的
    auto it3 = std::find_if_not(v.begin(), v.end(), [](int n) { return n < 35; });
    if (it3 != v.end())
        std::cout << "第一个>=35: " << *it3 << "\n"; // 输出: 第一个>=35: 40

    // 查找字符串
    std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
    auto it4 = std::find(names.begin(), names.end(), "Bob");
    if (it4 != names.end())
        std::cout << "找到: " << *it4 << "\n"; // 输出: 找到: Bob
}
```

### 2.3 count / count_if — 计数

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {1, 2, 2, 3, 2, 4, 2, 5};

    // count: 统计值出现次数
    auto c1 = std::count(v.begin(), v.end(), 2);
    std::cout << "2 出现了 " << c1 << " 次\n"; // 输出: 2 出现了 4 次

    // count_if: 按条件统计
    auto c2 = std::count_if(v.begin(), v.end(), [](int n) { return n % 2 == 0; });
    std::cout << "偶数个数: " << c2 << "\n"; // 输出: 偶数个数: 5
}
```

### 2.4 all_of / any_of / none_of — 逻辑判断（C++11）

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {2, 4, 6, 8, 10};

    // all_of: 所有元素都满足条件？
    bool allEven = std::all_of(v.begin(), v.end(), [](int n) { return n % 2 == 0; });
    std::cout << "全部偶数: " << std::boolalpha << allEven << "\n"; // true

    // any_of: 存在至少一个元素满足条件？
    bool anyGt5 = std::any_of(v.begin(), v.end(), [](int n) { return n > 5; });
    std::cout << "存在>5: " << anyGt5 << "\n"; // true

    // none_of: 没有元素满足条件？
    bool noneNeg = std::none_of(v.begin(), v.end(), [](int n) { return n < 0; });
    std::cout << "无负数: " << noneNeg << "\n"; // true

    // 注意: 空范围时 all_of 和 none_of 返回 true，any_of 返回 false
    std::vector<int> empty;
    std::cout << "空 all_of: " << std::all_of(empty.begin(), empty.end(), [](int) { return false; }) << "\n"; // true
}
```

### 2.5 equal / mismatch — 序列比较

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<int> b = {1, 2, 3, 4, 5};
    std::vector<int> c = {1, 2, 9, 4, 5};

    // equal: 判断两个序列是否相等
    std::cout << "a == b: " << std::boolalpha << std::equal(a.begin(), a.end(), b.begin()) << "\n"; // true
    std::cout << "a == c: " << std::equal(a.begin(), a.end(), c.begin()) << "\n"; // false

    // 自定义比较（忽略差值<=1的差异）
    std::cout << "近似相等: " << std::equal(a.begin(), a.end(), c.begin(),
        [](int x, int y) { return std::abs(x - y) <= 1; }) << "\n"; // false (3 vs 9 差6)

    // mismatch: 找到第一个不匹配的位置
    auto [it1, it2] = std::mismatch(a.begin(), a.end(), c.begin());
    if (it1 != a.end())
        std::cout << "不匹配位置: " << std::distance(a.begin(), it1)
                  << " (" << *it1 << " vs " << *it2 << ")\n";
    // 输出: 不匹配位置: 2 (3 vs 9)
}
```

### 2.6 search / find_end — 子序列查找

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> haystack = {1, 2, 3, 4, 1, 2, 3, 4, 5};
    std::vector<int> needle = {1, 2, 3};

    // search: 查找子序列第一次出现的位置
    auto it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end());
    if (it != haystack.end())
        std::cout << "首次出现位置: " << std::distance(haystack.begin(), it) << "\n"; // 0

    // find_end: 查找子序列最后一次出现的位置
    auto it2 = std::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
    if (it2 != haystack.end())
        std::cout << "最后出现位置: " << std::distance(haystack.begin(), it2) << "\n"; // 4

    // search_n: 查找连续 n 个相同值
    std::vector<int> v = {1, 1, 2, 2, 2, 3, 3};
    auto it3 = std::search_n(v.begin(), v.end(), 3, 2); // 查找连续3个2
    if (it3 != v.end())
        std::cout << "连续3个2: 位置 " << std::distance(v.begin(), it3) << "\n"; // 2

    // adjacent_find: 查找相邻重复元素
    std::vector<int> v2 = {1, 2, 3, 3, 4, 5};
    auto it4 = std::adjacent_find(v2.begin(), v2.end());
    if (it4 != v2.end())
        std::cout << "相邻重复: " << *it4 << " at " << std::distance(v2.begin(), it4) << "\n"; // 3 at 2
}
```

---

## 3. 修改序列算法

这类算法会修改目标序列中的元素值。

### 3.1 copy / copy_if / copy_n / copy_backward — 复制

```cpp
#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>

int main() {
    std::vector<int> src = {1, 2, 3, 4, 5, 6, 7, 8};

    // copy: 复制到另一个容器
    std::vector<int> dst(src.size());
    std::copy(src.begin(), src.end(), dst.begin());

    // copy_if: 条件复制
    std::vector<int> evens;
    std::copy_if(src.begin(), src.end(), std::back_inserter(evens),
        [](int n) { return n % 2 == 0; });
    // evens: {2, 4, 6, 8}

    // copy_n: 复制前 n 个元素
    std::vector<int> first3;
    std::copy_n(src.begin(), 3, std::back_inserter(first3));
    // first3: {1, 2, 3}

    // copy_backward: 从后向前复制（用于重叠区间右移）
    std::vector<int> v = {1, 2, 3, 4, 5, 0, 0};
    std::copy_backward(v.begin(), v.begin() + 5, v.end());
    // v: {1, 2, 1, 2, 3, 4, 5}

    // 输出到标准输出
    std::copy(evens.begin(), evens.end(), std::ostream_iterator<int>(std::cout, " "));
    // 输出: 2 4 6 8
}
```

### 3.2 move — 移动

```cpp
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>

int main() {
    std::vector<std::string> src = {"Hello", "World", "C++", "STL"};
    std::vector<std::string> dst(src.size());

    // move: 移动元素（转移资源所有权，源变为有效但未指定状态）
    std::move(src.begin(), src.end(), dst.begin());

    std::cout << "dst: ";
    for (const auto& s : dst) std::cout << s << " ";
    std::cout << "\n"; // dst: Hello World C++ STL

    std::cout << "src (moved-from): ";
    for (const auto& s : src) std::cout << "[" << s << "] ";
    std::cout << "\n"; // src 中的字符串可能为空（取决于实现）
}
```

### 3.3 transform — 变换

`transform` 是最常用的算法之一，它将函数应用于每个元素并将结果写入目标。

```cpp
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <cctype>

int main() {
    // 一元 transform: 对每个元素应用变换
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::vector<int> squares(v.size());
    std::transform(v.begin(), v.end(), squares.begin(),
        [](int n) { return n * n; });
    // squares: {1, 4, 9, 16, 25}

    // 就地变换
    std::transform(v.begin(), v.end(), v.begin(),
        [](int n) { return n * 2; });
    // v: {2, 4, 6, 8, 10}

    // 二元 transform: 对两个序列的对应元素应用变换
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {10, 20, 30};
    std::vector<int> sums(3);
    std::transform(a.begin(), a.end(), b.begin(), sums.begin(),
        [](int x, int y) { return x + y; });
    // sums: {11, 22, 33}

    // 字符串大写转换
    std::string s = "hello world";
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::toupper(c); });
    std::cout << s << "\n"; // HELLO WORLD

    // 输出
    for (int x : squares) std::cout << x << " ";
    std::cout << "\n"; // 1 4 9 16 25
}
```

### 3.4 fill / fill_n / generate / generate_n — 填充与生成

```cpp
#include <algorithm>
#include <vector>
#include <iostream>
#include <random>

int main() {
    // fill: 用指定值填充
    std::vector<int> v(5);
    std::fill(v.begin(), v.end(), 42);
    // v: {42, 42, 42, 42, 42}

    // fill_n: 填充前 n 个元素
    std::fill_n(v.begin(), 3, 0);
    // v: {0, 0, 0, 42, 42}

    // generate: 用生成器函数填充
    int counter = 0;
    std::generate(v.begin(), v.end(), [&counter]() { return ++counter; });
    // v: {1, 2, 3, 4, 5}

    // generate_n: 生成前 n 个元素
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(1, 100);
    std::vector<int> randoms(10);
    std::generate_n(randoms.begin(), 10, [&]() { return dist(rng); });

    for (int x : randoms) std::cout << x << " ";
    std::cout << "\n";
}
```

### 3.5 replace / replace_if / replace_copy — 替换

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {1, 2, 3, 2, 5, 2, 7};

    // replace: 替换所有等于指定值的元素
    std::replace(v.begin(), v.end(), 2, 99);
    // v: {1, 99, 3, 99, 5, 99, 7}

    // replace_if: 按条件替换
    std::vector<int> v2 = {1, -2, 3, -4, 5};
    std::replace_if(v2.begin(), v2.end(), [](int n) { return n < 0; }, 0);
    // v2: {1, 0, 3, 0, 5}

    // replace_copy: 替换并复制到新容器（不修改原容器）
    std::vector<int> v3 = {1, 2, 3, 2, 5};
    std::vector<int> result(v3.size());
    std::replace_copy(v3.begin(), v3.end(), result.begin(), 2, 0);
    // v3 不变: {1, 2, 3, 2, 5}
    // result: {1, 0, 3, 0, 5}

    for (int x : result) std::cout << x << " ";
    std::cout << "\n";
}
```

### 3.6 remove / remove_if / erase-remove 惯用法 — 删除

**重要**：`remove` / `remove_if` 不会真正删除元素，它们只是将不需要删除的元素前移，并返回新的逻辑结尾迭代器。需要配合容器的 `erase` 才能真正删除。

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    // remove 的工作原理
    std::vector<int> v = {1, 2, 3, 2, 5, 2, 7};
    auto newEnd = std::remove(v.begin(), v.end(), 2);
    // 此时 v 的前部分为 {1, 3, 5, 7, ...}，newEnd 指向逻辑结尾
    // 尾部元素处于未指定状态
    std::cout << "逻辑大小: " << std::distance(v.begin(), newEnd) << "\n"; // 4
    std::cout << "物理大小: " << v.size() << "\n"; // 7（未变）

    // erase-remove 惯用法: 真正删除
    v.erase(newEnd, v.end());
    // v: {1, 3, 5, 7}
    std::cout << "删除后大小: " << v.size() << "\n"; // 4

    // 一行写法
    std::vector<int> v2 = {1, -2, 3, -4, 5, -6};
    v2.erase(std::remove_if(v2.begin(), v2.end(), [](int n) { return n < 0; }), v2.end());
    // v2: {1, 3, 5}

    // C++20: std::erase / std::erase_if 直接操作容器（推荐）
    // std::erase(v2, 3);           // 删除值为3的元素
    // std::erase_if(v2, pred);     // 按条件删除

    for (int x : v2) std::cout << x << " ";
    std::cout << "\n"; // 1 3 5
}
```

### 3.7 unique — 去重

`unique` 移除**相邻**重复元素，通常需要先排序。

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};

    // 先排序
    std::sort(v.begin(), v.end());
    // v: {1, 1, 2, 3, 3, 4, 5, 5, 6, 9}

    // unique + erase
    v.erase(std::unique(v.begin(), v.end()), v.end());
    // v: {1, 2, 3, 4, 5, 6, 9}

    for (int x : v) std::cout << x << " ";
    std::cout << "\n";

    // 自定义比较：去除差值 <= 1 的相邻元素
    std::vector<int> v2 = {1, 2, 3, 10, 11, 12, 20};
    v2.erase(std::unique(v2.begin(), v2.end(),
        [](int a, int b) { return std::abs(a - b) <= 1; }), v2.end());
    // v2: {1, 3, 10, 12, 20}

    for (int x : v2) std::cout << x << " ";
    std::cout << "\n";
}
```

### 3.8 reverse / rotate / shuffle — 重排

```cpp
#include <algorithm>
#include <vector>
#include <iostream>
#include <random>

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};

    // reverse: 反转
    std::reverse(v.begin(), v.end());
    // v: {5, 4, 3, 2, 1}

    // rotate: 旋转，使 middle 成为新的首元素
    std::vector<int> v2 = {1, 2, 3, 4, 5};
    std::rotate(v2.begin(), v2.begin() + 2, v2.end());
    // v2: {3, 4, 5, 1, 2}（左旋 2 位）

    // rotate 实现右旋
    std::vector<int> v3 = {1, 2, 3, 4, 5};
    std::rotate(v3.begin(), v3.end() - 2, v3.end());
    // v3: {4, 5, 1, 2, 3}（右旋 2 位）

    // shuffle: 随机打乱
    std::vector<int> v4 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::mt19937 rng(42);
    std::shuffle(v4.begin(), v4.end(), rng);
    for (int x : v4) std::cout << x << " ";
    std::cout << "\n";
}
```

### 3.9 swap_ranges / iter_swap — 交换

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<int> b = {10, 20, 30, 40, 50};

    // swap_ranges: 交换两个范围内的元素
    std::swap_ranges(a.begin(), a.begin() + 3, b.begin());
    // a: {10, 20, 30, 4, 5}
    // b: {1, 2, 3, 40, 50}

    // iter_swap: 交换两个迭代器指向的元素
    std::iter_swap(a.begin(), a.begin() + 4);
    // a[0] 和 a[4] 交换

    for (int x : a) std::cout << x << " ";
    std::cout << "\n";
}
```

---

## 4. 排序与相关算法

### 4.1 sort — 排序

`std::sort` 使用 IntroSort（快排 + 堆排 + 插入排序混合），平均时间复杂度 O(n log n)，**不保证稳定性**。

```cpp
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>

int main() {
    std::vector<int> v = {5, 2, 8, 1, 9, 3};

    // 升序排序（默认）
    std::sort(v.begin(), v.end());
    // v: {1, 2, 3, 5, 8, 9}

    // 降序排序
    std::sort(v.begin(), v.end(), std::greater<int>());
    // v: {9, 8, 5, 3, 2, 1}

    // 自定义排序：按绝对值升序
    std::vector<int> v2 = {-5, 3, -1, 4, -2};
    std::sort(v2.begin(), v2.end(), [](int a, int b) {
        return std::abs(a) < std::abs(b);
    });
    // v2: {-1, -2, 3, 4, -5}

    // 对结构体排序
    struct Student {
        std::string name;
        int score;
    };
    std::vector<Student> students = {{"Alice", 85}, {"Bob", 92}, {"Charlie", 78}, {"Diana", 92}};
    std::sort(students.begin(), students.end(), [](const Student& a, const Student& b) {
        return a.score > b.score; // 按分数降序
    });
    for (const auto& s : students)
        std::cout << s.name << ": " << s.score << "\n";
}
```

### 4.2 stable_sort — 稳定排序

保证相等元素的相对顺序不变，时间复杂度 O(n log²n)。

```cpp
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>

int main() {
    struct Student {
        std::string name;
        int score;
    };

    std::vector<Student> students = {
        {"Alice", 85}, {"Bob", 92}, {"Charlie", 78},
        {"Diana", 92}, {"Eve", 85}
    };

    // stable_sort 保证同分数的学生保持原始顺序
    std::stable_sort(students.begin(), students.end(),
        [](const Student& a, const Student& b) {
            return a.score > b.score;
        });

    for (const auto& s : students)
        std::cout << s.name << ": " << s.score << "\n";
    // Bob: 92
    // Diana: 92（Bob 和 Diana 同分，顺序与原始一致）
    // Alice: 85
    // Eve: 85（Alice 和 Eve 同分，顺序与原始一致）
    // Charlie: 78
}
```

### 4.3 partial_sort — 部分排序

只排序前 N 个最小/最大元素，时间复杂度 O(n log k)。适用于 Top-K 场景。

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {5, 7, 4, 2, 8, 6, 1, 9, 0, 3};

    // 只排序前 3 个最小元素
    std::partial_sort(v.begin(), v.begin() + 3, v.end());
    // v 的前3个元素为 {0, 1, 2}，其余元素顺序不确定
    std::cout << "Top 3 最小: ";
    for (int i = 0; i < 3; ++i) std::cout << v[i] << " ";
    std::cout << "\n"; // 0 1 2

    // partial_sort_copy: 不修改原容器，复制前 K 个最小到新容器
    std::vector<int> v2 = {5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
    std::vector<int> top3(3);
    std::partial_sort_copy(v2.begin(), v2.end(), top3.begin(), top3.end());
    // top3: {0, 1, 2}, v2 不变
}
```

### 4.4 nth_element — 第 N 小元素

将第 N 小的元素放到正确位置，其左侧都 ≤ 它，右侧都 ≥ 它。时间复杂度 O(n)。

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {5, 7, 4, 2, 8, 6, 1, 9, 0, 3};

    // 找中位数
    size_t mid = v.size() / 2;
    std::nth_element(v.begin(), v.begin() + mid, v.end());
    std::cout << "中位数: " << v[mid] << "\n"; // 输出: 中位数: 5（第5小的元素在索引4）

    // 找第 3 小的元素
    std::vector<int> v2 = {5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
    std::nth_element(v2.begin(), v2.begin() + 2, v2.end());
    std::cout << "第3小: " << v2[2] << "\n"; // 2

    // n th_element + partial_sort 实现高效 Top-K
    // nth_element 找到分界点，然后只对前 K 个排序
    std::vector<int> v3 = {5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
    std::nth_element(v3.begin(), v3.begin() + 3, v3.end());
    std::sort(v3.begin(), v3.begin() + 3); // 只对前3个排序
    std::cout << "Top 3: ";
    for (int i = 0; i < 3; ++i) std::cout << v3[i] << " ";
    std::cout << "\n"; // 0 1 2
}
```

### 4.5 is_sorted / is_sorted_until — 排序检查

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v1 = {1, 2, 3, 4, 5};
    std::vector<int> v2 = {1, 3, 2, 4, 5};

    std::cout << "v1 已排序: " << std::boolalpha << std::is_sorted(v1.begin(), v1.end()) << "\n"; // true
    std::cout << "v2 已排序: " << std::is_sorted(v2.begin(), v2.end()) << "\n"; // false

    // is_sorted_until: 找到第一个破坏排序的位置
    auto it = std::is_sorted_until(v2.begin(), v2.end());
    std::cout << "排序到位置: " << std::distance(v2.begin(), it) << "\n"; // 2（v2[2]=2 破坏了排序）
}
```

### 4.6 partition / stable_partition — 分区

将满足条件的元素移到前半部分，不满足的移到后半部分。

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // partition: 偶数在前，奇数在后（不保证相对顺序）
    auto pivotIt = std::partition(v.begin(), v.end(), [](int n) { return n % 2 == 0; });
    std::cout << "偶数部分: ";
    for (auto it = v.begin(); it != pivotIt; ++it) std::cout << *it << " ";
    std::cout << "\n奇数部分: ";
    for (auto it = pivotIt; it != v.end(); ++it) std::cout << *it << " ";
    std::cout << "\n";

    // stable_partition: 保持相对顺序
    std::vector<int> v2 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto pivot2 = std::stable_partition(v2.begin(), v2.end(), [](int n) { return n % 2 == 0; });
    // 偶数部分: 2 4 6 8 10（顺序保持）
    // 奇数部分: 1 3 5 7 9（顺序保持）

    std::cout << "stable 偶数: ";
    for (auto it = v2.begin(); it != pivot2; ++it) std::cout << *it << " ";
    std::cout << "\nstable 奇数: ";
    for (auto it = pivot2; it != v2.end(); ++it) std::cout << *it << " ";
    std::cout << "\n";

    // partition_point: 在已分区序列中找到分界点
    auto pp = std::partition_point(v2.begin(), v2.end(), [](int n) { return n % 2 == 0; });
    std::cout << "分界元素: " << *pp << "\n";

    // is_partitioned: 检查是否已分区
    std::cout << "已分区: " << std::boolalpha
              << std::is_partitioned(v2.begin(), v2.end(), [](int n) { return n % 2 == 0; }) << "\n"; // true
}
```

---

## 5. 二分查找算法

**前提**：序列必须是已排序的（或至少对查找的元素满足分区条件）。

### 5.1 lower_bound / upper_bound

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {1, 2, 4, 4, 4, 6, 7, 9};

    // lower_bound: 返回第一个 >= value 的迭代器
    auto lb = std::lower_bound(v.begin(), v.end(), 4);
    std::cout << "lower_bound(4): 位置 " << std::distance(v.begin(), lb) << "\n"; // 2

    // upper_bound: 返回第一个 > value 的迭代器
    auto ub = std::upper_bound(v.begin(), v.end(), 4);
    std::cout << "upper_bound(4): 位置 " << std::distance(v.begin(), ub) << "\n"; // 5

    // 值 4 出现的次数
    std::cout << "4 出现次数: " << std::distance(lb, ub) << "\n"; // 3

    // 查找不存在的值
    auto lb2 = std::lower_bound(v.begin(), v.end(), 5);
    std::cout << "lower_bound(5): 位置 " << std::distance(v.begin(), lb2)
              << ", 值: " << *lb2 << "\n"; // 位置 5, 值: 6

    // 用于有序插入
    std::vector<int> sorted = {1, 3, 5, 7, 9};
    auto pos = std::lower_bound(sorted.begin(), sorted.end(), 6);
    sorted.insert(pos, 6);
    // sorted: {1, 3, 5, 6, 7, 9}
}
```

### 5.2 equal_range — 等值范围

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {1, 2, 4, 4, 4, 6, 7, 9};

    // equal_range: 同时返回 lower_bound 和 upper_bound
    auto [lo, hi] = std::equal_range(v.begin(), v.end(), 4);
    std::cout << "4 的范围: [" << std::distance(v.begin(), lo)
              << ", " << std::distance(v.begin(), hi) << ")\n"; // [2, 5)
    std::cout << "4 出现次数: " << std::distance(lo, hi) << "\n"; // 3

    // 查找不存在的值
    auto [lo2, hi2] = std::equal_range(v.begin(), v.end(), 5);
    std::cout << "5 的范围: [" << std::distance(v.begin(), lo2)
              << ", " << std::distance(v.begin(), hi2) << ")\n"; // [5, 5)（空范围）
}
```

### 5.3 binary_search — 二分查找

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {1, 2, 4, 4, 4, 6, 7, 9};

    // binary_search: 只返回 bool，判断值是否存在
    std::cout << "存在4: " << std::boolalpha << std::binary_search(v.begin(), v.end(), 4) << "\n"; // true
    std::cout << "存在5: " << std::binary_search(v.begin(), v.end(), 5) << "\n"; // false

    // 注意：binary_search 不返回位置，如需位置请用 lower_bound
    // 自定义比较
    struct Person {
        std::string name;
        int age;
    };
    std::vector<Person> people = {{"Alice", 20}, {"Bob", 25}, {"Charlie", 30}};
    bool found = std::binary_search(people.begin(), people.end(), Person{"", 25},
        [](const Person& a, const Person& b) { return a.age < b.age; });
    std::cout << "存在 age=25: " << found << "\n"; // true
}
```

---

## 6. 集合操作算法

**前提**：两个输入序列都必须是已排序的。结果也是排序的。

```cpp
#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>

int main() {
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<int> b = {3, 4, 5, 6, 7};

    // set_union: 并集
    std::vector<int> unionResult;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(),
        std::back_inserter(unionResult));
    std::cout << "并集: ";
    for (int x : unionResult) std::cout << x << " ";
    std::cout << "\n"; // 1 2 3 4 5 6 7

    // set_intersection: 交集
    std::vector<int> interResult;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
        std::back_inserter(interResult));
    std::cout << "交集: ";
    for (int x : interResult) std::cout << x << " ";
    std::cout << "\n"; // 3 4 5

    // set_difference: 差集（在 a 中但不在 b 中）
    std::vector<int> diffResult;
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(),
        std::back_inserter(diffResult));
    std::cout << "差集(a-b): ";
    for (int x : diffResult) std::cout << x << " ";
    std::cout << "\n"; // 1 2

    // set_symmetric_difference: 对称差集（在 a 或 b 中但不同时在两者中）
    std::vector<int> symDiffResult;
    std::set_symmetric_difference(a.begin(), a.end(), b.begin(), b.end(),
        std::back_inserter(symDiffResult));
    std::cout << "对称差集: ";
    for (int x : symDiffResult) std::cout << x << " ";
    std::cout << "\n"; // 1 2 6 7

    // includes: 检查 a 是否包含 b 的所有元素
    std::vector<int> sub = {3, 4};
    std::cout << "a 包含 {3,4}: " << std::boolalpha
              << std::includes(a.begin(), a.end(), sub.begin(), sub.end()) << "\n"; // true

    // merge: 合并两个有序序列
    std::vector<int> merged;
    std::merge(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(merged));
    std::cout << "合并: ";
    for (int x : merged) std::cout << x << " ";
    std::cout << "\n"; // 1 2 3 3 4 4 5 5 6 7

    // inplace_merge: 就地合并同一序列中的两个有序子序列
    std::vector<int> v = {1, 3, 5, 2, 4, 6}; // 前3个和后3个分别有序
    std::inplace_merge(v.begin(), v.begin() + 3, v.end());
    // v: {1, 2, 3, 4, 5, 6}
}
```

---

## 7. 堆操作算法

堆（Heap）是一种特殊的完全二叉树结构，STL 提供了基于数组/vector 的最大堆操作。

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};

    // make_heap: 将序列构建为最大堆
    std::make_heap(v.begin(), v.end());
    std::cout << "堆顶(最大): " << v.front() << "\n"; // 9

    // push_heap: 在堆尾添加元素后，恢复堆性质
    v.push_back(10);
    std::push_heap(v.begin(), v.end());
    std::cout << "新堆顶: " << v.front() << "\n"; // 10

    // pop_heap: 将堆顶移到末尾，然后缩小堆范围
    std::pop_heap(v.begin(), v.end());
    int top = v.back();
    v.pop_back();
    std::cout << "弹出: " << top << "\n"; // 10
    std::cout << "新堆顶: " << v.front() << "\n"; // 9

    // sort_heap: 对堆排序（升序），排序后不再是堆
    std::sort_heap(v.begin(), v.end());
    for (int x : v) std::cout << x << " ";
    std::cout << "\n"; // 1 1 2 3 4 5 6 9

    // is_heap: 检查是否为堆
    std::cout << "是堆: " << std::boolalpha << std::is_heap(v.begin(), v.end()) << "\n"; // false

    // 最小堆：使用 greater
    std::vector<int> minHeap = {3, 1, 4, 1, 5};
    std::make_heap(minHeap.begin(), minHeap.end(), std::greater<int>());
    std::cout << "最小堆顶: " << minHeap.front() << "\n"; // 1

    // 实际应用：优先队列可以直接使用 std::priority_queue
    // 但手动堆操作在某些场景更灵活
}
```

---

## 8. 最小/最大算法

### 8.1 min / max / minmax / clamp

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    // min / max: 两个值
    std::cout << "min(3, 5): " << std::min(3, 5) << "\n"; // 3
    std::cout << "max(3, 5): " << std::max(3, 5) << "\n"; // 5

    // 初始化列表版本
    std::cout << "min{3,1,4,1,5}: " << std::min({3, 1, 4, 1, 5}) << "\n"; // 1
    std::cout << "max{3,1,4,1,5}: " << std::max({3, 1, 4, 1, 5}) << "\n"; // 5

    // minmax: 同时获取最小和最大
    auto [lo, hi] = std::minmax({3, 1, 4, 1, 5, 9, 2, 6});
    std::cout << "min=" << lo << ", max=" << hi << "\n"; // min=1, max=9

    // clamp (C++17): 限定值在范围内
    std::cout << "clamp(15, 0, 10): " << std::clamp(15, 0, 10) << "\n"; // 10
    std::cout << "clamp(-5, 0, 10): " << std::clamp(-5, 0, 10) << "\n"; // 0
    std::cout << "clamp(5, 0, 10): " << std::clamp(5, 0, 10) << "\n";   // 5
}
```

### 8.2 min_element / max_element / minmax_element

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {5, 2, 8, 1, 9, 3};

    // min_element: 返回最小元素的迭代器
    auto minIt = std::min_element(v.begin(), v.end());
    std::cout << "最小值: " << *minIt << " at " << std::distance(v.begin(), minIt) << "\n"; // 1 at 3

    // max_element: 返回最大元素的迭代器
    auto maxIt = std::max_element(v.begin(), v.end());
    std::cout << "最大值: " << *maxIt << " at " << std::distance(v.begin(), maxIt) << "\n"; // 9 at 4

    // minmax_element: 同时获取
    auto [minI, maxI] = std::minmax_element(v.begin(), v.end());
    std::cout << "min=" << *minI << ", max=" << *maxI << "\n"; // min=1, max=9

    // 自定义比较：按字符串长度
    std::vector<std::string> words = {"apple", "hi", "banana", "cat"};
    auto longest = std::max_element(words.begin(), words.end(),
        [](const std::string& a, const std::string& b) { return a.size() < b.size(); });
    std::cout << "最长单词: " << *longest << "\n"; // banana
}
```

---

## 9. 数值算法

需要包含 `<numeric>` 头文件。

### 9.1 accumulate — 累加

```cpp
#include <numeric>
#include <vector>
#include <string>
#include <iostream>

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};

    // 累加（默认加法）
    int sum = std::accumulate(v.begin(), v.end(), 0);
    std::cout << "总和: " << sum << "\n"; // 15

    // 注意初始值的类型决定了返回类型
    // 错误：整数溢出或截断
    std::vector<double> d = {1.5, 2.5, 3.5};
    // int wrong = std::accumulate(d.begin(), d.end(), 0);   // 结果为 6（截断！）
    double correct = std::accumulate(d.begin(), d.end(), 0.0); // 结果为 7.5
    std::cout << "double和: " << correct << "\n";

    // 自定义操作：累乘
    int product = std::accumulate(v.begin(), v.end(), 1, std::multiplies<int>());
    std::cout << "乘积: " << product << "\n"; // 120

    // 字符串拼接
    std::vector<std::string> words = {"Hello", " ", "World", "!"};
    std::string sentence = std::accumulate(words.begin(), words.end(), std::string{});
    std::cout << sentence << "\n"; // Hello World!

    // 自定义操作：统计
    struct Stats {
        int count = 0;
        double sum = 0.0;
    };
    std::vector<double> scores = {85.0, 92.0, 78.0, 95.0, 88.0};
    Stats stats = std::accumulate(scores.begin(), scores.end(), Stats{},
        [](Stats s, double score) {
            return Stats{s.count + 1, s.sum + score};
        });
    std::cout << "平均分: " << stats.sum / stats.count << "\n"; // 87.6
}
```

### 9.2 reduce（C++17）— 并行友好的累加

```cpp
#include <numeric>
#include <vector>
#include <iostream>
#include <execution>

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};

    // reduce: 类似 accumulate，但操作的顺序不确定（允许并行）
    int sum = std::reduce(v.begin(), v.end(), 0);
    std::cout << "reduce 总和: " << sum << "\n"; // 15

    // 无初始值版本（使用值类型的默认构造）
    int sum2 = std::reduce(v.begin(), v.end());
    std::cout << "reduce 总和2: " << sum2 << "\n"; // 15

    // 并行版本（需要链接 TBB 等并行库）
    // int parSum = std::reduce(std::execution::par, v.begin(), v.end(), 0);

    // 注意: reduce 要求操作满足交换律和结合律
    // 对于浮点数加法，结果可能与 accumulate 稍有不同（浮点精度）
}
```

### 9.3 inner_product — 内积

```cpp
#include <numeric>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};

    // 内积: sum(a[i] * b[i]) = 1*4 + 2*5 + 3*6 = 32
    int dot = std::inner_product(a.begin(), a.end(), b.begin(), 0);
    std::cout << "内积: " << dot << "\n"; // 32

    // 自定义操作: 计算曼哈顿距离
    std::vector<int> p1 = {1, 2, 3};
    std::vector<int> p2 = {4, 6, 8};
    int manhattan = std::inner_product(p1.begin(), p1.end(), p2.begin(), 0,
        std::plus<int>(),                                    // 外层操作: sum
        [](int a, int b) { return std::abs(a - b); });      // 内层操作: |a-b|
    std::cout << "曼哈顿距离: " << manhattan << "\n"; // |1-4|+|2-6|+|3-8| = 3+4+5 = 12
}
```

### 9.4 partial_sum / inclusive_scan / exclusive_scan — 前缀和

```cpp
#include <numeric>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};

    // partial_sum: 前缀和
    std::vector<int> prefix(v.size());
    std::partial_sum(v.begin(), v.end(), prefix.begin());
    // prefix: {1, 3, 6, 10, 15}
    std::cout << "前缀和: ";
    for (int x : prefix) std::cout << x << " ";
    std::cout << "\n";

    // 前缀积
    std::vector<int> prefixProd(v.size());
    std::partial_sum(v.begin(), v.end(), prefixProd.begin(), std::multiplies<int>());
    // prefixProd: {1, 2, 6, 24, 120}

    // C++17: inclusive_scan（类似 partial_sum，支持并行）
    std::vector<int> incScan(v.size());
    std::inclusive_scan(v.begin(), v.end(), incScan.begin());
    // incScan: {1, 3, 6, 10, 15}

    // C++17: exclusive_scan（不包含当前元素）
    std::vector<int> excScan(v.size());
    std::exclusive_scan(v.begin(), v.end(), excScan.begin(), 0);
    // excScan: {0, 1, 3, 6, 10}
    std::cout << "exclusive_scan: ";
    for (int x : excScan) std::cout << x << " ";
    std::cout << "\n";

    // 实际应用: 用前缀和快速求区间和
    // sum(v[l..r]) = prefix[r] - (l > 0 ? prefix[l-1] : 0)
    int l = 1, r = 3; // v[1]+v[2]+v[3] = 2+3+4 = 9
    int rangeSum = prefix[r] - (l > 0 ? prefix[l - 1] : 0);
    std::cout << "v[1..3] 区间和: " << rangeSum << "\n"; // 9
}
```

### 9.5 adjacent_difference — 相邻差

```cpp
#include <numeric>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {1, 3, 6, 10, 15};

    // adjacent_difference: 相邻元素差
    std::vector<int> diff(v.size());
    std::adjacent_difference(v.begin(), v.end(), diff.begin());
    // diff: {1, 2, 3, 4, 5}（第一个元素保持不变）
    std::cout << "相邻差: ";
    for (int x : diff) std::cout << x << " ";
    std::cout << "\n";

    // 自定义操作: 相邻比率
    std::vector<double> prices = {100.0, 105.0, 102.0, 110.0, 108.0};
    std::vector<double> changes(prices.size());
    std::adjacent_difference(prices.begin(), prices.end(), changes.begin(),
        [](double curr, double prev) { return (curr - prev) / prev * 100.0; });
    std::cout << "价格变化(%): ";
    for (size_t i = 1; i < changes.size(); ++i)
        std::cout << changes[i] << "% ";
    std::cout << "\n"; // 5% -2.85714% 7.84314% -1.81818%
}
```

### 9.6 iota — 连续递增填充

```cpp
#include <numeric>
#include <vector>
#include <iostream>

int main() {
    // iota: 用连续递增值填充
    std::vector<int> v(10);
    std::iota(v.begin(), v.end(), 1);
    // v: {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}

    // 常用于创建索引数组
    std::vector<int> indices(10);
    std::iota(indices.begin(), indices.end(), 0);
    // indices: {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}

    // 间接排序（按另一个数组的值排序索引）
    std::vector<int> data = {50, 30, 80, 10, 60};
    std::vector<int> idx(data.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&data](int a, int b) {
        return data[a] < data[b];
    });
    std::cout << "排序后索引: ";
    for (int i : idx) std::cout << i << "(" << data[i] << ") ";
    std::cout << "\n"; // 3(10) 1(30) 0(50) 4(60) 2(80)
}
```

### 9.7 transform_reduce（C++17）— 变换 + 归约

```cpp
#include <numeric>
#include <vector>
#include <iostream>
#include <cmath>

int main() {
    // transform_reduce: 先对每个元素 transform，再 reduce
    std::vector<int> v = {1, 2, 3, 4, 5};

    // 计算平方和
    int sqSum = std::transform_reduce(v.begin(), v.end(), 0,
        std::plus<int>(),                         // 归约操作
        [](int n) { return n * n; });             // 变换操作
    std::cout << "平方和: " << sqSum << "\n"; // 1+4+9+16+25 = 55

    // 二元版本: 向量点积
    std::vector<double> a = {1.0, 2.0, 3.0};
    std::vector<double> b = {4.0, 5.0, 6.0};
    double dot = std::transform_reduce(a.begin(), a.end(), b.begin(), 0.0);
    std::cout << "点积: " << dot << "\n"; // 32

    // 计算欧几里得距离
    double dist = std::sqrt(std::transform_reduce(a.begin(), a.end(), b.begin(), 0.0,
        std::plus<double>(),
        [](double x, double y) { return (x - y) * (x - y); }));
    std::cout << "欧氏距离: " << dist << "\n"; // sqrt(9+9+9) = 5.19615

    // 计算加权平均
    std::vector<double> values = {85.0, 92.0, 78.0};
    std::vector<double> weights = {0.3, 0.5, 0.2};
    double weightedAvg = std::transform_reduce(values.begin(), values.end(),
        weights.begin(), 0.0);
    std::cout << "加权平均: " << weightedAvg << "\n"; // 85*0.3 + 92*0.5 + 78*0.2 = 87.1
}
```

---

## 10. 内存未初始化算法

用于在未初始化的内存上构造对象，适用于自定义内存分配器和容器实现。

```cpp
#include <memory>
#include <vector>
#include <string>
#include <iostream>

int main() {
    // uninitialized_fill: 在未初始化内存上构造对象
    alignas(std::string) char buffer[sizeof(std::string) * 3];
    std::string* p = reinterpret_cast<std::string*>(buffer);

    std::uninitialized_fill(p, p + 3, "Hello");
    for (int i = 0; i < 3; ++i)
        std::cout << p[i] << " ";
    std::cout << "\n"; // Hello Hello Hello

    // 手动析构
    std::destroy(p, p + 3);

    // uninitialized_copy: 从已有范围复制构造到未初始化内存
    std::vector<std::string> src = {"A", "B", "C"};
    alignas(std::string) char buf2[sizeof(std::string) * 3];
    std::string* p2 = reinterpret_cast<std::string*>(buf2);

    std::uninitialized_copy(src.begin(), src.end(), p2);
    for (int i = 0; i < 3; ++i)
        std::cout << p2[i] << " ";
    std::cout << "\n"; // A B C

    std::destroy(p2, p2 + 3);

    // C++17: uninitialized_move, uninitialized_default_construct, uninitialized_value_construct
    // C++17: destroy, destroy_n
    // C++20: uninitialized_fill 的 ranges 版本
}
```

---

## 11. C++17 并行算法

C++17 引入了执行策略（Execution Policies），让大部分算法支持并行执行。

```cpp
#include <algorithm>
#include <numeric>
#include <execution>
#include <vector>
#include <iostream>
#include <chrono>

int main() {
    const int N = 10'000'000;
    std::vector<int> v(N);
    std::iota(v.begin(), v.end(), 0);

    // 执行策略:
    // std::execution::seq       — 顺序执行（默认）
    // std::execution::par       — 并行执行
    // std::execution::par_unseq — 并行 + 向量化
    // std::execution::unseq     — 向量化（C++20）

    // 顺序排序
    auto v1 = v;
    auto t1 = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::seq, v1.begin(), v1.end(), std::greater<int>());
    auto t2 = std::chrono::high_resolution_clock::now();
    auto seqTime = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    // 并行排序
    auto v2 = v;
    auto t3 = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::par, v2.begin(), v2.end(), std::greater<int>());
    auto t4 = std::chrono::high_resolution_clock::now();
    auto parTime = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count();

    std::cout << "顺序排序: " << seqTime << "ms\n";
    std::cout << "并行排序: " << parTime << "ms\n";

    // 并行 reduce
    long long sum = std::reduce(std::execution::par, v.begin(), v.end(), 0LL);
    std::cout << "总和: " << sum << "\n";

    // 并行 transform_reduce
    double sqSum = std::transform_reduce(std::execution::par,
        v.begin(), v.end(), 0.0,
        std::plus<double>(),
        [](int n) { return static_cast<double>(n) * n; });

    // 并行 for_each
    std::vector<int> data(N, 1);
    std::for_each(std::execution::par, data.begin(), data.end(),
        [](int& n) { n *= 2; });

    // 注意事项:
    // 1. 并行算法中不能使用 mutex 或其他可能死锁的同步原语
    // 2. Lambda 中避免数据竞争
    // 3. 对于小数据量，并行可能比顺序更慢（线程开销）
    // 4. 需要链接 TBB 库（MSVC 自带，GCC/Clang 需要安装）
}
```

---

## 12. C++20 Ranges 算法

C++20 引入了 Ranges 库，提供了更简洁、更安全的算法接口。

### 12.1 基本用法

```cpp
#include <algorithm>
#include <ranges>
#include <vector>
#include <iostream>
#include <string>

int main() {
    std::vector<int> v = {5, 2, 8, 1, 9, 3, 7, 4, 6};

    // 传统写法 vs Ranges 写法
    // 传统: std::sort(v.begin(), v.end());
    // Ranges: 直接传容器
    std::ranges::sort(v);
    // v: {1, 2, 3, 4, 5, 6, 7, 8, 9}

    // ranges::find
    auto it = std::ranges::find(v, 5);
    if (it != v.end())
        std::cout << "找到: " << *it << "\n";

    // ranges::count_if
    auto c = std::ranges::count_if(v, [](int n) { return n > 5; });
    std::cout << ">5 的个数: " << c << "\n"; // 4

    // ranges::for_each
    std::ranges::for_each(v, [](int n) { std::cout << n << " "; });
    std::cout << "\n";

    // 投影（Projection）: 不需要自定义比较器的新方式
    struct Student {
        std::string name;
        int score;
    };
    std::vector<Student> students = {{"Alice", 85}, {"Bob", 92}, {"Charlie", 78}};

    // 用投影按 score 排序（无需自定义比较器）
    std::ranges::sort(students, std::ranges::greater{}, &Student::score);
    for (const auto& s : students)
        std::cout << s.name << ": " << s.score << "\n";
    // Bob: 92
    // Alice: 85
    // Charlie: 78

    // ranges::min / max 直接操作容器
    auto [minS, maxS] = std::ranges::minmax(students, {}, &Student::score);
    std::cout << "最低分: " << minS.name << ", 最高分: " << maxS.name << "\n";
}
```

### 12.2 Views（视图）— 惰性求值管道

```cpp
#include <algorithm>
#include <ranges>
#include <vector>
#include <iostream>
#include <string>

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // views::filter + views::transform: 管道操作
    auto result = v
        | std::views::filter([](int n) { return n % 2 == 0; })  // 过滤偶数
        | std::views::transform([](int n) { return n * n; });    // 平方

    std::cout << "偶数的平方: ";
    for (int x : result) std::cout << x << " ";
    std::cout << "\n"; // 4 16 36 64 100

    // views::take / views::drop
    auto first5 = v | std::views::take(5);   // 取前5个
    auto skip3 = v | std::views::drop(3);    // 跳过前3个

    // views::reverse
    auto reversed = v | std::views::reverse;
    for (int x : reversed) std::cout << x << " ";
    std::cout << "\n"; // 10 9 8 7 6 5 4 3 2 1

    // 复合管道
    auto pipeline = v
        | std::views::filter([](int n) { return n > 3; })
        | std::views::transform([](int n) { return n * 2; })
        | std::views::take(4);
    // 惰性求值：只在遍历时才计算
    for (int x : pipeline) std::cout << x << " ";
    std::cout << "\n"; // 8 10 12 14

    // views::iota: 生成无限序列
    auto squares = std::views::iota(1)
        | std::views::transform([](int n) { return n * n; })
        | std::views::take(10);
    for (int x : squares) std::cout << x << " ";
    std::cout << "\n"; // 1 4 9 16 25 36 49 64 81 100

    // views::zip (C++23)
    // views::enumerate (C++23)
    // views::chunk (C++23)
    // views::slide (C++23)
}
```

### 12.3 Ranges 实用案例

```cpp
#include <algorithm>
#include <ranges>
#include <vector>
#include <string>
#include <iostream>
#include <map>

int main() {
    // 案例1: 处理字符串
    std::string text = "Hello, World! This is C++20 Ranges.";
    auto words = text
        | std::views::split(' ')
        | std::views::transform([](auto word) {
            return std::string(word.begin(), word.end());
          });
    for (const auto& w : words) std::cout << "[" << w << "] ";
    std::cout << "\n";

    // 案例2: 处理 map 的 keys 和 values
    std::map<std::string, int> scores = {{"Alice", 85}, {"Bob", 92}, {"Charlie", 78}};
    auto names = scores | std::views::keys;
    auto vals = scores | std::views::values;
    for (const auto& name : names) std::cout << name << " ";
    std::cout << "\n";

    // 案例3: 链式处理，找出前 3 个能被 3 整除的平方数
    auto result = std::views::iota(1)
        | std::views::transform([](int n) { return n * n; })
        | std::views::filter([](int n) { return n % 3 == 0; })
        | std::views::take(3);
    for (int x : result) std::cout << x << " ";
    std::cout << "\n"; // 9 36 81
}
```

---

## 13. 实战综合案例

### 案例1: 学生成绩分析系统

```cpp
#include <algorithm>
#include <numeric>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <map>

struct Student {
    std::string name;
    std::vector<int> scores; // 各科成绩
};

int main() {
    std::vector<Student> students = {
        {"Alice",   {85, 92, 78, 95, 88}},
        {"Bob",     {72, 68, 85, 79, 91}},
        {"Charlie", {90, 95, 92, 88, 97}},
        {"Diana",   {65, 70, 60, 75, 68}},
        {"Eve",     {88, 82, 90, 85, 93}},
    };

    // 1. 计算每个学生的平均分
    std::vector<std::pair<std::string, double>> averages;
    std::transform(students.begin(), students.end(), std::back_inserter(averages),
        [](const Student& s) {
            double avg = std::accumulate(s.scores.begin(), s.scores.end(), 0.0)
                         / s.scores.size();
            return std::make_pair(s.name, avg);
        });

    // 2. 按平均分降序排序
    std::sort(averages.begin(), averages.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    std::cout << "=== 成绩排名 ===\n";
    int rank = 1;
    for (const auto& [name, avg] : averages) {
        std::cout << rank++ << ". " << std::setw(10) << std::left << name
                  << " 平均分: " << std::fixed << std::setprecision(1) << avg << "\n";
    }

    // 3. 找到总分最高和最低的学生
    auto [minIt, maxIt] = std::minmax_element(students.begin(), students.end(),
        [](const Student& a, const Student& b) {
            return std::accumulate(a.scores.begin(), a.scores.end(), 0)
                 < std::accumulate(b.scores.begin(), b.scores.end(), 0);
        });
    std::cout << "\n总分最高: " << maxIt->name << "\n";
    std::cout << "总分最低: " << minIt->name << "\n";

    // 4. 统计及格（>=60）学生数
    auto passCount = std::count_if(students.begin(), students.end(),
        [](const Student& s) {
            return std::all_of(s.scores.begin(), s.scores.end(),
                [](int score) { return score >= 60; });
        });
    std::cout << "\n所有科目都及格的学生数: " << passCount << "\n";

    // 5. 各科目最高分
    std::cout << "\n=== 各科最高分 ===\n";
    for (size_t subj = 0; subj < 5; ++subj) {
        auto best = std::max_element(students.begin(), students.end(),
            [subj](const Student& a, const Student& b) {
                return a.scores[subj] < b.scores[subj];
            });
        std::cout << "科目" << subj + 1 << ": " << best->name
                  << " (" << best->scores[subj] << "分)\n";
    }

    // 6. 将学生按平均分分为优秀（>=85）和普通两组
    auto pivotIt = std::stable_partition(students.begin(), students.end(),
        [](const Student& s) {
            double avg = std::accumulate(s.scores.begin(), s.scores.end(), 0.0)
                         / s.scores.size();
            return avg >= 85.0;
        });

    std::cout << "\n=== 优秀学生 ===\n";
    std::for_each(students.begin(), pivotIt, [](const Student& s) {
        std::cout << s.name << "\n";
    });
    std::cout << "\n=== 普通学生 ===\n";
    std::for_each(pivotIt, students.end(), [](const Student& s) {
        std::cout << s.name << "\n";
    });
}
```

### 案例2: 文本词频统计

```cpp
#include <algorithm>
#include <numeric>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <cctype>

int main() {
    std::string text = "the quick brown fox jumps over the lazy dog "
                       "the dog barked at the fox and the fox ran away";

    // 1. 分词
    std::istringstream iss(text);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) {
        // 转小写
        std::transform(word.begin(), word.end(), word.begin(),
            [](unsigned char c) { return std::tolower(c); });
        words.push_back(word);
    }

    // 2. 排序后统计（利用 adjacent_find 和 equal_range）
    std::sort(words.begin(), words.end());
    std::vector<std::pair<std::string, int>> freq;
    auto it = words.begin();
    while (it != words.end()) {
        auto [lo, hi] = std::equal_range(it, words.end(), *it);
        freq.emplace_back(*it, std::distance(lo, hi));
        it = hi;
    }

    // 3. 按频率降序排序
    std::sort(freq.begin(), freq.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    // 4. 输出 Top 5
    std::cout << "=== 词频 Top 5 ===\n";
    auto top5 = std::min(freq.size(), size_t(5));
    for (size_t i = 0; i < top5; ++i) {
        std::cout << freq[i].first << ": " << freq[i].second << "\n";
    }

    // 5. 统计信息
    auto totalWords = words.size();
    auto uniqueWords = freq.size();
    auto maxFreq = std::max_element(freq.begin(), freq.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    std::cout << "\n总词数: " << totalWords << "\n";
    std::cout << "不同词数: " << uniqueWords << "\n";
    std::cout << "最高频词: " << maxFreq->first << " (" << maxFreq->second << " 次)\n";
}
```

### 案例3: 高效数据流水线处理

```cpp
#include <algorithm>
#include <numeric>
#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <functional>

// 模拟传感器数据处理管道
struct SensorData {
    int sensorId;
    double value;
    bool valid;
};

int main() {
    // 1. 生成模拟数据
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> valueDist(0.0, 100.0);
    std::uniform_int_distribution<int> idDist(1, 5);
    std::bernoulli_distribution validDist(0.9); // 90% 有效

    std::vector<SensorData> rawData(1000);
    std::generate(rawData.begin(), rawData.end(), [&]() {
        return SensorData{idDist(rng), valueDist(rng), validDist(rng)};
    });

    // 2. 过滤无效数据
    rawData.erase(
        std::remove_if(rawData.begin(), rawData.end(),
            [](const SensorData& d) { return !d.valid; }),
        rawData.end());
    std::cout << "有效数据: " << rawData.size() << " 条\n";

    // 3. 按传感器分组统计
    std::sort(rawData.begin(), rawData.end(),
        [](const SensorData& a, const SensorData& b) { return a.sensorId < b.sensorId; });

    auto it = rawData.begin();
    while (it != rawData.end()) {
        int id = it->sensorId;
        auto groupEnd = std::find_if(it, rawData.end(),
            [id](const SensorData& d) { return d.sensorId != id; });

        // 计算该传感器的统计信息
        auto count = std::distance(it, groupEnd);
        double sum = std::accumulate(it, groupEnd, 0.0,
            [](double acc, const SensorData& d) { return acc + d.value; });
        auto [minIt, maxIt] = std::minmax_element(it, groupEnd,
            [](const SensorData& a, const SensorData& b) { return a.value < b.value; });

        std::cout << "传感器 #" << id
                  << " | 数据量: " << count
                  << " | 平均: " << sum / count
                  << " | 最小: " << minIt->value
                  << " | 最大: " << maxIt->value << "\n";

        it = groupEnd;
    }

    // 4. 检测异常值（超过2个标准差）
    double mean = std::accumulate(rawData.begin(), rawData.end(), 0.0,
        [](double acc, const SensorData& d) { return acc + d.value; }) / rawData.size();

    double variance = std::accumulate(rawData.begin(), rawData.end(), 0.0,
        [mean](double acc, const SensorData& d) {
            double diff = d.value - mean;
            return acc + diff * diff;
        }) / rawData.size();
    double stddev = std::sqrt(variance);

    auto anomalies = std::count_if(rawData.begin(), rawData.end(),
        [mean, stddev](const SensorData& d) {
            return std::abs(d.value - mean) > 2.0 * stddev;
        });

    std::cout << "\n全局平均: " << mean
              << "\n标准差: " << stddev
              << "\n异常值数量: " << anomalies << "\n";
}
```

---

## 算法速查表

| 分类 | 算法 | 用途 | 复杂度 |
|------|------|------|--------|
| **查找** | `find` / `find_if` | 线性查找 | O(n) |
| | `binary_search` | 二分查找（仅判断存在） | O(log n) |
| | `lower_bound` / `upper_bound` | 二分查找（返回位置） | O(log n) |
| | `equal_range` | 等值范围 | O(log n) |
| | `search` | 子序列查找 | O(n×m) |
| **计数** | `count` / `count_if` | 统计 | O(n) |
| | `all_of` / `any_of` / `none_of` | 逻辑判断 | O(n) |
| **排序** | `sort` | 不稳定排序 | O(n log n) |
| | `stable_sort` | 稳定排序 | O(n log²n) |
| | `partial_sort` | 部分排序 | O(n log k) |
| | `nth_element` | 第 N 元素 | O(n) |
| **修改** | `transform` | 变换 | O(n) |
| | `copy` / `copy_if` | 复制 | O(n) |
| | `fill` / `generate` | 填充 | O(n) |
| | `replace` / `replace_if` | 替换 | O(n) |
| | `remove` / `remove_if` | 逻辑删除 | O(n) |
| | `unique` | 去除相邻重复 | O(n) |
| | `reverse` / `rotate` | 重排 | O(n) |
| **分区** | `partition` | 不稳定分区 | O(n) |
| | `stable_partition` | 稳定分区 | O(n log n) |
| **集合** | `set_union` / `set_intersection` | 集合运算 | O(n+m) |
| | `merge` | 合并有序序列 | O(n+m) |
| **堆** | `make_heap` | 建堆 | O(n) |
| | `push_heap` / `pop_heap` | 入堆/出堆 | O(log n) |
| **数值** | `accumulate` / `reduce` | 累加/归约 | O(n) |
| | `partial_sum` | 前缀和 | O(n) |
| | `inner_product` | 内积 | O(n) |
| | `transform_reduce` | 变换归约 | O(n) |
| | `iota` | 递增填充 | O(n) |

---

## 最佳实践与陷阱

### 1. 选择正确的算法

```
需要查找？
├── 已排序 → lower_bound / binary_search (O(log n))
└── 未排序 → find / find_if (O(n))

需要排序？
├── 完全排序 → sort / stable_sort
├── Top-K → partial_sort / nth_element
└── 检查 → is_sorted

需要删除？
├── C++20 → std::erase / std::erase_if
└── C++17及以前 → erase-remove 惯用法
```

### 2. 常见陷阱

```cpp
// 陷阱1: 对未排序容器使用二分查找
std::vector<int> v = {5, 3, 1, 4, 2};
// 错误！v 未排序
// std::binary_search(v.begin(), v.end(), 3);     // 结果不确定
std::sort(v.begin(), v.end());                      // 先排序
bool found = std::binary_search(v.begin(), v.end(), 3); // 正确

// 陷阱2: remove 不会真正删除（要配合 erase）
// std::remove(v.begin(), v.end(), 3);  // 仅移动，size 不变

// 陷阱3: accumulate 初始值类型
std::vector<double> d = {1.5, 2.5};
// int sum = std::accumulate(d.begin(), d.end(), 0);     // 结果为 3（截断！）
double sum = std::accumulate(d.begin(), d.end(), 0.0);   // 结果为 4.0

// 陷阱4: 迭代器失效
// 不要在 for_each/transform 中修改容器结构（如 push_back、erase）

// 陷阱5: sort 的比较函数必须满足严格弱序
// 错误: [](int a, int b) { return a <= b; }  // 不是严格弱序！
// 正确: [](int a, int b) { return a < b; }
```

### 3. 性能建议

- 优先使用 STL 算法而非手写循环——算法经过高度优化。
- 对大数据集考虑 C++17 并行执行策略。
- `reserve()` 配合 `back_inserter` 避免多余的内存分配。
- 对 `std::string` 密集操作，考虑 `std::string_view` 减少拷贝。
- 使用 `ranges::sort(v)` 代替 `std::sort(v.begin(), v.end())` 更简洁安全。
