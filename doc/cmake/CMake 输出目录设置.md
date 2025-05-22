## 设置输出目录（旧方式）
```bash
set(EXECUTABLE_OUTPUT_PATH ${MY_BUILD_BINDIR})
set(LIBRARY_OUTPUT_PATH ${MY_BUILD_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MY_BUILD_BINDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MY_BUILD_LIBDIR})
```

## CMake 中写更现代风格的设置（适用于多配置构建）
在 现代 CMake（>=2.8） 中，如果你要支持 Debug / Release 等配置明确区分，应该只使用 CMAKE_RUNTIME_OUTPUT_DIRECTORY_<CONFIG> 这套变量，不要混用旧的：
```bash
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${MY_BUILD_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${MY_BUILD_BINDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${MY_BUILD_LIBDIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${MY_BUILD_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${MY_BUILD_BINDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${MY_BUILD_LIBDIR})
#这样 Debug / Release 可以分别控制输出位置。
```

## 为了兼容不同平台与生成器，可混合使用多配置和默认路径
```bash
# 默认路径（适用于单配置生成器）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MY_BUILD_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MY_BUILD_BINDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MY_BUILD_LIBDIR})

# 多配置支持（适用于 VS、Xcode）
foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${MY_BUILD_BINDIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${MY_BUILD_BINDIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${MY_BUILD_LIBDIR})
endforeach()

```

## 区别
| 设置方式                                             | 是否推荐           | 适用性                   | 会不会被覆盖                                       |
| ------------------------------------------------ | -------------- | --------------------- | -------------------------------------------- |
| `EXECUTABLE_OUTPUT_PATH` / `LIBRARY_OUTPUT_PATH` | ❌ 不推荐（旧方式）     | 全局、单配置                | 会被 `CMAKE_RUNTIME_OUTPUT_DIRECTORY_*` 等新变量覆盖 |
| `CMAKE_RUNTIME_OUTPUT_DIRECTORY[_<CONFIG>]` 等    | ✅ 推荐（现代 CMake） | 多配置（支持 Debug/Release） | 不会被旧变量覆盖，**反而会覆盖旧变量**的行为                     |