cmake_minimum_required(VERSION 3.16)

project(
    QmlMaterial
    VERSION 1.0.0
    DESCRIPTION "Qml Material Shared Library Project"
    LANGUAGES CXX
)

set(MY_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include(${MY_CMAKE_DIR}/public.cmake)

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

# 查找 Qt 资源文件
set(MY_PROJECT_QRC )
file(GLOB_RECURSE MY_PROJECT_QRC "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc")

if (MSVC)
    MY_ADD_BUILD_DEFINE_FUNC(
        -DQMLMATERIAL_LIB
    )
else()
endif()

MY_ADD_QT_PACK_FUNC(
    Gui Quick
)

include(${MY_CMAKE_DIR}/option.cmake)
include(${MY_CMAKE_DIR}/code.cmake)
include(${MY_CMAKE_DIR}/so.cmake)
include(${MY_CMAKE_DIR}/link.cmake)
include(${MY_CMAKE_DIR}/install.cmake)
