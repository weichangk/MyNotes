在 VSCode 中调试多个进程（例如多线程应用或客户端-服务器架构）时，你可以使用 launch.json 配置来启动并调试多个进程。VSCode 支持几种方式来实现多进程调试，主要包括并行配置和复合配置。

<h4>1. 并行配置</h4>
这是最常用的方式，允许你同时启动多个调试配置。每个进程都有自己的调试配置，并可以单独启动和控制。

<b>配置步骤：</b>
- 定义多个调试配置：
    在 launch.json 文件中，为每个进程定义一个单独的调试配置。

- 启动每个配置：
    你可以手动逐个启动这些配置，也可以将它们组合成一个复合配置，一键启动。

<b>示例：</b>
```
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug Server",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build/server",
      "args": [],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "miDebuggerPath": "/usr/bin/gdb"
    },
    {
      "name": "Debug Client",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build/client",
      "args": [],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "miDebuggerPath": "/usr/bin/gdb"
    }
  ]
}
```
在这种情况下，你可以分别启动 "Debug Server" 和 "Debug Client" 配置，从而分别调试服务器和客户端进程。