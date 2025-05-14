include(${CMAKE_SOURCE_DIR}/3rdparty/QtMaterial/cmake/public.cmake)

project(
    QMLTest
    VERSION 1.0.0
    DESCRIPTION "QMLTest Project"
    LANGUAGES CXX
)

MY_ADD_QT_PACK_FUNC(
    Core
    Qml
    Quick
)

include(${CMAKE_SOURCE_DIR}/3rdparty/QtMaterial/qml/vendor.cmake)
MY_ADD_VENDOR_SOURCES_FUNC(
    ${QMLMATERIAL_VENDOR_SOURCES}
)

include(${CMAKE_SOURCE_DIR}/3rdparty/QtMaterial/cmake/option.cmake)
include(${CMAKE_SOURCE_DIR}/3rdparty/QtMaterial/cmake/code.cmake)
include(${CMAKE_SOURCE_DIR}/3rdparty/QtMaterial/cmake/exe.cmake)
include(${CMAKE_SOURCE_DIR}/3rdparty/QtMaterial/cmake/link.cmake)