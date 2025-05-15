使用主cmake项目进行构建时，子项目cmake可以这样写
```
include(${CMAKE_SOURCE_DIR}/cmake/public.cmake)

project(
    QtTest
    VERSION 1.0.0
    DESCRIPTION "qttest project"
    LANGUAGES CXX
)
```

如果要使得子项目能单独构建需要如下改造
```
cmake_minimum_required(VERSION 3.16) # 需要设置cmake最小版本

project(
    QtMaterial
    VERSION 1.0.0
    DESCRIPTION "Qt Material Shared Library Project"
    LANGUAGES CXX
)

set(MY_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cmake) # 需要设置cmake脚本路径
include(${MY_CMAKE_DIR}/public.cmake) # 设置project要在public.cmake之前，否则会遇到public.cmake中的if(MSVC)不生效！
```