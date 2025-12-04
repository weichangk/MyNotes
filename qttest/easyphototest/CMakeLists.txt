include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/public.cmake)

project(
    EasyPhotoTest
    VERSION 1.0.0
    DESCRIPTION "EasyPhotoTest Project"
    LANGUAGES CXX
)

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

MY_ADD_PROJECT_INCLUDE_FUNC(
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/zlib/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/inc
)

MY_ADD_LINK_DIR_FUNC(
    debug ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/zlib/lib/debug
    debug ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/debug
    optimized ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/zlib/lib/release
    optimized ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/release
)

# 定义 quazip 使用 Qt5 的 QTextCodec
# add_definitions(-DQUAZIP_CAN_USE_QTEXTCODEC)

MY_ADD_QT_PACK_FUNC(
    Widgets
)

MY_ADD_LINK_LIB_FUNC(
    debug QtMaterial
    debug quazip1-qt5d
    debug zlibd
    optimized QtMaterial
    optimized quazip1-qt5
    optimized zlib
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/option.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/code.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/exe.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/link.cmake)

# 复制运行时 DLL
add_custom_command(TARGET EasyPhotoTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/quazip1-qt5$<$<CONFIG:Debug>:d>.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/zlib$<$<CONFIG:Debug>:d>1.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/bz2$<$<CONFIG:Debug>:d>.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/zlib/lib/$<CONFIG>/zlib$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:EasyPhotoTest>
    COMMENT "Copying DLLs according to configuration ($<CONFIG>)"
)

# 备注关于 POST_BUILD 和 copy_if_different 的行为：
#
#-E copy_if_different 作用：复制文件到指定位置，但如果目标文件已经存在且内容相同，就不会覆盖。这样可以避免每次编译都重复复制文件，提高效率。
#
# 但是也存在DLL没有被复制的问题，原因如下：
#
#只有源文件和目标文件不同时才会执行复制。
#
#如果目标文件不存在，它一般会复制，但有一个关键点：
#
#自定义命令只会在 CMake 认为目标需要“更新”时才运行。
#
#对于 Visual Studio 等 多配置生成器，如果可执行文件本身没有被重新生成，POST_BUILD 命令可能不会触发，即使 DLL 被删掉也不行。
#
#简而言之：
#
#POST_BUILD 是依附于 目标文件是否重建 的。
#如果你删除了 DLL，但没有修改可执行文件或源文件，可执行文件没有重建，那么 POST_BUILD 不会被执行。