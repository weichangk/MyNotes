c_cpp_properties.json 配置实现qt源码跳转
```
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**",
                // "E:/Qt/5.15.2/msvc2019_64/include/**"
                "E:/Qt/5.15.2/Src/qtbase/include**",
                "E:/Qt/5.15.2/Src/qtbase/src**"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "windowsSdkVersion": "10.0.19041.0",
            "compilerPath": "D:\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.36.32532\\bin\\Hostx64\\x64\\cl.exe",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "windows-msvc-x64"
            // "configurationProvider": "ms-vscode.cmake-tools"
        }
    ],
    "version": 4
}

```

关键配置
```
"includePath": [
    "${workspaceFolder}/**",
    // "E:/Qt/5.15.2/msvc2019_64/include/**"
    "E:/Qt/5.15.2/Src/qtbase/include**",
    "E:/Qt/5.15.2/Src/qtbase/src**"
],
// "configurationProvider": "ms-vscode.cmake-tools"
```

configurationProvider 是 VSCode 的 C/C++ 扩展中的一个配置项，用于指定哪个扩展或者工具负责为当前项目提供 IntelliSense 配置。它能够让 VSCode 自动从外部工具（如 CMake、Makefile 等）中提取项目的编译信息，包括头文件路径、宏定义、编译器选项等，从而减少手动维护 c_cpp_properties.json 的复杂度。
但是想要能够实现qt源码跳转需要将其屏蔽，因为配置了他会根据CMake qt配置默认跳转 `E:/Qt/5.15.2/msvc2019_64/include/`头文件无法跳转源文件


launch.json 配置实现qt源码调试
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
                    "value": "E:/Qt/5.15.2/msvc2019_64/bin"
                }
            ],
            "console": "integratedTerminal",
            "visualizerFile": "${workspaceRoot}/3rdparty/QtMaterial/qt5.natvis.xml",
            "symbolSearchPath": "E:/Qt/5.15.2/msvc2019_64/bin",
            "sourceFileMap": {
                "C:/Users/qt/work/qt": "E:/Qt/5.15.2/Src"
            }
        }
    ]
}

```

关键配置
```
"visualizerFile": "${workspaceRoot}/3rdparty/QtMaterial/qt5.natvis.xml", //配置视觉化调试文件，在调试器中显示 Qt 对象的内部数据
"symbolSearchPath": "E:/Qt/5.15.2/msvc2019_64/bin;otherSearchPath;", //配置调试符号文件
"sourceFileMap": {
    "C:/Users/qt/work/qt": "E:/Qt/5.15.2/Src" //源码路径映射
    //其他三方库也是符号文件和源码路径映射配置进行调试
}
```

qt5.natvis.xml 文件获取 `https://wiki.qt.io/IDE_Debug_Helpers`
需要将文件中的`##NAMESPACE##::`命名空间删除，文档内有说明
