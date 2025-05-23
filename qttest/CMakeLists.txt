cmake_minimum_required(VERSION 3.16)

project(
    QtNote
    VERSION 1.0.0
    DESCRIPTION "QtTest Project"
    LANGUAGES CXX
)

if (MSVC)
    set(NOTE_SYSTEM_PLATFORM "win") # 设置cmake自定义变量用于指定三方库加载路径
    add_compile_definitions(WIN_PLATFORM=1) # 添加编译器定义识别平台，=前后不能有空格！

elseif(APPLE)

    message("CMAKE_OSX_ARCHITECTURES : " ${CMAKE_OSX_ARCHITECTURES})
    
    if(NOT ${CMAKE_OSX_ARCHITECTURES} STREQUAL "")
        set(SYSTEM_PLATFORM ${CMAKE_OSX_ARCHITECTURES})
    else()
        execute_process(COMMAND uname -m OUTPUT_VARIABLE NOTE_SYSTEM_PLATFORM) # 获取当前系统处理器架构
    endif()

    message("NOTE_SYSTEM_PLATFORM : " ${NOTE_SYSTEM_PLATFORM})
    
    if(NOTE_SYSTEM_PLATFORM MATCHES "arm*")
        # set(CMAKE_PREFIX_PATH "/xxx)
        set(CMAKE_OSX_ARCHITECTURES "arm64") # 设置macOS系统处理器架构
        set(NOTE_SYSTEM_PLATFORM "arm")
        add_compile_definitions(ARM_PLATFORM=1)
    elseif(NOTE_SYSTEM_PLATFORM MATCHES "x86_64")
        # set(CMAKE_PREFIX_PATH "/xxx")
        set(CMAKE_OSX_ARCHITECTURES "x86_64")
        set(NOTE_SYSTEM_PLATFORM "intel")
        add_compile_definitions(INTEL_PLATFORM=1)
    else()
        # set(CMAKE_PREFIX_PATH "/xxx")
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
        set(NOTE_SYSTEM_PLATFORM "universal")
        add_compile_definitions(UNIVERSAL_PLATFORM=1)
    endif()
endif()

add_subdirectory(qtstart)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/QtMaterial)
add_subdirectory(qwidget)
add_subdirectory(qml)
