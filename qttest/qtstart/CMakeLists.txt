# cmake_minimum_required(VERSION 3.16)

# project(
#     QtStart
#     VERSION 1.0.0
#     DESCRIPTION "QtStart Project"
#     LANGUAGES CXX
# )

# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)

# set(MY_PROJECT_HEADER )
# file(GLOB_RECURSE MY_PROJECT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

# set(MY_PROJECT_SOURCE )
# file(GLOB_RECURSE MY_PROJECT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

# set(MY_PROJECT_QRC qtstart.qrc)

# find_package(Qt5 COMPONENTS Widgets REQUIRED)

# add_executable(${PROJECT_NAME}
#     ${MY_PROJECT_HEADER}
#     ${MY_PROJECT_SOURCE}
#     ${MY_PROJECT_QRC}
# )

# target_link_libraries(${PROJECT_NAME} Qt5::Widgets)


include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/public.cmake)

project(
    QtStart
    VERSION 1.0.0
    DESCRIPTION "QtStart Project"
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
)

MY_ADD_QT_PACK_FUNC(
    Widgets
)

MY_ADD_LINK_LIB_FUNC(
    QtMaterial
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/option.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/code.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/exe.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/link.cmake)