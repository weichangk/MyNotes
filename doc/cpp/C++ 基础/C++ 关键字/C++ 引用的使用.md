#### 作为函数参数（避免拷贝，提高效率）

```cpp
void swap(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}
```

- ✅ 使用引用可以直接操作实参。
- ❌ 不使用引用只能交换副本，原数据不变。

#### const 引用（只读引用，允许绑定临时对象）

```cpp
void print(const std::string& msg) {
    std::cout << msg << std::endl;
}
print("Hello");  // 绑定到 const 引用，临时 string 被接受。"Hello" 是一个 临时对象（类型是 const char[6]，会隐式转换为 std::string）
// const int& ref = 10; // 临时变量存储 10，ref 绑定到它
```

#### 作为函数返回值（注意作用域）

```cpp
int& getRef(int& x) {
    return x;
}
getRef(a) = 100;  // 可以修改 a
```

⚠️ 小心不要返回局部变量的引用！

#### 引用与范围 for 循环

```cpp
std::vector<int> vec = {1, 2, 3};
for (int& x : vec) {
    x *= 2;
}
```

- `int& x`：修改原数组
- `int x`：仅修改副本

#### 引用作为类型别名（少见）

```cpp
typedef int& IntRef;
```

#### 右值引用（C++11）

```
int&& r = 10;  // 右值引用，类似一个临时变量的引用
```

用于实现**移动语义**，避免深拷贝：

```cpp
std::vector<int> v1 = {1, 2, 3};
std::vector<int> v2 = std::move(v1);  // 使用右值引用进行资源转移
```