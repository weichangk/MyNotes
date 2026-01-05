Qt Mac 程序打包脚本

```bash
#!/bin/bash

echo "🛠️ 开始打包..."

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
OUTPUT_DIR="$SOURCE_DIR/${BUILD_TYPE}" # 要和CMake配置的构建输出目录一致
ARCHIVE_DIR="$SOURCE_DIR/archive"
DSYM_DIR="$ARCHIVE_DIR/alldsym"

if command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
    BUILD_CMD="$CMAKE_PATH --build \"$BUILD_DIR\" --config \"$BUILD_TYPE\""
    echo "✅ 使用构建系统: Ninja"
else
    GENERATOR="Unix Makefiles"
    CPU_CORES=$(sysctl -n hw.logicalcpu)
    BUILD_CMD="$CMAKE_PATH --build \"$BUILD_DIR\" --target all -j $CPU_CORES"
    echo "⚠️ 未安装 Ninja，使用构建系统: Unix Makefiles ($CPU_CORES 线程)"
fi

# 不清理减少耗时
# echo "🧹 清理构建目录 $BUILD_DIR"
# rm -rf "$BUILD_DIR"

echo "🧹 清理输出目录 $OUTPUT_DIR"
rm -rf "$OUTPUT_DIR"

echo "🧹 清归档目录 $ARCHIVE_DIR"
rm -rf "$ARCHIVE_DIR"
mkdir "$ARCHIVE_DIR"
mkdir "$DSYM_DIR"

echo "🛠️ 开始构建..."
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
    echo "❌ 构建失败"
    exit 1
fi

end_time=$(date +%s)
duration=$((end_time - start_time))
echo "✅ 构建完成，用时 ${duration}s"

echo "🚀 开始编译..."
start_time=$(date +%s)

eval "$BUILD_CMD"

if [ $? -ne 0 ]; then
    echo "❌ 编译失败"
    exit 1
fi

end_time=$(date +%s)
duration=$((end_time - start_time))
echo "🎉 编译完成，用时 ${duration}s"


echo "收集符号文件"
cp -Rf ../Release/*.dSYM ./all_dsym
cp -Rf ../Release/Wondershare\ DemoCreator\ 8.app/Contents/MacOS/*.dSYM ./all_dsym
cp -Rf ../Release/Wondershare\ DemoCreator\ 8.app/Contents/MacOS/DemoCreator\ Record\ 8.app/Contents/MacOS/*.dSYM ./all_dsym

rm -rf ../Release/*.dSYM
rm -rf ./Wondershare\ DemoCreator\ 8.app/Contents/MacOS/*.dSYM
rm -rf ./Wondershare\ DemoCreator\ 8.app/Contents/MacOS/DemoCreator\ Record\ 8.app/Contents/MacOS/*.dSYM


echo "打包 Qt 依赖"


echo "添加rpath"
#修改dylib,frameworks中的rpath
echo "start add_rpath"
source ./add_rpath.sh ./Wondershare\ DemoCreator\ 8.app/Contents/MacOS
source ./add_rpath.sh ./Wondershare\ DemoCreator\ 8.app/Contents/Frameworks
source ./add_rpath_frameworks.sh ./Wondershare\ DemoCreator\ 8.app/Contents/Frameworks
echo "end add_rpath"


echo "添加软链接"
#!/bin/bash
cd `dirname $0`
CURRENT_DIR=$(pwd)

echo "start make Wondershare DemoCreator 8.app reyon";


list=("./Default Effects" "./Device" "./Fonts" "./Guide" "./GxBeautifyConfig" "./lang" "./Motions" "./preEffects" "./script" "./Sounds"
      "./SubtitlePulsation" "./UIImages" "./WaterMark" "./AudioPlugins" "./CameraSettingFiles" "./Effect" "./pysocialnetwork")


cd ./Wondershare\ DemoCreator\ 8.app/Contents/MacOS/DemoCreator\ Record\ 8.app/Contents/Resources

exit_code=$?
if [[ $exit_code != 0 ]]; then
    echo "run relyon error: "$exit_code
    exit $exit_code
fi

# 循环处理列表
for dir in "${list[@]}"; do
  if [ -d "$dir" ]; then
    echo "relyon path: $dir"

    rm -r "$dir"
    dir_name=$(basename "$dir")
    ln -s "../../../../Resources/$dir_name" "$dir"
  fi
done


cd ../../../DemoCreatorEditor\ 8.app/Contents/Resources

exit_code=$?
if [[ $exit_code != 0 ]]; then
    echo "run relyon error: "$exit_code
    exit $exit_code
fi

for dir in "${list[@]}"; do
  if [ -d "$dir" ]; then
    echo "relyon path: $dir"

    rm -r "$dir"
    dir_name=$(basename "$dir")
    ln -s "../../../../Resources/$dir_name" "$dir"
  fi
done

cd $CURRENT_DIR

echo "end make Wondershare DemoCreator 8.app reyon ";









echo "签名"
echo "start Codesign =========== ";

KPAppCodesign -s "$appPath" -t 1 --timestamp -o "runtime"

echo "end Codesign =========== ";











echo "公证"
echo "start Notarized Developer ID =========== ";

sh notarizationx.sh -s "$appPath" -u "MacNotarization@wondershare.com" -p "numw-kuet-rwgy-qycf" -t "B72G3TW8NM"

echo "end Notarized Developer ID =========== ";






echo "归档"

ditto -c -k --sequesterRsrc --keepParent ./Wondershare\ DemoCreator\ 8.app ./output/Wondershare\ DemoCreator\ 8.zip

# 编译日志文件
#cp -f ./build.log ./all_dsym

rm -f ./output/all_dsym.zip
ditto -c -k --sequesterRsrc --keepParent ./all_dsym ./output/all_dsym.zip



```