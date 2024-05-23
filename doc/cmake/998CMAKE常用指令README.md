# CMAKE常用指令

生成一个项目构建系统
```
cmake [<options>] -B <path-to-build> [-S <path-to-source>]
eg: cmake -B build
```

构建项目
```
cmake --build <dir> [<options>] [-- <build-tool-options>]
eg: cmake --build build
```