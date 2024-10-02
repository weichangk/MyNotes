# 设置宏定义
add_definitions(${MY_BUILD_DEFINE})

# 设置头文件搜索目录
include_directories(${MY_PROJECT_INCLUDE})

# 设置依赖库链接目录
link_directories(${MY_PROJECT_LINKDIR})

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${MY_BUILD_BINDIR})
set(LIBRARY_OUTPUT_PATH ${MY_BUILD_BINDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MY_BUILD_LIBDIR})