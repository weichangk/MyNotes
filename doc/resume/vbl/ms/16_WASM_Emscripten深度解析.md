# 16 · WASM/Emscripten 深度解析

> **面试价值**：⭐⭐⭐⭐  **优先级**：P1
> **相关文件**：
> - `tools/cmake/modules/modules_basic.cmake`（VBL_WEB 编译/链接选项）
> - `CMakeLists.txt`（VBL_WEB 选项定义）
> - `wrapper/vbl_web_sdk/Interface/ProjectHelper/IVbWebProjectParse.h`（C API）
> - `wrapper/vbl_web_sdk/modules/WebProjectParse/WebProjectParser.cpp`（实现）
> - `3rdparty/WES-wasm/`（第三方 wasm 依赖）

---

## 1. 概念介绍

WebAssembly（WASM）是一种在浏览器中运行接近原生速度代码的二进制格式。Emscripten 是将 C/C++ 编译为 WASM 的工具链。

**核心原理**：
- C++ 代码通过 Emscripten 编译为 `.wasm` + `.js` 胶水代码
- JS 通过胶水代码调用 WASM 中导出的函数
- WASM 拥有线性内存（Linear Memory），JS 通过 typed array 读写
- pthreads 通过 SharedArrayBuffer + WebWorker 实现

**VBL 使用 WASM 的场景**：
- Filmora Online（Web 版视频编辑器）
- 在浏览器中运行 C++ 视频处理引擎（WES-wasm）
- Web 项目格式（JSON）与 WES 项目格式的互转

---

## 2. CMake 构建配置

### 2.1 开启 VBL_WEB 模式

```cmake
# CMakeLists.txt（顶层）
option(VBL_WEB "Enable install vbl web" OFF)

# 构建命令
cmake -DVBL_WEB=ON -G "Ninja Multi-Config" -S . -B build
cmake --build build --config Release
```

### 2.2 modules_basic.cmake 中的 VBL_WEB 完整配置

当 `VBL_WEB=ON` 时，`AppendModulesBasic` 为每个模块追加以下链接选项：

```cmake
# tools/cmake/modules/modules_basic.cmake（VBL_WEB 分支完整摘录）
if (VBL_WEB)
    target_link_options(${name}
        PRIVATE
        -lidbfs.js                    # IndexedDB 文件系统（持久化虚拟 FS）
        -pthread                      # 启用 pthreads（SharedArrayBuffer + WebWorker）
        -sWASM=1                      # 输出 wasm 格式
        -sFORCE_FILESYSTEM=1          # 强制挂载虚拟文件系统
        -sPTHREAD_POOL_SIZE=32        # 预分配 32 个 WebWorker 线程
        -sMIN_WEBGL_VERSION=2         # 要求 WebGL 2
        -sMAX_WEBGL_VERSION=2
        -sFULL_ES3=1                  # 完整 OpenGL ES3 支持
        -sOFFSCREEN_FRAMEBUFFER=1     # 离屏帧缓冲
        -sOFFSCREENCANVAS_SUPPORT=1  # 离屏 Canvas 支持（多线程渲染）
        -sALLOW_MEMORY_GROWTH=1       # 运行时内存动态增长
        -sMAXIMUM_MEMORY=4GB          # 最大内存限制 4GB
        -sSTACK_SIZE=1MB              # 每个线程栈大小 1MB
        -sINITIAL_MEMORY=128MB        # 初始内存 128MB
        -sEXCEPTION_CATCHING_ALLOWED=[..]  # 允许的异常类型
        -sWASM_BIGINT=1               # 启用 BigInt 互操作（64 位整数）
        -sAUDIO_WORKLET=1             # 音频工作线程（低延迟音频）
        -sWASM_WORKERS=1              # 启用 WASM Workers
        -sEXPORTED_RUNTIME_METHODS=['UTF8ToString','cwrap','FS','getValue','setValue']
        -sEXPORTED_FUNCTIONS=['_malloc','_free','_main']
    )
    # Debug/Release 优化级别
    if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
        target_link_options(${name} PRIVATE -O0 -g)
    else()
        target_link_options(${name} PRIVATE -O3)
    endif()
endif()
```

---

## 3. 关键配置项深度解析

### 3.1 内存管理三件套

```
-sINITIAL_MEMORY=128MB   → wasm 实例启动时分配的线性内存大小
-sMAXIMUM_MEMORY=4GB     → 允许增长到的最大内存（4GB 支持大型视频文件）
-sALLOW_MEMORY_GROWTH=1 → 允许运行时动态增长（初始小，按需扩展）
-sSTACK_SIZE=1MB         → 每个线程的栈空间（递归/深调用场景）
```

**设计考量**：视频编辑处理的 frame 数据量大（4K 帧约 30MB），4GB 最大内存确保不会 OOM。`ALLOW_MEMORY_GROWTH` 牺牲一定性能换取灵活性。

### 3.2 多线程支持

```
-pthread                  → 编译时启用 pthreads API
-sPTHREAD_POOL_SIZE=32   → 预创建 32 个 WebWorker（避免按需创建的延迟）
-sWASM_WORKERS=1         → 启用 WASM Workers（更轻量的线程通信机制）
-sAUDIO_WORKLET=1        → 音频工作线程（AudioWorklet，低延迟音频渲染）
```

**浏览器限制**：pthreads 依赖 `SharedArrayBuffer`，浏览器要求 HTTPS 且响应头包含：
```
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```
这是部署 Web 版必须处理的安全要求。

### 3.3 JS 互操作导出

```cmake
-sEXPORTED_RUNTIME_METHODS=['UTF8ToString','cwrap','FS','getValue','setValue']
```

| 方法 | 用途 |
|------|------|
| `UTF8ToString` | 将 wasm 内存中的 C 字符串转为 JS string |
| `cwrap` | 将 C 函数包装为 JS 可调用的函数 |
| `FS` | 访问 Emscripten 虚拟文件系统（读写 idbfs 中的文件）|
| `getValue` | 从 wasm 内存中读取值（类型安全）|
| `setValue` | 向 wasm 内存中写入值 |

```cmake
-sEXPORTED_FUNCTIONS=['_malloc','_free','_main']
```

导出 C 函数给 JS 调用：`_malloc`/`_free` 用于 JS 侧申请/释放 wasm 内存（传递大 buffer 时必须）。

---

## 4. C API 导出设计

VBL 的 WASM wrapper 采用 `extern "C"` C API（而非 embind），原因是 C ABI 更稳定、无依赖：

```cpp
// wrapper/vbl_web_sdk/Interface/ProjectHelper/IVbWebProjectParse.h
#ifdef __cplusplus
extern "C" {
#endif

// Web 项目 JSON → WES 项目文件（在 wasm 虚拟 FS 中生成）
bool webProject2ExportProject(
    char* webProjectDataBuffer,  // JS 传入的 Web 项目 JSON（通过 malloc 分配内存）
    long long bufferSize,
    char* wesProjectPath,        // 输出：WES 项目文件路径（虚拟 FS 中）
    char* resourceListPath       // 输出：需要下载的资源列表 JSON 路径
);

#ifdef __cplusplus
}
#endif
```

**JS 侧调用方式**：

```javascript
// 在 JS 中通过 cwrap 调用 wasm 函数
const webProject2ExportProject = Module.cwrap(
    'webProject2ExportProject',
    'boolean',
    ['number', 'number', 'number', 'number']
);

// 分配 wasm 内存传递大 buffer
const inputBuffer = Module._malloc(jsonData.length);
Module.HEAPU8.set(jsonData, inputBuffer);

const outputPath = Module._malloc(256);
const resourcePath = Module._malloc(256);

const success = webProject2ExportProject(
    inputBuffer, jsonData.length, outputPath, resourcePath
);

// 读取结果
const wesPath = Module.UTF8ToString(outputPath);
const resPath = Module.UTF8ToString(resourcePath);

// 释放内存
Module._free(inputBuffer);
Module._free(outputPath);
Module._free(resourcePath);
```

---

## 5. WebProjectParser — Web 项目解析

```cpp
// wrapper/vbl_web_sdk/modules/WebProjectParse/WebProjectParser.cpp
bool webProject2ExportProject(
    char* webProjectDataBuffer, long long bufferSize,
    char* wesProjectPath, char* resourceListPath)
{
    // 创建 Parser，传入输出路径
    WebProjectParser parse(wesProjectPath, resourceListPath);
    // 解析 Web 项目 JSON → WES 项目（在虚拟 FS 中生成文件）
    auto res = parse.parse(webProjectDataBuffer, bufferSize);

    // 释放 JS 侧分配的内存（JS 用 _malloc 分配后传入）
    if (webProjectDataBuffer) { delete[] webProjectDataBuffer; }
    if (wesProjectPath)       { delete[] wesProjectPath; }
    if (resourceListPath)     { delete[] resourceListPath; }
    return res;
}
```

**parse 函数的核心逻辑**：
1. 用 rapidjson 解析 Web 项目 JSON（递归遍历所有节点）
2. 识别资源引用（`filename` 字段）→ 替换为 `cloud_uuid`（WES 格式使用 UUID 引用）
3. 构建需要下载的资源列表（effects/fonts/media），输出为 JSON
4. 输出 WES 格式项目文件到虚拟 FS

**关键技术**：
- rapidjson `Document`/`Value`/`Writer`/`StringBuffer` 高效处理 JSON
- base64 解码（项目数据可能被 base64 编码传输）
- Emscripten `idbfs`（IndexedDB FS）持久化输出文件

---

## 6. 第三方 WES-wasm 使用的 Emscripten API

第三方依赖（`3rdparty/WES-wasm/`）的底层 codec 直接使用 emscripten 线程 API：

```cpp
// 3rdparty/WES-wasm/internal/wes-codec/include/Interface/C++/Common/MMPTaskPool.h
#include <emscripten.h>
#include <emscripten/emscripten.h>
#include <emscripten/threading.h>   // emscripten_has_threading_support()
```

这说明底层 codec 使用了 emscripten 的任务池/线程模型，与 `-sPTHREAD_POOL_SIZE=32` 配合使用。

---

## 7. 虚拟文件系统（idbfs）

```cmake
-lidbfs.js          # IndexedDB 文件系统
-sFORCE_FILESYSTEM=1  # 强制加载虚拟 FS
```

**idbfs 的作用**：
- Emscripten 提供虚拟 FS（内存 FS 或 idbfs）让 C++ 代码用文件 I/O API
- idbfs 将虚拟 FS 持久化到浏览器 IndexedDB，跨页面刷新保留数据
- VBL 用来：存储项目临时文件、MediaInfo 缓存（media.json）、导出的 WES 项目

---

## 8. 面试要点

1. **WASM 多线程需要浏览器安全头**：`-pthread` + `SharedArrayBuffer` 要求服务器响应头包含 `COOP: same-origin` 和 `COEP: require-corp`，这是部署 Web 版视频编辑器必须处理的关键配置，否则多线程不可用。

2. **内存配置是 WASM 性能关键**：`INITIAL_MEMORY=128MB` + `ALLOW_MEMORY_GROWTH + MAXIMUM_MEMORY=4GB` 的组合，让 wasm 实例启动快（不预分配 4GB），同时能处理大型视频数据（最大 4GB）。视频编辑场景对内存的需求比普通 web 应用高出 2 个数量级。

3. **C API 比 embind 更适合这个场景**：VBL 选择 `extern "C"` 而非 embind，因为 C ABI 稳定、无运行时开销、可以跨工具链使用。embind 的类型映射更方便，但会引入额外的 JS 胶水代码大小开销，对视频编辑器的加载时间有影响。

4. **JS 与 wasm 传递大 buffer 的模式**：JS 通过 `_malloc` 在 wasm 线性内存中申请空间 → 写入数据 → 调用函数传指针 → wasm 处理完成 → JS 读取结果 → `_free` 释放。这是 JS/WASM 大数据互操作的标准模式。

5. **idbfs 解决了 wasm "无磁盘"的问题**：浏览器中没有真实磁盘，Emscripten 用虚拟 FS + IndexedDB 模拟。VBL 的 C++ 代码用标准文件 I/O（fopen/fwrite）写输出，对代码本身完全透明，只需要在初始化时挂载 idbfs。

---

## 9. 可能被追问的问题

**Q1：WASM 的线性内存和 JS 堆是隔离的吗？如何共享数据？**
> 完全隔离：wasm 线性内存是 `ArrayBuffer`，JS 堆是独立的。共享数据需要：① JS 用 `_malloc` 在 wasm 内存中分配 → 用 typed array（`HEAPU8`）写入 → 传指针给 wasm；② wasm 计算结果写入内存 → JS 用 `getValue` 或 typed array 读取。大数据传输是开销主要来源，需要尽量减少跨界传递次数。

**Q2：`-sPTHREAD_POOL_SIZE=32` 是什么意思？为什么要预分配？**
> emscripten 的 pthreads 通过 WebWorker 实现，每个线程是一个 Worker。`PTHREAD_POOL_SIZE=32` 表示在 wasm 模块加载时预创建 32 个空闲 WebWorker，当代码 pthread_create 时从池中取出。预分配避免了按需创建 Worker 的延迟（创建 Worker 约 10-100ms），对实时性要求高的视频处理重要。

**Q3：`-sWASM_BIGINT=1` 解决了什么问题？**
> JS 的 `Number` 类型只能精确表示 53 位整数，无法精确表示 64 位整数（int64_t）。启用 `WASM_BIGINT=1` 后，wasm 的 i64 类型直接映射为 JS 的 `BigInt`，避免精度丢失。视频时间戳通常用微秒表示（int64），4K 视频 1 小时 = 3.6 × 10^12 微秒，超过 53 位，必须启用 BIGINT。

**Q4：为什么 VBL 不用 embind？**
> embind 适合对象映射，会生成大量 JS 胶水代码，增加 wasm 文件体积（视频编辑器 wasm 通常已经有几十 MB）。VBL 的 Web API 只需要少量 C 函数（项目转换），用 `extern "C"` 导出更轻量。另外 embind 需要额外的头文件依赖，而 C API 与其他语言 wrapper（C/ObjC/CLR）的风格一致，维护成本低。

**Q5：如何调试 WASM 中的 C++ 代码？**
> Debug 构建加 `-O0 -g`（modules_basic.cmake 中的 Debug 分支）：`-g` 保留 DWARF 调试信息，现代 Chrome DevTools 支持直接在 WASM 的 C++ 源码上打断点（通过 DWARF）。还可以用 `emscripten_log` 向 JS 控制台输出日志，或用 `-sSAFE_HEAP=1` 检测内存越界（Debug 模式专用，有性能开销）。
