include(${CMAKE_SOURCE_DIR}/cmake/c.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/cpublic.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/q.cmake)

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

include(${CMAKE_SOURCE_DIR}/cmake/coption.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/ccode.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/qcode.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/qso.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/clink.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/qlink.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/install.cmake)
