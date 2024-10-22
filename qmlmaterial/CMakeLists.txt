cmake_minimum_required(VERSION 3.16)

project(
    QmlMaterial
    VERSION 1.0.0
    DESCRIPTION "Qml Material Shared Library Project"
    LANGUAGES CXX
)

set(MY_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include(${MY_CMAKE_DIR}/public.cmake)


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
