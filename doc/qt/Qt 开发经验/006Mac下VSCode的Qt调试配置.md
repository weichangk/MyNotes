## c_cpp_properties.json配置
可以使用 C/C++: Edit Configurations (UI) 会根据环境生成默认配置 c_cpp_properties.json

## launch.json配置方式如下

### 使用cppdbg调试
```
{
    "name": "mac-qtnote-cppdbg-attach",
    "type": "cppdbg",
    "request": "attach",
    "program": "${workspaceFolder}/path_to_your_binary",
    // "processId": 1234,
    "processId": "${command:pickProcess}",
    "MIMode": "lldb",
    "additionalSOLibSearchPath": "/Users/ws/Qt/6.5.0/clang_64/lib",
    "symbolLoadInfo": {
        "loadAll": true,
        "exceptionList": ""
    },
    "visualizerFile": "${workspaceFolder}/qt6.natvis.xml",
    "showDisplayString": true,
    "sourceFileMap": {
        "/Users/qt/work/qt": "/Users/ws/Qt/6.5.0/Src"
    }
},
{
    "name": "mac-qtnote-cppdbg",
    "type": "cppdbg",
    "request": "launch",
    "program": "${command:cmake.launchTargetPath}",
    "args": [],
    "stopAtEntry": false,
    "cwd": "${workspaceRoot}",
    "environment": [],
    "externalConsole": false,
    "MIMode": "lldb",
    "additionalSOLibSearchPath": "/Users/ws/Qt/6.5.0/clang_64/lib",
    "symbolLoadInfo": {
        "loadAll": true,
        "exceptionList": ""
    },
    "visualizerFile": "${workspaceFolder}/qt6.natvis.xml",
    "showDisplayString": true, // 使得natvis生效
    "sourceFileMap": {
        "/Users/qt/work/qt": "/Users/ws/Qt/6.5.0/Src"
    }
},
```

#### cppdbg调试参数
这个配置文件是VSCode中用于调试C++项目的launch.json配置，特别是在macOS上使用LLDB调试器和Qt项目。以下是配置项的详细分析：

- name: "mac-qtnote-cppdbg"
调试配置的名称，用于标识当前调试会话的配置。

- type: "cppdbg"
指定调试器的类型。这里使用的是C++调试扩展（cppdbg），在macOS上通常会与LLDB一起使用。

- request: "launch"
表示要启动调试目标。常见的选项有launch（启动程序）和attach（附加到正在运行的进程）。

- program: "${command:cmake.launchTargetPath}"
通过CMake生成的可执行文件的路径。cmake.launchTargetPath命令会自动解析并指向生成的可执行文件。

- args: []
传递给可执行文件的命令行参数。目前是空数组，即不传递任何参数。

- stopAtEntry: false
是否在程序入口处暂停。false表示直接运行程序，不在入口处暂停。

- cwd: "\${workspaceRoot}"
当前工作目录（current working directory）。${workspaceRoot}是工作区的根目录，通常指向项目的根路径。

- environment: []
环境变量配置。如果需要为调试过程指定特定的环境变量，可以在此处添加。

- externalConsole: false
是否使用外部控制台运行程序。false表示在VSCode内部的终端窗口中运行程序。

- MIMode: "lldb"
指定调试器的模式。这里选择的是LLDB，它是macOS的默认调试器。

- additionalSOLibSearchPath: "/Users/ws/Qt/6.5.0/clang_64/lib"
指定加载共享库（.so文件）的路径。在这里，配置了Qt 6.5.0的Clang版本库路径，调试器会在这个路径中查找库文件。

- symbolLoadInfo:
loadAll: true
表示加载所有的调试符号。
exceptionList: ""
如果有需要排除的符号文件，可以在此列出。当前为空，表示没有排除任何符号。

- visualizerFile: "${workspaceFolder}/qt6.natvis.xml"
使用Qt 6的NatVis文件（qt6.natvis.xml）来增强调试器的显示效果。NatVis文件定义了自定义的变量可视化方式，使调试更加直观。

- showDisplayString: true
使得NatVis生效。NatVis是用于C++调试的可视化工具，showDisplayString选项开启后会显示用户定义的变量格式。

- sourceFileMap:
"/Users/qt/work/qt": 源码路径
"/Users/ws/Qt/6.5.0/Src": 映射的本地路径
用于在调试过程中映射源文件路径。在远程构建或者源代码存储在不同路径时，可以通过此映射让调试器能够正确找到本地的源文件。


### 使用lldb调试
```
{
    "name": "mac-qtnote-cppdbg-attach",
    "type": "cppdbg",
    "request": "attach",
    "program": "${command:cmake.launchTargetPath}",
    // "processId": 1234,
    "processId": "${command:pickProcess}",
    "MIMode": "lldb",
    "additionalSOLibSearchPath": "/Users/ws/Qt/6.5.0/clang_64/lib",
    "symbolLoadInfo": {
        "loadAll": true,
        "exceptionList": ""
    },
    "visualizerFile": "${workspaceFolder}/qt6.natvis.xml",
    "showDisplayString": true,
    "sourceFileMap": {
        "/Users/qt/work/qt": "/Users/ws/Qt/6.5.0/Src"
    }
},
{
    "name": "mac-qtnote-lldb",
    "type": "lldb",
    "request": "launch",
    // "program": "${workspaceFolder}/Debug/QtNote.app/Contents/MacOS/QtNote", // 直接指定可执行文件，启动调试不会在编译，速度更快
    "program": "${command:cmake.launchTargetPath}",
    "args": [],
    "cwd": "${workspaceRoot}",
    "initCommands": [
        "settings set target.process.follow-fork-mode child",
        "settings set target.inline-breakpoint-strategy always",
        "command script import ${workspaceFolder}/lldb/all.py"
    ],
    "sourceMap": {
        "/Users/qt/work/qt": "/Users/ws/Qt/6.5.0/Src"
    }
},
```
#### lldb调试参数
这个配置文件也是VSCode的launch.json，用于在macOS上使用LLDB调试Qt项目。以下是各个配置项的详细分析：

- name: "mac-qtnote-lldb"
调试配置的名称，用于标识该调试会话的具体配置。

- type: "lldb"
指定调试器类型，这里直接使用LLDB调试器，而不是通过cppdbg扩展来进行调试。

- request: "launch"
表示这是启动调试目标的请求。与attach不同，launch用于启动程序。

- program:
"program": "\${workspaceFolder}/Debug/QtNote.app/Contents/MacOS/QtNote"
注释掉的内容显示直接指定了编译后的可执行文件路径。此方法可以省略编译过程，加快调试启动速度，适合调试阶段。
"program": "${command:cmake.launchTargetPath}"
当前使用的配置是通过CMake生成的可执行文件路径，这种方式更动态，适合CMake项目。

- args: []
传递给调试目标的命令行参数。这里为空，表示不传递任何参数。

- cwd: "\${workspaceRoot}"
当前工作目录，通常是项目的根目录。${workspaceRoot}表示工作区根路径。

- initCommands:
这是LLDB启动时要执行的一些初始化命令：
"settings set target.process.follow-fork-mode child"
允许调试器在调试过程中跟随子进程（child），例如程序中的fork操作。
"settings set target.inline-breakpoint-strategy always"
设置内联断点策略为“总是”。这意味着即使代码内联，断点也会被设置。
"command script import ${workspaceFolder}/lldb/all.py"
导入自定义的LLDB脚本（all.py），这个脚本可以包含一些调试器的定制化逻辑或命令。通过这种方式，你可以自动化某些调试行为或者扩展LLDB功能。

- sourceMap:
用于源代码的路径映射，便于调试时关联不同路径中的源文件。
"/Users/qt/work/qt": "/Users/ws/Qt/6.5.0/Src"
这里映射的是远程/其他路径（如构建时的路径）到本地源代码路径，调试器将会将远程路径转换成本地路径，以便正确找到源文件。


### cppdbg和lldb调试区别
这两个配置的主要区别在于调试器类型、初始化设置以及一些使用场景上的细节处理。以下是对比分析：

1. 调试器类型：
- 第一个配置:
    - type: "cppdbg"
使用的是VSCode的C++调试扩展（cppdbg），它支持多种调试器，包括LLDB和GDB，但这个配置使用的是LLDB作为后端。cppdbg是一种更通用的调试器设置，适合在不同平台和调试环境中使用。
- 第二个配置:
    - type: "lldb"
直接使用了LLDB调试器，而不是通过C++扩展。这种方式是更直接的调试方式，完全依赖LLDB自身的功能，通常在macOS系统中更常见和高效，特别是在调试macOS专有的应用时。
2. 程序启动方式：
- 第一个配置:
    - "program": "\${command:cmake.launchTargetPath}"
通过CMake生成的可执行文件路径来启动调试目标，这是动态的、基于CMake构建系统的做法。
- 第二个配置:
    - 注释掉的内容："program": "\${workspaceFolder}/Debug/QtNote.app/Contents/MacOS/QtNote"
提供了直接指定可执行文件路径的选项，跳过了CMake动态获取的步骤。注释说明这样做的好处是调试启动速度更快，因为无需重新编译。这是一个更手动和快速的方式，适合反复调试时使用。
实际使用的内容：也是通过CMake生成路径启动。
3. 初始化命令：
- 第一个配置:
没有自定义的初始化命令，依赖cppdbg和LLDB的默认配置。

- 第二个配置:
使用了initCommands来进行一些调试前的设置：
    - target.process.follow-fork-mode child：允许调试器跟随子进程。
    - target.inline-breakpoint-strategy always：确保即使在内联函数中也可以设置断点。
    - command script import ${workspaceFolder}/lldb/all.py：导入自定义脚本，用于扩展LLDB的功能或行为。
4. 调试符号与NatVis：
- 第一个配置:
启用了Qt的NatVis（qt6.natvis.xml）文件，这是一种调试时自定义对象可视化的方式，通常能使复杂对象（如Qt的内部数据结构）更直观地显示。
配置了additionalSOLibSearchPath来加载动态库（.so文件）中的调试符号。
symbolLoadInfo配置了加载所有调试符号。
- 第二个配置:
没有提到调试符号加载和NatVis的配置，调试时更偏向直接使用LLDB的默认可视化方式。

5. 源代码路径映射：
两个配置都使用了相似的sourceFileMap和sourceMap来进行路径映射，将远程/构建时的源代码路径映射到本地文件系统，确保调试时能够正确定位源文件。

6. 外部控制台和调试显示：
- 第一个配置:
    - externalConsole: false：调试时使用VSCode内置的控制台，而不是外部控制台。
配置了showDisplayString: true，启用了自定义调试器显示（NatVis）的支持。
- 第二个配置:
没有配置控制台显示选项和自定义显示支持。

7. 使用场景：
- 第一个配置:
更通用，适合跨平台调试或在不同环境下通过cppdbg扩展进行调试。配置中对调试符号和Qt的NatVis支持使它更适合复杂项目的调试和UI相关的数据结构调试。

- 第二个配置:
更轻量和直接，偏向高效启动和调试，特别是在需要直接使用LLDB的高级功能（如fork跟踪、自定义脚本）时更灵活。适合快速迭代调试和调试一些特定的LLDB扩展功能。

#### 总结：
第一个配置更加通用和全面，适合对调试符号、Qt可视化有更高需求的情况。
第二个配置则更轻量化，适合直接使用LLDB调试的场景，且通过自定义脚本和命令提升了调试器的灵活性。

### 注意
- 注意1：
使用lldb调试需要安装CodeLLDB插件

- 注意2：
使用cppdbg调试时在附加进程调试中，program 字段的确看起来不总是必要，因为你已经选择了要附加的进程。然而，VSCode 使用 cppdbg 调试配置时，仍然要求 program 字段用于以下情况：

    - 符号文件关联：调试器需要知道调试符号（如 .dSYM 或 -g 编译的调试信息）对应的可执行文件，以便解析变量、调用堆栈、设置断点等。即使你附加到正在运行的进程，VSCode 仍需要可执行文件路径来加载这些符号。

    - 辅助功能：如果调试器无法自动找到可执行文件的位置或没有符号，program 字段为调试器提供了文件位置的明确指引。

    - 调试进程重启：在某些调试场景中，VSCode 允许你重新启动进程。program 字段确保调试器知道要启动哪个二进制文件。

    - 如果你明确不需要指定可执行文件路径，你可以尝试使用 CodeLLDB 扩展中的 LLDB 配置，而不是 cppdbg。CodeLLDB 更加灵活，允许你进行更细粒度的附加进程调试，不一定需要 program 字段

