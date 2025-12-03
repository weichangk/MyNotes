## quazip 开源地址

https://github.com/stachenov/quazip



## quazip 编译问题

- 根据 CMakePresets.json 文件，需要配置 VCPKG_ROOT 环境变量

- 系统没有 vcpkg.cmake 则安装，地址 https://github.com/microsoft/vcpkg

- 如何编译查看 https://github.com/stachenov/quazip

- 使用 quazip 遇到 Debug CRT 堆断言问题

  ```
  _CrtIsValidHeapPointer(block)
  is_block_type_valid(header->_block_use)
  Access violation at `0xFFFFFFFFFFFFFFFF`
  ```

  内存由不同运行时分配/释放导致堆损坏。结合我们现状，最有可能的是在 Debug 模式下运行主程序，但使用了 Release 构建的 quazip1-qt5.dll（或其依赖的 zlib），从而跨 CRT 边界分配/释放内存，最终在解压结束后触发断言。

- quazip 依赖 zlib，所以也要编译 zlib



## zlib 开源地址

https://github.com/madler/zlib

使用 CMake 编译 zlib（跨平台）

zlib 从 v1.2.11 开始自带 CMake 构建脚本（zlib/CMakeLists.txt）。

**步骤：**

```
git clone https://github.com/madler/zlib.git
cd zlib
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

**编译后的文件位置**

- Windows（MSVC）：
   `zlib/build/Release/zlib.dll`
   `zlib/build/Release/zlib.lib`
- Linux/macOS：
   `zlib/build/libz.a` 或 `libz.so`

这套方式简单、跨平台、适合 Qt、vcpkg、CMake 项目。

**zlib 编译输出**

假设你已经用 CMake 或 VS 编译好了 zlib：

- **库文件**
  - Windows MSVC：`zlib.lib`（静态库）、`zlib.dll`（动态库）
  - Windows MinGW / Linux / macOS：`libz.a`（静态库）、`libz.so` / `libz.dylib`（动态库）
- **头文件**：
  - `zlib.h`
  - `zconf.h`
  - （可选：`zlibadler32.h`、`zlibinflate.h` 等，如果你用到）

默认这些头文件在 zlib 根目录下，或者 `build` 文件夹里会生成副本。



