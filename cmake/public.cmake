# Cmake 最小版本
cmake_minimum_required(VERSION 3.16)

# 使用 C++17
set(CMAKE_CXX_STANDARD 17)

# 强制要求使用 C++ 标准版本
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动将当前目录添加到编译器的包含路径中，源文件中的 #include 语句可以直接引用当前目录中的头文件，而不需要使用相对路径或绝对路径。
set(CMAKE_INCLUDE_CURRENT_DIR ON) 

# macOS 的最低部署目标版本
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

# 启用 Objective-C 编程语言的支持，识别 .m 文件
enable_language(OBJC)

# 启用 Objective-C++ 编程语言的支持，识别 .mm 文件
enable_language(OBJCXX)

# 查找项目头文件
set(MY_PROJECT_HEADER )
file(GLOB_RECURSE MY_PROJECT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

# 查找项目源文件
set(MY_PROJECT_SOURCE )
if(MSVC)
    file(GLOB_RECURSE MY_PROJECT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
else()
    file(GLOB_RECURSE MY_PROJECT_SOURCE
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.mm"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.m"
    )
endif()

# 设置 QT 主版本
set(MY_QT_VERSION_MAJOR 6)

# 设置 QT 路径
set(MY_QT_DIR $ENV{QTDIR})

# 封装 Qt 路径查找设置
function(MY_SET_QT_PATH_FUNC)
    foreach(dir ${ARGN})
        if(EXISTS ${dir})
            set(CMAKE_PREFIX_PATH ${dir} PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

# 查找 QT 路径并设置
if(NOT EXISTS ${MY_QT_DIR})
    if(MSVC)
        MY_SET_QT_PATH_FUNC("D:/Qt/6.5.0/msvc2019_64" "D:/Qt/5.15.2/msvc2019_64" "D:/QtCopy/5.15.2/msvc2019_64" "E:/Qt/6.7.3/msvc2019_64")
    else()
        MY_SET_QT_PATH_FUNC("/Users/ws/Qt/6.5.0/clang_64" "/Users/ws/Qt/5.15.2/clang_64")
    endif()
else()
    set(CMAKE_PREFIX_PATH ${MY_QT_DIR})
endif()

if(NOT CMAKE_PREFIX_PATH)
    message(FATAL_ERROR "Could not find a valid Qt path.")
endif()

# 自动将当前源文件目录加入头文件搜索路径。
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 自动处理 Qt .ui 文件，生成相应的 C++ 代码。
set(CMAKE_AUTOUIC ON)

# 自动处理 Qt MOC 文件，生成信号与槽等功能所需的代码。
set(CMAKE_AUTOMOC ON)

# 自动处理 Qt 资源文件（.qrc），生成相应的 C++ 代码
set(CMAKE_AUTORCC ON)

# 查找 Qt ui文件
set(MY_PROJECT_UI )
file(GLOB_RECURSE MY_PROJECT_UI "${CMAKE_CURRENT_SOURCE_DIR}/*.ui")

# 查找 Qt ts文件
set(MY_PROJECT_TS )
file(GLOB_RECURSE MY_PROJECT_TS "${CMAKE_CURRENT_SOURCE_DIR}/*.ts")

# 查找 Qt 资源文件
set(MY_PROJECT_QRC )
file(GLOB_RECURSE MY_PROJECT_QRC "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc")

# Qt 依赖库变量及追加方法
set(MY_FIND_QT_PACK )
set(MY_FIND_QT_PACK_LINK )
macro(MY_ADD_QT_PACK_FUNC)
  list(APPEND MY_FIND_QT_PACK ${ARGN})
  foreach(arg ${ARGN})
      list(APPEND MY_FIND_QT_PACK_LINK Qt${MY_QT_VERSION_MAJOR}::${arg})
  endforeach()
endmacro(MY_ADD_QT_PACK_FUNC)

# 设置输出目录
if (NOT DEFINED MY_BUILD_BINDIR)
    if(MSVC)
        set(MY_BUILD_BINDIR ${CMAKE_SOURCE_DIR}/bin/x64/${CMAKE_BUILD_TYPE})
    else()
        set(BASE_BINDIR ${CMAKE_SOURCE_DIR}/${CMAKE_BUILD_TYPE})
        if (${MY_DST_PROJECT_NAME} STREQUAL ${MY_MAIN_APP_NAME})
            set(MY_BUILD_BINDIR ${BASE_BINDIR})
        else()
            set(MY_BUILD_BINDIR "${BASE_BINDIR}/${MY_MAIN_APP_NAME}.app/Contents/MacOS")
        endif()
    endif()
endif()
if (NOT DEFINED MY_BUILD_LIBDIR)
    set(MY_BUILD_LIBDIR ${CMAKE_SOURCE_DIR}/lib/x64/${CMAKE_BUILD_TYPE})
endif()

# 宏定义变量及追加方法
set(MY_BUILD_DEFINE )
if (MSVC)
    list(APPEND MY_BUILD_DEFINE
        -D_AMD64_
        -DWIN32_LEAN_AND_MEAN
        -DUNICODE
        -D_WINDOWS
        -D_UNICODE
        -DWIN32
        -DWIN64
    )
else()
    list(APPEND MY_BUILD_DEFINE -DOBJC_OLD_DISPATCH_PROTOTYPES)
endif()
macro(MY_ADD_BUILD_DEFINE_FUNC)
    list(APPEND MY_BUILD_DEFINE ${ARGN})
endmacro()

# 头文件的搜索路径变量及追加方法
set(MY_PROJECT_INCLUDE )
macro(MY_ADD_PROJECT_INCLUDE_FUNC)
  foreach(arg ${ARGN})
      list(APPEND MY_PROJECT_INCLUDE ${arg})
  endforeach()
endmacro(MY_ADD_PROJECT_INCLUDE_FUNC)

# 编译源文件变量及追加方法，用于添加程序外的源代码
set(MY_PROJECT_SOURCE_EXT )
macro(MY_ADD_PROJECT_SOURCE_EXT_FILE_FUNC)
    file(GLOB_RECURSE  MY_PROJECT_SOURCE_EXT_TEMP ${ARGN})
    list(APPEND MY_PROJECT_SOURCE_EXT ${MY_PROJECT_SOURCE_EXT_TEMP})
endmacro(MY_ADD_PROJECT_SOURCE_EXT_FILE_FUNC)

# 依赖库目录变量及追加方法
set(MY_PROJECT_LINKDIR )
set(MY_PROJECT_LINKDIR
    "${CMAKE_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}"
)
if (MSVC)
    list(APPEND MY_PROJECT_LINKDIR
        "${CMAKE_SOURCE_DIR}/../lib/x64/"
        "${CMAKE_SOURCE_DIR}/../lib/x64/${CMAKE_BUILD_TYPE}"
    )
else()
    list(APPEND MY_PROJECT_LINKDIR
    )
endif()
macro(MY_ADD_LINK_DIR_FUNC)
  foreach(arg ${ARGN})
      list(APPEND MY_PROJECT_LINKDIR ${arg})
  endforeach()
endmacro(MY_ADD_LINK_DIR_FUNC)

# 依赖库变量及追加方法
set(MY_PROJECT_LINKLIB )
macro(MY_ADD_LINK_LIB_FUNC)
  foreach(arg ${ARGN})
      list(APPEND MY_PROJECT_LINKLIB ${arg})
  endforeach()
endmacro(MY_ADD_LINK_LIB_FUNC)
macro(MY_ADD_LINK_FRAMEWORK_FUNC)
  foreach(arg ${ARGN})
      list(APPEND MY_PROJECT_LINKLIB "-framework ${arg}")
  endforeach()
endmacro(MY_ADD_LINK_FRAMEWORK_FUNC)