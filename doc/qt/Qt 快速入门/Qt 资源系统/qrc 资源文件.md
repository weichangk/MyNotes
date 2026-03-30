# qrc 资源文件

Qt 资源系统（Qt Resource System）将图片、QSS、翻译文件、字体等静态资源**编译到可执行文件内部**，使应用以单一文件发布，无需担心运行时资源文件缺失或路径问题。`.qrc` 文件是该系统的核心配置文件。

---

## 1. 核心概念

### 工作原理

```
.qrc 文件（XML 索引）
    ↓  rcc 编译器（构建时）
qrc_*.cpp（C++ 数据数组）
    ↓  C++ 编译器
嵌入到可执行文件 / 库中
    ↓  运行时
通过 :/ 前缀或 qrc:/ URL 访问
```

- **rcc**（Resource Compiler）：Qt 自带的资源编译器，将 `.qrc` 引用的文件转换为 C++ 源码
- 资源数据以**压缩**形式存储在二进制文件中（默认 zlib，Qt 5.13+ 可选 zstd）
- 运行时通过 Qt 的虚拟文件系统访问，接口与普通文件 I/O 一致

### 优势

| 特性 | 说明 |
|---|---|
| 部署简单 | 资源嵌入可执行文件，不需要额外分发 |
| 路径稳定 | `:/` 前缀路径不受运行目录影响 |
| 跨平台一致 | 不受文件系统大小写、路径分隔符差异影响 |
| 防篡改 | 资源在二进制内部，不易被最终用户修改 |
| 支持压缩 | 自动压缩，减小体积 |

---

## 2. .qrc 文件格式

### 2.1 基本结构

`.qrc` 是标准 XML 文件：

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource>
        <file>images/logo.png</file>
        <file>images/icon.png</file>
        <file>styles/app.qss</file>
        <file>fonts/NotoSansSC-Regular.ttf</file>
    </qresource>
</RCC>
```

**规则**：
- `<file>` 中的路径**相对于 `.qrc` 文件所在目录**
- 路径使用 `/` 分隔符（即使在 Windows 上）
- 不支持通配符（不能写 `images/*.png`）

### 2.2 前缀（prefix）

使用 `prefix` 属性为资源分组，形成虚拟目录结构：

```xml
<RCC version="1.0">
    <qresource prefix="/icons">
        <file>close.png</file>
        <file>settings.png</file>
    </qresource>
    <qresource prefix="/styles">
        <file>light.qss</file>
        <file>dark.qss</file>
    </qresource>
    <qresource prefix="/i18n">
        <file>app_zh_CN.qm</file>
        <file>app_en_US.qm</file>
    </qresource>
</RCC>
```

访问路径 = `前缀 + 文件名`：
```cpp
QIcon(":/icons/close.png");
QFile(":/styles/dark.qss");
QTranslator t; t.load(":/i18n/app_zh_CN.qm");
```

### 2.3 别名（alias）

使用 `alias` 属性为文件创建虚拟名称，可简化路径或实现条件替换：

```xml
<qresource prefix="/icons">
    <!-- 用简短别名访问深层路径的文件 -->
    <file alias="logo.png">assets/branding/company_logo_128x128.png</file>

    <!-- 不同分辨率用相同别名 -->
    <file alias="close.png">icons/16x16/close.png</file>
</qresource>
```

```cpp
// 用别名访问
QIcon(":/icons/logo.png");  // 实际指向 assets/branding/company_logo_128x128.png
```

### 2.4 语言属性（lang）

`lang` 属性可根据系统语言自动选择资源文件：

```xml
<qresource prefix="/images">
    <!-- 默认版本（英文） -->
    <file>welcome.png</file>
</qresource>

<qresource prefix="/images" lang="zh_CN">
    <!-- 中文版本：系统语言为中文时自动替换 -->
    <file alias="welcome.png">welcome_zh.png</file>
</qresource>

<qresource prefix="/images" lang="ja_JP">
    <file alias="welcome.png">welcome_ja.png</file>
</qresource>
```

```cpp
// 设置 locale 后，同一路径自动指向对应语言的资源
QLocale::setDefault(QLocale(QLocale::Chinese, QLocale::China));
QPixmap pix(":/images/welcome.png");  // 自动加载 welcome_zh.png
```

---

## 3. 项目目录组织

### 3.1 典型目录结构

```
project/
├── CMakeLists.txt
├── main.cpp
├── resources/
│   ├── resources.qrc          # 主资源文件
│   ├── images/
│   │   ├── logo.png
│   │   ├── splash.png
│   │   └── icons/
│   │       ├── close.png
│   │       ├── minimize.png
│   │       └── settings.png
│   ├── styles/
│   │   ├── light.qss
│   │   └── dark.qss
│   ├── fonts/
│   │   └── NotoSansSC-Regular.ttf
│   └── translations/
│       ├── app_zh_CN.qm
│       └── app_en_US.qm
```

对应的 `resources.qrc`：

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource prefix="/images">
        <file>images/logo.png</file>
        <file>images/splash.png</file>
    </qresource>
    <qresource prefix="/icons">
        <file>images/icons/close.png</file>
        <file>images/icons/minimize.png</file>
        <file>images/icons/settings.png</file>
    </qresource>
    <qresource prefix="/styles">
        <file>styles/light.qss</file>
        <file>styles/dark.qss</file>
    </qresource>
    <qresource prefix="/fonts">
        <file>fonts/NotoSansSC-Regular.ttf</file>
    </qresource>
    <qresource prefix="/i18n">
        <file>translations/app_zh_CN.qm</file>
        <file>translations/app_en_US.qm</file>
    </qresource>
</RCC>
```

### 3.2 多 qrc 文件拆分

大型项目建议按模块拆分 `.qrc` 文件，避免单一文件过大，也减少增量编译时间：

```
resources/
├── icons.qrc         # 图标资源
├── styles.qrc        # 样式资源
├── fonts.qrc         # 字体资源
├── translations.qrc  # 翻译资源
```

---

## 4. 构建系统集成

### 4.1 CMake（Qt 5）

```cmake
# 方式 1：直接添加到目标源文件
add_executable(myapp
    main.cpp
    mainwindow.cpp
    resources/resources.qrc    # 直接列为源文件
)

# 方式 2：使用 qt5_add_resources（生成 qrc_*.cpp）
qt5_add_resources(RCC_SOURCES
    resources/resources.qrc
    resources/icons.qrc
)
add_executable(myapp
    main.cpp
    mainwindow.cpp
    ${RCC_SOURCES}
)
```

### 4.2 CMake（Qt 6）

```cmake
# Qt 6 推荐方式
qt_add_resources(myapp "app_resources"
    PREFIX "/"
    FILES
        images/logo.png
        images/icons/close.png
        styles/light.qss
        styles/dark.qss
)

# 或使用传统 qrc 文件
qt_add_resources(myapp
    resources/resources.qrc
)
```

`qt_add_resources` 的优势：
- 无需手动编写 `.qrc` XML 文件
- CMake 自动跟踪文件依赖
- 支持大资源文件的优化处理

### 4.3 qmake

```pro
RESOURCES += \
    resources/resources.qrc \
    resources/icons.qrc
```

### 4.4 构建过程

```
resources.qrc  ──→  rcc  ──→  qrc_resources.cpp  ──→  编译器  ──→  .obj  ──→  链接到可执行文件
```

查看 rcc 生成的文件内容（调试用）：

```bash
rcc --verbose resources.qrc             # 列出包含的文件
rcc -o qrc_resources.cpp resources.qrc  # 手动生成 C++ 文件
```

---

## 5. 在代码中访问资源

### 5.1 路径格式

Qt 资源系统支持两种路径格式：

```cpp
// 格式 1：冒号前缀（推荐，最常用）
":/images/logo.png"
":/styles/dark.qss"

// 格式 2：qrc URL scheme
"qrc:/images/logo.png"
"qrc:///images/logo.png"   // 三斜杠形式
```

`:/` 前缀可用于所有接受文件路径的 Qt API，`qrc:/` 主要用于 QML 和 QUrl。

### 5.2 加载图片

```cpp
// QPixmap
QPixmap logo(":/images/logo.png");
label->setPixmap(logo);

// QIcon
QIcon closeIcon(":/icons/close.png");
btn->setIcon(closeIcon);

// QImage
QImage img(":/images/photo.jpg");

// 在 QSS 中
// background-image: url(:/images/bg.png);
```

### 5.3 加载样式表

```cpp
QFile file(":/styles/dark.qss");
if (file.open(QFile::ReadOnly | QFile::Text)) {
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);
    file.close();
}
```

### 5.4 加载字体

```cpp
int fontId = QFontDatabase::addApplicationFont(":/fonts/NotoSansSC-Regular.ttf");
if (fontId != -1) {
    QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    if (!families.isEmpty()) {
        QFont font(families.first(), 12);
        qApp->setFont(font);
    }
}
```

### 5.5 加载翻译

```cpp
QTranslator translator;
if (translator.load(":/i18n/app_zh_CN.qm")) {
    qApp->installTranslator(&translator);
}
```

### 5.6 加载文本/数据文件

```cpp
// 读取 JSON 配置
QFile file(":/config/default.json");
if (file.open(QFile::ReadOnly)) {
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject config = doc.object();
    file.close();
}

// 读取 XML
QFile xmlFile(":/data/template.xml");
if (xmlFile.open(QFile::ReadOnly)) {
    QDomDocument doc;
    doc.setContent(&xmlFile);
    xmlFile.close();
}
```

### 5.7 在 QML 中使用

```qml
Image {
    source: "qrc:/images/logo.png"
}

// 加载 QML 组件
Loader {
    source: "qrc:/qml/MyComponent.qml"
}
```

---

## 6. QDir 和 QFileInfo 操作资源

资源文件可以像普通文件一样使用 Qt 文件 API 操作（只读）：

```cpp
// 检查资源是否存在
bool exists = QFile::exists(":/images/logo.png");

// QFileInfo
QFileInfo info(":/images/logo.png");
qDebug() << "大小:" << info.size();
qDebug() << "存在:" << info.exists();
qDebug() << "绝对路径:" << info.absoluteFilePath();  // ":/images/logo.png"

// QDir 遍历资源目录
QDir dir(":/icons");
QStringList icons = dir.entryList(QStringList() << "*.png", QDir::Files);
for (const QString &icon : icons) {
    qDebug() << "图标:" << icon;
}

// 递归遍历
QDirIterator it(":/", QDirIterator::Subdirectories);
while (it.hasNext()) {
    it.next();
    qDebug() << it.filePath();
}
```

> **注意**：资源文件是**只读**的。`QFile::open(QIODevice::WriteOnly)` 会失败。

---

## 7. 压缩控制

### 7.1 rcc 压缩选项

```bash
# 默认压缩（zlib）
rcc resources.qrc -o qrc_resources.cpp

# 指定压缩算法
rcc --compress-algo zlib resources.qrc        # zlib（默认）
rcc --compress-algo zstd resources.qrc        # zstd（Qt 5.13+，更快解压）
rcc --compress-algo none resources.qrc        # 不压缩

# 指定压缩级别（1-9，默认随算法而定）
rcc --compress 9 resources.qrc                # 最高压缩
rcc --compress 1 resources.qrc                # 最快压缩

# 设置压缩阈值（低于此大小的文件不压缩）
rcc --threshold 70 resources.qrc              # 压缩率低于 70% 的文件不压缩
```

### 7.2 在 .qrc 中控制单文件压缩

```xml
<qresource>
    <!-- 已经是压缩格式的文件，禁止二次压缩 -->
    <file compress="0">images/photo.jpg</file>
    <file compress="0">audio/bgm.mp3</file>

    <!-- 指定压缩算法（Qt 5.13+） -->
    <file compress-algo="zstd">data/large_data.json</file>

    <!-- 普通文件使用默认压缩 -->
    <file>styles/app.qss</file>
</qresource>
```

### 7.3 哪些文件适合压缩？

| 文件类型 | 建议 | 原因 |
|---|---|---|
| `.qss`、`.json`、`.xml`、`.txt` | ✅ 压缩 | 文本文件压缩率高 |
| `.svg` | ✅ 压缩 | XML 格式，压缩效果好 |
| `.png`（小图标） | ✅ 默认即可 | 虽已压缩，但 zlib 仍有少量收益 |
| `.jpg`、`.mp3`、`.mp4` | ❌ 禁止 | 已高度压缩，二次压缩浪费 CPU 且几乎无收益 |
| `.ttf`、`.otf` | ✅ 压缩 | 通常有 30~50% 压缩率 |

---

## 8. 运行时注册外部资源（RCC Binary）

除了编译期嵌入，Qt 还支持运行时动态加载外部资源包。

### 8.1 生成二进制资源文件

```bash
# 将 .qrc 编译为独立的 .rcc 二进制文件
rcc --binary resources.qrc -o resources.rcc
```

### 8.2 运行时注册

```cpp
// 注册外部 .rcc 文件
bool ok = QResource::registerResource("plugins/extra_icons.rcc");
if (ok) {
    // 注册成功后，可以通过 :/ 前缀访问其中的资源
    QIcon icon(":/extra/icon1.png");
}

// 卸载已注册的资源
QResource::unregisterResource("plugins/extra_icons.rcc");
```

### 8.3 指定映射根路径

```cpp
// 注册到指定的虚拟路径下
QResource::registerResource("theme_blue.rcc", "/themes/blue");

// 访问时加上映射路径
QFile style(":/themes/blue/style.qss");
```

### 8.4 应用场景

| 场景 | 说明 |
|---|---|
| 插件系统 | 每个插件附带自己的 `.rcc` 资源包 |
| 主题包 | 运行时下载和加载主题资源 |
| DLC / 增量更新 | 不需要重新编译主程序 |
| 减小主程序体积 | 大资源文件外置，按需加载 |

---

## 9. QResource 类详解

`QResource` 类可以直接访问资源的底层数据：

```cpp
QResource res(":/images/logo.png");

if (res.isValid()) {
    qDebug() << "路径:" << res.absoluteFilePath();
    qDebug() << "大小:" << res.size();                    // 原始大小
    qDebug() << "已压缩:" << res.isCompressed();
    qDebug() << "压缩大小:" << res.compressionAlgorithm(); // Qt 5.13+

    // 获取原始数据指针（零拷贝，直接指向内存中的数据）
    const uchar *data = res.data();
    qint64 size = res.size();

    // 获取解压后的数据
    QByteArray bytes = res.uncompressedData();  // Qt 5.15+
}
```

### 设置搜索路径

```cpp
// 添加资源搜索路径
QDir::addSearchPath("icons", ":/images/icons");
QDir::addSearchPath("icons", ":/extra_icons");

// 使用搜索路径前缀访问
QPixmap pix("icons:close.png");   // 注意：用冒号而非斜杠
// 会依次在 :/images/icons/ 和 :/extra_icons/ 中查找 close.png
```

---

## 10. 大文件处理策略

嵌入资源会增加可执行文件大小。对于大文件需要权衡策略。

### 10.1 体积影响评估

```bash
# 查看 rcc 生成的资源总大小
rcc --list resources.qrc     # 列出所有文件
rcc --verbose resources.qrc  # 显示详细信息（含压缩后大小）
```

### 10.2 大文件处理建议

| 文件大小 | 建议 |
|---|---|
| < 1 MB | 直接嵌入 qrc |
| 1~10 MB | 视情况嵌入或外置 |
| > 10 MB | 外置为独立文件或 .rcc |
| > 50 MB | 必须外置，考虑延迟加载 |

### 10.3 混合方案

```
应用/
├── myapp.exe                  # 内嵌小型资源（图标、QSS、翻译）
├── resources/
│   ├── fonts.rcc              # 字体包（运行时加载）
│   ├── templates/             # 模板文件（直接读文件系统）
│   └── videos/                # 视频文件（直接读文件系统）
```

```cpp
// 启动时加载外置资源包
QResource::registerResource(appDir + "/resources/fonts.rcc");

// 大文件直接从文件系统读
QFile video(appDir + "/resources/videos/tutorial.mp4");
```

### 10.4 编译时间优化

每次修改 `.qrc` 中的任何文件都会触发 `qrc_*.cpp` 重新生成和编译。对于大型资源：

```cmake
# 拆分为多个 qrc，修改时只重编受影响的部分
qt5_add_resources(RCC_ICONS    resources/icons.qrc)
qt5_add_resources(RCC_STYLES   resources/styles.qrc)
qt5_add_resources(RCC_FONTS    resources/fonts.qrc)

# Qt 6：大资源模式（生成独立的 .obj，不作为 C++ 源码编译）
qt_add_resources(myapp "big_resources"
    BIG_RESOURCES             # 启用大资源模式
    PREFIX "/"
    FILES
        fonts/LargeFont.ttf
        data/big_data.bin
)
```

---

## 11. 高 DPI 图标支持

### 11.1 Retina/@2x 支持

Qt 自动支持高 DPI 图标命名约定：

```
resources/
├── icons/
│   ├── logo.png        # 标准分辨率 (32x32)
│   ├── logo@2x.png     # 2x 高 DPI (64x64)
│   └── logo@3x.png     # 3x 高 DPI (96x96)
```

```xml
<qresource prefix="/icons">
    <file>icons/logo.png</file>
    <file>icons/logo@2x.png</file>
</qresource>
```

```cpp
// Qt 自动选择适合当前 DPI 的版本
QIcon icon(":/icons/logo.png");
// 在 2x 屏幕上自动使用 logo@2x.png
```

### 11.2 显式多分辨率图标

```cpp
QIcon icon;
icon.addFile(":/icons/16x16/settings.png", QSize(16, 16));
icon.addFile(":/icons/24x24/settings.png", QSize(24, 24));
icon.addFile(":/icons/32x32/settings.png", QSize(32, 32));
icon.addFile(":/icons/48x48/settings.png", QSize(48, 48));
btn->setIcon(icon);
```

---

## 12. 完整实战示例

### 12.1 项目结构

```
MyApp/
├── CMakeLists.txt
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── res/
│   ├── app.qrc
│   ├── icons/
│   │   ├── app.png
│   │   ├── close.png
│   │   ├── minimize.png
│   │   ├── maximize.png
│   │   └── settings.png
│   ├── styles/
│   │   ├── light.qss
│   │   └── dark.qss
│   └── fonts/
│       └── SourceHanSansSC-Regular.otf
```

### 12.2 app.qrc

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource prefix="/icons">
        <file alias="app.png">icons/app.png</file>
        <file alias="close.png">icons/close.png</file>
        <file alias="minimize.png">icons/minimize.png</file>
        <file alias="maximize.png">icons/maximize.png</file>
        <file alias="settings.png">icons/settings.png</file>
    </qresource>
    <qresource prefix="/styles">
        <file alias="light.qss">styles/light.qss</file>
        <file alias="dark.qss">styles/dark.qss</file>
    </qresource>
    <qresource prefix="/fonts">
        <file alias="SourceHanSansSC.otf" compress="9">fonts/SourceHanSansSC-Regular.otf</file>
    </qresource>
</RCC>
```

### 12.3 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(MyApp
    main.cpp
    mainwindow.h
    mainwindow.cpp
    res/app.qrc
)

target_link_libraries(MyApp PRIVATE Qt5::Widgets)
```

### 12.4 main.cpp

```cpp
#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include "mainwindow.h"

void loadFont() {
    int id = QFontDatabase::addApplicationFont(":/fonts/SourceHanSansSC.otf");
    if (id != -1) {
        QStringList families = QFontDatabase::applicationFontFamilies(id);
        if (!families.isEmpty()) {
            qApp->setFont(QFont(families.first(), 11));
        }
    }
}

void loadStyleSheet(const QString &path) {
    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/app.png"));

    loadFont();
    loadStyleSheet(":/styles/light.qss");

    MainWindow w;
    w.show();
    return app.exec();
}
```

### 12.5 mainwindow.cpp 中使用资源

```cpp
#include "mainwindow.h"
#include <QToolBar>
#include <QAction>
#include <QDir>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("MyApp");
    resize(800, 600);

    // 工具栏使用资源图标
    QToolBar *toolbar = addToolBar("主工具栏");
    toolbar->addAction(QIcon(":/icons/settings.png"), "设置");
    toolbar->addAction(QIcon(":/icons/close.png"), "关闭", this, &QWidget::close);

    // 列出所有已注册的图标资源
    QDir iconDir(":/icons");
    qDebug() << "已注册的图标:" << iconDir.entryList();
}
```

---

## 13. 常见问题与注意事项

### Q1：资源路径写对了，但运行时加载失败？

**检查列表**：
1. `.qrc` 文件是否已添加到构建系统（CMakeLists.txt / .pro）
2. `<file>` 路径是否相对于 `.qrc` 文件所在目录
3. 路径**大小写**是否匹配（Linux 文件系统区分大小写）
4. 是否遗漏了 `prefix`（访问路径 = prefix + file）

```cpp
// 检查资源是否存在
qDebug() << QFile::exists(":/icons/close.png");  // true / false

// 列出某前缀下所有资源
QDir dir(":/icons");
qDebug() << dir.entryList();
```

### Q2：资源文件太大导致编译慢？

- 拆分为多个 `.qrc` 文件
- 大文件改为外置 `.rcc` 运行时加载
- Qt 6 使用 `BIG_RESOURCES` 选项
- JPEG/PNG 等已压缩文件设 `compress="0"` 避免浪费编译时间

### Q3：如何在不重新编译的情况下更新资源？

使用外置 `.rcc` 方案：

```bash
# 修改资源后重新打包
rcc --binary updated.qrc -o updated.rcc
# 将 updated.rcc 分发给用户替换旧文件即可
```

### Q4：发布后资源被提取怎么办？

Qt 资源嵌入在二进制中，尽管理论上可以被提取（通过逆向工程），但对普通用户已有足够的保护。如需更高安全性：
- 对敏感数据进行加密后再嵌入，运行时解密
- 关键资源通过网络获取而非嵌入

### Q5：QSS 中 url() 路径怎么写？

```css
/* 使用 :/ 前缀 */
QPushButton {
    background-image: url(:/images/btn_bg.png);
    /* 注意：url 和括号之间不能有空格 */
}

QCheckBox::indicator:checked {
    image: url(:/icons/check.png);
}
```

### Q6：资源和本地文件同名，优先哪个？

`:/ ` 前缀路径始终访问资源系统，不带前缀的路径访问文件系统。两者互不干扰：

```cpp
QFile res(":/config.json");     // 从资源系统读
QFile local("config.json");     // 从文件系统读
```

---

## 14. 速查表

```
功能                    语法 / API
──────────────────────────────────────────────────────
.qrc 根元素             <RCC version="1.0">
资源分组                <qresource prefix="/icons">
添加文件                <file>path/file.png</file>
文件别名                <file alias="short.png">long/path.png</file>
语言特化                <qresource prefix="/" lang="zh_CN">
禁止压缩                <file compress="0">photo.jpg</file>
压缩算法                <file compress-algo="zstd">data.json</file>

代码访问                ":/prefix/file"
QML 访问                "qrc:/prefix/file"
QSS 访问                url(:/prefix/file)

检查存在                QFile::exists(":/path")
遍历目录                QDir(":/prefix").entryList()
资源信息                QResource(":/path").size()

生成 rcc 二进制         rcc --binary res.qrc -o res.rcc
运行时注册              QResource::registerResource("res.rcc")
运行时卸载              QResource::unregisterResource("res.rcc")
搜索路径                QDir::addSearchPath("name", ":/path")

CMake (Qt5)             qt5_add_resources(VAR file.qrc)
CMake (Qt6)             qt_add_resources(target "name" FILES ...)
qmake                   RESOURCES += file.qrc
```
