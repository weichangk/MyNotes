## 函数指针类型的基本格式

```cpp
返回类型 (*指针名)(参数类型列表);
```

其中：

- `*指针名` 表示这是一个指针（指向函数的指针）
- 圆括号必须加上，否则编译器会当成返回函数的函数，语义错误。

## 示例 1：声明一个指向函数的指针（基础）

我们先有一个函数：

```cpp
void myFunc(int x) {
    std::cout << "x = " << x << std::endl;
}
```

### 声明一个函数指针类型，并使用它：

```cpp
// 1. 声明函数指针类型（指向 void(int) 类型的函数）
void (*funcPtr)(int);

// 2. 赋值
funcPtr = myFunc;

// 3. 调用函数
funcPtr(10);  // 输出 x = 10
```

> ⚠️ 括号不能省略！下面这样是错误的：

```cpp
void *funcPtr(int);  // ❌ 这是声明一个函数，返回 void*，不是函数指针
```

## 示例 2：组合使用 typedef / using 简化语法（推荐）

### C++98：使用 `typedef`

```cpp
typedef void (*FuncType)(int);

void myFunc(int x) {
    std::cout << "x = " << x << std::endl;
}

int main() {
    FuncType ptr = myFunc;
    ptr(100);  // 输出 x = 100
}
```

### C++11：使用 `using` 更直观

```cpp
using FuncType = void(*)(int);
```

## 示例 3：多个参数 + 返回值的函数指针

假设函数原型是：

```cpp
double compute(int a, float b);
```

那么函数指针类型是：

```cpp
double (*ComputeFunc)(int, float);

double compute(int a, float b) {
    return a + b;
}

int main() {
    ComputeFunc f = compute;
    std::cout << f(2, 3.5f);  // 输出 5.5
}
```

## 补充：函数指针作为参数

```cpp
void operate(int a, int b, int (*op)(int, int)) {
    std::cout << "Result = " << op(a, b) << std::endl;
}

int add(int x, int y) {
    return x + y;
}

int main() {
    operate(3, 4, add);  // 输出 Result = 7
}
```

## 总结表

| 原函数声明                   | 函数指针类型声明                 |
| ---------------------------- | -------------------------------- |
| `void f(int)`                | `void (*)(int)`                  |
| `int sum(int, int)`          | `int (*)(int, int)`              |
| `double compute(int, float)` | `double (*)(int, float)`         |
| 使用 `typedef`               | `typedef int (*Func)(int, int);` |
| 使用 `using`（推荐）         | `using Func = int(*)(int, int);` |