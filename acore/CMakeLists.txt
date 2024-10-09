# cmake_minimum_required(VERSION 3.5)

# if (MSVC)
#     # Specify MSVC UTF-8 encoding
#     add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
#     add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
# endif()

# project(
#     ACore
#     VERSION 1.0.0
#     DESCRIPTION "acore static library project"
#     LANGUAGES CXX)

# if (MSVC)
#     set(BUILD_DEFINE
#         -DACORE_LIB
#         -D_AMD64_
#         -DWIN32_LEAN_AND_MEAN
#         -DUNICODE
#         -D_WINDOWS
#         -D_UNICODE
#         -DWIN32
#         -DWIN64
#     )
# else()
#     set(BUILD_DEFINE
#         -DOBJC_OLD_DISPATCH_PROTOTYPES
#     )
# endif()
# add_definitions(${BUILD_DEFINE})

# set(CMAKE_PREFIX_PATH "d:/Qt/6.5.1/msvc2019_64") # Qt Kit Dir
# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(Qt6 COMPONENTS Widgets REQUIRED) # Qt COMPONENTS

# file(GLOB_RECURSE INC "inc/*.h" "inc/*.hpp")
# file(GLOB_RECURSE SRC "src/*.cpp" "src/*.cc")

# qt_add_library(${PROJECT_NAME} SHARED
#     ${INC}
#     ${SRC}
# )
# target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)



include(${CMAKE_SOURCE_DIR}/cmake/public.cmake)

project(
    ACore
    VERSION 1.0.0
    DESCRIPTION "acore static library project"
    LANGUAGES CXX
)

if (MSVC)
    MY_ADD_BUILD_DEFINE_FUNC(
        -DACORE_LIB
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