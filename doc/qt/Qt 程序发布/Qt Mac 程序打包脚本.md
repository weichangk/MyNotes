
```bash
#!/bin/bash

QT_PATH="/Users/ws/qt5.15-macos-build/qtbase" #/Users/ws/qt5.15-macos-build/qtbase #/Users/ws/Qt/5.15.2/clang_64
CMAKE_OSX_ARCHITECTURES="arm64" #arm64 #x86_64 #arm64;x86_64
BUILD_TYPE="Release" #Debug Release RelWithDebInfo MinSizeRel
CMAKE_PATH=$(which cmake)
CLANG_PATH=$(which clang)
CLANGPP_PATH=$(which clang++)

WORK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$WORK_DIR/../../.."
SOURCE_DIR="$(pwd)"
if [ "$CMAKE_OSX_ARCHITECTURES" == "arm64;x86_64" ]; then
    ARCH_ALIAS="universal"
else
    ARCH_ALIAS=$(echo "$CMAKE_OSX_ARCHITECTURES" | tr ';' '-')
fi
BUILD_DIR="$SOURCE_DIR/build_${ARCH_ALIAS}_${BUILD_TYPE}"

if command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
    BUILD_CMD="$CMAKE_PATH --build \"$BUILD_DIR\" --config \"$BUILD_TYPE\""
    echo "✅ 使用构建系统: Ninja"
else
    GENERATOR="Unix Makefiles"
    CPU_CORES=$(sysctl -n hw.logicalcpu)
    BUILD_CMD="$CMAKE_PATH --build \"$BUILD_DIR\" --target all -j $CPU_CORES"
    echo "⚠️ 未安装 Ninja，使用 Unix Makefiles ($CPU_CORES 线程)"
fi

echo "🛠️ 配置 CMake ($GENERATOR)..."
start_time=$(date +%s)

"$CMAKE_PATH" \
    -DCMAKE_OSX_ARCHITECTURES="$CMAKE_OSX_ARCHITECTURES" \
    -DCMAKE_PREFIX_PATH="$QT_PATH" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_C_COMPILER="$CLANG_PATH" \
    -DCMAKE_CXX_COMPILER="$CLANGPP_PATH" \
    --no-warn-unused-cli \
    -G "$GENERATOR" \
    -S "$SOURCE_DIR" \
    -B "$BUILD_DIR"

if [ $? -ne 0 ]; then
    echo "❌ CMake 配置失败"
    exit 1
fi

end_time=$(date +%s)
duration=$((end_time - start_time))
echo "✅ 配置 CMake 完成，用时 ${duration}s"

echo "🚀 开始构建..."
start_time=$(date +%s)

eval "$BUILD_CMD"

if [ $? -ne 0 ]; then
    echo "❌ 构建失败"
    exit 1
fi

end_time=$(date +%s)
duration=$((end_time - start_time))
echo "🎉 构建成功，用时 ${duration}s"


echo "收集符号文件"

echo "打包 Qt 依赖"

echo "添加rpath"

echo "添加软链接"

echo "签名"

echo "公证"

echo "归档"
```