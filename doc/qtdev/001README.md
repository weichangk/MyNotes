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

Qt 工程中的 .qrc 资源文件需要手动添加，并在 CMakeLists.txt 中配置
```
add_executable(${PROJECT_NAME}
    WIN32
    ${srcs}
    ${qrcs} # 资源文件配置，需要设置变量 set(qrcs ./resource_file.qrc)，也可直接使用 resource_file.qrc 配置
) 
```
编译类型配置，默认 Debug，可使用 set(CMAKE_BUILD_TYPE "Release") 配置切换，也可以在 VSCode 底部工具栏选择切换 Debug, Release, RelWithDebInfo, MinSizeRel

### 使用 Cmake 编译调试
- CMake:Scan for kit 扫描编译工具
- CMake:select a kit 选择编译工具
- F7 使用 cmake 编译
- Ctrl + F5 使用 cmake 执行调试

使用 CMake:Scan for kit 扫描编译工具后配置是在 cmake-tools-kits.json 文件中，可删除后重新扫描生成的。

### 使用 VSCode 运行和调试
使用 QtConfigure:New Project 创建的工程是会生成默认的 launch.json 初始配置文件，可直接使用 F5 运行和调试

### 使用Natvis进行Qt感知对象可视化
安装了 visualstudio 的话用 Everything 搜索一下 qt6.natvis.xml，如果没有的话可以通过链接下载：https://github.com/KDAB/KDToolBox/tree/master/qt/qt6_natvis
复制到项目根目录下，在 launch.json 配置文件 configurations 节点中新增配置 "visualizerFile": "${workspaceRoot}/qt6.natvis.xml"，使用 Mingw 需要设置 "showDisplayString":true
F5使用启动文件 launch.json 启动运行调试，因为配置了 visualizerFile 才会有Qt 感知对象可视化。


### Qt 编译器路径和智能感知设置
使用 C/C++ 插件已经有带有自能提示功能，但是时需要对 Qt 代码提示和头文件跳转还需要添加一些配置。
使用 C/C++: Edit Configurations (UI) 生成 c_cpp_properties.json 默认配置
```
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
            "compilerPath": "cl.exe",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "windows-msvc-x64",
            "configurationProvider": "ms-vscode.cmake-tools"
        }
    ],
    "version": 4
}
```
修改 compilerPath 中 cl.exe 真实路径。如果使用 Mingw 就 g++.exe 的真实路径，intelliSenseMode 改为 windows-gcc-x64。
在 includePath 节点添加 Qt 头文件路径即可实现 Qt 代码提示和头文件跳转。
```
"includePath": [
    "${workspaceFolder}/**",
    "D:/Qt/6.5.0/msvc2019_64/include/**"
],
```

defines 用于预处理宏配置