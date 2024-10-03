# 设置输出目录
if (NOT DEFINED MY_BUILD_BINDIR)
    if(MSVC)
        set(MY_BUILD_BINDIR ${CMAKE_SOURCE_DIR}/bin/x64/${CMAKE_BUILD_TYPE})
    else()
        set(BASE_BINDIR ${CMAKE_SOURCE_DIR}/${CMAKE_BUILD_TYPE})
        if (${MY_DST_PROJECT_NAME} STREQUAL ${MY_MAIN_APP_NAME})
            set(MY_BUILD_BINDIR ${BASE_BINDIR})
        else()
            set(MY_BUILD_BINDIR "${BASE_BINDIR}/${MY_MAIN_APP_NAME}.app/Contents/MacOS")
        endif()
    endif()
endif()
if (NOT DEFINED MY_BUILD_LIBDIR)
    set(MY_BUILD_LIBDIR ${CMAKE_SOURCE_DIR}/lib/x64/${CMAKE_BUILD_TYPE})
endif()

# 宏定义变量及追加方法
set(MY_BUILD_DEFINE )
if (MSVC)
    list(APPEND MY_BUILD_DEFINE
        -D_AMD64_
        -DWIN32_LEAN_AND_MEAN
        -DUNICODE
        -D_WINDOWS
        -D_UNICODE
        -DWIN32
        -DWIN64
    )
else()
    list(APPEND MY_BUILD_DEFINE -DOBJC_OLD_DISPATCH_PROTOTYPES)
endif()
macro(MY_ADD_BUILD_DEFINE_FUNC)
    list(APPEND MY_BUILD_DEFINE ${ARGN})
endmacro()

# 头文件的搜索路径变量及追加方法
set(MY_PROJECT_INCLUDE )
macro(MY_ADD_PROJECT_INCLUDE_FUNC)
  foreach(arg ${ARGN})
      list(APPEND MY_PROJECT_INCLUDE ${arg})
  endforeach()
endmacro(MY_ADD_PROJECT_INCLUDE_FUNC)

# 编译源文件变量及追加方法
set(MY_PROJECT_SOURCE )
macro(MY_ADD_PROJECT_SOURCE_FILE_FUNC)
    file(GLOB_RECURSE  MY_PROJECT_SOURCE_TEMP ${ARGN})
    list(APPEND MY_PROJECT_SOURCE ${MY_PROJECT_SOURCE_TEMP})
endmacro(MY_ADD_PROJECT_SOURCE_FILE_FUNC)

# 依赖库目录变量及追加方法
set(MY_PROJECT_LINKDIR )
set(MY_PROJECT_LINKDIR
    "${CMAKE_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}"
)
if (MSVC)
    list(APPEND MY_PROJECT_LINKDIR
        "${CMAKE_SOURCE_DIR}/../lib/x64/"
        "${CMAKE_SOURCE_DIR}/../lib/x64/${CMAKE_BUILD_TYPE}"
    )
else()
    list(APPEND MY_PROJECT_LINKDIR
    )
endif()
macro(MY_ADD_LINK_DIR_FUNC)
  foreach(arg ${ARGN})
      list(APPEND MY_PROJECT_LINKDIR ${arg})
  endforeach()
endmacro(MY_ADD_LINK_DIR_FUNC)

# 依赖库变量及追加方法
set(MY_PROJECT_LINKLIB )
macro(MY_ADD_LINK_LIB_FUNC)
  foreach(arg ${ARGN})
      list(APPEND MY_PROJECT_LINKLIB ${arg})
  endforeach()
endmacro(MY_ADD_LINK_LIB_FUNC)
macro(MY_ADD_LINK_FRAMEWORK_FUNC)
  foreach(arg ${ARGN})
      list(APPEND MY_PROJECT_LINKLIB "-framework ${arg}")
  endforeach()
endmacro(MY_ADD_LINK_FRAMEWORK_FUNC)

# 二方库依赖库变量及追加方法
set(MY_FIND_2NDPARTY_PACK )
set(MY_FIND_2NDPARTY_PACK_LINK )
macro(MY_ADD_2NDPARTY_PACK_FUNC)
  list(APPEND MY_FIND_2NDPARTY_PACK ${ARGN})
  foreach(arg ${ARGN})
      list(APPEND MY_FIND_2NDPARTY_PACK_LINK ${arg})
  endforeach()
endmacro(MY_ADD_2NDPARTY_PACK_FUNC)

# 三方库依赖库变量及追加方法
set(MY_FIND_3RDPARTY_PACK )
set(MY_FIND_3RDPARTY_PACK_LINK )
macro(MY_ADD_3RDPARTY_PACK_FUNC)
  list(APPEND MY_FIND_3RDPARTY_PACK ${ARGN})
  foreach(arg ${ARGN})
      list(APPEND MY_FIND_3RDPARTY_PACK_LINK ${arg})
  endforeach()
endmacro(MY_ADD_3RDPARTY_PACK_FUNC)