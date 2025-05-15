搭建 Qt 开发环境不同平台（Windows、macOS 或 Linux）会略有不同
# Windows 平台
## 下载 Qt 安装器
- 官网下载：https://download.qt.io/
- 国内镜像：
    - 中国科学技术大学：http://mirrors.ustc.edu.cn/qtproject/
    - 清华大学：https://mirrors.tuna.tsinghua.edu.cn/qt/
    - 北京理工大学：http://mirror.bit.edu.cn/qtproject/
    - 中国互联网络信息中心：https://mirrors.cnnic.cn/qt/ 

进入 official_releases/online_installers/ 目录下载对应平台的 Qt 在线安装器，在 archive/online_installers/ 目录有所有版本下载器

## 安装 Qt SDK
- 运行下载器，登录或注册 Qt 账号，进入到选择 Qt 版本（Qt 6.5.0 或 Qt 5.15.2）
    - 勾选对应平台的 Qt 工具链（Qt kit）（MSVC 2019 64-bit 或 MinGW 64-bit）
    - 勾选必要的 Qt 版本组件（Sources 和 Qt WebEngine 和 Qt Debug Information Files，磁盘空间大就全选）
- 开发和设计工具
    - 勾选 Qt Creator，选择 CBD Debugger Suppor 的
    - 勾选 Debugging Tools for Windows
    - 勾选 Qt Design Studio
    - 勾选 Qt Installer Framework
    - 勾选 MinGW 编译工具链
    - 勾选 CMake 构建工具

## 配置 Qt 环境变量
- 假设 Qt 安装目录为：D:\Qt\6.5.0\msvc2019_64
    | 变量名                 | 值                               |
    | ------------------- | ------------------------------- |
    | QTDIR             | D:\Qt\6.5.0\msvc2019_64          |
    | PATH              | 添加一项：D:\Qt\6.5.0\msvc2019_64\bin |
    | CMAKE_PREFIX_PATH | D:\Qt\6.5.0\msvc2019_64          |
- 验证配置是否成功，如果你能在命令行执行 designer、assistant、windeployqt 等 Qt 工具，也说明配置成功。
    
    ```bash
    qmake -v
    ```

- 如果你只使用 Qt Creator 开发，不一定非要手动配置这些环境变量；如果你需要在命令行用 CMake 构建 Qt 工程，或者使用终端打包、测试、部署 Qt 应用，就非常推荐配置；

## 配置编译工具链（推荐使用 MSVC）
- 方式一：使用 MSVC（Visual Studio）
    - 安装 Visual Studio（社区版即可）
        - https://visualstudio.microsoft.com/zh-hans/
    - 勾选工作负载：
        - 使用 C++ 的桌面开发
        - 勾选 CMake 工具集（可选），或者使用 Qt 安装器中安装的 CMake（在 Qt 目录下的 Tools\CMake），配置环境变量即可

- 方式二：使用 MinGW
    - 使用 Qt 安装器中安装的 MinGW（在 Qt 目录下的 Tools\mingw），配置环境变量即可

## 使用 Qt Creator（官方 IDE，推荐入门）
- 安装过程中勾选 Qt Creator
- 启动 Qt Creator，它会自动检测你安装的 Qt 和工具链
- 可以新建 Qt Widgets Application 或 Qt Quick Application 项目


# Mac 平台

## Mac 平台 Intel 架构 Qt 安装
- 根据 macOS 版本安装对应版本的 Xcode：[Xcode Releases](https://xcodereleases.com/) 
- 下载 macOS Qt 在线安装器安装 Qt SDK，和 Windows 平台下的 Qt 安装基本一样


## Mac 平台 ARM64 架构 Qt 安装
- 根据 macOS 版本安装对应版本的 Xcode：[Xcode Releases](https://xcodereleases.com/) 
- 安装 Qt SDK，需要自编译，参考：[Qt for macOS ARM64 编译指南](https://www.reddit.com/r/QtFramework/comments/ll58wg/how_to_build_qt_creator_for_macos_arm64_a_guide/?rdt=61629)
    
    ```bash
    # 克隆 Qt5 仓库并切换到 5.15 分支
    git clone git://code.qt.io/qt/qt5.git
    cd qt5
    git checkout 5.15

    # 初始化子模块
    ./init-repository

    # 创建编译目录并进行配置，跳过 qt3d 和 qtwebengine，目前不支持 macOS ARM64 构建
    cd ..
    mkdir qt5-5.15-macOS-release
    cd qt5-5.15-macOS-release
    ../qt5/configure -release -prefix ./qtbase -nomake examples -nomake tests QMAKE_APPLE_DEVICE_ARCHS=arm64 -opensource -confirm-license -skip qt3d -skip qtwebengine

    # 编译
    make -j$(sysctl -n hw.logicalcpu)
    ```

- 安装 Qt Creator，或参考文档[Qt for macOS ARM64 编译指南](https://www.reddit.com/r/QtFramework/comments/ll58wg/how_to_build_qt_creator_for_macos_arm64_a_guide/?rdt=61629)，好像也可以使用 Qt 在线安装器安装的 Qt Creator
- 使用 Homebrew 安装 CMake
    ```bash
    brew install cmake
    # 或安装指定版本
    brew install cmake@3.25
    # Homebrew 自动配置环境变量，无需手动调整。
    ```
- 配置 Qt 环境变量，环境变量文件 /Users/xxx/.zprofile
    
    ```bash
    # 假设 Qt 安装在 /Users/ws/qt5-5.15-macOS-release/qtbase
    export QTDIR=/Users/ws/qt5-5.15-macOS-release/qtbase
    export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
    export PATH=$QTDIR/bin:$PATH
    ```

## 使用 VSCode（跨平台）
目前个人 win/mac 下都是使用 VSCode 进行 c++ qt 项目开发，其他文章有写到如何使用