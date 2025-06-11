

### CMake + C++ 工程构建流程

用的 CMake + C++ 工程（跨平台，如 Windows/macOS） 为例

```text
源码（.cpp/.h） 
   ↓
[1] 预处理（Preprocessing）        →  .i 文件
   ↓
[2] 编译（Compiling）             →  .s 汇编文件（可选）
   ↓
[3] 汇编（Assembling）            →  .o 目标文件（Unix/mac）或 .obj（Windows）
   ↓
[4] 链接（Linking）               →  .exe / .dll / .lib / .so / .dylib / .a
```
1. 预处理（Preprocessing）

   展开所有 `#include`、`#define`、条件编译等预处理指令，生成纯净的代码，`.i` 文件，是纯文本的展开源码。

   - 替换 `#include`（把头文件内容插入）

   - 宏展开（`#define`）

   - 条件编译处理（如 `#ifdef DEBUG`）

   ```bash
   g++ -E main.cpp -o main.i
   ```

2. 编译（Compiling）

   把 `.i`（纯文本）转换为汇编语言 `.s` 汇编文件

   - 语法/语义分析

   - 中间表示（IR）优化

   - 转换为目标平台的汇编指令

   ```bash
   g++ -S main.i -o main.s
   ```

3. 汇编（Assembling）

   将汇编代码 `.s` 编译成机器码 `.o` 或 `.obj`，这是目标文件，包含了代码但没有链接外部符号

   - macOS / Linux：`.o`

   - Windows：`.obj`

   ```bash
   g++ -c main.s -o main.o   # mac/Linux
   cl /c main.cpp            # Windows: 生成 main.obj
   ```

4. 链接（Linking）

   把多个 `.o` / `.obj` 文件与库（`.a`, `.lib`, `.so`, `.dll`）链接起来，生成最终的可执行文件或库

   - 可执行文件

     ```bash
     g++ main.o utils.o -o myapp      # Linux/mac: 可执行文件 myapp
     link main.obj utils.obj /OUT:myapp.exe  # Windows
     ```

   - 静态库

     ```bash
     ar rcs libmylib.a main.o utils.o        # 静态库（mac/Linux）
     lib main.obj utils.obj /OUT:mylib.lib   # 静态库（Windows）
     ```

   - 动态库

     ```bash
     g++ -shared -o libmylib.so main.o       # Linux
     cl /LD main.cpp                         # Windows: 生成 .dll + .lib + .exp
     ```

### CMake 构建目录结构

CMake 构建流程确实包括预处理 → 编译 → 汇编 → 链接这些阶段，但默认情况下，CMake 不生成也不保留 `.i` 或 `.s` 文件
- 编译器直接从 `.cpp` 编译成 `.obj/.o`（目标文件）；
- `.i`（预处理文件） 和 `.s`（汇编文件）只是调试或分析用才会生成；
- 构建系统默认不会保留它们。

构建目录结构如下（无 `.i` 或 `.s`）：
```bash
build/
├── CMakeFiles/
│   ├── MyApp.dir/
│   │   ├── main.cpp.obj       ← 编译后的目标文件
│   │   ├── utils.cpp.obj
├── MyApp.exe
```
`.obj`（Windows）或 `.o`（Unix）是由编译器直接从 `.cpp` 生成的结果，预处理与汇编阶段在内部完成了。

### CMake 构建产物分类总结
| 文件类型  | 扩展名                       | 平台                    | 说明            |
| ----- | ------------------------- | --------------------- | ------------- |
| 目标文件  | `.o` / `.obj`             | Linux/mac / Windows   | 中间编译产物        |
| 静态库   | `.a` / `.lib`             | Linux/mac / Windows   | 静态链接          |
| 动态库   | `.so` / `.dylib` / `.dll` | Linux / mac / Windows | 动态链接          |
| 可执行文件 | 无扩展名 / `.out` / `.exe`    | Linux/mac / Windows   | 最终程序          |
| 导出表文件 | `.exp`                    | Windows               | DLL 链接辅助文件    |
| 导入库   | `.lib`（DLL 时）             | Windows               | 用于链接 DLL 的辅助库 |

### CMake 构建流程细节解读

CMake 是生成构建系统的工具，分析 `CMakeLists.txt`，生成构建描述文件（包含编译、链接规则），通过构建生成器执行构建，也就是调用编译器和链接器执行编译和链接

```text
 CMakeLists.txt
     ↓
[ CMake ] 解析 CMake 配置
     ↓
生成构建脚本：
 - Makefile（Unix Makefiles）
 - build.ninja（Ninja）
 - .vcxproj / .sln（Visual Studio）
     ↓
[ Ninja / Make / xcodebuild / msbuild ] 执行构建脚本
     ↓
调用编译器[ gcc/g++, clang/clang++，cl.exe ] → 生成 .o / .obj
调用链接器[ ld/lld, link.exe ] → 生成 .exe / .dll / .so
```

CMake 支持的构建生成器

| Generator          | 平台        | 构建脚本            | 构建工具     |
| ------------------ | ----------- | ------------------- | ------------ |
| **Unix Makefiles** | Linux/macOS | `Makefile`          | `make`       |
| **Ninja**          | 跨平台      | `build.ninja`       | `ninja`      |
| **Visual Studio**  | Windows     | `.vcxproj` / `.sln` | `MSBuild`    |
| **Xcode**          | macOS       | `.xcodeproj`        | `xcodebuild` |

构建工具角色

| 工具分类   | Linux / macOS                        | Windows（MSVC）           | 说明                                          |
| ---------- | ------------------------------------ | ------------------------- | --------------------------------------------- |
| 构建生产器 | `Unix Makefiles` / `Ninja` / `Xcode` | `Ninja` / `Visual Studio` | 生成构建脚本（Makefile / build.ninja / .sln） |
| 构建执行器 | `make` / `ninja` / `xcodebuild`      | `ninja` / `msbuild`       | 读取构建脚本并调度执行                        |
| 编译器     | `gcc/g++`, `clang/clang++`           | `cl.exe`                  | 把 `.cpp` 转为 `.o/.obj`                      |
| 汇编器     | `as`（LLVM 内部）                    | `ml.exe`（MSVC）          | `.s` 汇编为目标文件（通常被编译器内部调用）   |
| 链接器     | `ld`, `lld`                          | `link.exe`                | 把目标文件链接成可执行程序或库                |

### 各平台开发工具链安装

- Linux 平台

  安装方式

  ```bash
  sudo apt install build-essential   # Debian / Ubuntu
  sudo dnf groupinstall "Development Tools"  # Fedora
  sudo pacman -S base-devel          # Arch
  ```

  安装后提供的工具

  | 工具           | 命令          | 来源             | 说明              |
  | -------------- | ------------- | ---------------- | ----------------- |
  | 编译器         | `gcc` / `g++` | GCC              | C/C++ 编译器      |
  | 汇编器         | `as`          | GNU Binutils     | 被 `gcc` 自动调用 |
  | 链接器         | `ld`          | GNU Binutils     | 被 `gcc` 自动调用 |
  | 构建器（可选） | `make`        | GNU Make         | 处理 Makefile     |
  | 生成器         | `cmake`       | 系统包管理器安装 | 项目生成工具      |

  

- macOS 平台

  安装方式

  ```
  xcode-select --install   # 安装命令行工具（推荐）
  ```

  安装后提供的工具

  | 工具           | 命令                | 来源                            | 说明             |
  | -------------- | ------------------- | ------------------------------- | ---------------- |
  | 编译器         | `clang` / `clang++` | LLVM                            | Apple 默认使用   |
  | 汇编器         | `as`（LLVM 内部）   | Xcode 工具链                    | 自动使用         |
  | 链接器         | `ld`（ld64）        | Xcode 工具链                    | Apple 的 ld 版本 |
  | 构建器（可选） | `make` / `ninja`    | Homebrew 可装                   | 执行构建任务     |
  | 生成器         | `cmake`             | Homebrew (`brew install cmake`) | 项目生成工具     |

- Windows 平台（两个选择）

  1. MSVC 工具链（Visual Studio）安装方式

     - 安装 **Visual Studio**（勾选 C++ 开发组件）

     - 或安装 **Visual Studio Build Tools**

     工具路径（命令行需用 `x64 Native Tools Command Prompt`）：

     | 工具   | 命令               | 路径大致位置                                     |
     | ------ | ------------------ | ------------------------------------------------ |
     | 编译器 | `cl.exe`           | `VC\Tools\MSVC\<version>\bin\Hostx64\x64\cl.exe` |
     | 汇编器 | `ml.exe`           | 同上或 `VC\Tools\MSVC\...\bin\Hostx64\x64`       |
     | 链接器 | `link.exe`         | 同上                                             |
     | 构建器 | `msbuild` / `.sln` | 用于构建 VS 工程                                 |
     | 生成器 | `cmake`            | 单独安装或选项中勾选安装                         |

  2. MinGW / MSYS2 工具链

     - 安装 [MSYS2](https://www.msys2.org/) 或 [MinGW-w64](https://www.mingw-w64.org/)

     - 安装完成后在 MSYS2 环境中执行：

       ```
       pacman -S mingw-w64-x86_64-toolchain cmake make ninja
       ```

     安装后提供的工具

     | 工具   | 命令             | 来源                |
     | ------ | ---------------- | ------------------- |
     | 编译器 | `gcc` / `g++`    | MinGW               |
     | 汇编器 | `as`             | Binutils            |
     | 链接器 | `ld`             | Binutils            |
     | 构建器 | `make` / `ninja` | GNU Make 或 Ninja   |
     | CMake  | `cmake`          | 官方包或 MSYS2 提供 |

