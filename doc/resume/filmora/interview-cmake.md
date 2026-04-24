interview-cmake.md
# CMake 技术面试知识点

> 基于 Wondershare Filmora Windows/macOS 客户端真实 CMake 构建系统整理
> 项目路径：`E:\dcproject\filmora11-win`

---

## 目录

1. [CMake 基础概念](#1-cmake-基础概念)
2. [project 与版本管理](#2-project-与版本管理)
3. [option 与 CACHE 变量](#3-option-与-cache-变量)
4. [add_library 与 add_executable](#4-add_library-与-add_executable)
5. [target_link_libraries](#5-target_link_libraries)
6. [target_include_directories](#6-target_include_directories)
7. [target_compile_options 与 add_compile_options](#7-target_compile_options-与-add_compile_options)
8. [target_compile_definitions 与 add_compile_definitions](#8-target_compile_definitions-与-add_compile_definitions)
9. [生成器表达式（Generator Expressions）](#9-生成器表达式generator-expressions)
10. [条件判断与跨平台处理](#10-条件判断与跨平台处理)
11. [add_subdirectory 与模块化组织](#11-add_subdirectory-与模块化组织)
12. [find_package](#12-find_package)
13. [自定义 function 与 macro](#13-自定义-function-与-macro)
14. [configure_file 与文件生成](#14-configure_file-与文件生成)
15. [Qt 与 CMake 集成（AUTOMOC/AUTORCC/AUTOUIC）](#15-qt-与-cmake-集成automocautorccautouic)
16. [预编译头（Precompiled Headers）](#16-预编译头precompiled-headers)
17. [install 规则](#17-install-规则)
18. [FetchContent 外部依赖管理](#18-fetchcontent-外部依赖管理)
19. [set_target_properties 与属性系统](#19-set_target_properties-与属性系统)
20. [输出目录与构建类型](#20-输出目录与构建类型)

---

## 1. CMake 基础概念

### 核心术语

| 术语 | 说明 |
|------|------|
| **CMakeLists.txt** | CMake 构建描述文件，每个目录一个 |
| **Target** | 构建产物（可执行文件、库、自定义目标） |
| **Generator** | CMake 生成的构建系统类型（Visual Studio、Ninja、Xcode 等） |
| **Build tree** | 构建目录（外部构建，源代码目录以外） |
| **Source tree** | 源代码目录 |
| **IMPORTED Target** | 描述已存在的外部库（不参与编译）的 target |
| **ALIAS Target** | 为已有 target 创建别名 |

### 运行 CMake 的两个阶段

```
1. 配置阶段（Configure）：执行 CMakeLists.txt，生成构建系统文件（.sln / Makefile / build.ninja）
2. 构建阶段（Build）：    调用实际构建工具（MSBuild / make / ninja）编译链接
```

### 项目实际构建命令

```bat
:: 文件：cmake-vs2017.bat
mkdir VSProject
cd VSProject
del CMakeCache.txt
cmake -DPRJ_GLOBAL_KEYWORD="Qt4VSv1.0" ^
      -DPRJ_FOR_DEV=ON ^
      -DFILMORA_USE_ANTIPIRACY=ON ^
      -G "Visual Studio 15 2017 Win64" ^
      -DcustomQtDir="C:\Qt\5.15.2\msvc2019_64" ^
      -DUSE_DEBUG_TOOL=OFF ^
      ..
pause
```

命令行传入 `-DVAR=VALUE` 即可覆盖 CMakeLists.txt 中的 `option` 或 `CACHE` 变量。

### 常见面试题

**Q：CMake 的 `configure` 和 `build` 阶段分别做什么？**

A：`configure` 阶段执行所有 CMakeLists.txt，检查编译器/依赖，生成 `CMakeCache.txt` 和目标构建系统文件（如 `.sln`）；`build` 阶段由实际构建工具（MSBuild/ninja）调用编译器链接器完成编译。两者分离的好处是：修改源码只需重新 build，修改 CMakeLists.txt 才需重新 configure。

---

## 2. project 与版本管理

### 项目代码示例

```cmake
# 文件：CMakeLists.txt（根目录）

cmake_minimum_required(VERSION 3.13)

# 手动维护的版本号变量
set(FILMORA_VERSION_MAJOR 14)
set(FILMORA_VERSION_MINOR 7)
set(FILMORA_VERSION_PATCH 4)
set(FILMORA_VERSION_TWEAK 0)
set(FILMORA_VERSION
    "${FILMORA_VERSION_MAJOR}.${FILMORA_VERSION_MINOR}.${FILMORA_VERSION_PATCH}.${FILMORA_VERSION_TWEAK}")

# project 声明 + 版本注入（自动设置 PROJECT_VERSION_MAJOR 等变量）
project(Filmora VERSION ${FILMORA_VERSION})
```

**子模块版本继承（Filmora-module.cmake）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake
macro(set_version)
    # 子模块未显式声明版本时，沿用根项目的版本号
    if(NOT PROJECT_VERSION)
        set(PROJECT_VERSION_MAJOR ${FILMORA_VERSION_MAJOR})
        set(PROJECT_VERSION_MINOR ${FILMORA_VERSION_MINOR})
        set(PROJECT_VERSION_PATCH ${FILMORA_VERSION_PATCH})
        set(PROJECT_VERSION_TWEAK ${FILMORA_VERSION_TWEAK})
        set(PROJECT_VERSION        ${FILMORA_VERSION})
    endif()
    if(NOT PROJECT_VERSION_TWEAK)
        set(PROJECT_VERSION_TWEAK ${FILMORA_VERSION_TWEAK})
    endif()
    # ...
endmacro()
```

**Xcode 平台版本属性注入**

```cmake
# 文件：Src/Filmora/CMakeLists.txt（macOS）
# 将 CMake 版本变量写入 Xcode 构建设置
set_target_properties(${PROJECT_NAME} PROPERTIES
    XCODE_ATTRIBUTE_MARKETING_VERSION
        "${FILMORA_VERSION_MAJOR}.${FILMORA_VERSION_MINOR}.${FILMORA_VERSION_PATCH}"
    XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION
        "${FILMORA_VERSION_MAJOR}.${FILMORA_VERSION_MINOR}.${FILMORA_VERSION_PATCH}.${FILMORA_VERSION_TWEAK}"
)
```

### 常见面试题

**Q：`cmake_minimum_required` 的作用是什么？**

A：声明本项目要求的最低 CMake 版本，同时激活该版本对应的 CMake Policy（行为规范）。若系统 CMake 版本低于此值则报错终止。应始终放在 CMakeLists.txt 第一行，在 `project()` 之前。

**Q：`project()` 会设置哪些变量？**

A：`project(Filmora VERSION 14.7.4.0)` 会自动设置：
- `PROJECT_NAME` = `Filmora`
- `PROJECT_VERSION` = `14.7.4.0`
- `PROJECT_VERSION_MAJOR/MINOR/PATCH/TWEAK` = `14/7/4/0`
- `CMAKE_PROJECT_NAME`（仅顶层 project 设置）

---

## 3. option 与 CACHE 变量

### 核心区别

| 命令 | 类型 | 说明 |
|------|------|------|
| `option(VAR "doc" ON/OFF)` | BOOL | 布尔开关，GUI 中显示为复选框 |
| `set(VAR val CACHE TYPE "doc")` | 任意类型 | 持久化变量，存入 CMakeCache.txt |
| `set(VAR val CACHE TYPE "doc" FORCE)` | — | 强制覆盖缓存中已有的值 |

### 项目代码示例

**根目录所有 option**

```cmake
# 文件：CMakeLists.txt
option(FILMORA_USE_ANTIPIRACY  "Enable AntiPiracy."             ON)
option(FILMORA_USE_LINGUIST    "Disable LinguistTools Update."  OFF)
option(BETA                    "Set Beta Icon and Title"        OFF)
option(USE_GTEST               "Enable GTest"                   OFF)
option(FILMORA_FOR_MIAOYING    "For Miao ying."                 OFF)
option(FILMORA_FOR_DESIGNER    "For Designer Version."          OFF)
option(DOWNLOAD_VBL_WES        "auto download and extract vbl wes" ON)
option(USE_DEBUG_TOOL          "Use debug tools"                OFF)
option(APP_STORE               "appstore version."              OFF)
```

**FORCE 强制覆盖（平台特殊逻辑）**

```cmake
# 文件：CMakeLists.txt
# Windows/Mac 构建时强制关闭防盗版（只在特定流水线开启）
if(WIN32)
    set(FILMORA_USE_ANTIPIRACY OFF CACHE BOOL "Enable AntiPiracy." FORCE)
endif()
```

**CACHE 变量作输出目录配置**

```cmake
# 文件：CMakeLists.txt
# 多配置生成器（VS/Xcode）使用生成器表达式
set(OUTPUT_DIR "${_OUTPUT_DIR}/$<CONFIG>" CACHE STRING "Bin Dir" FORCE)

# 单配置生成器（Ninja/Make）使用固定路径
set(OUTPUT_DIR ${_OUTPUT_DIR} CACHE STRING "Bin Dir" FORCE)
```

**平台条件控制 CACHE 变量**

```cmake
# 文件：Tools/CustomizedTool/CMakeLists.txt
# 移动/嵌入平台不构建测试
if(IOS OR ANDROID OR WINDOWS_STORE OR WINDOWS_PHONE)
    set(BUILD_TESTS OFF CACHE BOOL "Build tests.")
else()
    set(BUILD_TESTS ON  CACHE BOOL "Build tests.")
endif()
```

### 常见面试题

**Q：`option` 和 `set(... CACHE BOOL ...)` 的区别？**

A：`option` 是 `set(VAR OFF CACHE BOOL "doc")` 的语法糖，专门用于 BOOL 类型。区别在于：`option` 若 CMakeCache.txt 中已存在该变量则**不覆盖**（尊重用户已有设置）；`set(... CACHE ... FORCE)` 则无论如何都覆盖。

**Q：`CACHE` 变量和普通变量的作用域有何不同？**

A：普通变量有作用域（函数/目录），子目录看不到父目录定义的普通变量（除非用 `PARENT_SCOPE`）；`CACHE` 变量是全局的，存储在 `CMakeCache.txt`，整个构建系统任意位置可访问。

---

## 4. add_library 与 add_executable

### 核心用法

```cmake
add_library(<name> [STATIC|SHARED|MODULE|INTERFACE|OBJECT] [sources...])
add_executable(<name> [WIN32|MACOSX_BUNDLE] [sources...])
add_library(<name> ALIAS <target>)          # 别名
add_library(<name> SHARED IMPORTED)        # 导入已有库
```

### 项目代码示例

**动态库（模块标准模式）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake（AppendModulesBasic 函数内）
add_library(${PROJECT_NAME} SHARED
    ${prj_common_headerFiles}   # 公共接口头文件（PUBLIC）
    ${prj_headerFiles}          # 私有头文件
    ${prj_srcFiles}             # 源文件
    ${prj_form_files}           # Qt .ui 文件
    ${prj_translation_files}    # Qt .ts 翻译文件
    ${prj_resource_files}       # .rc 资源文件
    ${prj_resource_in_File}     # .rc.in 模板（中间产物）
)

# 同时注册 ALIAS，让链接方使用命名空间前缀
add_library(FILMORA::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
```

**可执行文件（平台差异）**

```cmake
# 文件：Src/Filmora/CMakeLists.txt
# MSVC: WIN32 = Windows 子系统（无控制台窗口）
# Apple: MACOSX_BUNDLE = 生成 .app 包
if(MSVC)
    set(app_format WIN32)
elseif(APPLE)
    set(app_format MACOSX_BUNDLE)
endif()

add_executable(${PROJECT_NAME} ${app_format}
    ${prj_common_headerFiles}
    ${prj_srcFiles}
    # 生成器表达式：仅 Windows 编译这两个文件
    $<$<PLATFORM_ID:Windows>:FSMBIOSReader.h>
    $<$<PLATFORM_ID:Windows>:FSMBIOSReader.cpp>
    main.cpp
    FilmoraApp.h
    FilmoraApp.cpp
    # ...
)
```

**IMPORTED 库（描述预编译的第三方库）**

```cmake
# 文件：3rdparty/WsPerformance/WsPerformanceConfig.cmake
add_library(WsPerformance::WsPerformance SHARED IMPORTED)
set_target_properties(WsPerformance::WsPerformance PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_LIB_DIR}/include"
    IMPORTED_LOCATION             "${_LIB_DIR}/bin/WsPerformance.dll"
    IMPORTED_IMPLIB               "${_LIB_DIR}/lib/WsPerformance.lib"
)
```

### 常见面试题

**Q：`STATIC`、`SHARED`、`INTERFACE` 库的区别？**

A：
- `STATIC`：静态库（`.lib`/`.a`），编译期链入，产物更大但部署简单
- `SHARED`：动态库（`.dll`/`.so`），运行期加载，多进程共享；Filmora 所有模块均用 SHARED
- `INTERFACE`：不产生编译产物，只传递属性（include 路径、编译定义等）给依赖它的 target；常用于 header-only 库

**Q：ALIAS target 有什么用？**

A：为 target 添加命名空间前缀（如 `FILMORA::FFWidgets`），让使用方的 `target_link_libraries` 代码风格统一，且能与 `find_package` 返回的 IMPORTED target 保持一致命名风格，避免同名冲突。

**Q：`WIN32` 和 `MACOSX_BUNDLE` 修饰符的作用？**

A：
- `WIN32`：设置 Windows 链接子系统为 `/SUBSYSTEM:WINDOWS`，程序入口为 `WinMain`，不弹出控制台窗口
- `MACOSX_BUNDLE`：生成 `.app` 目录结构（`Contents/MacOS/`、`Contents/Info.plist` 等），macOS GUI 应用必须使用

---

## 5. target_link_libraries

### 传播关键字

| 关键字 | 含义 |
|--------|------|
| `PRIVATE` | 仅自身编译需要，不传播给依赖者 |
| `PUBLIC` | 自身和依赖者都需要（传播） |
| `INTERFACE` | 只传播给依赖者，自身不直接使用 |

### 项目代码示例

**主程序链接（Filmora.exe）**

```cmake
# 文件：Src/Filmora/CMakeLists.txt
target_link_libraries(${PROJECT_NAME} PRIVATE
    # Qt 模块
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Xml
    Qt5::Network
    Qt5::WebSockets

    # 内部模块（ALIAS target）
    FF::FFWidgets
    FILMORA::FTimelineView
    FILMORA::FMediaLibraryView
    FILMORA::FPropertyPanelView
    FILMORA::FExportView

    # 平台条件链接（生成器表达式）
    $<$<PLATFORM_ID:Windows>:FILMORA::FFWsRegister>
    $<$<PLATFORM_ID:Windows>:FILMORA::FFWsAP>
    $<$<PLATFORM_ID:Darwin>:FILMORA::FFMacToQtSyncData>

    # 外部 IMPORTED 库
    WsPerformance::WsPerformance

    # 调试版本链接额外库（生成器表达式）
    $<${is_debug_or_FilmoraTest_in_APPLE}:FILMORA::FNativeSettings>

    # 直接路径链接（预编译 .lib）
    $<$<CONFIG:Debug>:${filmora_common_lib_dir}/Debug/WsAP-FilmoraX.lib>
    $<$<CONFIG:Release>:${filmora_common_lib_dir}/WsAP-FilmoraX.lib>
)
```

**测试目标链接 GTest**

```cmake
# 文件：3rdparty/FilmoraFrameworkPlatform/Tests/FFResourceConverter/CMakeLists.txt
target_link_libraries(${TEST_PROJECT_NAME}
    PRIVATE
    GTest::GTest
)
```

**工具程序链接 Qt**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake（Insert_Tool_Basic 函数）
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
)
```

### 常见面试题

**Q：`PRIVATE` / `PUBLIC` / `INTERFACE` 在实际项目中如何选择？**

A：规则：
- 依赖只在 `.cpp` 中使用（不在 `.h` 中 include）→ `PRIVATE`
- 依赖在 `.h` 中 include（使用者也需要它）→ `PUBLIC`
- 本身是 header-only，不编译，只传递依赖 → `INTERFACE`

Filmora 各模块对 Qt 和内部库使用 `PRIVATE`，因为接口头文件通过 `prj_common_include` 单独暴露，不通过 `target_link_libraries` 的传播机制。

**Q：直接写库文件路径和用 target 名称链接有何区别？**

A：用 target 名称（`Qt5::Widgets`）是"现代 CMake"风格，target 会自动携带 include 路径、编译定义等属性，一行搞定；直接写路径（`${lib_dir}/foo.lib`）需要手动补充 include 和 defines，且不可移植，是"旧式 CMake"做法。项目中两者都有，老旧第三方库用路径，内部模块统一用 target。

---

## 6. target_include_directories

### 项目代码示例

**自动从头文件列表推导 include 路径（Filmora-module.cmake）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake（set_target 宏内）

# 从私有头文件列表提取所有所在目录
foreach(child ${prj_headerFiles})
    get_filename_component(absolute_path ${child} ABSOLUTE)
    get_filename_component(parent ${absolute_path} DIRECTORY)
    list(APPEND prj_include_paths ${parent})
endforeach()
list(REMOVE_DUPLICATES prj_include_paths)

# 从公共头文件列表提取公共 include 目录
foreach(common_child ${prj_common_headerFiles})
    get_filename_component(absolute_path ${common_child} ABSOLUTE)
    get_filename_component(common_parent ${absolute_path} DIRECTORY)
    list(APPEND prj_common_include ${common_parent})
endforeach()
list(REMOVE_DUPLICATES prj_common_include)

# PRIVATE：仅本模块内部可见
# PUBLIC：依赖此模块的 target 也能看到（接口头文件目录）
target_include_directories(${PROJECT_NAME}
    PRIVATE ${prj_include_paths}
    PUBLIC  ${prj_common_include}
)
```

### 常见面试题

**Q：`include_directories` 和 `target_include_directories` 的区别？**

A：`include_directories` 是"旧式 CMake"，对当前目录及子目录所有 target 全局生效，容易污染；`target_include_directories` 精确到单个 target，可控制传播性（PRIVATE/PUBLIC/INTERFACE），是"现代 CMake"的推荐做法。

---

## 7. target_compile_options 与 add_compile_options

### 两者区别

- `add_compile_options`：对**当前目录及所有子目录**的 target 全局生效
- `target_compile_options`：仅对指定 target 生效，可控制传播性

### 项目代码示例

**全局编译选项（根目录）**

```cmake
# 文件：CMakeLists.txt

# 全局：MSVC 强制 UTF-8 源文件编码（C 和 C++ 分别设置）
add_compile_options($<$<C_COMPILER_ID:MSVC>:/utf-8>)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

# Windows 专属：将 C4715（不是所有路径都有返回值）升级为错误
if(WIN32)
    add_compile_options("/we4715")
endif()
```

**每模块编译选项（set_target 宏）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake（set_target 宏）
if(MSVC)
    # /MP：多进程并行编译
    target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
    # /Zi：生成完整调试信息（PDB 文件）
    target_compile_options(${PROJECT_NAME} PRIVATE "/Zi")
    # 按配置区分警告等级（生成器表达式）
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/W3>      # Debug: 警告等级 3
        $<$<CONFIG:Debug>:/wd4250>  # 禁用 C4250（继承歧义警告）
        $<$<CONFIG:Debug>:/wd4996>  # 禁用 C4996（弃用函数警告）
        $<$<CONFIG:Release>:/W0>    # Release: 关闭所有警告
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Clang-cl 模式：关闭全部警告（第三方代码用）
        target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-everything")
    endif()
endif()

if(APPLE)
    # Release 模式 macOS：最高优化 + 保留调试符号（方便崩溃分析）
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g")
endif()
```

**链接选项（target_link_options）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake
# 忽略 LNK4099（找不到 PDB）链接器警告
target_link_options(${PROJECT_NAME} PRIVATE "/ignore:4099")

# Release 构建同样生成 PDB（方便线上崩溃定位）
target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/DEBUG>)

# 文件：Src/Filmora/CMakeLists.txt
# Debug 构建使用控制台子系统（便于看 qDebug 输出）
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>)
```

**MSVC 多进程编译（CXX_FLAGS 旧式写法）**

```cmake
# 文件：Tools/CustomizedTool/CMakeLists.txt
# 较老的写法，直接修改全局 flags 字符串
set(CMAKE_CXX_FLAGS_DEBUG        "${CMAKE_CXX_FLAGS_DEBUG}        /MP")
set(CMAKE_CXX_FLAGS_RELEASE      "${CMAKE_CXX_FLAGS_RELEASE}      /MP")
set(CMAKE_CXX_FLAGS_MINSIZEREL   "${CMAKE_CXX_FLAGS_MINSIZEREL}   /MP")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MP")
```

### 常见面试题

**Q：`/MP`、`/Zi`、`/W3`、`/wd4250` 这些 MSVC 选项分别是什么意思？**

A：
- `/MP`：Multi-Process 并行编译，显著加快大型项目构建速度
- `/Zi`：生成完整程序数据库（PDB）调试信息
- `/W3`：警告等级 3（推荐级别，报告大多数有用警告）
- `/W0`：关闭所有警告（Release 常用，避免噪音）
- `/wd4250`：禁用特定警告（4250 = 通过支配继承接口），多继承场景常见

**Q：`add_compile_options` 和直接改 `CMAKE_CXX_FLAGS` 哪种更好？**

A：`add_compile_options` 更好：它附加到已有 flags，不会意外覆盖；支持生成器表达式；作用域可控（只影响当前目录）。直接改 `CMAKE_CXX_FLAGS` 是字符串操作，容易出错，且全局污染，是旧式做法。

---

## 8. target_compile_definitions 与 add_compile_definitions

### 项目代码示例

**全局预处理器宏（根目录）**

```cmake
# 文件：CMakeLists.txt

# 字符集宏（仅 Windows）
add_compile_definitions($<$<BOOL:${WIN32}>:UNICODE>)
add_compile_definitions($<$<BOOL:${WIN32}>:_UNICODE>)

# 测试宏（Debug 配置 或 FilmoraTest 选项开启时）
set(is_debug_or_FilmoraTest $<OR:$<CONFIG:Debug>,$<BOOL:${FilmoraTest}>>)
add_compile_definitions($<${is_debug_or_FilmoraTest}:FILMORA_TEST>)

# Release 屏蔽 Qt 调试输出（减少 I/O 开销）
add_compile_definitions($<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>)
add_compile_definitions($<$<CONFIG:Release>:QT_NO_INFO_OUTPUT>)
add_compile_definitions($<$<CONFIG:Release>:QT_NO_WARNING_OUTPUT>)

# 版本开关宏
add_compile_definitions($<$<BOOL:${BETA}>:BETA>)
add_compile_definitions($<$<BOOL:${FILMORA_FOR_MIAOYING}>:FILMORA_FOR_MIAOYING>)
add_compile_definitions($<$<BOOL:${APP_STORE}>:APP_STORE>)

# Qt 字面量问题规避
add_compile_definitions(QT_NO_UNICODE_LITERAL)
```

**per-target 导出宏（Filmora-module.cmake）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake（set_target 宏）
# 为每个模块自动定义 XXXX_LIB 宏，用于 __declspec(dllexport/dllimport) 控制
target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<UPPER_CASE:${PROJECT_NAME}>_LIB)
# 例如 PROJECT_NAME=FFWidgets → 定义 FFWIDGETS_LIB
# 头文件中：
#   #ifdef FFWIDGETS_LIB
#   #  define FFWIDGETS_EXPORT __declspec(dllexport)
#   #else
#   #  define FFWIDGETS_EXPORT __declspec(dllimport)
#   #endif
```

### 常见面试题

**Q：`add_definitions` 和 `add_compile_definitions` 有什么区别？**

A：`add_definitions` 是旧命令，参数需要手动加 `-D` 前缀（如 `add_definitions(-DFOO)`），且会影响链接器；`add_compile_definitions`（CMake 3.12+）是新命令，自动处理 `-D` 前缀，只影响编译器，更安全。

---

## 9. 生成器表达式（Generator Expressions）

生成器表达式在 CMake **构建阶段**（generate 时）才求值，而非配置阶段。语法：`$<...>`

### 常见形式

| 表达式 | 含义 |
|--------|------|
| `$<CONFIG:Debug>` | 当前配置是否为 Debug |
| `$<PLATFORM_ID:Windows>` | 平台是否为 Windows |
| `$<BOOL:${VAR}>` | 变量是否为真 |
| `$<AND:expr1,expr2>` | 逻辑与 |
| `$<OR:expr1,expr2>` | 逻辑或 |
| `$<NOT:expr>` | 逻辑非 |
| `$<TARGET_FILE:tgt>` | target 的输出文件路径 |
| `$<TARGET_FILE_DIR:tgt>` | target 的输出目录 |
| `$<UPPER_CASE:str>` | 字符串转大写 |
| `$<$<cond>:value>` | 条件为真则展开为 value，否则为空 |

### 项目代码示例

**按配置选择链接库**

```cmake
# 文件：CMakeLists.txt / Src/Filmora/CMakeLists.txt
# Debug 和 Release 链接不同后缀的 .lib（因为 CMAKE_DEBUG_POSTFIX="D"）
$<$<CONFIG:Debug>:${filmora_common_lib_dir}/Debug/WsAP-FilmoraX.lib>
$<$<CONFIG:Release>:${filmora_common_lib_dir}/WsAP-FilmoraX.lib>
```

**按平台选择源文件和链接库**

```cmake
# 文件：Src/Filmora/CMakeLists.txt
# 源文件：仅 Windows 编译这两个文件
$<$<PLATFORM_ID:Windows>:FSMBIOSReader.h>
$<$<PLATFORM_ID:Windows>:FSMBIOSReader.cpp>

# 链接库：仅 Windows
$<$<PLATFORM_ID:Windows>:FILMORA::FFWsRegister>
$<$<PLATFORM_ID:Windows>:FILMORA::FFWsAP>

# 链接库：仅 macOS
$<$<PLATFORM_ID:Darwin>:FILMORA::FFMacToQtSyncData>
```

**复合条件（AND / OR）**

```cmake
# 文件：CMakeLists.txt
# 变量：is_debug_or_FilmoraTest = (Config==Debug OR FilmoraTest==ON)
set(is_debug_or_FilmoraTest
    $<OR:$<CONFIG:Debug>,$<BOOL:${FilmoraTest}>>)

# 再组合：Apple 平台 AND (Debug OR FilmoraTest)
set(is_debug_or_FilmoraTest_in_APPLE
    $<AND:$<BOOL:${APPLE}>,${is_debug_or_FilmoraTest}>)

# 使用：满足条件才添加编译宏
add_compile_definitions($<${is_debug_or_FilmoraTest}:FILMORA_TEST>)

# 使用：满足条件才链接 FNativeSettings
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<${is_debug_or_FilmoraTest_in_APPLE}:FILMORA::FNativeSettings>)
```

**post-build 命令中使用 `$<TARGET_FILE_DIR:...>`**

```cmake
# 文件：Src/Filmora/CMakeLists.txt
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${GIT_BASH} ${COPY_SH}
        $<CONFIG>
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<$<PLATFORM_ID:Darwin>:..>"
        ${filmora_platform}
        "$<TARGET_FILE_DIR:Qt5::Core>/$<$<PLATFORM_ID:Darwin>:../..>"
        ${CMAKE_SOURCE_DIR}
        ${APP_STORE}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
)
```

**set_target 中用于导出宏自动大写**

```cmake
# $<UPPER_CASE:${PROJECT_NAME}> 在生成阶段展开为 PROJECT_NAME 的大写
target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<UPPER_CASE:${PROJECT_NAME}>_LIB)
```

### 常见面试题

**Q：生成器表达式为什么不能用 `if()` 代替？**

A：`if()` 在 CMake **配置阶段**求值，此时"当前配置"（Debug/Release）未知（多配置生成器如 Visual Studio 单次 configure 生成两种配置）；生成器表达式在 **generate 阶段**求值，此时才能区分 Debug/Release，因此多配置场景必须用生成器表达式。

**Q：`$<CONFIG:Debug>` 和 `CMAKE_BUILD_TYPE STREQUAL "Debug"` 的区别？**

A：`CMAKE_BUILD_TYPE` 只在单配置生成器（Ninja/Make）中有效；多配置生成器（VS/Xcode）中 `CMAKE_BUILD_TYPE` 为空，必须用 `$<CONFIG:Debug>`。Filmora 项目两种生成器都支持，因此全局宏统一用生成器表达式。

---

## 10. 条件判断与跨平台处理

### 项目代码示例

**平台检测与变量设置**

```cmake
# 文件：CMakeLists.txt
if(WIN32)
    set(FILMORA_USE_ANTIPIRACY OFF CACHE BOOL "Enable AntiPiracy." FORCE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64")
        set(filmora_platform "win")
    endif()
elseif(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")  # 最低支持 macOS 版本
    set(FILMORA_USE_ANTIPIRACY OFF CACHE BOOL "Enable AntiPiracy." FORCE)
    enable_language(OBJC OBJCXX)              # 启用 Objective-C/C++ 支持

    # Apple Silicon 通用二进制支持
    if(CMAKE_OSX_ARCHITECTURES MATCHES "universal")
        set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
        set(filmora_platform "universal")
    elseif(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        set(filmora_platform "arm64")
    else()
        set(CMAKE_OSX_ARCHITECTURES "x86_64")
        set(filmora_platform "mac")
    endif()
endif()

# 两个平台都不满足：致命错误
if(NOT filmora_platform)
    message(FATAL_ERROR "Now is '${CMAKE_SYSTEM_NAME}' OS. not supported yet.")
endif()
```

**平台相关子目录（仅 Windows/macOS 的模块）**

```cmake
# 文件：CMakeLists.txt
if(WIN32)
    add_subdirectory(Src/FUninstaller)   # Windows 卸载程序
    add_subdirectory(Src/FFWsRegister)   # Windows 注册表
    add_subdirectory(Src/FFWsAP)         # Windows 防盗版
elseif(APPLE)
    add_subdirectory(Src/FNativeSettings) # macOS 原生设置
endif()
```

**编译器 ID 判断**

```cmake
# 文件：Tools/CustomizedTool/CMakeLists.txt
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR IOS)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++20")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # GCC 选项
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/bigobj)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
endif()
```

**macOS 版本号运行期检测**

```cmake
# 文件：CMakeLists.txt（App Store 构建时）
execute_process(
    COMMAND sw_vers -productVersion
    OUTPUT_VARIABLE MACOS_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "." ";" MACOS_VERSION_LIST ${MACOS_VERSION})
list(GET MACOS_VERSION_LIST 0 MACOS_VERSION_MAJOR_STR)

if("${MACOS_VERSION_MAJOR_STR}" STRLESS "14" AND
   ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL ""))
    set(INCLUDES_FILMORA_WIDGET false)
else()
    set(INCLUDES_FILMORA_WIDGET true)
endif()
```

**App 包标识符按品牌/渠道区分**

```cmake
# 文件：CMakeLists.txt
if    (APP_STORE     AND     FILMORA_FOR_MIAOYING)
    set(Filmora_BUNDLE_IDENTIFIER "cn.wondershare.miaoying.appstore")
elseif(APP_STORE     AND NOT FILMORA_FOR_MIAOYING)
    set(Filmora_BUNDLE_IDENTIFIER "com.wondershare.filmora-app")
elseif(NOT APP_STORE AND     FILMORA_FOR_MIAOYING)
    set(Filmora_BUNDLE_IDENTIFIER "cn.wondershare.miaoying")
else()
    set(Filmora_BUNDLE_IDENTIFIER "com.wondershare.filmoramacos")
endif()
```

### 常见面试题

**Q：CMake 中判断平台的变量有哪些？**

A：常用内置变量：
- `WIN32`：Windows 平台（包括 64 位）
- `APPLE`：macOS 或 iOS
- `UNIX`：所有 Unix-like（Linux、macOS、AIX 等）
- `LINUX`：仅 Linux（CMake 3.25+）
- `MSVC`：Microsoft Visual C++ 编译器
- `CMAKE_SYSTEM_NAME`：操作系统名字符串（`Windows`、`Darwin`、`Linux`）
- `CMAKE_CXX_COMPILER_ID`：编译器 ID 字符串（`MSVC`、`Clang`、`GNU`）

**Q：`message(STATUS ...)` 和 `message(FATAL_ERROR ...)` 的区别？**

A：`STATUS` 输出信息性消息（前缀 `--`），继续执行；`FATAL_ERROR` 输出错误消息并**立即终止** CMake 配置。其他级别还有 `WARNING`（警告继续）、`SEND_ERROR`（继续执行但最终失败）、`VERBOSE`（仅 CMake 3.15+ 详细模式显示）。

---

## 11. add_subdirectory 与模块化组织

### 项目代码示例

**根目录组织大量子模块**

```cmake
# 文件：CMakeLists.txt

# 第三方框架（指定独立的二进制目录，避免与主项目混淆）
add_subdirectory(3rdparty/FilmoraFrameworkPlatform ${CMAKE_BINARY_DIR}/3rd)

# 功能模块（顺序即编译依赖顺序）
add_subdirectory(Src/FFAppSettings)
add_subdirectory(Src/FFAppLicense)
add_subdirectory(Src/FFMediaLibrary)
add_subdirectory(Src/FFAudioMixer)
add_subdirectory(Src/FMediaLibraryView)
add_subdirectory(Src/FTimelineView)
# ... 数十个子模块 ...
add_subdirectory(Src/Filmora)          # 主程序放最后（依赖所有模块）

# 平台专属子目录
if(WIN32)
    add_subdirectory(Src/FUninstaller)
    add_subdirectory(Src/FFWsRegister)
elseif(APPLE)
    add_subdirectory(Src/FNativeSettings)
endif()
```

**子模块的标准 CMakeLists.txt 结构**

```cmake
# 典型子模块（如 Src/FFAppSettings/CMakeLists.txt）

project(FFAppSettings)          # 模块独立 project

# 文件收集
set(prj_common_headerFiles      # 接口头文件（放 Include/ 下）
    ../../Include/FFAppSettings/IFFAppSettings.h
    ../../Include/FFAppSettings/FFAppSettingsDefine.h
)
set(prj_headerFiles             # 私有头文件
    FFAppSettings.h
    FFAppSettingsPrivate.h
)
set(prj_srcFiles                # 源文件
    FFAppSettings.cpp
)

# 链接依赖
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    FF::FFCore
)

# 调用标准模块宏（内含 add_library、set_target、set_install 等）
AppendModulesBasic("")
```

### 常见面试题

**Q：`add_subdirectory` 的第二个参数 `binary_dir` 有什么用？**

A：指定子目录的构建产物目录（默认与源目录同名，位于 `CMAKE_BINARY_DIR` 下）。当子目录不在源树内（如 `3rdparty/`）或需要隔离构建产物时使用，如：`add_subdirectory(3rdparty/FilmoraFrameworkPlatform ${CMAKE_BINARY_DIR}/3rd)` 让第三方库的所有构建产物集中在 `build/3rd/` 下，方便管理。

---

## 12. find_package

### 两种查找模式

| 模式 | 说明 | 文件名 |
|------|------|--------|
| **Module 模式** | CMake 内置或 `CMAKE_MODULE_PATH` 中的 `FindXXX.cmake` | `FindQt5.cmake` |
| **Config 模式** | 库自带的 `XXXConfig.cmake` 或 `xxx-config.cmake` | `Qt5Config.cmake` |

### 项目代码示例

**查找 Qt5（Config 模式）**

```cmake
# 文件：Src/Filmora/CMakeLists.txt
# 通过 customQtDir 指定 Qt 安装路径
if(customQtDir AND EXISTS ${customQtDir})
    set(ENV{QTDIR} ${customQtDir})
endif()
list(APPEND CMAKE_PREFIX_PATH $ENV{QTDIR})  # Qt Config 文件在此目录下

# 查找 Qt5 并指定所需组件（REQUIRED = 找不到就报错）
find_package(Qt5
    COMPONENTS Core Gui Xml Widgets Network WebSockets
    REQUIRED)
```

**查找自定义第三方库（WsPerformance）**

```cmake
# 文件：Src/Filmora/CMakeLists.txt
find_package(WsPerformance REQUIRED)
# WsPerformance 库提供了 WsPerformanceConfig.cmake
# 成功后：WsPerformance::WsPerformance target 可用
```

**WsPerformanceConfig.cmake 实现**

```cmake
# 文件：3rdparty/WsPerformance/WsPerformanceConfig.cmake
add_library(WsPerformance::WsPerformance SHARED IMPORTED)
set_target_properties(WsPerformance::WsPerformance PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_LIB_DIR}/include"
    IMPORTED_LOCATION             "${_LIB_DIR}/bin/WsPerformance.dll"
    IMPORTED_IMPLIB               "${_LIB_DIR}/lib/WsPerformance.lib"
)
```

**查找 Git（可选）**

```cmake
find_package(Git QUIET)  # QUIET = 找不到不报错，通过 Git_FOUND 判断
if(Git_FOUND)
    # 用 GIT_EXECUTABLE 执行 git 命令
endif()
```

### 常见面试题

**Q：`find_package(Qt5 REQUIRED COMPONENTS Widgets)` 之后能用什么变量？**

A：成功后可用：
- `Qt5_FOUND` = TRUE
- `Qt5Widgets_FOUND` = TRUE
- `Qt5::Widgets` target（现代 CMake 推荐方式）
- `Qt5Widgets_INCLUDE_DIRS`（旧式）
- `Qt5Widgets_LIBRARIES`（旧式）

现代 CMake 直接 `target_link_libraries(myTarget Qt5::Widgets)` 即可，include 路径自动传播。

**Q：如何让 `find_package` 找到非标准路径的库？**

A：有三种方式：
1. 设置 `CMAKE_PREFIX_PATH`（项目中 Qt 的做法）
2. 设置库专属的 `<PackageName>_DIR` 变量指向包含 Config.cmake 的目录
3. 在库的 CMakeLists.txt 中调用 `export(TARGETS ...)` 注册到构建树

---

## 13. 自定义 function 与 macro

### function 和 macro 的核心区别

| 特性 | `function` | `macro` |
|------|------------|---------|
| 作用域 | 独立作用域，局部变量不影响调用者 | 无独立作用域，相当于文本替换 |
| `return()` | 支持提前返回 | 不支持真正的返回 |
| 参数访问 | `${ARGV}`、`${ARGN}`、`${ARGn}` | 同左 |
| 修改父作用域 | 需要 `PARENT_SCOPE` | 直接修改（因无作用域隔离） |
| 调试 | 独立调用栈，易调试 | 展开困难 |

### 项目代码示例

**宏：配置版本和自动生成设置（set_version）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake
macro(set_version)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)

    # 版本继承（宏直接修改调用者作用域变量）
    if(NOT PROJECT_VERSION)
        set(PROJECT_VERSION_MAJOR ${FILMORA_VERSION_MAJOR})
        set(PROJECT_VERSION        ${FILMORA_VERSION})
    endif()
endmacro()
```

**宏：统一安装规则（set_install）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake
macro(set_install)
    install(
        TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION Bin/Release
    )
endmacro()
```

**函数：一键创建标准模块（AppendModulesBasic）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake
# 参数：pch_file_t = 预编译头文件名（空字符串表示不用）
function(AppendModulesBasic pch_file_t)
    set_version()      # 调用宏：设置 AUTOMOC 等
    set_rc_file()      # 调用宏：生成 .rc 资源文件

    add_library(${PROJECT_NAME} SHARED
        ${prj_common_headerFiles}
        ${prj_headerFiles}
        ${prj_srcFiles}
        ${prj_form_files}
        ${prj_translation_files}
        ${prj_resource_files}
        ${prj_resource_in_File}
    )
    add_library(FILMORA::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

    set_target()          # 宏：编译选项、include 路径
    set_code_analysis()   # 宏：VS 代码分析属性
    set_filter("Module")  # 宏：IDE 分组显示
    set_install()         # 宏：安装规则

    if(APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS
            "@executable_path/../Frameworks @loader_path @loader_path/../Frameworks")
    endif()
endfunction()
```

**函数：保持源文件目录结构（_tree_source_group）**

```cmake
# 文件：Tools/cmake/fvb_functions.cmake
# 在 IDE 中保留源文件的目录树结构，而不是全部平铺
function(_tree_source_group root files)
    foreach(source IN LISTS files)
        file(RELATIVE_PATH _source_rel ${root} ${source})
        get_filename_component(source_path_msvc ${_source_rel} PATH)
        string(REPLACE "/" "\\" source_path_msvc ${source_path_msvc})
        source_group(${source_path_msvc} FILES ${source})
    endforeach()
endfunction()
```

**函数：生成带版本信息的文件（generate_version_file）**

```cmake
# 文件：Tools/cmake/fvb_functions.cmake
# 将模板文件中的 %Target% 占位符替换为实际 target 名称
function(generate_version_file target_name template_file dest_path)
    if(NOT EXISTS ${template_file})
        return()  # function 支持提前 return，macro 不行
    endif()
    if(EXISTS ${dest_path})
        return()  # 已存在则跳过
    endif()

    file(READ ${template_file} contents)
    string(REGEX REPLACE "%Target%" "${target_name}" correct_contents ${contents})
    file(WRITE ${dest_path} ${correct_contents})
endfunction()
```

**函数：预编译头辅助（f_add_pch_to_target）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake
function(f_add_pch_to_target target_name)
    target_precompile_headers(${target_name}
        PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:stdafx.h>")
endfunction()
```

### 常见面试题

**Q：什么时候用 function，什么时候用 macro？**

A：优先用 `function`——有独立作用域，不会意外污染调用者变量，支持 `return()`，更安全。只有需要**直接修改调用者作用域变量**时才用 `macro`（如 `set_version` 宏需要在调用者作用域设置 `CMAKE_AUTOMOC` 等全局变量）。

---

## 14. configure_file 与文件生成

### 核心用途

将模板文件（`.in`）中的 CMake 变量或 `@VAR@` 占位符替换为实际值，生成最终文件（`.rc`、`.ini`、版本头文件等）。

### 项目代码示例

**生成 Windows .rc 资源文件（含版本号、产品名）**

```cmake
# 文件：Src/Filmora/CMakeLists.txt

# 先根据品牌设置变量
if(FILMORA_FOR_MIAOYING)
    set(FILMORA_RC_PRODUCT_NAME "万兴喵影")
else()
    set(FILMORA_RC_PRODUCT_NAME "Wondershare Filmora")
endif()
if(BETA)
    set(FILMORA_RC_PRODUCT_ICON "Filmora-beta.ico")
endif()

# configure_file：将 .rc.in 中的 @FILMORA_RC_PRODUCT_NAME@ 替换为变量值
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.rc.in   # 模板
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.rc      # 输出
)
```

**生成崩溃上报配置文件**

```cmake
# 文件：Src/Filmora/CMakeLists.txt
configure_file(
    ${CMAKE_SOURCE_DIR}/Build/CrashReport.ini.in   # 模板（含 @CRASH_REPORT_PRODUCT_NAME@ 等）
    ${CMAKE_SOURCE_DIR}/Build/CrashReport.ini      # 生成文件
)
```

**set_rc_file 宏中自动生成 .rc（通用）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake（set_rc_file 宏）
# 生成 .rc.in 中间文件，再 configure_file 替换变量
generate_resource_file(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/tools/cmake/Resource.rc.in
    ${prj_resource_in_File})

# 用 @ONLY 模式：只替换 @VAR@ 格式，不替换 ${VAR}（避免 .rc 中的 % 符号被误解析）
string(TIMESTAMP _YEAR "%Y")
configure_file(${prj_resource_in_File}
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc @ONLY)
```

### 常见面试题

**Q：`configure_file` 的 `@ONLY` 选项有什么作用？**

A：默认情况下 `configure_file` 同时替换 `${VAR}` 和 `@VAR@` 两种格式。加 `@ONLY` 后只替换 `@VAR@`，保留 `${VAR}` 不变。当模板文件是 shell 脚本、.rc 文件等含有 `${}` 语法的文件时，`@ONLY` 可避免误替换非 CMake 变量。

---

## 15. Qt 与 CMake 集成（AUTOMOC/AUTORCC/AUTOUIC）

### 三个自动化选项

| 选项 | 作用 |
|------|------|
| `CMAKE_AUTOMOC ON` | 自动对含 `Q_OBJECT` 的头文件运行 MOC，生成 `moc_xxx.cpp` |
| `CMAKE_AUTOUIC ON` | 自动对 `.ui` 文件运行 UIC，生成 `ui_xxx.h` |
| `CMAKE_AUTORCC ON` | 自动对 `.qrc` 文件运行 RCC，生成 `qrc_xxx.cpp` |
| `CMAKE_INCLUDE_CURRENT_DIR ON` | 将当前源目录和构建目录加入 include 路径（访问自动生成文件） |

### 项目代码示例

**统一在宏中开启（Filmora-module.cmake）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake
macro(set_version)
    set(CMAKE_AUTOMOC ON)             # MOC 自动化
    set(CMAKE_AUTOUIC ON)             # UIC 自动化
    set(CMAKE_AUTORCC ON)             # RCC 自动化
    set(CMAKE_INCLUDE_CURRENT_DIR ON) # 包含生成文件目录
    # ...
endmacro()
```

**主程序直接设置**

```cmake
# 文件：Src/Filmora/CMakeLists.txt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# .ui 文件列表（AUTOUIC 自动处理）
set(prj_form_files
    FCollectionDialog.ui
    FRatingDialog.ui
    FVCGDownloadResolutionSelectDialog.ui
)
```

**IDE 分组：自动生成文件归组**

```cmake
# 文件：CMakeLists.txt
# 让 MOC/UIC/RCC 生成的文件在 Visual Studio 中显示为独立分组
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated files")
```

### 常见面试题

**Q：不用 AUTOMOC 手动集成 MOC 的步骤是什么？**

A：需手动调用 `qt5_wrap_cpp(MOC_SOURCES ${HEADER_FILES})`，再将 `${MOC_SOURCES}` 加入 `add_executable/add_library` 的源文件列表。`AUTOMOC ON` 自动检测含 `Q_OBJECT` 的头文件并完成这一步，强烈推荐使用自动模式。

**Q：AUTOMOC 如何知道哪些文件需要 MOC 处理？**

A：CMake 扫描所有加入 target 的源文件，找到包含 `Q_OBJECT`、`Q_GADGET`、`Q_NAMESPACE` 等宏的文件（`.h`/`.cpp`），自动为其生成 `moc_xxx.cpp`，并加入编译单元。

---

## 16. 预编译头（Precompiled Headers）

### CMake 3.16+ 原生支持

```cmake
target_precompile_headers(<target> PRIVATE <header>)
```

### 项目代码示例

**现代 CMake 方式（target_precompile_headers）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake（set_target 宏内）
if(pch_file)
    message("${PROJECT_NAME} use precompile")
    target_precompile_headers(${PROJECT_NAME} PRIVATE stdafx.h)
endif()

# 封装为函数
function(f_add_pch_to_target target_name)
    target_precompile_headers(${target_name}
        PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:stdafx.h>")
        # $<$<COMPILE_LANGUAGE:CXX>:...> 确保只对 C++ 文件开启 PCH
endfunction()
```

**MSVC 传统方式（手动设置编译标志）**

```cmake
# 文件：Tools/CustomizedTool/cmake/function.cmake
function(configure_pch target precompile_header precomile_source)
    if(MSVC)
        # stdafx.cpp 用 /Yc 生成预编译头
        set_source_files_properties(${precomile_source} PROPERTIES
            COMPILE_FLAGS "/Yc${precompile_header}")
        target_sources(${target} PRIVATE ${precomile_source})
        # 其余文件用 /Yu 使用预编译头
        target_compile_options(${target} PRIVATE
            /Yu${precompile_header}
            ${pch_output_filepath_arg}
            ${ARGN})
    else()
        # 非 MSVC：使用现代方式
        target_precompile_headers(${target} PRIVATE ${precompile_header})
    endif()
endfunction()
```

### 常见面试题

**Q：预编译头的原理和优势是什么？**

A：将频繁 include 的稳定头文件（STL、Qt、Windows API 等）预先编译为 `.pch`（MSVC）或 `.gch`（GCC）二进制缓存，后续编译单元直接加载该缓存而不重复解析头文件，大幅减少编译时间。Filmora 用 `stdafx.h` 聚合所有频繁使用的头文件。

---

## 17. install 规则

### 项目代码示例

**模块统一安装宏**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake
macro(set_install)
    install(
        TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release      # 只安装 Release 配置
        RUNTIME DESTINATION Bin/Release  # DLL/EXE 输出目录
    )
endmacro()
```

**主程序安装**

```cmake
# 文件：Src/Filmora/CMakeLists.txt
if(MSVC)
    install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION Bin/Release
    )
endif()
```

### 常见面试题

**Q：`install(TARGETS ...)` 中 `RUNTIME`、`LIBRARY`、`ARCHIVE` 各代表什么？**

A：
- `RUNTIME`：可执行文件（`.exe`）和 Windows 动态库（`.dll`）
- `LIBRARY`：非 Windows 共享库（`.so`/`.dylib`）
- `ARCHIVE`：静态库（`.lib`/`.a`）和 Windows 导入库（`.lib`）

Windows 上 `.dll` 走 `RUNTIME`，`.lib`（导入库）走 `ARCHIVE`，需分别指定 `DESTINATION`。

---

## 18. FetchContent 外部依赖管理

### 核心流程

```cmake
include(FetchContent)
FetchContent_Declare(<name> URL/GIT_REPOSITORY ...)  # 声明
FetchContent_MakeAvailable(<name>)                    # 下载+配置（一步完成）
# 或分步：FetchContent_Populate + add_subdirectory
```

### 项目代码示例

**本地 zip 包（离线构建）**

```cmake
# 文件：Tools/CustomizedTool/CMakeLists.txt

# 规避 CMake 3.24 时间戳警告
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

# 从本地 zip 获取（CI 环境无网络时的常用方案）
FetchContent_Declare(
    googletest
    URL file://${CMAKE_SOURCE_DIR}/libs/googletest-release-1.11.0.zip
)
FetchContent_Declare(
    tinyxml2
    URL file://${CMAKE_SOURCE_DIR}/libs/tinyxml2-9.0.0.zip
)
FetchContent_Declare(
    cxxopts
    URL file://${CMAKE_SOURCE_DIR}/libs/cxxopts-3.0.0.zip
)

# 一次性下载解压并添加到构建
FetchContent_MakeAvailable(googletest tinyxml2 cxxopts)
```

### 常见面试题

**Q：`FetchContent` 和 `ExternalProject_Add` 的区别？**

A：
- `FetchContent`：在 **configure 阶段**下载并集成，子项目的 target 直接可用，可以用 `target_link_libraries` 引用；适合将依赖作为构建树的一部分
- `ExternalProject_Add`：在 **build 阶段**下载编译，产物是独立构建，不能直接引用其 CMake target，需要手动指定 include/lib 路径；适合真正独立的外部工程

**Q：`FetchContent_Declare` + `FetchContent_MakeAvailable` 和分步 `FetchContent_Populate` + `add_subdirectory` 的区别？**

A：`FetchContent_MakeAvailable` = 调用 `FetchContent_Populate` + `add_subdirectory`，是一步完成的简化写法（CMake 3.14+）。分步写法在需要在 add_subdirectory 前修改子项目 option 变量时更灵活（先 Populate，再设置变量，再 add_subdirectory）。

---

## 19. set_target_properties 与属性系统

### 项目代码示例

**调试/Release 输出名称统一**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake（set_target 宏）
# 不让 Debug 版自动加 CMAKE_DEBUG_POSTFIX 的 "D" 后缀（模块名保持一致）
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME_DEBUG   ${PROJECT_EXE_NAME}
    OUTPUT_NAME_RELEASE ${PROJECT_EXE_NAME}
)
```

**Visual Studio 属性（IDE 集成）**

```cmake
# VS 代码分析（set_code_analysis 宏）
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_RunCodeAnalysis          false
    VS_GLOBAL_EnableMicrosoftCodeAnalysis true
    VS_GLOBAL_CodeAnalysisRuleSet      NativeRecommendedRules.ruleset
    VS_GLOBAL_EnableClangTidyCodeAnalysis true
)

# Qt 项目关键字（VS Qt 插件识别）
if(PRJ_GLOBAL_KEYWORD)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_GLOBAL_KEYWORD ${PRJ_GLOBAL_KEYWORD})  # 值为 "Qt4VSv1.0"
endif()
```

**IDE 文件分组（FOLDER 属性）**

```cmake
# 文件：Tools/cmake/Filmora-module.cmake（set_filter 宏）
# 在 Visual Studio Solution Explorer 中将 target 归入指定文件夹
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Module")

# 根目录
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # 必须先开启
```

**Xcode 专属属性**

```cmake
# 文件：Src/Filmora/CMakeLists.txt（macOS）
set_target_properties(${PROJECT_NAME} PROPERTIES
    XCODE_ATTRIBUTE_MARKETING_VERSION       "${FILMORA_VERSION_MAJOR}.${FILMORA_VERSION_MINOR}.${FILMORA_VERSION_PATCH}"
    XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "${FILMORA_VERSION}"
    XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES"
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks @loader_path"
)
```

**`CMAKE_DEBUG_POSTFIX`（全局 Debug 后缀）**

```cmake
# 文件：CMakeLists.txt
# Debug 版 DLL 自动命名为 XXXd.dll（与 Release 版 XXX.dll 区分）
set(CMAKE_DEBUG_POSTFIX "D")
# 效果：Debug 配置下 FFWidgets.dll → FFWidgetsD.dll
# 子模块用 OUTPUT_NAME_DEBUG 覆盖此行为
```

### 常见面试题

**Q：`CMAKE_DEBUG_POSTFIX` 和 `OUTPUT_NAME_DEBUG` 的关系？**

A：`CMAKE_DEBUG_POSTFIX` 是全局默认值，对所有 target 的 Debug 配置生效；`set_target_properties(... OUTPUT_NAME_DEBUG ...)` 针对单个 target 覆盖该默认值。Filmora 的模块先全局设 `"D"` 后缀，再在 `set_target` 中用 `OUTPUT_NAME_DEBUG` 重设为不含后缀的名称，使模块 DLL 文件名在 Debug/Release 下保持一致（便于运行时加载）。

---

## 20. 输出目录与构建类型

### 项目代码示例

**单配置 vs 多配置生成器**

```cmake
# 文件：CMakeLists.txt

if(CMAKE_CONFIGURATION_TYPES)
    # Visual Studio / Xcode：多配置生成器
    # 一次 configure 生成 Debug + Release 两套
    message(STATUS "Multi-configuration generator")
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
    set(CMAKE_BUILD_TYPE "" CACHE STRING "build type" FORCE)  # 清空（由构建时选择）

    # 用生成器表达式占位，构建时展开
    set(_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Bin/x64)
    set(OUTPUT_DIR "${_OUTPUT_DIR}/$<CONFIG>" CACHE STRING "Bin Dir" FORCE)

else()
    # Ninja / Make：单配置生成器
    message(STATUS "Single-configuration generator")
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "build type" FORCE)
    endif()
    # 直接用固定路径
    set(_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Bin/x64/${CMAKE_BUILD_TYPE})
    set(OUTPUT_DIR ${_OUTPUT_DIR} CACHE STRING "Bin Dir" FORCE)
endif()

# 运行时文件（.dll/.exe）统一输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${_OUTPUT_DIR})
# PDB 调试符号输出目录
set(CMAKE_PDB_OUTPUT_DIRECTORY     ${_OUTPUT_DIR})
```

**C++ 标准全局设置**

```cmake
# 文件：CMakeLists.txt
set(CMAKE_CXX_STANDARD 17)           # 使用 C++17

# 文件：Tools/CustomizedTool/CMakeLists.txt
set(CMAKE_CXX_STANDARD 20)           # 工具程序用 C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 编译器不支持则报错（不降级）
```

**导出 compile_commands.json（供 clangd/IDE 使用）**

```cmake
# 文件：Tools/CustomizedTool/CMakeLists.txt
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

### 常见面试题

**Q：`CMAKE_RUNTIME_OUTPUT_DIRECTORY` 和 `CMAKE_LIBRARY_OUTPUT_DIRECTORY` 各管什么？**

A：
- `CMAKE_RUNTIME_OUTPUT_DIRECTORY`：可执行文件（`.exe`）和 Windows DLL（`.dll`）
- `CMAKE_LIBRARY_OUTPUT_DIRECTORY`：非 Windows 共享库（`.so`/`.dylib`）；macOS 上 `.dylib` 输出到此目录（Filmora 中用于 `.app/Contents/Frameworks/`）
- `CMAKE_ARCHIVE_OUTPUT_DIRECTORY`：静态库（`.a`/`.lib`）和 Windows 导入库

**Q：`CMAKE_CXX_STANDARD` 和 `target_compile_features` 有什么区别？**

A：`CMAKE_CXX_STANDARD` 全局设置 C++ 标准（翻译为 `-std=c++17`）；`target_compile_features` 更细粒度，声明 target 需要某个语言特性（如 `cxx_lambda`、`cxx_constexpr`），CMake 自动选择满足该特性所需的最低标准。`target_compile_features` 是更"声明式"的现代做法，但项目中更常见的是直接设 `CMAKE_CXX_STANDARD`。

---

## 附录：项目核心 CMake 文件索引

| 文件 | 主要内容 |
|------|---------|
| `CMakeLists.txt`（根） | project、version、option、平台检测、全局 compile_definitions、add_subdirectory |
| `cmake-vs2017.bat` | 生成 VS2017 解决方案的命令行脚本 |
| `Tools/cmake/Filmora-module.cmake` | 核心宏/函数：set_version、set_target、AppendModulesBasic、set_install 等 |
| `Tools/cmake/fvb_functions.cmake` | 工具函数：_tree_source_group、generate_version_file |
| `Tools/cmake/Filmora-antipiracy.cmake` | 防盗版库的链接宏，大量生成器表达式用法 |
| `Tools/CustomizedTool/CMakeLists.txt` | FetchContent、cmake_policy、编译器 ID 分支 |
| `Tools/CustomizedTool/cmake/function.cmake` | configure_pch（MSVC vs 现代 CMake 预编译头封装） |
| `Src/Filmora/CMakeLists.txt` | 主程序：平台分支、configure_file、target_link_libraries 综合示例 |
| `3rdparty/WsPerformance/WsPerformanceConfig.cmake` | IMPORTED target 标准写法 |
