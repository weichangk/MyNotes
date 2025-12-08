配置阶段都无法判断CMAKE_BUILD_TYPE，这正是 **多配置生成器（如 Visual Studio / Xcode）下的特点**：

- 在 **配置阶段** `CMAKE_BUILD_TYPE` **是空的**
- 所以你无法直接在配置阶段判断 Debug / Release

但是 **CMake 已经提供了解决方案**：使用 **生成表达式 (Generator Expressions)**，它会在 **构建阶段**根据实际配置自动选择 Debug/Release 库。

# ✅ 推荐方法：使用 debug/optimized 关键字

```
target_link_libraries(EasyPhoto
    PRIVATE
        debug       ${CMAKE_SOURCE_DIR}/3rdparty/quazip/lib/quazip1-qt5d.lib
        debug       ${CMAKE_SOURCE_DIR}/3rdparty/zlib/lib/zlibd.lib
        optimized   ${CMAKE_SOURCE_DIR}/3rdparty/quazip/lib/quazip1-qt5.lib
        optimized   ${CMAKE_SOURCE_DIR}/3rdparty/zlib/lib/zlib.lib
)
```

解释：

- `debug` → 只在 Debug 配置构建时使用
- `optimized` → 只在非 Debug 配置构建时使用（Release/RelWithDebInfo/MinSizeRel）
- VS 多配置工程会根据 `--config Debug` / `--config Release` 自动选择库



# ✅ 另一种方法：使用生成表达式

```
target_link_libraries(EasyPhoto
    PRIVATE
        $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/3rdparty/quazip/lib/quazip1-qt5d.lib>
        $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/3rdparty/zlib/lib/zlibd.lib>
        $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/3rdparty/quazip/lib/quazip1-qt5.lib>
        $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/3rdparty/zlib/lib/zlib.lib>
)
```

- `$<CONFIG:Debug>` → 构建阶段自动判断当前是 Debug
- `$<CONFIG:Release>` → 构建阶段自动判断当前是 Release

> 生成表达式可以在 **多配置环境下延迟判断**，无需依赖 `CMAKE_BUILD_TYPE`

用 `$<CONFIG>` 自动选择 Debug / Release DLL

```
add_custom_command(TARGET EasyPhotoTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/quazip1-qt5$<$<CONFIG:Debug>:d>.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/zlib$<$<CONFIG:Debug>:d>1.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/quazip/lib/$<CONFIG>/bz2$<$<CONFIG:Debug>:d>.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/zlib/lib/$<CONFIG>/zlib$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:EasyPhotoTest>
    COMMENT "Copying quazip DLLs to output directory"
)
```

### 🔹 解释

1. `$<CONFIG>` → 当前构建配置（Debug / Release / RelWithDebInfo / MinSizeRel）
2. `$<$<CONFIG:Debug>:d>` → 仅在 Debug 配置下在库名后加 `d`
3. 这样：

| 构建配置 | 生成路径                      |
| -------- | ----------------------------- |
| Debug    | `.../debug/quazip1-qt5d.dll`  |
| Release  | `.../release/quazip1-qt5.dll` |

# ✅ 总结

1. **CMAKE_BUILD_TYPE** 在 **多配置生成器下不可靠**
2. **不要在配置阶段判断 Debug/Release**
3. **用 debug/optimized 或生成表达式** 让 CMake 在 **构建阶段**自动选择库
4. DLL 只需在 **POST_BUILD** 拷贝，不会被 target_link_libraries 直接使用