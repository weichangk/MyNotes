cmake_minimum_required(VERSION 3.16)

if (MSVC)
    # Specify MSVC UTF-8 encoding
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

project(
    AmlLib
    VERSION 1.0.0
    DESCRIPTION "aml shared library project"
    LANGUAGES CXX
)

if (MSVC)
    set(BUILD_DEFINE
        -DAWIDGET_LIB
        -D_AMD64_
        -DWIN32_LEAN_AND_MEAN
        -DUNICODE
        -D_WINDOWS
        -D_UNICODE
        -DWIN32
        -DWIN64
    )
else()
    set(BUILD_DEFINE
        -DOBJC_OLD_DISPATCH_PROTOTYPES
    )
endif()
add_definitions(${BUILD_DEFINE})

set(CMAKE_PREFIX_PATH "d:/Qt/6.5.1/msvc2019_64") # Qt Kit Dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Gui Quick REQUIRED) # Qt COMPONENTS

qt_add_library(${PROJECT_NAME} SHARED)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Gui Qt6::Quick)

list(APPEND MODULE_QML_PRIVATE_FILES
    src/test1.qml
)
set_source_files_properties(${MODULE_QML_PRIVATE_FILES} PROPERTIES
    QT_QML_INTERNAL_TYPE  TRUE
)
list(APPEND MODULE_QML_FILES
    ${MODULE_QML_PRIVATE_FILES}
    src/test2.qml
)
list(APPEND MODULE_CPP_FILES 
    src/test.h
    src/test.cpp
)
list(APPEND MODULE_JS_FILES 
    src/test3.js
)
list(APPEND MODULE_ASSETS
    asset/icon/qt-logo.png
)

qt_add_qml_module(${PROJECT_NAME}
    URI Aml
    VERSION 1.0
    RESOURCE_PREFIX /weick.com/imports
    QML_FILES
        ${MODULE_QML_FILES}
        ${MODULE_JS_FILES}
    RESOURCES
        ${MODULE_ASSETS}
    SOURCES
        ${MODULE_CPP_FILES}
)