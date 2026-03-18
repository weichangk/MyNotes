# C++ assert 断言

## 1. 什么是 assert

`assert` 是 C/C++ 提供的**运行时断言宏**，用于在调试阶段检测程序中"不应该发生"的条件。如果条件为 `false`，程序会**立即终止**并输出诊断信息。

```cpp
#include <cassert>  // C++ 头文件
// 或
#include <assert.h> // C 头文件（C++ 中也可用）

assert(表达式);
```

> 核心思想：**assert 不是错误处理，而是 Bug 检测工具。** 它断言某个条件"一定为真"，如果不为真，说明程序有 Bug。

---

## 2. 基本用法

### 2.1 简单示例

```cpp
#include <cassert>
#include <iostream>

double divide(double a, double b) {
    assert(b != 0 && "除数不能为零");  // 断言 b 不为 0
    return a / b;
}

int main() {
    std::cout << divide(10, 2) << std::endl;  // 正常：5
    std::cout << divide(10, 0) << std::endl;  // 触发断言，程序终止

    return 0;
}
```

### 2.2 断言失败的输出

当断言失败时，程序输出类似以下信息并调用 `abort()` 终止：

```
Assertion failed: b != 0 && "除数不能为零", file main.cpp, line 5
```

输出包含：
- **失败的表达式**
- **文件名**
- **行号**

---

## 3. 带消息的 assert 技巧

`assert` 本身不支持自定义消息，但可以利用**字符串字面量始终为真**的特性：

```cpp
// 方式1：&& 附加消息（推荐）
assert(ptr != nullptr && "指针不能为空");

// 方式2：逗号表达式（不推荐，部分编译器会警告）
assert(("索引越界", index < size));
```

**原理：** `"除数不能为零"` 是一个非空指针，转为 `bool` 后恒为 `true`，所以 `&&` 不影响断言条件的判断，但失败时消息会被打印出来。

```
Assertion failed: b != 0 && "除数不能为零"
                              ↑ 这段文字会显示在诊断信息中
```

---

## 4. assert 的开关：NDEBUG

`assert` 是一个**宏**，它的行为受 `NDEBUG` 宏控制：

| 模式 | NDEBUG | assert 行为 |
|------|--------|-------------|
| Debug 构建 | 未定义 | 正常检查，失败时终止程序 |
| Release 构建 | 已定义 | **完全消失**，不产生任何代码 |

### 4.1 assert 的宏展开（简化版）

```cpp
// 伪代码，展示 assert 的实现原理
#ifdef NDEBUG
    #define assert(expr)  ((void)0)          // Release：什么都不做
#else
    #define assert(expr)  \
        ((expr) ? (void)0 : __assert_fail(#expr, __FILE__, __LINE__))
#endif
```

### 4.2 CMake 中的 NDEBUG

```cmake
# CMake 的 Release 构建会自动定义 NDEBUG
cmake -DCMAKE_BUILD_TYPE=Release ..

# Debug 构建不定义 NDEBUG，assert 生效
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

### 4.3 手动控制 NDEBUG

```cpp
// 在 #include <cassert> 之前定义 NDEBUG，可以禁用 assert
#define NDEBUG
#include <cassert>

int main() {
    assert(false);  // 不会触发，因为 assert 被禁用了
    return 0;
}
```

> ⚠️ **重要：** 因为 Release 模式下 assert 会被完全移除，所以**永远不要在 assert 中放有副作用的表达式**！

```cpp
// ❌ 严重错误：Release 下 processData() 不会被调用！
assert(processData() == true);

// ✅ 正确做法：
bool result = processData();
assert(result == true);
```

---

## 5. static_assert（C++11 编译期断言）

C++11 引入了 `static_assert`，在**编译期**进行检查，失败时产生编译错误。

```cpp
static_assert(条件表达式, "错误消息");      // C++11
static_assert(条件表达式);                  // C++17（消息可省略）
```

### 5.1 基本用法

```cpp
#include <cstdint>
#include <type_traits>

// 检查类型大小
static_assert(sizeof(int) >= 4, "int 必须至少 4 字节");
static_assert(sizeof(void*) == 8, "仅支持 64 位平台");

// 检查类型特性
template <typename T>
T add(T a, T b) {
    static_assert(std::is_arithmetic<T>::value,
                  "T 必须是算术类型（整数或浮点数）");
    return a + b;
}

int main() {
    add(1, 2);       // ✅ OK
    add(1.0, 2.0);   // ✅ OK
    // add("a", "b"); // ❌ 编译错误：T 必须是算术类型
    return 0;
}
```

### 5.2 常见使用场景

```cpp
#include <type_traits>

// 1. 检查平台/编译器假设
static_assert(sizeof(long) == 8, "需要 64 位 long");
static_assert(CHAR_BIT == 8, "需要 8 位字节");

// 2. 模板约束
template <typename T>
class SmartBuffer {
    static_assert(std::is_trivially_copyable<T>::value,
                  "SmartBuffer 只支持可平凡拷贝的类型");
    // ...
};

// 3. 枚举完整性检查
enum class Color { Red, Green, Blue, COUNT };
static_assert(static_cast<int>(Color::COUNT) == 3,
              "新增颜色后请更新 COUNT");

// 4. 结构体布局验证
struct Packet {
    uint8_t  type;
    uint8_t  flags;
    uint16_t length;
    uint32_t data;
};
static_assert(sizeof(Packet) == 8, "Packet 结构体布局不符合协议要求");
```

---

## 6. assert vs static_assert 对比

| 特性 | `assert` | `static_assert` |
|------|----------|-----------------|
| 检查时机 | **运行时** | **编译期** |
| 头文件 | `<cassert>` | 无需头文件（语言关键字） |
| 失败行为 | 调用 `abort()` 终止程序 | 编译错误 |
| Release 行为 | 被 `NDEBUG` 禁用 | **始终生效** |
| 条件要求 | 任意运行时表达式 | 必须是编译期常量表达式 |
| 自定义消息 | 需要 `&&` 技巧 | 直接支持（C++11） |
| 引入版本 | C89 / C++ | C++11 |

**选择原则：** 能用 `static_assert` 就用 `static_assert`，把错误尽早暴露在编译期。

---

## 7. 实际项目中的最佳实践

### 7.1 适合用 assert 的场景

```cpp
// ✅ 检查前置条件（函数入口）
void processArray(int* arr, int size) {
    assert(arr != nullptr && "数组指针不能为空");
    assert(size > 0 && "数组大小必须为正");
    // ...
}

// ✅ 检查不变量（逻辑上不该发生的情况）
void handleState(State s) {
    switch (s) {
        case State::Init: /* ... */ break;
        case State::Running: /* ... */ break;
        case State::Done: /* ... */ break;
        default:
            assert(false && "未知状态，不应到达此处");
    }
}

// ✅ 检查后置条件（函数返回前）
int factorial(int n) {
    assert(n >= 0);
    int result = 1;
    for (int i = 2; i <= n; ++i)
        result *= i;
    assert(result > 0 && "溢出检测");
    return result;
}
```

### 7.2 不适合用 assert 的场景

```cpp
// ❌ 不要用 assert 处理用户输入（Release 下会消失）
void readAge(int age) {
    assert(age > 0 && age < 150);  // 错误！用户可以输入任何值
}

// ✅ 正确：用异常或错误返回
void readAge(int age) {
    if (age <= 0 || age >= 150) {
        throw std::invalid_argument("年龄无效");
    }
}

// ❌ 不要用 assert 检查系统调用返回值
assert(fopen("data.txt", "r") != nullptr);  // Release 下文件打开失败不会被捕获

// ✅ 正确：
FILE* fp = fopen("data.txt", "r");
if (!fp) {
    std::cerr << "文件打开失败" << std::endl;
    return -1;
}
```

### 7.3 自定义 ASSERT 宏

在实际项目中，通常会封装自定义断言宏以提供更多功能：

```cpp
#include <iostream>
#include <cstdlib>

#ifdef _DEBUG
    #define MY_ASSERT(expr, msg)                                          \
        do {                                                              \
            if (!(expr)) {                                                \
                std::cerr << "断言失败: " << #expr << "\n"                 \
                          << "消息: " << msg << "\n"                       \
                          << "文件: " << __FILE__ << "\n"                  \
                          << "行号: " << __LINE__ << "\n"                  \
                          << "函数: " << __FUNCTION__ << std::endl;        \
                std::abort();                                             \
            }                                                             \
        } while (0)
#else
    #define MY_ASSERT(expr, msg) ((void)0)
#endif

// 使用
void process(int* data, int count) {
    MY_ASSERT(data != nullptr, "data 指针为空");
    MY_ASSERT(count > 0, "count 必须大于 0");
    // ...
}
```

**自定义宏的优势：**
- 支持自定义消息格式
- 可以输出函数名 (`__FUNCTION__`)
- 可以集成日志系统
- 可以增加断点（如 `__debugbreak()`）
- 可以按模块分别控制开关

---

## 8. 总结

```
┌─────────────────────────────────────────────────────────────┐
│                    C++ 断言体系                               │
├──────────────────────┬──────────────────────────────────────┤
│                      │                                      │
│    assert (运行时)    │      static_assert (编译期)           │
│                      │                                      │
│  • 检查运行时条件     │  • 检查编译期常量条件                   │
│  • Release 下消失     │  • 始终生效                           │
│  • 需要 <cassert>    │  • 语言内置关键字                      │
│  • 调用 abort() 终止  │  • 产生编译错误                       │
│                      │                                      │
├──────────────────────┴──────────────────────────────────────┤
│                                                             │
│  ✅ DO:   检查前置条件、不变量、后置条件、逻辑假设              │
│  ❌ DON'T: 检查用户输入、系统调用返回值、有副作用的表达式       │
│                                                             │
│  原则: 能在编译期检查的，用 static_assert                     │
│        运行时的内部逻辑检查，用 assert                         │
│        外部输入和错误处理，用异常/错误码                        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

*最后更新时间：2026年2月27日*
