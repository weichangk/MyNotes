### 一、准备工作

1. **注册 Apple Developer Program**
   - 确保你的 Apple ID 已加入 Apple Developer Program，年费99美元。
2. **在 App Store Connect 创建 App 记录**
   - 登录 [App Store Connect](https://appstoreconnect.apple.com)
   - 选择 “我的 App” → 点击加号 “+” → “新建 App”
   - 填写基本信息：
     - App 名称（唯一）
     - Bundle ID（与你项目中的 Info.plist 中的 Bundle Identifier 一致）
     - 平台选择 macOS
     - SKU（内部标识，无需公开）
3. **填写版本信息**
   - 在 App 记录中，填写版本号、隐私政策、支持网址等必填项。
   - 上传图标（1024×1024 PNG，无透明通道）
   - 准备并上传截图（根据设备类型，macOS App 需对应屏幕尺寸截图）
4. **准备好开发者证书和签名的 `.pkg` 包**
   - 你已经完成代码签名，使用 `pkgbuild` 或 `productbuild` 生成 `.pkg` 文件。
   - 这个 `.pkg` 会上传到 App Store Connect。
5. **配置沙盒权限和权限说明**
   - 如果你的 App 使用摄像头、麦克风、文件访问等权限，需要在 App Store Connect 填写用途说明（Privacy - Usage Description）。
6. **App Store Connect 账户设置**
   - 确保你的团队、付款信息、税务信息等都已经填写完整。

### 二、App 要求

- `YourApp.app` 必须启用 **沙盒（App Sandbox）**

- 所有权限（如摄像头、麦克风）都要在 `Info.plist` 中声明并获得用户授权

- 没有使用私有 API、未签名 dylib、不动态下载代码

  | 事项     | 说明                                                 |
  | -------- | ---------------------------------------------------- |
  | ✅ 签名   | 使用正确证书签名 `.app` 和 `.pkg`                    |
  | ✅ 沙盒   | 必须启用 App Sandbox                                 |
  | ✅ 权限   | 在 `Info.plist` 中声明 `NSCameraUsageDescription` 等 |
  | ❌ 不可用 | `Electron`、`JVM` 等可能因权限过多、沙盒限制无法上架 |
  | 📦 应用包 | 推荐使用 `.app` + `.pkg` 组合打包上传，不是 `.dmg`   |

### 三、打包上传

1. 使用 `productbuild`创建 `.pkg` 安装包（用于 App Store 审核上传）

   ```bash
   productbuild --component YourApp.app /Applications \
                --sign "3rd Party Mac Developer Installer: Your Name (TEAMID)" \
                YourApp.pkg
   ```

   - 这一步生成的是一个提交用的 `.pkg` 安装包（非用户直接使用）。
   - 签名证书需要是“**3rd Party Mac Developer Installer**”类型。

   

2. 使用 `altool` 上传到 App Store Connect

   ```bash
   xcrun altool --upload-package YourApp.pkg \
                --type macos \
                --username "你的 Apple ID" \
                --password "@keychain:AppStoreConnectPassword"
   ```

   > 💡 你可以通过 `Application Loader` 或 `Transporter` 上传，也可以用命令行。
   >
   > Apple 推荐改用 `xcrun notarytool` + Xcode Cloud 等新工具，`altool` 会逐步弃用。

3. 建议使用 Xcode 打包上传（更简单）

   如果你使用的是 **Xcode 创建的 macOS App 项目**，推荐使用：

   ```
   Product > Archive
   ```

   然后点击 **Distribute App**：

   - 选择 **App Store Connect**
   - 自动完成签名、打包、上传

### 四、自动化打包脚本

✅ 你需要准备的基本信息

请替换下列变量为你自己的信息：

```bash
APP_NAME="YourApp" # 应用名
APP_BUNDLE_ID="com.yourcompany.yourapp"
APP_VERSION="1.0.0"
DEVELOPER_ID_APP="3rd Party Mac Developer Application: Your Name (TEAMID)"
DEVELOPER_ID_INSTALLER="3rd Party Mac Developer Installer: Your Name (TEAMID)"
TEAM_ID="TEAMID"
```

📁 目录结构（假设）

```bash
build/
└── YourApp.app         # Qt 打包好的 .app 文件
scripts/
└── build_and_upload.sh # 打包上传脚本
```

🛠 脚本：`scripts/build_and_upload.sh`

```bash
#!/bin/bash

set -e

############################
# 变量设置
############################

APP_NAME="YourApp"
APP_BUNDLE_ID="com.yourcompany.yourapp"
APP_VERSION="1.0.0"
DEVELOPER_ID_APP="3rd Party Mac Developer Application: Your Name (TEAMID)"
DEVELOPER_ID_INSTALLER="3rd Party Mac Developer Installer: Your Name (TEAMID)"
TEAM_ID="TEAMID"

APP_PATH="../build/${APP_NAME}.app"
PKG_PATH="../build/${APP_NAME}.pkg"
KEYCHAIN_PROFILE="AC_PASSWORD"  # 使用 `xcrun notarytool store-credentials` 创建
USERNAME="your_apple_id@example.com"

############################
# 步骤 1：签名 App
############################

echo "🔐 Signing .app..."
codesign --deep --force --options runtime \
  --entitlements ./entitlements.plist \
  --sign "${DEVELOPER_ID_APP}" "${APP_PATH}"

codesign --verify --deep --strict --verbose=2 "${APP_PATH}"

############################
# 步骤 2：构建 .pkg
############################

echo "📦 Creating .pkg..."
productbuild \
  --component "${APP_PATH}" /Applications \
  --sign "${DEVELOPER_ID_INSTALLER}" \
  "${PKG_PATH}"
             
############################
# 步骤 3：验证签名
############################

echo "🔎 Verifying signature..."
spctl -a -vv -t install "${PKG_PATH}"

############################
# 步骤 4：上传到 App Store Connect
############################

echo "☁️ Uploading .pkg to App Store Connect..."
xcrun altool --upload-package "${PKG_PATH}" \
             --type macos \
             --username "${USERNAME}" \
             --password "@keychain:${KEYCHAIN_PROFILE}"

echo "✅ Done. Package uploaded for review."
```

🧾 entitlements.plist 示例（沙盒权限）

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
 "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.app-sandbox</key>
    <true/>
    <key>com.apple.security.network.client</key>
    <true/>
    <key>com.apple.security.files.user-selected.read-write</key>
    <true/>
    <key>com.apple.security.device.camera</key>
    <true/>
    <key>com.apple.security.device.microphone</key>
    <true/>
</dict>
</plist>
```

根据你的 App 权限要求选择性添加，比如：

- 文件访问权限：`com.apple.security.files.*`

- 摄像头：`com.apple.security.device.camera`

- 麦克风：`com.apple.security.device.microphone`

- 网络：`com.apple.security.network.*`

  

💡 准备 keychain 密码访问令牌（一次性操作）

```bash
xcrun notarytool store-credentials --apple-id "your_apple_id@example.com" \
                                   --team-id "TEAMID" \
                                   --password "app-specific-password" \
                                   --keychain-profile "AC_PASSWORD"
```

📝 注意事项

1. 打包前确保 `.app` 已启用沙盒（Qt 可用 `.pro` 中配置或手动改 `Info.plist`）
2. `.app` 中所有使用的 `.dylib`、插件也必须签名
3. 可选步骤：公证 `notarytool`，但上架 App Store 会自动处理



### 五、上传成功后如何发布

- 填写 App 版本的元数据和提交审核

  	- 版本描

  	- 新功能说明
  	- 关键词
  	- 支持网址
  	- 联系信息等

- 提交 App 审核
  - 点击 “提交审核” 按钮，将包提交给 Apple 审核团队。

- 等待审核结果
  - 审核通常需要几天时间，期间可能会收到审核员的反馈（如功能、权限等问题），需及时回复修改。

- 内测
  - 可以通过 App Store Connect 的 TestFlight 先进行内测。

- 审核通过后发布
  - **手动发布**：自己点击发布按钮上线。
  - **自动发布**：审核通过自动上架（需要提前设置）。

- App 上架后用户可以通过 Mac App Store 下载和更新

### 六、获取这些证书？

✅ 正确步骤：

1. 加入 Apple Developer Program（$99/年）
2. 打开 Xcode > Preferences > Accounts，登录 Apple ID
3. 自动下载证书，也可以在 [developer.apple.com](https://developer.apple.com/account/resources/certificates/list) 中下载：
   - `3rd Party Mac Developer Application`
   - `3rd Party Mac Developer Installer`
4. 在“钥匙串访问”中找到类似：
   - ✅ `3rd Party Mac Developer Application: Wei Changkun (A1B2C3D4E5)`
   - ✅ `3rd Party Mac Developer Installer: Wei Changkun (A1B2C3D4E5)`

🧪 如何验证证书是否存在？

```bash
security find-identity -v -p codesigning
```

输出示例：

```bash
1) XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX "3rd Party Mac Developer Application: Wei Changkun (A1B2C3D4E5)"
2) YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY "3rd Party Mac Developer Installer: Wei Changkun (A1B2C3D4E5)"
```

✅ 总体概念

| 证书名称                              | 用途       | 作用对象 | 使用命令                    | 是否必需 |
| ------------------------------------- | ---------- | -------- | --------------------------- | -------- |
| `3rd Party Mac Developer Application` | 应用签名   | `.app`   | `codesign`                  | ✅ 必需   |
| `3rd Party Mac Developer Installer`   | 安装包签名 | `.pkg`   | `pkgbuild` / `productbuild` | ✅ 必需   |

🧩 区别详解

1️⃣ `3rd Party Mac Developer Application`

- **用途**：签名你的 `YourApp.app` 文件。

- **作用**：确保你的 App 符合 Apple 沙盒、安全机制，并在 Mac App Store 中通过审核。

- **签名方式**（必须加 `--options runtime` 以开启 Hardened Runtime）：

  ```cpp
  codesign --deep --force --options runtime --sign "3rd Party Mac Developer Application: Your Name (TEAMID)" YourApp.app
  ```

- **签名后可以验证**：

  ```cpp
  codesign --verify --deep --strict --verbose=2 YourApp.app
  ```

- **位置**：Xcode 登录后自动添加到“钥匙串访问”。

2️⃣ `3rd Party Mac Developer Installer`

- **用途**：签名你用 `pkgbuild` 生成的 `.pkg` 安装包。

- **作用**：让 Apple 确认你的安装包是由可信开发者创建，符合 App Store 的打包要求。

- **使用命令**：

  ```bash
  productbuild --component YourApp.app /Applications \
    --sign "Developer ID Installer: Your Name (TEAMID)" \
    YourApp.pkg
  ```

- **验证**：

  ```bash
  spctl -a -t install -vv YourApp.pkg
  ```



### 七、官网分发 App，而不是上架 Mac App Store

✅ 1. 使用 `Developer ID Application` 证书签名 `.app`

```bash
codesign --deep --force --options runtime \
  --entitlements entitlements.plist \
  --sign "Developer ID Application: Your Name (TEAMID)" \
  YourApp.app
```

> ✅ 这可以避免用户看到“无法验证开发者”的安全警告。
> ✅ 此证书允许你将 App 分发给任意用户（只要他们的系统是 macOS 10.9+）。

✅ 2. 为 `.dmg` 或 `.pkg` 签名（可选）

- 如果你是打包成 `.dmg` 或 `.pkg` 安装包分发，可以**选择性使用 Installer 证书**签名它，提升信任度：

```bash
productbuild --component YourApp.app /Applications \
  --sign "Developer ID Installer: Your Name (TEAMID)" \
  YourApp.pkg
```

- 但如果只是压缩 `.app` 为 zip 或 dmg，可以只签 `.app`。

✅ 3. 使用 Notarization（强烈建议）

Apple 在 macOS Catalina（10.15）及之后系统强制要求：

> **所有通过网络下载的 App 都必须经过公证（Notarized），否则用户打开会看到安全警告，甚至打不开！**

所以你必须执行：

```bash
# 签名 .app
codesign --deep --force --options runtime \
  --entitlements entitlements.plist \
  --sign "Developer ID Application: Your Name (TEAMID)" \
  YourApp.app

# 创建 zip 包
ditto -c -k --keepParent YourApp.app YourApp.zip

# 上传公证
xcrun notarytool submit YourApp.zip \
  --keychain-profile "AC_PASSWORD" \
  --team-id "TEAMID" \
  --wait

# 公证成功后 stapler 附加公证票据
xcrun stapler staple YourApp.app
```

✅ 验证你的 .app 是否已正确签名与公证

```bash
codesign --verify --deep --strict --verbose=2 YourApp.app
spctl --assess --type exec --verbose=4 YourApp.app
```

如果你看到类似：

```bash
source=Notarized Developer ID
origin=Developer ID Application: Your Name (TEAMID)
```

就说明用户双击不会有任何阻碍。