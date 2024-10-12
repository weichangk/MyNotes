include(${CMAKE_SOURCE_DIR}/cmake/public.cmake)

project(
    QtMaterial
    VERSION 1.0.0
    DESCRIPTION "Qt Material Shared Library Project"
    LANGUAGES CXX
)

if (MSVC)
    MY_ADD_BUILD_DEFINE_FUNC(
        -DQTMATERIAL_LIB
    )
else()
endif()

MY_ADD_QT_PACK_FUNC(
    Widgets
)

MY_ADD_LINK_FRAMEWORK_FUNC(
    AppKit
    Foundation
)

include(${CMAKE_SOURCE_DIR}/cmake/option.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/code.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/so.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/link.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/install.cmake)
