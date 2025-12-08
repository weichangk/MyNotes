为什么我把执行目录里的dll删除了，重新编译没有拷贝dll到执行目录？

其实是 **CMake `POST_BUILD` 自定义命令的执行机制**导致的。让我详细解释一下原因和解决方案。

## 1️⃣ 原因分析

```
add_custom_command(TARGET EasyPhotoTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/quazip1-qt5$<$<CONFIG:Debug>:d>.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/zlib$<$<CONFIG:Debug>:d>1.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/bz2$<$<CONFIG:Debug>:d>.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/zlib/lib/$<CONFIG>/zlib$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:EasyPhotoTest>
    COMMENT "Copying DLLs according to configuration ($<CONFIG>)"
)
```

这里用了 **`copy_if_different`**，它的特性是：

- **只有源文件和目标文件不同**时才会执行复制。
- 如果目标文件不存在，它一般会复制，但有一个关键点：
  - **自定义命令只会在 CMake 认为目标需要“更新”时才运行**。
  - 对于 Visual Studio 等 **多配置生成器**，如果可执行文件本身没有被重新生成，`POST_BUILD` 命令可能不会触发，即使 DLL 被删掉也不行。

简而言之：

> `POST_BUILD` 是依附于 **目标文件是否重建** 的。
>  如果你删除了 DLL，但没有修改可执行文件或源文件，可执行文件没有重建，那么 `POST_BUILD` 不会被执行。

------

## 2️⃣ 解决方案

### 方法一：强制触发复制（推荐）

可以用 `ALL` 或添加一个 **自定义目标** 来保证每次构建都拷贝 DLL：

```
add_custom_target(copy_dlls ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/quazip1-qt5$<$<CONFIG:Debug>:d>.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/zlib$<$<CONFIG:Debug>:d>1.dll"
        $<TARGET_FILE_DIR:EasyPhotoTest>
    COMMENT "Copying DLLs to output directory"
)
```

然后让你的主目标依赖这个复制目标：

```
add_dependencies(EasyPhotoTest copy_dlls)
```

这样每次构建 `EasyPhotoTest`，都会先执行 `copy_dlls`，确保 DLL 始终存在。

------

### 方法二：删除后重新生成可执行文件

如果不想改 CMake 脚本：

1. 删除 DLL
2. **触发可执行文件重新编译**（例如修改一个源文件或者强制 Rebuild）
3. `POST_BUILD` 会执行，DLL 会被拷贝

> 缺点：每次都要手动修改文件或者 Rebuild，不方便。

------

### 方法三：使用 `copy` 替代 `copy_if_different`

```
add_custom_command(TARGET EasyPhotoTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/quazip1-qt5$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:EasyPhotoTest>
)
```

- 优点：每次执行都会覆盖
- 缺点：每次构建都会复制，即使 DLL 没变，效率略低

------

## 3️⃣ 总结

你的现象的根本原因是：

> `POST_BUILD` 命令依赖于 **目标文件是否重新生成**，删除 DLL 不会触发它。

解决方式：

1. 推荐：使用 **自定义目标 + add_dependencies** 强制每次拷贝
2. 或者修改源文件 / Rebuild 可执行文件
3. 或者用 `-E copy` 替代 `copy_if_different`（每次都复制）