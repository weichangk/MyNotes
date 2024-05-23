
如果使用 CMake 构建的 Qt 工程，由 CMakeLists.txt 文件进行工程管理，QtCreator 和 VSCode 都可以打开工程进行开发，但是 VSCode 需要配置相关环境，后面再讲如何使用 VSCode 进行 Qt 开发

如果使用 QMake 构建的 Qt 工程，由 .pro 文件进行工程管理，默认只能使用 QtCreator 打开工程进行开发

Microsoft Visual Studio  装了 Qt VS Tools 插件，可以直接创建 Qt 项目，生成的是 Visual Studio 解决方案 .sln

如果使用 Microsoft Visual Studio 打开使用 QtCreator 创建的 QMake/CMake Qt 项目就需要进行工程转换了

**QtCreator QMake .pro 转 Visual Studio .sln**
Microsoft Visual Studio 的扩展 >> Qt VS Tools >> Open Qt Project File (.pro) 可直接打开并生成 Visual Studio 解决方案 .sln

**QtCreator CMake CMakeLists.txt 转 Visual Studio .sln**
generate_vs_solution.bat 脚本文件调用 CMake 生成 Visual Studio 解决方案 .sln，脚本如下：
```
@echo off

rem 创建 VSProject 文件夹
mkdir VSProject               

rem 对 VSProject 清理 CMake 缓存          
cmake clean VSProject                   

rem 读取 cmake_option.txt 到 OPT 变量中：-G "Visual Studio 17 2022" 或 -G "Visual Studio 16 2019"
set /P OPT=<cmake_option.txt        

rem 生成 VS 项目文件
cmake -S %~dp0 %OPT% -B VSProject       
pause
```

cmake_option.txt 文件指定 CMake 生成 vs 版本，内容如下：
```
-G "Visual Studio 17 2022"
```

将脚本文件创建到和 CMakeLists.txt 文件同级目录下，双击执行 generate_vs_solution.bat 脚本就能生成的 Visual Studio 解决方案到 VSProject 文件夹中，使用 Visual Studio 打开 .sln 文件即可


也可以使用 CMake 图形化工具（cmake-gui） 进行 CMakeLists.txt 生成 Visual Studio 解决方案 .sln，后续再补充。