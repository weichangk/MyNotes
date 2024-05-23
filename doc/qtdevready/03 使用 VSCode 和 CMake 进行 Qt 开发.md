# CMake介绍

https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html
https://www.bookstack.cn/read/CMake-Cookbook/README.md

CMake是一个使用c++编写的开源的跨平台构建工具，用于管理和构建C/C++项目。它使用简单的配置文件来控制整个构建过程，并生成适用于各种操作系统和编译器的构建脚本（如Makefile、Visual Studio项目文件等），从而使得项目能够在不同的平台上进行构建。

这节课先演示一个hello qt and cmake demo，先简单了解Qt项目是如何使用CMake进行构建的，后面会继续介绍Qt项目中CMake的使用方法

假设我们想要使用cmake编译一个小型的qt应用程序，我们必须在名为cmakelist.txt的文件中定义要执行的操作

```
#cmake版本
cmake_minimum_required(VERSION 3.16)

#项目名
project(hello_qt_and_cmake)

#设置工程包含当前目录，非必须
set(CMAKE_INCLUDE_CURRENT_DIR ON) 

#开启一些Qt的特定功能
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#用于在 CMake 构建过程中查找并加载 Qt6 框架中的 Widgets 组件
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# 用于向 CMake 构建系统添加一个可执行文件目标。在这个例子中，它创建了一个名为 hello_qt_and_cmake 的可执行文件目标，该目标由 main.cpp 源文件构建，除了main.cpp之外还可以添加.h头文件和.qrc资源文件，以空格间隔追加即可
add_executable(hello_qt_and_cmake main.cpp)

#用于将特定的库链接到目标可执行文件上。在这个例子中，它将 Qt6 框架中的 Widgets 模块链接到 hello_qt_and_cmake 可执行文件上。
target_link_libraries(hello_qt_and_cmake Qt6::Widgets)
```

set(CMAKE_AUTOMOC ON) 是一个CMake命令，用于启用自动的Qt元对象编译（MOC，Meta-Object Compiler）。
在使用Qt编写程序时，通常需要在类的声明中使用一些特殊的宏，如Q_OBJECT，这些宏指示Qt元对象编译器（MOC）生成额外的C++代码，用于信号与槽、属性等特性的实现。

当你启用CMAKE_AUTOMOC选项时，CMake会自动检测项目中的所有包含Qt元对象的类，然后为每个类调用MOC来生成相应的代码。这样做的好处是，你不再需要手动编写MOC的调用命令，CMake会自动处理。这大大简化了Qt项目的构建过程，使得开发者可以更专注于代码编写而不必担心元对象编译的细节。

set(CMAKE_AUTORCC ON) 是一个 CMake 命令，用于启用自动资源编译 (RCC，Resource Compiler) 功能。在 Qt 项目中，通常会使用.qrc 文件来管理项目中的资源文件，比如图像、字体、翻译文件等。这些资源文件可以被编译成 Qt 资源文件 (.qrc) 并由 Qt 的 RCC 工具进行处理，使得它们可以被 Qt 应用程序直接访问。

启用 CMAKE_AUTORCC 选项后，CMake 会自动检测项目中的.qrc 文件，并调用 RCC 工具来将其编译成相应的资源文件。这样做的好处是，你无需手动编写 RCC 的调用命令，CMake 会自动处理资源文件的编译过程，简化了项目的构建流程。

set(CMAKE_AUTOUIC ON) 是一个 CMake 命令，用于启用自动用户界面文件编译 (UIC，User Interface Compiler) 功能。在 Qt 项目中，通常会使用.ui 文件来设计用户界面，包括窗口布局、控件设置等。这些.ui 文件需要被编译成相应的 C++ 代码才能被 Qt 应用程序使用。

启用 CMAKE_AUTOUIC 选项后，CMake 会自动检测项目中的.ui 文件，并调用 UIC 工具来将其编译成相应的 C++ 代码。这样做的好处是，你无需手动编写 UIC 的调用命令，CMake 会自动处理用户界面文件的编译过程，简化了项目的构建流程。

set(CMAKE_INCLUDE_CURRENT_DIR ON) 是一个 CMake 命令，用于在当前构建目录中包含当前源文件目录。当这个选项被设置为 ON 时，CMake 将会在构建过程中将当前源文件目录添加到包含路径中，使得在构建过程中可以直接引用当前源文件目录中的头文件而无需指定完整路径。

这个选项通常用于处理当前项目中的源文件之间的相互依赖关系，特别是当项目的源文件分布在不同的子目录中时。通过设置这个选项，可以方便地在不同源文件之间引用彼此的头文件，从而简化了源文件的包含路径管理。

需要注意的是，虽然这个选项可以简化头文件的包含路径管理，但过度使用它可能会导致一些不必要的依赖关系，因此在使用时应该谨慎考虑，避免引入不必要的耦合。




