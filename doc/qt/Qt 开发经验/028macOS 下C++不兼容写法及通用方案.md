
## 🍏 macOS 不兼容写法及通用解决方案整理（C++/Qt）

| ⚠️ 不兼容写法 | 💡 通用写法 | 📋 说明 |
|---------------|-------------|---------|
| `for each(auto item in items)` | `for (auto item : items)` | `for each` 是 MSVC 扩展，macOS 不支持，推荐使用 C++11 范式 |
| `QStringList list; list << "a" << "b";` | `list.append("a"); list.append("b");` | `<<` 重载在不同平台兼容性不一致，使用 `append` 更稳定 |
| `__super::foo();` | `BaseClass::foo();` | `__super` 是 MSVC 特有，用父类名访问基类成员通用 |
| `_countof(arr)` | 见右侧宏 | 定义通用宏：<br>`#ifdef Q_OS_WIN`<br>`#define COUNT_OF _countof`<br>`#else`<br>`#define COUNT_OF(arr) (sizeof(arr) / sizeof((arr)[0]))`<br>`#endif` |
| `#include "xx\xx"` | `#include "xx/xx"` | macOS 识别 `\` 可能失败，统一使用 `/` 路径分隔符 |
| 非 `const` 实参传入 `const` 形参函数报错 | 所有参数均加 `const` 修饰 | 保持接口一致性，避免编译器推导冲突，减少副作用 |
| 模板类中调用模板方法：`ptr->get<T>()` | `ptr->template get<T>()` | 在模板环境中调用模板成员时需用 `.template` 明确指示 |
