VSCode + CMake Tools 插件开发 C++ 项目时，默认构建目录是 build
也可以在 settings.json 添加 cmake.buildDirectory 配置来修改构建目录
```bash
"cmake.buildDirectory": "out/build"
```

如果使用脚本构建编译时修改了构建目录，如 `cmake -S . -B out/build`，要使得 launch.json 的 program 配置为 `${command:cmake.launchTargetPath}` 生效，需要配置 `cmake.buildDirectory`