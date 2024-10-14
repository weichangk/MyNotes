include(${CMAKE_SOURCE_DIR}/cmake/public.cmake)

project(
    QmlMaterialTest
    VERSION 1.0.0
    DESCRIPTION "Qml Material Test project"
    LANGUAGES CXX
)

MY_ADD_QT_PACK_FUNC(
    Core
    Qml
    Quick
)

include(${CMAKE_SOURCE_DIR}/qmlmaterial/vendor.cmake)
MY_ADD_VENDOR_SOURCES_FUNC(
    ${QMLMATERIAL_VENDOR_SOURCES}
)

include(${CMAKE_SOURCE_DIR}/cmake/option.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/code.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/exe.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/link.cmake)