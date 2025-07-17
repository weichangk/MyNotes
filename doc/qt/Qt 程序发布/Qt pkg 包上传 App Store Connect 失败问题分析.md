#### Validation failed (409) Invalid bundle. The “xxx.app” bundle supports arm64 but not Intel-based Mac computers. Your build must include the x86_64 architecture to support Intel-based Mac computers. To support arm64 only, your macOS deployment target must be 12.0 or higher. For details, view: https://developer.apple.com/documentation/xcode/building_a_universal_macos_binary. (ID: efb52904-f848-4046-bf4d-5bed380889bf)

你上传的 `.app` 包（`xxx.app`）只支持 **arm64 架构（即 Apple Silicon）**，而不支持 **x86_64 架构（即 Intel Mac）**。

Apple 要求：

- 如果你只支持 **arm64（Apple Silicon）**，那你必须把 **macOS 最低部署目标设为 macOS 12.0**。
- 如果你希望支持 **macOS 11 或更早版本**，你必须构建 **Universal Binary（同时包含 x86_64 和 arm64）**。
- 当前你的 `.app`：
  - 只包含 arm64。
  - 但部署目标小于 macOS 12（例如 11 或更早）。
  - 因此导致验证失败。

解决方案，两个选择：

**方案一：构建 Universal Binary（支持 arm64 + x86_64），这样可以兼容大多数 Mac，包括 Intel 和 Apple Silicon。**

修改 CMake 构建设置：

在 `CMakeLists.txt` 或构建脚本中添加：

```cmake
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
```

或者，在终端构建时添加参数：

```cmake
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ..
```

然后重新构建 `.app` 包。

**方案二：只支持 arm64，但设置 macOS 最低部署版本为 12.0**

在 `CMakeLists.txt` 中添加：

```cmake
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")
```

或者使用环境变量或命令行：

```cmake
export MACOSX_DEPLOYMENT_TARGET=12.0
```

⚠️ 注意：这种方式打包后的 `.app` 将无法在 macOS 11 或更早系统上运行。

##### 

#### Validation failed (409) App sandbox not enabled. The following executables must include the "com.apple.security.app-sandbox" entitlement with a Boolean value of true in the entitlements property list: [( "com.xxx.pkg/Payload/xxx.app/Contents/MacOS/xxx" )] Refer to App Sandbox page at https://developer.apple.com/documentation/security/app_sandbox for more information on sandboxing your app. (ID: 1162f1c6-c50f-4bdc-a4fe-5caf700198af)

这个错误的核心提示是：

> **你的应用未启用 App Sandbox，Apple 不允许上传未启用沙箱的应用。**

Apple 要求 **提交到 App Store 或使用 notarization（公证）分发的 macOS 应用**必须启用 **App Sandbox**。

错误中提到的二进制文件是：

```
xxx.app/Contents/MacOS/xxx
```

这个主程序没有启用 App Sandbox。

解决方法：添加沙箱权限到你的 App，需要做两步：

**创建并配置 `entitlements.plist`**

创建一个名为 `entitlements.plist` 的文件，内容如下：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
   "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.app-sandbox</key>
    <true/>
</dict>
</plist>
```

如果你还需要访问网络、文件系统等能力，可以添加更多的 entitlement，例如：

```xml
<key>com.apple.security.network.client</key>
<true/>
<key>com.apple.security.files.user-selected.read-write</key>
<true/>
```

👉 参考：[Apple App Sandbox 权限文档](https://developer.apple.com/documentation/security/app_sandbox)

**使用 `codesign` 添加沙箱权限**

在打包完成 `.app` 后，使用以下命令进行签名：

```cpp
codesign --deep --force --options runtime \
  --entitlements path/to/entitlements.plist \
  --sign "Developer ID Application: Your Company (TeamID)" \
  "xxx.app"
```

⚠️ 注意事项：

- `--entitlements` 指定你刚刚创建的 `entitlements.plist`。
- `--options runtime` 是为启用 App Sandbox 和 Hardened Runtime。
- `--sign` 后面的签名证书要与你的开发者账号匹配。

**检查签名是否生效**

签名后，你可以运行以下命令确认：

```
codesign -d --entitlements :- "xxx.app"
```

输出中应该包含：

```xml
<key>com.apple.security.app-sandbox</key>
<true/>
```

**如果你用的是 CMake + Xcode**

你可以在 `CMakeLists.txt` 中添加：

```cmake
set_target_properties(${PROJECT_NAME} PROPERTIES
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_SOURCE_DIR}/entitlements.plist"
)
```

总结

| 步骤 | 操作                                                     |
| ---- | -------------------------------------------------------- |
| 1    | 创建 `entitlements.plist`，添加沙箱权限                  |
| 2    | 使用 `codesign` 命令签名并指定沙箱                       |
| 3    | 确保签名成功并包含 `com.apple.security.app-sandbox` 权限 |



##### 您说：

<?xml version="1.0" encoding="UTF-8"?> <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd"> <plist version="1.0"> <dict> 	<key>com.apple.security.app-sandbox</key> 	<true/> 	<key>com.apple.security.application-groups</key> 	<array> 		<string>YZC2T44ZDX.filmora.shared</string> 		<string>YZC2T44ZDX.filmora.subapp</string> 		<string>group.com.wondershare.filmora-app</string> 	</array> 	<key>com.apple.security.assets.movies.read-write</key> 	<true/> 	<key>com.apple.security.assets.music.read-only</key> 	<true/> 	<key>com.apple.security.assets.pictures.read-only</key> 	<true/> 	<key>com.apple.security.cs.allow-dyld-environment-variables</key> 	<true/> 	<key>com.apple.security.cs.allow-jit</key> 	<true/> 	<key>com.apple.security.cs.allow-unsigned-executable-memory</key> 	<true/> 	<key>com.apple.security.cs.disable-executable-page-protection</key> 	<true/> 	<key>com.apple.security.cs.disable-library-validation</key> 	<true/> 	<key>com.apple.security.device.audio-input</key> 	<true/> 	<key>com.apple.security.device.camera</key> 	<true/> 	<key>com.apple.security.files.bookmarks.app-scope</key> 	<true/> 	<key>com.apple.security.files.bookmarks.document-scope</key> 	<true/> 	<key>com.apple.security.files.downloads.read-write</key> 	<true/> 	<key>com.apple.security.files.user-selected.read-write</key> 	<true/> 	<key>com.apple.security.network.client</key> 	<true/> 	<key>com.apple.security.personal-information.photos-library</key> 	<true/> 	<key>com.apple.security.print</key> 	<true/> 	<key>com.apple.security.temporary-exception.apple-events</key> 	<array> 		<string>com.apple.itunes</string> 	</array> 	<key>com.apple.security.temporary-exception.shared-preference.read-only</key> 	<array> 		<string>com.apple.iphoto</string> 		<string>com.apple.photobooth</string> 		<string>com.apple.photos</string> 	</array> 	<key>com.apple.security.device.usb</key> 	<true/> </dict> </plist>



###### ChatGPT 说：

你贴出的 `entitlements.plist` 已经启用了沙箱权限（`com.apple.security.app-sandbox` 为 `true`），并配置了大量权限项，包括访问相机、麦克风、照片、网络、文件系统、音频输入等，**内容本身没有明显问题**。

但如果 Apple 仍然报 “App sandbox not enabled” 的错误，可能原因如下：

------

### ✅ **检查关键点**

#### ✅ 1. 是否真的在签名时使用了这个 `entitlements.plist`？

你需要确保在 `codesign` 时指定了这个文件，例如：

```
bash复制编辑codesign --deep --force --options runtime \
  --entitlements path/to/entitlements.plist \
  --sign "Developer ID Application: Your Company (Team ID)" \
  "Wondershare DemoCreator 8.app"
```

👉 `--entitlements` 是必须指定的，否则签名时不会生效。

------

#### ✅ 2. 是否签名了正确的可执行文件（主程序）？

错误提示中提到的路径是：

```
swift


复制编辑
Wondershare DemoCreator 8.app/Contents/MacOS/Wondershare DemoCreator 8
```

你必须签名的是整个 `.app` 包：

```
css


复制编辑
codesign --deep --force ... "Wondershare DemoCreator 8.app"
```

✅ **推荐加 `--deep` 参数**，确保签名作用于 `.app` 里面的每个子可执行文件。

------

#### ✅ 3. 签名后是否成功包含沙箱？

可以验证签名是否真的生效：

```
bash


复制编辑
codesign -d --entitlements :- "Wondershare DemoCreator 8.app"
```

输出中应该包含：

```
xml复制编辑<key>com.apple.security.app-sandbox</key>
<true/>
```

如果没有，说明签名时没生效，要检查 `codesign` 命令或路径是否正确。

------

### ✅ 4. 检查包内是否有其他可执行文件未被签名或未使用相同的沙箱配置

除了主可执行文件，`.app/Contents/Frameworks` 或插件（如 `.dylib`、`.so`、`.framework`）也必须进行相同方式签名。

通常建议这样做：

```
bash


复制编辑
codesign --force --options runtime --entitlements entitlements.plist --sign "Developer ID Application: ..." -v --deep Wondershare\ DemoCreator\ 8.app
```

------

### ✅ 5. 如果使用 `xcodebuild` 或 `CMake + Xcode`，你应设置：

```
cmake复制编辑set_target_properties(${PROJECT_NAME} PROPERTIES
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_SOURCE_DIR}/entitlements.plist"
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "YZC2T44ZDX" # 替换为你的 Team ID
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "YES"
)
```

或者 `.xcodeproj` 里手动设置签名方式为 `Manual` 并附加 `entitlements.plist`。

------

## 总结检查清单 ✅

| 检查项                                                       | 是否正确 |
| ------------------------------------------------------------ | -------- |
| 使用 `--entitlements` 签名整个 `.app`？                      | ✅        |
| 签名命令中使用了 `--deep --options runtime`？                | ✅        |
| 主程序签名后包含 `com.apple.security.app-sandbox`？          | ✅        |
| `.app` 内部其他可执行文件也签名了？                          | ✅        |
| 如果是 Xcode 构建，配置了 `XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS`？ | ✅        |



------

如果你确认这些都设置了还是不通过，可以贴出你的实际签名命令或 `CMakeLists.txt` 构建逻辑，我可以帮你检查具体细节。





##### 您说：

Validation failed (409) This bundle is invalid. The value for key CFBundleVersion [8.6.0.12] in the Info.plist file must be a period-separated list of at most three non-negative integers. Please find more information about CFBundleVersion at https://developer.apple.com/documentation/bundleresources/information_property_list/cfbundleversion (ID: ced19dec-e822-4b1a-859a-1b2a7e8858be)



###### ChatGPT 说：

这个错误提示说明：

> **你的 `Info.plist` 中的 `CFBundleVersion` 值不合法。它不能是 4 段（8.6.0.12），只能是“最多 3 个由点号分隔的非负整数”。**

------

## ✅ 正确格式要求（Apple 官方说明）

Apple 要求 `CFBundleVersion`（**内部版本号**）必须是格式：

```
php-template


复制编辑
<major>.<minor>[.<patch>]
```

最多 **三段**，例如：

- ✅ `1`
- ✅ `1.0`
- ✅ `1.0.0`
- ❌ `1.0.0.1`（**非法**）

------

## ✅ 修复方法

打开你的 `Info.plist`，找到这一行：

```
xml复制编辑<key>CFBundleVersion</key>
<string>8.6.0.12</string>  <!-- ❌ 错误 -->
```

将其改为：

```
xml复制编辑<key>CFBundleVersion</key>
<string>8.6.0</string>  <!-- ✅ 合法 -->
```

------

### ℹ️ 补充说明

- `CFBundleVersion` 是用于系统区分每次构建的内部编号，常在发布 App 时递增。
- 它和 `CFBundleShortVersionString` 不同，后者是给用户看的（可以是 `8.6.0.12`，没问题）。

例如合法配置：

```
xml复制编辑<key>CFBundleShortVersionString</key>
<string>8.6.0.12</string>  <!-- 用户可见版本号，可以保留 -->

<key>CFBundleVersion</key>
<string>8.6.0</string>      <!-- ✅ 修正为三段 -->
```

------

## ✅ 最佳实践

| 字段名称                     | 含义                         | 示例               |
| ---------------------------- | ---------------------------- | ------------------ |
| `CFBundleShortVersionString` | 展示给用户的版本号           | `8.6.0.12`         |
| `CFBundleVersion`            | 内部构建号，系统用来判断更新 | `8.6.0` 或 `80612` |



------

修改后重新打包、签名并提交即可。如果你用的是 `CMake` 或脚本生成 Info.plist，也需要一并改掉对应字段。需要我协助自动改写 `CMakeLists.txt` 或 plist 文件，也可以发我看一下。





##### 您说：

Validation failed (409) Invalid Bundle. The application bundle may not contain tools or frameworks provided by Apple, or using bundle identifiers in the 'com.apple.' namespace. Invalid bundle: [com.wondershare.democreator8.pkg/Payload/Wondershare DemoCreator 8.app/Contents/Frameworks/BugsplatMac.framework.dSYM], with bundle identifier 'com.apple.xcode.dsym.com.bugsplat.BugsplatMac'. (ID: 4ffa3aa2-a72f-4029-be3c-b7f4e5c83df6)



###### ChatGPT 说：

这个错误说明：

> **你的应用包里包含了一个非法的 dSYM 文件（调试符号文件），其 Bundle Identifier 使用了 `com.apple.\*` 前缀，这是 Apple 不允许的。**

------

## ❌ 错误详情

错误文件路径：

```
swift


复制编辑
Wondershare DemoCreator 8.app/Contents/Frameworks/BugsplatMac.framework.dSYM
```

它的 **Bundle Identifier 是**：

```
复制编辑
com.apple.xcode.dsym.com.bugsplat.BugsplatMac
```

这违反了 Apple 的规定：**任何第三方组件都不得使用 `com.apple.` 开头的 bundle ID。**

------

## ✅ 解决方案

### ✅ **1. 移除该 dSYM 文件（推荐做法）**

在提交到 Apple 时，不需要包含 `.dSYM` 调试符号文件。它们应该单独上传到你的崩溃分析服务（如 BugSplat 或 Apple 的 Crashlytics）。

删除路径如下：

```
bash


复制编辑
rm -rf "Wondershare DemoCreator 8.app/Contents/Frameworks/BugsplatMac.framework.dSYM"
```

------

### ✅ **2. 或：彻底从打包脚本中排除 dSYM 文件**

如果你是用 `CMake`、`xcodebuild`、`pkgbuild` 或其他自动化打包流程生成 `.app`，确保不将 `*.dSYM` 拷贝进最终的 `.app` 包中。

例如用 `CMake` 时，可以加：

```
cmake复制编辑install(DIRECTORY BugsplatMac.framework
        DESTINATION "${CMAKE_INSTALL_PREFIX}/Contents/Frameworks"
        FILES_MATCHING PATTERN "*.dSYM" EXCLUDE)
```

------

### ✅ **3. 如果你非要保留 `.dSYM`，需修改 bundle ID（不推荐）**

你**可以用 `plutil` 或手动修改 `Contents/Info.plist`**，将其中的 `CFBundleIdentifier` 改为你自己的命名空间，比如：

```
xml复制编辑<key>CFBundleIdentifier</key>
<string>com.wondershare.bugsplat.BugsplatMac</string>
```

然后重新签名：

```
bash


复制编辑
codesign --force --sign "Developer ID Application: ..." "BugsplatMac.framework.dSYM"
```

但**更推荐做法还是移除**。

------

## ✅ 总结

| 步骤 | 操作                                                         |
| ---- | ------------------------------------------------------------ |
| 1    | 移除 `.dSYM` 文件夹（推荐）                                  |
| 2    | 不要将 `.dSYM` 文件打包进 `.app`                             |
| 3    | 或更改其 `CFBundleIdentifier`（不推荐，非官方工具可能仍拦截） |



------

如果你打包是自动化的（比如 `.pkg` 是用 `productbuild` 生成的），我也可以帮你改打包脚本，把 `.dSYM` 自动排除。需要可以贴出打包命令或脚本内容。





##### 您说：

Validation failed (409) Missing required icon. The application bundle does not contain an icon in ICNS format, containing both a 512x512 and a 512x512@2x image. For further assistance, see the Human Interface Guidelines at https://developer.apple.com/design/human-interface-guidelines/foundations/app-icons/. (ID: 19504609-5738-48dc-8433-7e2bd29958cb)



###### ChatGPT 说：

这个错误说明：

> **你的应用包中缺少合法的 `.icns` 格式图标文件，或者图标文件中缺少必要尺寸（512x512 和 512x512@2x，也就是 1024x1024）的图像。**

------

## ✅ Apple 图标文件（`.icns`）要求

Apple 要求 `.icns` 文件中必须包含以下尺寸，特别是：

- ✅ `512x512`（标准 Retina 图标）
- ✅ `512x512@2x`（即实际尺寸为 `1024x1024`）

------

## 🛠 修复步骤

### ✅ 1. 准备一张 `1024x1024` 的 PNG 图标图像

确保你的原始图标为 `1024x1024` 的 PNG 文件，例如：`AppIcon.png`

------

### ✅ 2. 使用 `iconutil` 生成 `.icns` 文件

#### 方法一：用 macOS 原生命令行生成

```
bash复制编辑mkdir MyApp.iconset
sips -z 16 16     AppIcon.png --out MyApp.iconset/icon_16x16.png
sips -z 32 32     AppIcon.png --out MyApp.iconset/icon_16x16@2x.png
sips -z 32 32     AppIcon.png --out MyApp.iconset/icon_32x32.png
sips -z 64 64     AppIcon.png --out MyApp.iconset/icon_32x32@2x.png
sips -z 128 128   AppIcon.png --out MyApp.iconset/icon_128x128.png
sips -z 256 256   AppIcon.png --out MyApp.iconset/icon_128x128@2x.png
sips -z 256 256   AppIcon.png --out MyApp.iconset/icon_256x256.png
sips -z 512 512   AppIcon.png --out MyApp.iconset/icon_256x256@2x.png
sips -z 512 512   AppIcon.png --out MyApp.iconset/icon_512x512.png
cp AppIcon.png MyApp.iconset/icon_512x512@2x.png

iconutil -c icns MyApp.iconset -o AppIcon.icns
```

生成后你会得到 `AppIcon.icns`，包含所有必要尺寸。

------

### ✅ 3. 添加到 `.app` 中

将生成的 `AppIcon.icns` 文件放入你的 `.app` 中的资源目录：

```
bash


复制编辑
cp AppIcon.icns "Wondershare DemoCreator 8.app/Contents/Resources/"
```

然后修改 `Info.plist` 中增加一项：

```
xml复制编辑<key>CFBundleIconFile</key>
<string>AppIcon.icns</string>
```

注意：**不要写成完整路径或带扩展名 `.icns`，仅写文件名即可。**

------

## ✅ 总结检查项

| 项目                                           | 检查点                                                     |
| ---------------------------------------------- | ---------------------------------------------------------- |
| `.icns` 文件是否存在                           | `Contents/Resources/AppIcon.icns`                          |
| `.icns` 文件是否包含 `512x512` 和 `512x512@2x` | 用 `iconutil -c iconset` 可解包检查                        |
| `Info.plist` 是否正确配置图标                  | `<key>CFBundleIconFile</key><string>AppIcon.icns</string>` |



------

如果你希望我帮你写个自动脚本来生成 `.icns`，也可以告诉我图标原图的路径。完成后重新签名并上传即可解决该问题。





##### 您说：

使用 iconutil 生成 .icns 文件 的具体做法是？



###### ChatGPT 说：

使用 `iconutil` 工具生成 `.icns` 文件的完整做法如下：

------

## ✅ 前提准备

你需要准备一张**正方形**的 PNG 图标图片，推荐尺寸为：

```
复制编辑
AppIcon.png（1024x1024）
```

------

## 🧰 步骤详解：使用 `iconutil` 生成 `.icns`

### ✅ 第一步：创建 `.iconset` 文件夹

```
bash


复制编辑
mkdir MyApp.iconset
```

------

### ✅ 第二步：使用 `sips` 工具生成不同尺寸图标

在终端中执行以下命令，将原始 PNG 图标转换为多尺寸：

```
bash复制编辑sips -z 16 16     AppIcon.png --out MyApp.iconset/icon_16x16.png
sips -z 32 32     AppIcon.png --out MyApp.iconset/icon_16x16@2x.png
sips -z 32 32     AppIcon.png --out MyApp.iconset/icon_32x32.png
sips -z 64 64     AppIcon.png --out MyApp.iconset/icon_32x32@2x.png
sips -z 128 128   AppIcon.png --out MyApp.iconset/icon_128x128.png
sips -z 256 256   AppIcon.png --out MyApp.iconset/icon_128x128@2x.png
sips -z 256 256   AppIcon.png --out MyApp.iconset/icon_256x256.png
sips -z 512 512   AppIcon.png --out MyApp.iconset/icon_256x256@2x.png
sips -z 512 512   AppIcon.png --out MyApp.iconset/icon_512x512.png
cp AppIcon.png                     MyApp.iconset/icon_512x512@2x.png
```

> 说明：
>
> - `sips -z 宽 高` 是 macOS 自带的图像缩放命令。
> - 最后一行直接复制原始图片作为最大尺寸（1024x1024）。

------

### ✅ 第三步：生成 `.icns` 文件

```
bash


复制编辑
iconutil -c icns MyApp.iconset -o AppIcon.icns
```

> 成功后，会生成 `AppIcon.icns` 文件。

------

## ✅ 使用说明：放入 App Bundle 并修改 Info.plist

1. 将 `AppIcon.icns` 拷贝到你的 `.app` 包中：

```
bash


复制编辑
cp AppIcon.icns "Wondershare DemoCreator 8.app/Contents/Resources/"
```

1. 修改 `Info.plist`，添加或确认存在以下键值：

```
xml复制编辑<key>CFBundleIconFile</key>
<string>AppIcon.icns</string>
```

注意：不要加 `.icns` 后缀。

------

## ✅ 验证 .icns 是否正确

你可以双击 `.icns` 文件在 Finder 里预览；或者用如下命令反解回 `.iconset` 来检查内容：

```
bash


复制编辑
iconutil -c iconset AppIcon.icns -o Extracted.iconset
```

------

如果你愿意，我也可以生成一份完整的 bash 脚本给你一键生成 `.icns` 文件。是否需要？





##### 您说：

"Cannot be used with TestFlight because the bundle at “Wondershare DemoCreator 8.app” is missing a provisioning profile. Main bundles are expected to have provisioning profiles in order to be eligible for TestFlight." (90889) Incomplete Document Type Configuration. The CFBundleDocumentTypes dictionary array in the 'com.wondershare.democreator8' Info.plist should contain an LSHandlerRank value for the CFBundleTypeName 'DemoCreator Project' entry. Refer to https://developer.apple.com/library/archive/documentation/General/Reference/InfoPlistKeyReference/Articles/CoreFoundationKeys.html#//apple_ref/doc/uid/TP40009249-SW1 for more information on the LSHandlerRank key. (90788) Incomplete Document Type Configuration. The CFBundleDocumentTypes dictionary array in the 'com.wondershare.democreator8' Info.plist should contain an LSHandlerRank value for the CFBundleTypeName 'Recorder File' entry. Refer to https://developer.apple.com/library/archive/documentation/General/Reference/InfoPlistKeyReference/Articles/CoreFoundationKeys.html#//apple_ref/doc/uid/TP40009249-SW1 for more information on the LSHandlerRank key. (90788)



###### ChatGPT 说：

你遇到的这些 App Store Connect 上传错误一共有三个，我们逐个解释并给出修复方式：

------

## ❗1. 缺少 Provisioning Profile（TestFlight 不可用）

```
csharp


复制编辑
Cannot be used with TestFlight because the bundle at “Wondershare DemoCreator 8.app” is missing a provisioning profile. (90889)
```

### ✅ 解决方法：

你需要为主 App 配置正确的签名证书和 **Provisioning Profile（描述文件）**。

- 如果你用的是 `xcodebuild` 打包，请确保命令中加入了正确的签名参数：

```
bash复制编辑xcodebuild -workspace YourApp.xcworkspace -scheme YourScheme \
  -configuration Release \
  -archivePath build/YourApp.xcarchive \
  archive \
  CODE_SIGN_IDENTITY="Apple Distribution: Wondershare ..." \
  PROVISIONING_PROFILE_SPECIFIER="YourProfileName"
```

- 如果用的是 `codesign`，需要在 `.app` 签名时指定：

```
bash复制编辑codesign --deep --force --verbose \
  --options runtime \
  --sign "Apple Distribution: Wondershare ..." \
  --entitlements YourEntitlements.plist \
  Wondershare\ DemoCreator\ 8.app
```

- 也可以通过 Xcode 自动签名，确保在 `Signing & Capabilities` 中启用了自动签名。

------

## ❗2. Info.plist 中缺少 `LSHandlerRank`（DemoCreator Project）

```
pgsql


复制编辑
Incomplete Document Type Configuration. The CFBundleDocumentTypes dictionary array in the 'Info.plist' should contain an LSHandlerRank value for the CFBundleTypeName 'DemoCreator Project' entry.
```

你在 `Info.plist` 中定义了文档类型 `DemoCreator Project`，但缺少关键字段 `LSHandlerRank`。

### ✅ 修改方式：

在 `Info.plist` 中找到：

```
xml复制编辑<key>CFBundleDocumentTypes</key>
<array>
  <dict>
    <key>CFBundleTypeName</key>
    <string>DemoCreator Project</string>
    ...
  </dict>
</array>
```

需要补充：

```
xml复制编辑<key>LSHandlerRank</key>
<string>Owner</string>
```

完整示例：

```
xml复制编辑<key>CFBundleDocumentTypes</key>
<array>
  <dict>
    <key>CFBundleTypeName</key>
    <string>DemoCreator Project</string>
    <key>CFBundleTypeExtensions</key>
    <array>
      <string>wdp</string>
    </array>
    <key>CFBundleTypeRole</key>
    <string>Editor</string>
    <key>LSHandlerRank</key>
    <string>Owner</string>
  </dict>
</array>
```

------

## ❗3. Info.plist 中缺少 `LSHandlerRank`（Recorder File）

同上，另一个文档类型 `Recorder File` 也需要补充 `LSHandlerRank`：

```
xml复制编辑<key>CFBundleTypeName</key>
<string>Recorder File</string>
...
<key>LSHandlerRank</key>
<string>Owner</string>
```

------

### ✅ `LSHandlerRank` 可选值：

- `"Owner"`：App 是该文档类型的主要拥有者。
- `"Default"`：默认打开者。
- `"Alternate"`：可打开但非默认。
- `"None"`：不参与。

通常推荐设置为 `"Owner"` 或 `"Default"`。

------

如果你愿意，我也可以帮你生成一个完整的 Info.plist 示例，供参考使用。是否需要？





##### 您说：

Validation failed (409) This bundle is invalid. The Info.plist file is missing or could not be parsed. Please check it for embedded control characters. (ID: 4ca50405-4086-465a-9340-e17754b15f94)



###### ChatGPT 说：

这个错误提示：

> **"The Info.plist file is missing or could not be parsed. Please check it for embedded control characters."**

说明你的 `Info.plist` 文件存在以下问题之一：

------

### ✅ 可能原因及解决方案：

#### ✅ 1. **Info.plist 文件缺失**

- 检查 `.app/Contents/Info.plist` 路径下是否存在 `Info.plist` 文件。
- 如果你用 `xcodebuild` 或 `cmake` 构建，请确保构建产物包含 `Info.plist`。

------

#### ✅ 2. **格式错误或包含非法字符**

- 如果 `Info.plist` 被错误编辑（如粘贴时含不可见字符），Apple 会拒绝。

##### ✅ 解决方法：

- 使用 `plutil` 工具检查格式是否合法：

```
bash


复制编辑
plutil -lint path/to/Info.plist
```

如果合法，会输出：

```
bash


复制编辑
path/to/Info.plist: OK
```

如果非法，会输出具体报错，如包含控制字符（如 `\x0`、`\x1b` 等）。

------

#### ✅ 3. **文件编码错误**

- `Info.plist` 必须为 UTF-8 编码（**无 BOM**），格式必须是 XML 或 binary plist。

##### 建议：

- 用 `iconv` 重新转码一次：

```
bash复制编辑iconv -f utf-8 -t utf-8 Info.plist -o Info-clean.plist
mv Info-clean.plist Info.plist
```

或用 `plutil` 转换为 binary 格式（Apple 推荐）：

```
bash


复制编辑
plutil -convert binary1 Info.plist
```

------

#### ✅ 4. **构建时未正确复制 Info.plist**

- 如果你使用 `CMake` 或手动打包，请确认 `Info.plist` 被正确复制到 `.app/Contents/` 路径下。

------

### ✅ 示例校验命令：

```
bash


复制编辑
plutil -lint Wondershare\ DemoCreator\ 8.app/Contents/Info.plist
```

------

如果你愿意，你可以把当前 `Info.plist` 文件发给我，我可以帮你检查其中是否含非法字符并修复。是否上传一下？





##### 您说：

Validation failed (409) Invalid kext file: Kernel extensions are not permitted: com.wondershare.democreator8.pkg/Payload/Wondershare DemoCreator 8.app/Contents/MacOS/DemoCreator Record 8.app/Contents/Resources/SystemAudioRecorder.kext (ID: 4db13bf6-8e3e-49e1-8d11-a9288373371a)



###### ChatGPT 说：

这个错误提示是：

> **Invalid kext file: Kernel extensions are not permitted**

也就是说，你的 App 包中包含了一个 **内核扩展（Kext）** 文件：

```
swift


复制编辑
Wondershare DemoCreator 8.app/Contents/MacOS/DemoCreator Record 8.app/Contents/Resources/SystemAudioRecorder.kext
```

------

### ❗ 这是 App Store 无法接受的内容

Apple 明确规定：

> **App Store 禁止提交任何含有 `.kext` 文件（内核扩展）** 的应用。

------

### ✅ 解决方案

#### 🔧 **方式 1：移除 kext，改用 Apple 支持的音频框架**

- 替代方式是使用 `AVAudioEngine`、`Audio Unit`、或借助 `AudioWorklet` 类库。
- 对于**系统音频录制功能**，可改用 **Apple 推荐的 Virtual Audio Driver** 实现，例如：
  - 自研的 CoreAudio 驱动（必须通过 DriverKit）
  - 使用如 [BlackHole](https://github.com/ExistentialAudio/BlackHole)、Loopback（注意授权）

------

#### 🔧 **方式 2：将 Kext 改为独立驱动安装器（非 App 包内）**

- 将 `SystemAudioRecorder.kext` 移出 `.app` 包。
- 提供单独的驱动安装器（用户单独授权并安装），参考 Zoom、OBS 做法。
- **注意**：即使你单独提供，Apple 依然不允许该驱动随 App Bundle 一起提交。

------

#### 🔧 **方式 3：使用 Apple 的 DriverKit 实现音频扩展**

- macOS Catalina（10.15）以后，Apple 推荐通过 **DriverKit** 开发用户态驱动，而不是 `kext`。
- DriverKit 驱动需通过 **System Extension** 签名及公证，不可嵌入 App 内部，需单独管理。

------

### ✅ 总结

| 方法                           | 可被 App Store 接受      | 说明 |
| ------------------------------ | ------------------------ | ---- |
| `.kext` 嵌入 App 中            | ❌ 不被接受               |      |
| 独立驱动安装器（不打包进 App） | ✅ 但需额外流程           |      |
| 使用 DriverKit 实现            | ✅ 推荐方式（但开发复杂） |      |
| 使用 AVFoundation 等录音方法   | ✅ 若无系统音频需求       |      |