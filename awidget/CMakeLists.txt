include(${CMAKE_SOURCE_DIR}/cmake/public.cmake)

project(
    AWidget
    VERSION 1.0.0
    DESCRIPTION "awidget shared library project"
    LANGUAGES CXX
)

if (MSVC)
    MY_ADD_BUILD_DEFINE_FUNC(
        -DAWIDGET_LIB
    )
else()
endif()

MY_ADD_QT_PACK_FUNC(
    Widgets
)

include(${CMAKE_SOURCE_DIR}/cmake/option.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/code.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/so.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/link.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/install.cmake)
