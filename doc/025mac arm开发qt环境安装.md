# 安装xcode
根据macos版本安装对应版本xcode
安装xcode会自带git

# 编译安装arm版本qt
```
git clone git://code.qt.io/qt/qt5.git
cd qt5
git checkout 5.15
./init-repository
cd ..
mkdir qt5-5.15-macOS-release
cd qt5-5.15-macOS-release
../qt5/configure -release -prefix ./qtbase -nomake examples -nomake tests QMAKE_APPLE_DEVICE_ARCHS=arm64 -opensource -confirm-license -skip qt3d -skip qtwebengine
make -j15
```
参考：https://www.reddit.com/r/QtFramework/comments/ll58wg/how_to_build_qt_creator_for_macos_arm64_a_guide/?rdt=61629

# 安装hombrew
```
/bin/bash -c "$(curl -fsSL https://gitee.com/ineo6/homebrew-install/raw/master/install.sh)"
```
安装后自动配置环境变量
```
eval $(/opt/homebrew/bin/brew shellenv) #brew.idayer.com
export HOMEBREW_API_DOMAIN=https://mirrors.ustc.edu.cn/homebrew-bottles/api #brew.idayer.com
export HOMEBREW_BOTTLE_DOMAIN=https://mirrors.ustc.edu.cn/homebrew-bottles/bottles #brew.idayer.com
export HOMEBREW_PIP_INDEX_URL=https://mirrors.aliyun.com/pypi/simple/ #brew.idayer.com
```
参考：https://brew.idayer.com/guide/m1/

# 使用homebrew安装cmake
```
brew install cmake
//or
brew install cmake@3.25
```
使用 Homebrew 安装的软件通常不需要手动配置环境变量，这是因为 Homebrew 已经帮你处理好了路径问题。

# 配置qt环境变量
```
export QTDIR=/Users/ws/qt5-5.15-macOS-release/qtbase
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
export PATH=$QTDIR/bin:$PATH
```

# 关于环境变量配置
在终端执行命令 echo $SHELL 获得终端类型
在 macOS 上，~/.zprofile 和 ~/.zshrc 都是与 Zsh (Z shell) 相关的配置文件，用于定义 shell 的行为。它们的主要区别在于加载时机和用途。

| 特性 | .zprofile| .zshrc |
| ----------- | ----------- |----------- |
|加载时机|登录 shell 时加载|交互式 shell 时加载|
|用途|配置环境变量、登录会话的初始化|配置用户界面、自定义 shell 行为|
|优先级|影响整个登录会话|仅影响当前 shell 会话|

.zprofile：放置全局的环境变量设置，比如 PATH 或其他影响整个会话的配置。
.zshrc：用于日常使用的 shell 配置，比如别名、主题设置和插件加载。


```
// 检查是否已经存在 .zprofile 文件
ls -a ~ | grep .zprofile

// 创建 .zprofile 文件
touch ~/.zprofile

// 使修改生效
source ~/.zprofile

// 验证修改
echo $PATH
```

