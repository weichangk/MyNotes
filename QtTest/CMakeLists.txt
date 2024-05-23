cmake_minimum_required(VERSION 3.16)

if (MSVC)
    # Specify MSVC UTF-8 encoding
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

project(
    QtTest
    VERSION 1.0.0
    DESCRIPTION "qttest project"
    LANGUAGES CXX
)

if (MSVC)
    set(BUILD_DEFINE
        -D_AMD64_
        -DWIN32_LEAN_AND_MEAN
        -DUNICODE
    )
else()
    set(BUILD_DEFINE
        -DOBJC_OLD_DISPATCH_PROTOTYPES
    )
endif()
add_definitions(${BUILD_DEFINE})

set(CMAKE_PREFIX_PATH "d:/Qt/6.5.1/msvc2019_64") # Qt Kit Dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QRC ./qtest.qrc)

set(PROJECT_LINKDIR
    "${CMAKE_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}"
)
if (MSVC)
    list(APPEND PROJECT_LINKDIR
        "${CMAKE_SOURCE_DIR}/../lib/x64/"
        "${CMAKE_SOURCE_DIR}/../lib/x64/${CMAKE_BUILD_TYPE}"
    )
else()
    list(APPEND PROJECT_LINKDIR
    )
endif()
link_directories(${PROJECT_LINKDIR}) # 由于依赖其他模块，使单独构建能够通过

find_package(Qt6 COMPONENTS Widgets REQUIRED) # Qt COMPONENTS

file(GLOB_RECURSE INC "inc/*.h" "inc/*.hpp")
file(GLOB_RECURSE SRC "inc/*.h" "inc/*.hpp" "src/*.cpp" "src/*.cc")

add_executable(${PROJECT_NAME}
    ${QRC}
    ${INC}
    ${SRC}
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

include(config.cmake)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_ID}")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    APPLICATION_ID="${PROJECT_ID}"
    APPLICATION_DISPLAY_NAME="${PROJECT_NAME}"
    APPLICATION_DESCRIPTION="${PROJECT_DESCRIPTION}"
    APPLICATION_ORGANIZATION_DOMAIN="${PROJECT_ORGANIZATION_URL}"
    APPLICATION_ORGANIZATION_NAME="${PROJECT_ORGANIZATION_NAME}"
    APPLICATION_DATA_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_ID}"
    APPLICATION_ICON_NAME="${PROJECT_ICON_FILE_NAME}"
    APPLICATION_ICON_SYSTEM_PATH="${PROJECT_ICON_FILE_PATH}/${PROJECT_ICON_FILE_NAME}"
)