`const` 是 C++ 中非常重要的关键字，用于表示**常量**（constant），即不可修改的值。它可以用于修饰变量、函数参数、返回值、成员函数等。掌握 `const` 的用法对于写出高质量、可读性强、可靠性高的 C++ 程序非常关键。

------

## 一、`const` 修饰变量

```cpp
const int a = 10;
a = 20; // 错误，a是只读的
```

含义：`a` 是一个常量，一旦初始化后就不能再被修改。

## 二、`const` 修饰指针

| 写法                 | 解释                                                         |
| -------------------- | ------------------------------------------------------------ |
| `const int *p`       | `p` 是指向常量的指针，不能通过 `*p` 修改值，但可以改指针本身指向的地址。 |
| `int *const p`       | `p` 是常量指针，指针地址不能变，但可以通过 `*p` 修改值。     |
| `const int *const p` | 指针和指向的值都不能变。                                     |



例子：

```cpp
const int val = 5;
const int* p1 = &val;    // 指向const int的指针
int* const p2 = new int; // const指针，地址不可变
```

## 三、`const` 修饰函数参数

```cpp
void print(const std::string& str); // 不会修改传入的字符串
```

优点：防止函数内部修改参数，支持传入临时对象，提升代码的健壮性。

## 四、`const` 修饰函数返回值

```cpp
const int getValue();
```

一般用于防止返回值被当作左值修改（但对基本类型无实际意义，主要用于对象引用返回时）。

## 五、`const` 成员函数

```cpp
class MyClass {
public:
    int getValue() const; // 承诺不修改成员变量
private:
    int value;
};
```

- 只能调用 `const` 成员函数
- 不允许修改成员变量（除非加 `mutable`）

## 六、`const` 对象

```cpp
const MyClass obj;
obj.getValue(); // 可以
obj.setValue(10); // 错误，不能调用非 const 成员函数
```

## 七、`mutable` 与 `const`

`mutable` 允许成员变量即使在 `const` 对象中也可以被修改。

```
class MyClass {
    mutable int counter = 0;
public:
    void increment() const { counter++; }
};
```

## 总结表：

| 位置                      | 意义                              |
| ------------------------- | --------------------------------- |
| `const int a = 10;`       | 常量，不能修改                    |
| `const int* p`            | 指向常量的指针（值不可改）        |
| `int* const p`            | 常量指针（地址不可改）            |
| `const int* const p`      | 指向常量的常量指针                |
| `void func(const int& x)` | 参数不可修改                      |
| `int get() const`         | 不修改成员变量的成员函数          |
| `const MyClass obj`       | 常量对象，只能调用 `const` 成员函数 |

## 问题

1. `int *const p` 可以指向常量吗？

   **答案：❌不能。**

   解释：

   ```cpp
   int *const p = &x;
   ```

   含义是：

   - `p` 是一个 **常量指针**（const pointer），即 `p` 的地址不可变。
   - 但 `*p` 是 `int`，不是 `const int`，所以通过 `p` 可以修改指向对象的值。

   因此，如果你写：

   ```cpp
   const int x = 10;
   int *const p = &x; // ❌ 错误，不能用 int* 指向 const int
   ```

   > 编译器会报错，因为你试图用 `int*`（可以修改值的指针）去指向一个常量 `const int`，不安全。

2. `const int *const p` 可以指向非常量吗？

   **答案：✅可以。**

   解释：

   ```cpp
   int x = 10;
   const int *const p = &x; // ✅ 合法
   ```

   含义是：

   - `p` 是一个 **常量指针**，地址不可变。
   - `*p` 是 `const int`，不能通过 `p` 修改值。
   - 但它可以指向一个 **非常量变量**，只是你通过这个指针不能修改它的值。

   也就是说，你告诉编译器：“我保证不通过这个指针去改 `x` 的值”，但别的地方仍然可以改。

3. 函数参数被 `const` 修饰，但你确实**想要在函数内部修改它**怎么办？

   正确方式一：**去掉 const**

   如果你就是要修改参数值，**直接删掉 const 修饰符**：

   ```cpp
   void func(int x) {
       x = 10; // ✅ 合法，但这只改了局部副本
   }
   ```

   > 注意：`int x` 是值传递，修改的是副本，不影响外部变量。

   

   正确方式二：**使用 `const_cast`（慎用）**

   ```cpp
   void func(const int& x) {
       int& y = const_cast<int&>(x);
       y = 10; // ❗危险：仅当 x 本来不是 const 时才安全
   }
   ```

   ⚠️ 注意事项：

   - 如果传进来的确实是一个 `const` 对象（如 `const int a = 5;`），`const_cast` 后修改会导致**未定义行为**！

   - 仅在你**100% 确定**传入的对象原本不是 const 的时候使用（如模板、封装库里处理非 const 的引用）。

     

   正确方式三：**复制一份临时变量**

   当你只是需要在函数内部**修改临时值**而不影响原始值，推荐这种写法：

   ```cpp
   void func(const int& x) {
       int temp = x;  // 拷贝
       temp += 10;
       std::cout << temp << std::endl;
   }
   ```

   适用场景：你只是临时计算处理，但又不想改变原始输入值。

   

   正确方式四：使用 `mutable`（用于成员变量）

   如果你是在 `const` 成员函数中，想修改类成员变量，可用 `mutable` 修饰该变量：

   ```
   class MyClass {
       mutable int counter = 0;
   
   public:
       void doSomething() const {
           counter++;  // ✅ 合法
       }
   };
   ```

​	**总结建议：**去掉 `const`，或传引用，或拷贝一个临时变量

