## 构建工具
- QMake
    - Qt 自带的构建工具
    - 写 .pro 文件
    - 所有平台工具链兼容，但 Qt 6 后期逐步弃用

- CMake
    - 现代跨平台构建系统
    - Qt 6 官方推荐（特别是 Qt 6.2 以后）
    - 与 MSVC、MinGW、Clang 完全兼容
    - 配合 Ninja 使用构建速度更快

- Ninja
    - 不是独立的构建工具链，而是 CMake 的 backend
    - 写 CMakeLists.txt，通过 -G Ninja 让 CMake 使用 Ninja
    - 通常比 Make 或 NMake 更快

## 编译器
MSVC、MinGW、Clang 都是 C/C++ 编译器或编译工具链（Toolchain），负责把你的 C/C++ 代码编译成可执行程序，它们运行的平台、支持的特性和编译出的程序格式有所不同。
MSVC 是 Windows 下微软官方的编译器，MinGW 是开源轻量版，Clang 是 macOS 和 Linux 上的现代主流编译器，如果你是在 mac 上开发 Qt 项目，推荐默认用 Clang；如果你在 Windows 上，MSVC 更适合做正式发布，MinGW 更轻便学习用。

| 特性      | MSVC              | MinGW             | Clang                |
| ------- | ----------------- | ----------------- | -------------------- |
| 适用平台    | Windows           | Windows           | macOS / Linux / Win  |
| 是否官方    | 微软官方              | 非官方               | LLVM 官方              |
| 编译标准    | 支持 C++11/14/17/20 | 支持 C++11/14/17/20 | 支持 C++11/14/17/20/23 |
| 调试器     | CDB               | GDB               | LLDB（mac）            |
| 与 Qt 兼容 | 是                 | 是                | 是（mac 默认）            |



## 工具链
编译器（MSVC/MinGW/Clang）+ 构建系统（CMake/QMake/Ninja）= 程序构建工具链
```markdown
      ┌─────────────┐
      │ 你写的代码   │
      └─────┬───────┘
            │（调用构建系统）
    ┌───────▼─────────┐
    │ 构建系统：CMake / QMake │
    └───────┬─────────┘
            │（调用编译器）
  ┌─────────▼──────────┐
  │ 编译器：MSVC / MinGW / Clang │
  └─────────┬──────────┘
            │（生成可执行程序）
        ┌───▼────┐
        │ .exe / │
        │ .app等 │
        └────────┘
```

Qt 平台构建工具链推荐
| 平台         | Qt 版本                | 编译器        | 推荐构建工具组合                       |
| ----------- | --------------------- | ------------ | -------------------------------------    |
| **Windows** | msvc2019_64         | MSVC 2019    |  **CMake + Ninja**（或 nmake）        |
| **Windows** | mingw_64            | MinGW 11.2.0 |  **CMake + Ninja**（或 mingw32-make） |
| **macOS**   | clang_64            | Apple Clang  |  **CMake + Ninja**（或 make）         |
| **Linux**   | gcc_64 / clang_64 | GCC 或 Clang  |  **CMake + Ninja**（或 make）         |