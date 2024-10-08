set(MY_QT_VERSION_MAJOR 6)

set(MY_QT_DIR $ENV{QTDIR})

# 封装 Qt 路径查找设置
function(MY_SET_QT_PATH_FUNC)
    foreach(dir ${ARGN})
        if(EXISTS ${dir})
            set(CMAKE_PREFIX_PATH ${dir} PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

if(NOT EXISTS ${MY_QT_DIR})
    if(MSVC)
        MY_SET_QT_PATH_FUNC("D:/Qt/6.5.0/msvc2019_64" "D:/Qt/5.15.2/msvc2019_64" "D:/QtCopy/5.15.2/msvc2019_64" "E:/Qt/6.7.3/msvc2019_64")
    else()
        MY_SET_QT_PATH_FUNC("/Users/ws/Qt/6.5.0/clang_64" "/Users/ws/Qt/5.15.2/clang_64")
    endif()
else()
    set(CMAKE_PREFIX_PATH ${MY_QT_DIR})
endif()

if(NOT CMAKE_PREFIX_PATH)
    message(FATAL_ERROR "Could not find a valid Qt path.")
endif()

# 自动将当前源文件目录加入头文件搜索路径。
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 自动处理 Qt .ui 文件，生成相应的 C++ 代码。
set(CMAKE_AUTOUIC ON)

# 自动处理 Qt MOC 文件，生成信号与槽等功能所需的代码。
set(CMAKE_AUTOMOC ON)

# 自动处理 Qt 资源文件（.qrc），生成相应的 C++ 代码
set(CMAKE_AUTORCC ON)

# 查找 Qt ui文件
set(MY_PROJECT_UI )
file(GLOB_RECURSE MY_PROJECT_UI "${CMAKE_CURRENT_SOURCE_DIR}/*.ui")

# 查找 Qt ts文件
set(MY_PROJECT_TS )
file(GLOB_RECURSE MY_PROJECT_TS "${CMAKE_CURRENT_SOURCE_DIR}/*.ts")

# 查找 Qt 资源文件
set(MY_PROJECT_QRC )
file(GLOB_RECURSE MY_PROJECT_QRC "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc")

# # 处理 Qt 版本相关的宏，使用CMAKE_AUTOxxx后就不需要了
# if(${MY_QT_VERSION_MAJOR} GREATER_EQUAL 6)
#     qt6_wrap_cpp(MY_PROJECT_HEADER1 ${MY_PROJECT_HEADER})
#     qt6_wrap_ui(MY_PROJECT_UI ${MY_PROJECT_UI})
#     qt6_add_resources(MY_PROJECT_QRC ${MY_PROJECT_QRC})
# else()
#     qt5_wrap_cpp(MY_PROJECT_HEADER ${MY_PROJECT_HEADER})
#     qt5_wrap_ui(MY_PROJECT_UI ${MY_PROJECT_UI})
#     qt5_add_resources(MY_PROJECT_QRC ${MY_PROJECT_QRC})
# endif()

# Qt 依赖库变量及追加方法
set(MY_FIND_QT_PACK )
set(MY_FIND_QT_PACK_LINK )
macro(MY_ADD_QT_PACK_FUNC)
  list(APPEND MY_FIND_QT_PACK ${ARGN})
  foreach(arg ${ARGN})
      list(APPEND MY_FIND_QT_PACK_LINK Qt${MY_QT_VERSION_MAJOR}::${arg})
  endforeach()
endmacro(MY_ADD_QT_PACK_FUNC)