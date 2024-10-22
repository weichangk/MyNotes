cmake_minimum_required(VERSION 3.16)

project(
    QtMaterial
    VERSION 1.0.0
    DESCRIPTION "Qt Material Shared Library Project"
    LANGUAGES CXX
)

set(MY_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include(${MY_CMAKE_DIR}/public.cmake)

if (MSVC)
    MY_ADD_BUILD_DEFINE_FUNC(
        -DQTMATERIAL_LIB
    )
else()
endif()

MY_ADD_PROJECT_INCLUDE_FUNC(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

MY_ADD_QT_PACK_FUNC(
    Widgets
)

if(APPLE)
MY_ADD_LINK_FRAMEWORK_FUNC(
    AppKit
    Foundation
)
endif()

include(${MY_CMAKE_DIR}/option.cmake)
include(${MY_CMAKE_DIR}/code.cmake)
include(${MY_CMAKE_DIR}/so.cmake)
include(${MY_CMAKE_DIR}/link.cmake)
include(${MY_CMAKE_DIR}/install.cmake)
