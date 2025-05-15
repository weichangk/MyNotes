# 使用 VSCode 开发 Qt

## 开发环境搭建
Qt6 + CMake + Mingw/MSVC + VSCode

## 安装 QT
官网下载
https://download.qt.io/

国内镜像
- 中国科学技术大学：http://mirrors.ustc.edu.cn/qtproject/
- 清华大学：https://mirrors.tuna.tsinghua.edu.cn/qt/
- 北京理工大学：http://mirror.bit.edu.cn/qtproject/
- 中国互联网络信息中心：https://mirrors.cnnic.cn/qt/ 

archive 和 official_releases 两个目录都有最新的 Qt 开发环境安装包，由于 Qt5.15 及以上版本不提供离线安装包，则需要使用在线安装进行安装。
下载在线安装器：https://mirrors.cloud.tencent.com/qt/official_releases/online_installers/

## 使用 VSCode 开发 Qt
### 插件安装
- C/C++
- C/C++ Extension Pack
- CMake
- CMake Tools
- Qt Configure
- Qt tools
- QML
- CodeLLDB

### 配置环境变量
- D:\Qt\Tools\CMake_64\bin
- D:\Qt\Tools\mingw1120_64\bin （gdb.exe编译工具，配置后才能通过 CMake:Scan for kit 才能扫描出来使用）
- cl.exe 编译工具不需要配置环境变量，这个安装有 visualstudio 后就能 CMake:Scan for kit 扫描出来了）
- D:\Qt\6.5.0\msvc2019_64\bin （qt msvc 编译套件）
- D:\Qt\6.5.0\mingw_64\bin （qt mingw 编译套件）

在 Qt 安装的时候 cmake 是可以选装的，cmake也可独立安装。mingw 编译工具也可选装或独立安装，独立安装 mingw 参考：https://zhuanlan.zhihu.com/p/355510947
不想装 visualstudio 也是可以的，使用 Microsoft C++ 生成工具独立必要组件，根据需求选对应版本
- Windows 10 SDK(10.0.20348.0)
- MSVC V143-VS 2022 C++ X64/x86 生成工具

Microsoft C++ 生成工具： https://visualstudio.microsoft.com/zh-hans/visual-cpp-build-tools/
如果之前使用在线工具安装了 vs 进行其他语言开发，现在要配置 C++ 编译环境也是可以通过在线安装工具添加上面的组件
visualstudio 在线下载工具：https://visualstudio.microsoft.com/zh-hans/downloads/

### 新建 Qt 工程
使用Qt Configure插件
- QtConfigure:Set Qt Dir 配置Qt安装目录
- QtConfigure:New Project 新建项目名称、选择编译套件（Mingw 或 MSVC），选择构建工具（Cmake 或 QMake），是否带 ui 文件

选择构建工具 CMake 会生成 CMakeLists.txt 初始配置文件，有关 Cmake 可以参考官网：
https://cmake.org/documentation/
https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html

一个基础的 Qt cmake工程
```
cmake_minimum_required(VERSION 3.16)

if (MSVC)
    # Specify MSVC UTF-8 encoding
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

project(
    QtTest
    VERSION 1.0.0
    DESCRIPTION "qttest project"
    LANGUAGES CXX
)

if (MSVC)
    set(BUILD_DEFINE
        -D_AMD64_
        -DWIN32_LEAN_AND_MEAN
        -DUNICODE
    )
else()
    set(BUILD_DEFINE
        -DOBJC_OLD_DISPATCH_PROTOTYPES
    )
endif()
add_definitions(${BUILD_DEFINE})

set(CMAKE_PREFIX_PATH "d:/Qt/6.5.0/msvc2019_64")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QRC ./qtest.qrc)

find_package(Qt6 COMPONENTS Widgets REQUIRED) # Qt COMPONENTS

file(GLOB_RECURSE INC "inc/*.h" "inc/*.hpp")
file(GLOB_RECURSE SRC "inc/*.h" "inc/*.hpp" "src/*.cpp" "src/*.cc")

add_executable(${PROJECT_NAME}
    ${QRC}
    ${INC}
    ${SRC}
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
```

编译类型配置，默认 Debug，可使用 set(CMAKE_BUILD_TYPE "Release") 配置切换，也可以在 VSCode 底部工具栏选择切换 Debug, Release, RelWithDebInfo, MinSizeRel

### 使用 Cmake 编译调试
- CMake:Scan for kit 扫描编译工具
- CMake:select a kit 选择编译工具
- F7 使用 cmake 编译
- Ctrl + F5 使用 cmake 执行调试
- 可以通过vscode的左侧菜单cmake插件进行可视化操作

使用 CMake:Scan for kit 扫描编译工具后配置是在 cmake-tools-kits.json 文件中，可删除后重新扫描生成的。

### 使用 VSCode 运行和调试
参考下面系列教程
https://code.visualstudio.com/docs/cpp/introvideos-cpp
https://www.kdab.com/using-visual-studio-code-for-writing-qt-applications/
https://github.com/microsoft/vscode-cmake-tools/blob/main/docs/debug-launch.md#debug-using-a-launchjson-file

在 VSCode 中，launch.json 和 c_cpp_properties.json 是两个重要的配置文件，它们分别用于配置调试和编译环境。

launch.json 文件用于配置调试器的相关选项。它定义了如何启动和调试你的项目，包括设置要调试的程序、传递给程序的参数、调试程序的环境变量等。这里是一些常见的配置项：
- program: 指定要调试的可执行文件的路径。
- args: 传递给程序的命令行参数。
- cwd: 指定程序的工作目录。
- env: 指定环境变量。
- stopAtEntry: 是否在程序入口处停止。
- preLaunchTask: 在启动调试之前运行的任务（通常用于编译）。
- console: 指定调试时使用的控制台类型（如 integratedTerminal 或 externalTerminal）。

c_cpp_properties.json 文件用于配置 C/C++ 扩展的 IntelliSense、代码浏览、编译数据库等功能。这个文件帮助 VSCode 理解你的项目结构，以便提供准确的代码补全和语法检查。
- includePath: 指定头文件的搜索路径。
- defines: 指定预处理器宏定义。
- compilerPath: 指定编译器路径，以便 C/C++ 扩展能够解析编译器的系统头文件和编译选项。
- cStandard 和 cppStandard: 指定使用的 C 和 C++ 标准版本。
- intelliSenseMode: 指定 IntelliSense 的模式（例如 gcc-x64, clang-x64）

IntelliSense 是 VSCode 的智能代码补全和语法提示功能，提供了以下几种支持：
- 代码补全: 当你输入代码时，IntelliSense 会提供建议的补全项，包括函数、变量、类名等。
- 参数提示: 在函数调用时，显示函数的参数信息。
- 快速信息: 悬停在代码元素上时显示相关的文档和信息。
- 代码片段: 提供常用代码片段的快捷方式，帮助快速插入代码模板。
- IntelliSense 通过解析代码文件、分析语法和使用编译器信息来提供这些功能。在 C/C++ 开发中，IntelliSense 需要正确配置头文件路径、宏定义和编译器设置。

代码浏览功能帮助你理解和导航代码库：
- 跳转到定义: 能够快速跳转到变量、函数或类的定义位置。
- 查找引用: 显示变量、函数或类的所有引用位置，帮助你了解代码的使用情况。
- 符号查找: 允许你快速查找和浏览代码中的符号（如函数、变量、类等）。

一个基础的调试qt的launch.json和c_cpp_properties.json配置
```
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "win-qtnote-cppvsdbg",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${command:cmake.launchTargetPath}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceRoot}",
            "environment": [
                {
                    "name": "PATH",
                    "value": "D:/Qt/6.5.0/msvc2019_64/bin"
                }
            ],
            "console": "integratedTerminal",
            "visualizerFile": "${workspaceRoot}/qt6.natvis.xml",
            "symbolSearchPath": "otherSearchPath;D:/Qt/6.5.0/msvc2019_64/bin",
            "sourceFileMap": {
                "C:/Users/qt/work/qt": "D:/Qt/6.5.0/Src"
            }
        }
    ]
}

```
```
//使用 C/C++: Edit Configurations (UI) 会根据环境生成默认配置 c_cpp_properties.json 
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "windowsSdkVersion": "10.0.19041.0",
            "compilerPath": "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Tools\\MSVC\\14.37.32822\\bin\\Hostx64\\x64\\cl.exe",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "windows-msvc-x64",
            "configurationProvider": "ms-vscode.cmake-tools"
        }
    ],
    "version": 4
}

```

- <b>配置调试器</b>
修改 compilerPath 中 cl.exe 真实路径。如果使用 Mingw 就 g++.exe 的真实路径，intelliSenseMode 改为 windows-gcc-x64。

- <b>智能感知设置</b>
使用 C/C++ 插件已经有带有自能提示功能，但是时需要对 Qt 代码提示和头文件跳转还需要对c_cpp_properties.json添加一些配置。
在 includePath 节点添加 Qt 头文件路径即可实现 Qt 代码提示和头文件跳转。
    ```
    "includePath": [
        "${workspaceFolder}/**",
        "D:/Qt/6.5.0/msvc2019_64/include/**"
    ],
    ```

- <b>Qt感知对象可视化</b>
    ```
    "visualizerFile": "${workspaceRoot}/qt6.natvis.xml",
    ```
安装了 visualstudio 的话用 Everything 搜索一下 qt6.natvis.xml，如果没有的话可以通过链接下载：https://github.com/KDAB/KDToolBox/tree/master/qt/qt6_natvis
复制到项目根目录下，在 launch.json 配置文件 configurations 节点中新增配置 "visualizerFile": "${workspaceRoot}/qt6.natvis.xml"，使用 Mingw 需要设置 "showDisplayString":true

- <b>符号加载</b>
    ```
    "symbolSearchPath": "otherSearchPath;D:/Qt/6.5.0/msvc2019_64/bin",
    ```

- <b>defines 指定预处理器宏</b>
预处理器宏（如 DEBUG 或平台相关的宏）确实应该写在构建系统的配置文件中，比如 CMake 的 CMakeLists.txt 中。这是因为这些宏会直接影响编译器的行为，应该在构建过程中统一管理。
c_cpp_properties.json 中的 defines 预处理器宏也可以在开发中使用，但是打包环境不会生效。

- <b>Qt源码调试</b>
    ```
    "sourceFileMap": {
        "C:/Users/qt/work/qt": "D:/Qt/6.5.0/Src"
    }
    ```
- <b>dump调试</b>
    ```
    "symbolSearchPath": "path"
    "dumpPath": "path"
    ```
- <b>进程调试</b>
    ```
    {
        "name": "win-qtnote-cppvsdbg-attach",
        "type": "cppvsdbg",
        "request": "attach",
        // "processId": 1234,
        "processId": "${command:pickProcess}",
        "visualizerFile": "${workspaceRoot}/qt6.natvis.xml",
        "symbolSearchPath": "otherSearchPath;D:/Qt/6.5.0/msvc2019_64/bin",
        "sourceFileMap": {
            "C:/Users/qt/work/qt": "D:/Qt/6.5.0/Src"
        }
    },
    ```

### 使用 VSCode tasks.json
在 VSCode 中，tasks.json 是用来配置任务（Tasks）的文件，这些任务通常用于自动化开发流程中的常见操作，例如编译代码、运行测试、构建项目等。

- <b>tasks.json 的作用</b>
tasks.json 文件定义了可以在 VSCode 中运行的任务。这些任务可以是编译、测试、运行脚本或任何可以通过命令行执行的操作。配置 tasks.json 可以帮助你在开发过程中快速执行这些操作，而无需手动输入命令。


- <b>以下是 tasks.json 中一些常见的配置项及其作用：</b>
    - label: 任务的名称，用于识别任务。
    - type: 任务的类型，例如 shell 表示在 shell 中运行命令，process 表示直接运行某个可执行文件。
    - command: 要运行的命令或脚本。
    - args: 命令的参数。
    - group: 将任务归类为某一组，例如 build 或 test，并定义是否是默认任务。
    - problemMatcher: 用于解析任务输出并将错误或警告显示在 VSCode 的问题面板中。
    - isBackground: 指定任务是否是后台任务（持续运行）。
    - dependsOn: 指定此任务依赖的其他任务，可以在运行此任务时自动先运行其他任务。
    - presentation: 控制任务的输出如何显示，如是否显示在终端中。

- <b>使用场景</b>
    - 编译代码: 可以设置编译命令作为任务，当你需要编译代码时，只需运行任务而不用手动输入命令。
    - 运行测试: 可以设置测试脚本作为任务，快速执行单元测试或集成测试。
    - 自动化操作: 任何需要频繁执行的命令或脚本都可以配置成任务，简化开发流程。

- <b>任务与调试的结合</b>
在 launch.json 中，你可以将 preLaunchTask 配置为 tasks.json 中定义的任务，这样在开始调试之前，VSCode 会自动执行这个任务。例如，先编译代码再启动调试器。通过使用 tasks.json，你可以大大简化和加快开发过程中的常见操作，提高效率。