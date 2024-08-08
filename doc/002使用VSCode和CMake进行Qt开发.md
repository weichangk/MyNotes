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

使用vscode进行调试关键是要理解launch.json和c_cpp_properties.json配置

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
                "${workspaceFolder}/**",
                "D:/Qt/6.5.0/msvc2019_64/include/**"
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

- <b>Qt源码调试</b>
    ```
    "sourceFileMap": {
        "C:/Users/qt/work/qt": "D:/Qt/6.5.0/Src"
    }
    ```
- <b>dump调试</b>
    ```
    "dumpPath": "path"
    ```
- <b>进程调试</b>
    ```
    {
        "name": "win-qtnote-cppvsdbg-attach",
        "type": "cppvsdbg",
        "request": "attach",
        "processId": 1234,
        "visualizerFile": "${workspaceRoot}/qt6.natvis.xml",
        "symbolSearchPath": "otherSearchPath;D:/Qt/6.5.0/msvc2019_64/bin",
        "sourceFileMap": {
            "C:/Users/qt/work/qt": "D:/Qt/6.5.0/Src"
        }
    },
    ```

defines 用于预处理宏配置