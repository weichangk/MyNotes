<b>使用lldb调试</b>

launch.json有两种配置方式

```
{
    "name": "mac-qtnote-cppdbg-attach",
    "type": "cppdbg",
    "request": "attach",
    "processId": 1234,
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

<b>使用 C/C++: Edit Configurations (UI) 会根据环境生成默认配置 c_cpp_properties.json </b>