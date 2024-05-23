# QT使用CMAKE注意事项
QT 子模块没有生成 moc 文件导致引用模块无法链接 QT 子模块

使用 target_include_directories 添加头文件目录不会生成 moc 文件，没找到对应的设置！！！

使用变量的方式添加头文件可以生成 moc 文件
```
set(inc
    ./inc/awidget.h
)
aux_source_directory(./src src)
qt_add_library(${PROJECT_NAME} STATIC
    ${inc}
    #${src}
)
target_sources(${PROJECT_NAME} PRIVATE ${src})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
```

使用遍历文件到列表里，不需要一个个添加
```
file(GLOB_RECURSE inc "${PROJECT_SOURCE_DIR}/*.h" "${PROJECT_SOURCE_DIR}/*.hpp")
file(GLOB_RECURSE src "${PROJECT_SOURCE_DIR}/*.cpp" "${PROJECT_SOURCE_DIR}/*.cc")

file(GLOB_RECURSE INC "inc/*.h" "inc/*.hpp")
file(GLOB_RECURSE SRC "src/*.cpp" "src/*.cc")
```