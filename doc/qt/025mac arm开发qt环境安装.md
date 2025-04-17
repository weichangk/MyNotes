# macOS Qt 开发环境搭建指南

## 🛠️ 1. 安装 Xcode

根据 macOS 版本安装对应版本的 Xcode：
- 📥 [Xcode Releases](https://xcodereleases.com/) 
- 安装 Xcode 会自带 Git。

---

## 🛠️ 2. 安装 Qt

### 🔹 Intel 架构
- 下载链接： [Qt 在线安装器](https://download.qt.io/archive/online_installers/4.0/)

### 🔹 ARM 架构（自编译）
```bash
# 克隆 Qt5 仓库并切换到 5.15 分支
git clone git://code.qt.io/qt/qt5.git
cd qt5
git checkout 5.15

# 初始化子模块
./init-repository

# 创建编译目录并进行配置
cd ..
mkdir qt5-5.15-macOS-release
cd qt5-5.15-macOS-release
../qt5/configure -release -prefix ./qtbase -nomake examples -nomake tests QMAKE_APPLE_DEVICE_ARCHS=arm64 -opensource -confirm-license -skip qt3d -skip qtwebengine

# 编译
make -j$(sysctl -n hw.logicalcpu)
```
📖 参考：[Qt for macOS ARM64 编译指南](https://www.reddit.com/r/QtFramework/comments/ll58wg/how_to_build_qt_creator_for_macos_arm64_a_guide/?rdt=61629)

### 🔹 ARM 机器交叉编译 Intel 版本 Qt 项目
1. 在 ARM 机器安装 Intel 版本的 Qt。
2. 在项目的 CMake 中添加以下配置：
    ```cmake
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
    ```
3. 设置 Qt 环境变量，指定为 Intel 版本的 Qt 路径。
4. 打包机通常使用 ARM 机器进行 Intel 包的打包。

---

## 🍺 3. 安装 Homebrew

```bash
/bin/bash -c "$(curl -fsSL https://gitee.com/ineo6/homebrew-install/raw/master/install.sh)"
```

安装后自动配置环境变量：
```bash
eval $(/opt/homebrew/bin/brew shellenv)

# 设置国内镜像源
export HOMEBREW_API_DOMAIN=https://mirrors.ustc.edu.cn/homebrew-bottles/api
export HOMEBREW_BOTTLE_DOMAIN=https://mirrors.ustc.edu.cn/homebrew-bottles/bottles
export HOMEBREW_PIP_INDEX_URL=https://mirrors.aliyun.com/pypi/simple/
```
📖 参考：[Homebrew 国内镜像](https://brew.idayer.com/guide/start)

---

## 🛠️ 4. 使用 Homebrew 安装 CMake

```bash
brew install cmake
# 或安装指定版本
brew install cmake@3.25
```
💡 Homebrew 自动配置环境变量，无需手动调整。

---

## 🖼️ 5. 配置 Qt 环境变量

```bash
# 假设 Qt 安装在 /Users/ws/qt5-5.15-macOS-release/qtbase
export QTDIR=/Users/ws/qt5-5.15-macOS-release/qtbase
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
export PATH=$QTDIR/bin:$PATH
```

---

## 🌐 6. 关于环境变量配置

在终端中输入以下命令，检查当前终端类型：
```bash
echo $SHELL
```

在 macOS 上，Zsh 是默认终端，主要配置文件有以下两个：

| **特性** | **.zprofile** | **.zshrc** |
|----------|--------------|-----------|
| **加载时机** | 登录 shell | 交互式 shell |
| **用途** | 配置环境变量 | 自定义 shell 行为 |
| **影响范围** | 整个登录会话 | 当前 shell 会话 |

- **`.zprofile`**：放置全局环境变量（如 `PATH`）。
- **`.zshrc`**：用于日常使用的 shell 配置（如别名、主题和插件）。

### 📋 配置 `.zprofile`
```bash
# 检查是否存在 .zprofile
ls -a ~ | grep .zprofile

# 如果不存在则创建
touch ~/.zprofile

# 使修改生效
source ~/.zprofile

# 验证环境变量
echo $PATH
```

---

## 🔍 使用 `otool` 的作用

`otool` 是 macOS 上用于分析可执行文件、动态库（dylib）、目标文件（.o）等 Mach-O 文件结构的命令行工具，类似于 Linux 下的 `ldd` 或 `objdump`。

### ✅ 常用命令：检查动态库依赖
```bash
otool -L /路径/文件名
```

### 📌 示例
```bash
otool -L /usr/local/bin/your_app
```

### 🔎 输出示例：
```
/usr/local/bin/your_app:
    @rpath/QtCore.framework/Versions/5/QtCore (compatibility version 5.15.0, current version 5.15.2)
    /usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 120.0.0)
    /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1292.60.1)
```

### 📖 用途说明：
- 查看一个二进制文件依赖了哪些动态库。
- 检查 Qt 编译出来的可执行程序是否正确链接到预期的 Qt 动态库路径。
- 诊断运行时找不到库（如 `dyld: Library not loaded`）的问题。

🚀 **至此，macOS Qt 开发环境已成功搭建！** 🎉

