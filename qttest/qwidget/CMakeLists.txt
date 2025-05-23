include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/public.cmake)

project(
    QWidgetTest
    VERSION 1.0.0
    DESCRIPTION "QWidgetTest Project"
    LANGUAGES CXX
)

# 查找项目头文件
set(MY_PROJECT_HEADER )
file(GLOB_RECURSE MY_PROJECT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

# 查找项目源文件
set(MY_PROJECT_SOURCE )
if(MSVC)
    file(GLOB_RECURSE MY_PROJECT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
else()
    file(GLOB_RECURSE MY_PROJECT_SOURCE
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.mm"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.m"
    )
endif()

MY_ADD_PROJECT_INCLUDE_FUNC(
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/inc
)

MY_ADD_QT_PACK_FUNC(
    Widgets
)

if(APPLE)
    MY_ADD_LINK_FRAMEWORK_FUNC(
        AppKit Metal DiskArbitration IOKit OpenGL AGL
        AVFoundation Carbon CoreAudio
        CoreFoundation CoreServices
    )
endif()

MY_ADD_LINK_LIB_FUNC(
    QtMaterial
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/option.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/code.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/exe.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/QtMaterial/cmake/link.cmake)

include(config.cmake)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_ID}")

if(MSVC)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.rc")
    target_sources(${PROJECT_NAME} PRIVATE ${APP_ICON_RESOURCE_WINDOWS})
endif()

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