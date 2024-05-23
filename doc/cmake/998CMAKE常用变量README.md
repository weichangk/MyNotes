# CMAKE常用变量

### 输出目录变量
- **CMAKE_LIBRARY_OUTPUT_DIRECTORY 和 LIBRARY_OUTPUT_DIRECTORY  和 LIBRARY_OUTPUT_PATH:**
    - 用于指定生成的共享库（动态链接库）的输出目录。
    - 共享库通常以 .so（在类Unix系统中）或 .dll（在Windows系统中）等文件扩展名结尾。
    - 这个变量用于控制共享库文件的输出路径。
- **CMAKE_ARCHIVE_OUTPUT_DIRECTORY 和 ARCHIVE_OUTPUT_DIRECTORY:**
    - 用于指定生成的静态库（归档文件）的输出目录。
    - 静态库通常以 .a（在类Unix系统中）或 .lib（在Windows系统中）等文件扩展名结尾。
    - 这个变量用于控制静态库文件的输出路径。
- **EXECUTABLE_OUTPUT_PATH:**
    - 用于指定生成的可执行文件的输出路径。
    - 可执行文件通常没有特定的文件扩展名（例如，没有 .exe 扩展名在类Unix系统中）。
    - 这个变量用于控制可执行文件的输出路径。

总的来说，这些变量的区别在于它们控制的构建输出类型不同。`CMAKE_LIBRARY_OUTPUT_DIRECTORY` 用于共享库，`CMAKE_ARCHIVE_OUTPUT_DIRECTORY` 用于静态库，而 `EXECUTABLE_OUTPUT_PATH` 用于可执行文件。你可以根据项目需要分别设置这些变量，以指定不同类型的构建输出的存储位置，以使项目的组织结构更清晰。需要注意的是，自 CMake 3.0 版本开始，建议使用目标属性来设置输出路径，而不是直接设置这些变量。 `CMAKE_LIBRARY_OUTPUT_DIRECTORY` 和 `LIBRARY_OUTPUT_DIRECTORY` 设置变量不生产可以使用目标属性来设置，`LIBRARY_OUTPUT_PATH` 设置变量生效
```
# 使用目标属性设置可执行文件的输出路径
set_target_properties(my_executable PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 使用目标属性设置共享库(动态链接库DLL)的输出路径
# CMAKE_LIBRARY_OUTPUT_DIRECTORY
set_target_properties(my_shared_library PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# 使用目标属性设置静态库的输出路径
set_target_properties(my_static_library PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
```

但是在启动模块依赖多个模块时，在启动模块CMakeList.txt中直接设置输出路径变量可以控制依赖模块输出到指定目录方便启动模块进行依赖管理。

输出变量setoutput.cmake
```
if (NOT DEFINED BUILD_BINDIR)
    if(MSVC)
        set(BUILD_BINDIR  ${CMAKE_SOURCE_DIR}/bin/x64/${CMAKE_BUILD_TYPE})
    else()
        set(BUILD_BINDIR  ${CMAKE_SOURCE_DIR}/${CMAKE_BUILD_TYPE})
    endif()
endif()
if (NOT DEFINED BUILD_LIBDIR)
    if(MSVC)
        set(BUILD_LIBDIR  ${CMAKE_SOURCE_DIR}/lib/x64/${CMAKE_BUILD_TYPE})
    else()
        set(BUILD_LIBDIR  ${CMAKE_SOURCE_DIR}/${CMAKE_BUILD_TYPE})
    endif()
endif()

set(EXECUTABLE_OUTPUT_PATH ${BUILD_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_BINDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_LIBDIR})
```

启动模块CMakeLists.txt设置输出变量
```
set(PROJECT_OUTPUT ${CMAKE_SOURCE_DIR}/cmake/setoutput.cmake)
include(${PROJECT_OUTPUT})
```