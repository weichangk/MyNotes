# 09 · 多平台适配与 CMake 模块化构建

> **面试价值**：⭐⭐⭐⭐  **优先级**：P1
> **相关文件**：
> - CMakeLists.txt（根目录）
> - modules/CMakeLists.txt
> - configs/ — 平台配置
> - modules/BaseService/BsProxy/BsCloudStorage.cpp — 动态库跨平台加载
> - wrapper/vbl_web_sdk/ — WASM 构建

---

## 1. 支持的平台矩阵

    平台          | 编译器/工具链       | 特殊处理
    --------------|--------------------|---------
    Windows       | MSVC / Ninja       | .rc 资源文件，WIN32 宏，LoadLibrary
    macOS         | Xcode / Clang      | .mm 文件，APPLE 宏，dlopen
    iOS           | Xcode / Clang      | MOBILE + __APPLE__ 宏
    Android       | NDK / Clang        | __ANDROID__ 宏，dlopen
    Web (WASM)    | emscripten         | VBL_WEB 宏，rapidjson，特殊链接参数

---

## 2. 条件编译宏体系

    // 平台检测（典型代码散落在各 .cpp 文件中）
    
    // Windows 特有（如动态库加载）
    #ifdef WIN32
        HMODULE handle = LoadLibrary(libPath.c_str());
        auto fn = (FuncType)GetProcAddress(handle, "FuncName");
    #elif defined(__APPLE__) || defined(__ANDROID__)
        void* handle = dlopen(libPath.c_str(), RTLD_LAZY);
        auto fn = (FuncType)dlsym(handle, "FuncName");
    #endif
    
    // 移动平台特有功能
    #if defined(MOBILE) || defined(__ANDROID__)
        // 使用触摸事件、移动端文件路径约定等
    #endif
    
    // Web/WASM 特有
    #ifdef VBL_WEB
        // 使用 emscripten 特有 API
        // 禁用线程相关功能（WASM 线程支持有限）
    #endif

**文件路径示例**：modules/BaseService/BsProxy/BsCloudStorage.cpp

    // BsCloudStorage.cpp（动态库加载的跨平台封装）
    static void* LoadDynamicLib(const std::string& path) {
    #ifdef WIN32
        return (void*)LoadLibraryA(path.c_str());
    #else
        return dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
    #endif
    }
    
    static void* GetSymbol(void* handle, const char* name) {
    #ifdef WIN32
        return (void*)GetProcAddress((HMODULE)handle, name);
    #else
        return dlsym(handle, name);
    #endif
    }
    
    static void CloseDynamicLib(void* handle) {
    #ifdef WIN32
        FreeLibrary((HMODULE)handle);
    #else
        dlclose(handle);
    #endif
    }

---

## 3. CMake 模块化构建 — AppendModulesBasic()

VBL 的核心 CMake 函数 `AppendModulesBasic()` 实现了"约定优于配置"的模块化构建：

    # CMakeLists.txt（根目录或 modules/CMakeLists.txt 的核心函数）
    function(AppendModulesBasic MODULE_NAME)
        # 1. 自动发现源文件（约定目录结构）
        file(GLOB_RECURSE SRC_FILES "${MODULE_NAME}/*.cpp" "${MODULE_NAME}/*.mm")
        file(GLOB_RECURSE HDR_FILES "${MODULE_NAME}/*.h" "${MODULE_NAME}/*.hpp")
        
        # 2. 创建静态库 target
        add_library(${MODULE_NAME} STATIC ${SRC_FILES} ${HDR_FILES})
        
        # 3. 自动处理版本文件（version.h.in -> version.h）
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}/version.h.in")
            configure_file(
                "${MODULE_NAME}/version.h.in"
                "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}/version.h"
            )
        endif()
        
        # 4. Windows 资源文件（.rc.in -> .rc）
        if(WIN32 AND EXISTS "${MODULE_NAME}/${MODULE_NAME}.rc.in")
            configure_file(
                "${MODULE_NAME}/${MODULE_NAME}.rc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}/${MODULE_NAME}.rc"
            )
            target_sources(${MODULE_NAME} PRIVATE
                "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}/${MODULE_NAME}.rc")
        endif()
        
        # 5. 自动生成测试 target（如果有 Test/ 目录）
        if(VBL_BUILD_TESTS AND EXISTS "${MODULE_NAME}/Test")
            file(GLOB_RECURSE TEST_FILES "${MODULE_NAME}/Test/*.cpp")
            add_executable(${MODULE_NAME}_test ${TEST_FILES})
            target_link_libraries(${MODULE_NAME}_test ${MODULE_NAME} GTest::gtest_main)
            gtest_discover_tests(${MODULE_NAME}_test)
        endif()
    endfunction()
    
    # 使用：每个模块只需一行
    AppendModulesBasic(BsProxy)
    AppendModulesBasic(BsUndoManager)
    AppendModulesBasic(MsEventBus)

这个函数体现了 CMake 中的"约定优于配置"思想：只要遵循目录结构约定，
一行代码就能完成模块的完整构建配置。

---

## 4. 多配置构建（Ninja Multi-Config）

    # CMakePresets.json（示意）
    {
      "configurePresets": [
        {
          "name": "windows-debug",
          "generator": "Ninja Multi-Config",
          "cacheVariables": {
            "CMAKE_CONFIGURATION_TYPES": "Debug;Release;RelWithDebInfo"
          }
        },
        {
          "name": "wasm",
          "generator": "Ninja",
          "toolchainFile": "${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake",
          "cacheVariables": {
            "VBL_WEB": "ON",
            "CMAKE_BUILD_TYPE": "Release"
          }
        }
      ]
    }

---

## 5. WASM/emscripten 特殊配置

    # WASM 构建的特殊 CMake 设置
    if(VBL_WEB)
        # emscripten 链接参数
        set_target_properties(vbl_web_sdk PROPERTIES
            LINK_FLAGS "-s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 \
                        -s EXPORT_NAME='VBL' --bind"
        )
        
        # 禁用多线程（WASM 线程需要 SharedArrayBuffer，有浏览器兼容性问题）
        target_compile_definitions(vbl_web_sdk PRIVATE
            VBL_WEB=1
            VBL_NO_THREAD=1
        )
        
        # WASM 下只用 rapidjson（避免 jsoncpp 的线程局部存储问题）
        target_link_libraries(vbl_web_sdk PRIVATE rapidjson)
    endif()

---

## 6. Xcode 平台特殊属性

    # macOS/iOS 的 Xcode 特有设置
    if(APPLE)
        set_target_properties(${TARGET} PROPERTIES
            XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "12.0"
            XCODE_ATTRIBUTE_SWIFT_VERSION "5.0"
        )
        
        # .mm 文件（Objective-C++）自动用 ObjC++ 编译器
        set_source_files_properties(
            BsWrapper.mm
            PROPERTIES COMPILE_FLAGS "-x objective-c++"
        )
    endif()

---

## 7. 面试要点

1. **AppendModulesBasic 体现了约定优于配置**：模块只需遵循固定目录结构（src/header/test），
   一行 CMake 函数完成完整构建配置，包括版本文件、.rc 资源、测试 target，
   大幅降低了新增模块的配置成本。

2. **条件编译宏是多平台适配的基础**：VBL 用 WIN32/__APPLE__/__ANDROID__/VBL_WEB 四组宏
   覆盖5个平台，关键的平台差异（动态库加载、线程API、文件路径）都集中在少数封装函数中，
   业务代码无需感知平台。

3. **动态库加载的跨平台封装**：LoadLibrary/dlopen 的封装让插件加载逻辑与平台解耦，
   BsCloudStorage 同一套代码可以在 Windows/macOS/Android 上运行。

4. **WASM 是 VBL 适配的最特殊平台**：不支持真正的多线程（SharedArrayBuffer 限制），
   需要禁用线程相关功能；不支持动态库，所有代码静态链接；
   使用 emscripten 的 --bind 做 C++->JS 接口绑定。

5. **Ninja Multi-Config 支持单次 configure 多次 build**：
   configure 一次后可以用 `cmake --build --config Debug` 和 `--config Release`
   分别构建调试和发布版本，比传统单配置 Ninja 更高效。

---

## 8. 可能被追问的问题

**Q1：为什么 WASM 平台禁用多线程？如何替代？**
WASM 多线程需要浏览器支持 SharedArrayBuffer，受跨源隔离策略（COOP/COEP HTTP 头）限制，
不是所有部署环境都支持。禁用后，原来后台线程的任务改为同步执行或拆分成异步回调。

**Q2：AppendModulesBasic 中 GLOB_RECURSE 收集源文件有什么缺点？**
新增/删除文件时 CMake 不能自动检测到（只有重新 configure 才更新文件列表）。
替代方案是显式列出每个源文件（更精确但维护成本高）。VBL 选择 GLOB_RECURSE 是
因为模块内部结构相对稳定，且 CI/CD 流程总是完整 configure，不会漏掉新文件。

**Q3：条件编译和运行时 if 判断相比，各有什么优势？**
条件编译：编译期决定，零运行时开销，可以使用平台特有 API 而不报编译错误。
运行时判断：灵活，可以热切换，不需要重新编译。
VBL 中平台特有 API（LoadLibrary vs dlopen）必须用条件编译，因为 Windows 没有 dlopen。
功能开关（如是否启用 GPU 加速）可以用运行时判断。

**Q4：emscripten 的 --bind 做了什么？**
--bind 启用 Emscripten Bindings（embind），自动为 C++ 类生成对应的 JavaScript 包装代码，
允许 JavaScript 直接调用 C++ 函数、访问 C++ 类的方法和属性，比手写 EM_ASM/EM_JS 更方便。
