# QFile 与 QDir

Qt 的文件 I/O 体系围绕 `QIODevice` 基类构建，`QFile` 是最常用的文件读写类，`QDir` 负责目录操作，`QFileInfo` 提供文件元信息查询。三者配合覆盖日常文件系统操作的全部需求，且天然跨平台。

---

## 1. 类体系概览

```
QIODevice（抽象基类：所有 I/O 设备的统一接口）
├── QFile              磁盘文件读写
│   └── QTemporaryFile 自动管理的临时文件
├── QBuffer            内存缓冲区（QByteArray 做后端）
├── QProcess           进程 I/O
└── QAbstractSocket    网络 I/O

QDir                   目录浏览与操作（非 QIODevice 子类）
QFileInfo              文件/目录元信息查询
QStandardPaths         系统标准目录路径
QStorageInfo           磁盘/分区信息
```

### 模块依赖

所有类均在 `QtCore` 中，无需额外链接：

```cpp
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDataStream>
```

---

## 2. QFile：文件读写

### 2.1 打开与关闭

```cpp
QFile file("data.txt");

// 打开文件（返回 bool）
if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Open failed:" << file.errorString();
    return;
}

// ... 读写操作 ...

file.close();  // 显式关闭（QFile 析构时也会自动关闭）
```

### 2.2 打开模式（OpenMode 标志位）

| 标志 | 值 | 说明 |
|---|---|---|
| `QIODevice::ReadOnly` | 只读 | 文件必须存在 |
| `QIODevice::WriteOnly` | 只写 | 文件不存在则创建，存在则**清空** |
| `QIODevice::ReadWrite` | 读写 | 文件不存在则创建 |
| `QIODevice::Append` | 追加 | 写入数据追加到文件末尾 |
| `QIODevice::Truncate` | 截断 | 打开时清空文件内容 |
| `QIODevice::Text` | 文本模式 | Windows 上自动转换 `\r\n` ↔ `\n` |
| `QIODevice::Unbuffered` | 无缓冲 | 绕过 Qt 的内部缓冲区 |
| `QIODevice::NewOnly` | 新建 | 文件已存在则打开失败（Qt 5.11+） |
| `QIODevice::ExistingOnly` | 已存在 | 文件不存在则打开失败（Qt 5.11+） |

```cpp
// 常见组合
QIODevice::ReadOnly | QIODevice::Text       // 读文本文件
QIODevice::WriteOnly | QIODevice::Text       // 写文本文件（覆盖）
QIODevice::WriteOnly | QIODevice::Append     // 追加写入
QIODevice::ReadWrite | QIODevice::Truncate   // 读写，先清空
QIODevice::WriteOnly | QIODevice::NewOnly    // 仅新建，已存在则失败
```

### 2.3 读取文件

```cpp
// ===== 方式 1：一次性读取全部内容 =====
QFile file("data.txt");
file.open(QIODevice::ReadOnly);
QByteArray allData = file.readAll();
file.close();
// 适合小文件（几 MB 以内）

// ===== 方式 2：逐行读取（文本文件） =====
QFile file("log.txt");
file.open(QIODevice::ReadOnly | QIODevice::Text);
while (!file.atEnd()) {
    QByteArray line = file.readLine();  // 包含末尾 \n
    QString text = QString::fromUtf8(line).trimmed();
    qDebug() << text;
}

// ===== 方式 3：按块读取（大文件） =====
QFile file("bigfile.bin");
file.open(QIODevice::ReadOnly);
const qint64 blockSize = 64 * 1024;  // 64KB
while (!file.atEnd()) {
    QByteArray block = file.read(blockSize);
    processBlock(block);
}

// ===== 方式 4：用 QTextStream 读取（推荐文本文件） =====
QFile file("data.txt");
file.open(QIODevice::ReadOnly | QIODevice::Text);
QTextStream in(&file);
in.setCodec("UTF-8");  // Qt 5；Qt 6 默认 UTF-8
while (!in.atEnd()) {
    QString line = in.readLine();  // 自动去除换行符
    qDebug() << line;
}

// ===== 方式 5：读取全部文本 =====
QFile file("data.txt");
file.open(QIODevice::ReadOnly | QIODevice::Text);
QTextStream in(&file);
QString allText = in.readAll();
```

### 2.4 写入文件

```cpp
// ===== 方式 1：写入原始字节 =====
QFile file("output.bin");
file.open(QIODevice::WriteOnly);
file.write("Hello, Qt!\n");                     // const char*
file.write(QByteArray::fromHex("48656C6C6F"));  // 二进制数据
file.close();

// ===== 方式 2：用 QTextStream 写入文本（推荐） =====
QFile file("output.txt");
file.open(QIODevice::WriteOnly | QIODevice::Text);
QTextStream out(&file);
out.setCodec("UTF-8");  // Qt 5
out << "Name: " << name << "\n";
out << "Score: " << score << "\n";
out << QString("中文内容") << "\n";
file.close();

// ===== 方式 3：追加模式 =====
QFile logFile("app.log");
logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
QTextStream log(&logFile);
log << QDateTime::currentDateTime().toString(Qt::ISODate)
    << " [INFO] Application started\n";
logFile.close();
```

### 2.5 文件定位

```cpp
QFile file("data.bin");
file.open(QIODevice::ReadOnly);

qint64 size = file.size();          // 文件总大小（字节）
qint64 pos  = file.pos();           // 当前读写位置

file.seek(100);                     // 跳到第 100 字节
file.seek(0);                       // 回到文件开头
file.seek(file.size());             // 跳到文件末尾

QByteArray data = file.peek(10);    // 预读 10 字节（不移动位置）

file.reset();                       // 等价于 seek(0)
```

### 2.6 错误处理

```cpp
QFile file("missing.txt");
if (!file.open(QIODevice::ReadOnly)) {
    // 错误信息
    qWarning() << "Error:" << file.errorString();
    // "No such file or directory" / "Permission denied" 等

    // 错误码
    QFileDevice::FileError err = file.error();
    switch (err) {
        case QFileDevice::NoError:         break;
        case QFileDevice::ReadError:       qWarning() << "Read error"; break;
        case QFileDevice::WriteError:      qWarning() << "Write error"; break;
        case QFileDevice::OpenError:       qWarning() << "Open error"; break;
        case QFileDevice::PermissionsError:qWarning() << "Permission denied"; break;
        case QFileDevice::ResourceError:   qWarning() << "Disk full?"; break;
        default: break;
    }
}
```

### 2.7 RAII 模式（利用作用域自动关闭）

```cpp
{
    QFile file("data.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll();
        processContent(content);
    }
    // file 离开作用域时自动析构并关闭
}
```

---

## 3. QFile 静态操作

无需创建 `QFile` 对象即可执行的文件操作：

```cpp
// 文件是否存在
bool exists = QFile::exists("data.txt");

// 复制文件
bool ok = QFile::copy("source.txt", "dest.txt");
// 注意：目标文件已存在时返回 false（不覆盖）

// 覆盖复制（先删后拷）
if (QFile::exists("dest.txt")) {
    QFile::remove("dest.txt");
}
QFile::copy("source.txt", "dest.txt");

// 重命名/移动文件
QFile::rename("old.txt", "new.txt");
QFile::rename("file.txt", "subdir/file.txt");  // 移动到子目录

// 删除文件
QFile::remove("temp.txt");

// 创建符号链接
QFile::link("target.txt", "link.txt");    // Unix 软链接
QFile::link("target.txt", "link.lnk");   // Windows 快捷方式

// 读取符号链接目标
QString target = QFile::symLinkTarget("link.txt");

// 设置文件权限
QFile::setPermissions("script.sh",
    QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
    QFileDevice::ReadGroup | QFileDevice::ExeGroup);
// 等价于 chmod 750

// 查看文件权限
QFile::Permissions perms = QFile::permissions("data.txt");
bool readable = perms & QFileDevice::ReadOwner;
bool writable = perms & QFileDevice::WriteOwner;
```

---

## 4. QFileInfo：文件元信息

### 4.1 基本信息

```cpp
QFileInfo info("/home/user/documents/report.pdf");

// 路径分解
info.absoluteFilePath();  // "/home/user/documents/report.pdf"
info.absolutePath();      // "/home/user/documents"
info.fileName();          // "report.pdf"
info.baseName();          // "report"
info.completeSuffix();    // "pdf"（对 "file.tar.gz" 返回 "tar.gz"）
info.suffix();            // "pdf"（对 "file.tar.gz" 返回 "gz"）
info.path();              // 构造时传入的路径部分
info.canonicalFilePath(); // 解析符号链接后的绝对路径
info.canonicalPath();     // 解析符号链接后的目录路径
```

```cpp
// 路径分解示例
QFileInfo info("C:/Users/me/archive.tar.gz");
info.fileName();          // "archive.tar.gz"
info.baseName();          // "archive"（第一个 . 之前）
info.completeBaseName();  // "archive.tar"（最后一个 . 之前）
info.suffix();            // "gz"（最后一个 . 之后）
info.completeSuffix();    // "tar.gz"（第一个 . 之后）
```

### 4.2 存在性与类型

```cpp
QFileInfo info("path/to/something");

info.exists();            // 是否存在
info.isFile();            // 是否为普通文件
info.isDir();             // 是否为目录
info.isSymLink();         // 是否为符号链接
info.isSymbolicLink();    // 是否为符号链接（Qt 5.14+ 新名称）
info.isShortcut();        // 是否为 Windows .lnk 快捷方式
info.isHidden();          // 是否为隐藏文件（Unix 以 . 开头 / Windows 隐藏属性）
info.isReadable();        // 是否可读
info.isWritable();        // 是否可写
info.isExecutable();      // 是否可执行
info.isRelative();        // 是否为相对路径
info.isAbsolute();        // 是否为绝对路径
```

### 4.3 大小与时间

```cpp
QFileInfo info("video.mp4");

// 文件大小
qint64 bytes = info.size();  // 字节数
double mb = bytes / (1024.0 * 1024.0);

// 时间信息
QDateTime created  = info.birthTime();       // 创建时间（Qt 5.10+）
QDateTime modified = info.lastModified();    // 最后修改时间
QDateTime accessed = info.lastRead();        // 最后读取时间
// Qt 5 旧接口：info.created()

// 格式化输出
qDebug() << info.fileName()
         << QString::number(mb, 'f', 2) + " MB"
         << modified.toString("yyyy-MM-dd hh:mm:ss");
```

### 4.4 所有者与权限

```cpp
QFileInfo info("/etc/passwd");

// 所有者（Unix）
info.owner();          // "root"
info.ownerId();        // 0
info.group();          // "root"
info.groupId();        // 0

// 权限检查
info.permission(QFile::ReadUser);   // 当前用户是否可读
info.permission(QFile::WriteUser);  // 当前用户是否可写

// 获取完整权限标志
QFile::Permissions perms = info.permissions();
```

### 4.5 性能注意事项

```cpp
// QFileInfo 默认会缓存文件系统信息
QFileInfo info("data.txt");
info.size();       // 第一次访问：查询文件系统
info.size();       // 后续访问：使用缓存

// 如果文件可能已变化，手动刷新缓存
info.refresh();
info.size();       // 重新查询文件系统

// 对大量文件操作时，缓存能显著提升性能
// 但如果需要实时信息，每次操作前调用 refresh()

// 也可以设置为不缓存（Qt 6.0+）
info.setCaching(false);
```

---

## 5. QDir：目录操作

### 5.1 创建 QDir 对象

```cpp
// 指定路径
QDir dir("/home/user/documents");
QDir dir("C:/Users/me/Desktop");

// 当前工作目录
QDir currentDir = QDir::current();
QString cwd = QDir::currentPath();  // 静态方法

// 用户主目录
QDir homeDir = QDir::home();
QString home = QDir::homePath();    // "/home/user" 或 "C:/Users/me"

// 系统根目录
QDir rootDir = QDir::root();
QString root = QDir::rootPath();    // "/" 或 "C:/"

// 临时目录
QDir tempDir = QDir::temp();
QString tmp = QDir::tempPath();     // "/tmp" 或 "C:/Users/me/AppData/Local/Temp"
```

### 5.2 目录信息

```cpp
QDir dir("/home/user/documents");

dir.exists();             // 目录是否存在
dir.isAbsolute();         // 是否为绝对路径
dir.isRelative();         // 是否为相对路径
dir.isReadable();         // 是否可读
dir.isEmpty();            // 目录是否为空（Qt 5.9+）

dir.absolutePath();       // 绝对路径
dir.canonicalPath();      // 解析符号链接后的路径
dir.dirName();            // 最后一段目录名："documents"
dir.path();               // 构造时传入的路径

dir.count();              // 条目数量（含 . 和 ..）
```

### 5.3 列出目录内容

```cpp
QDir dir("/home/user/projects");

// ===== 列出文件名 =====

// 所有条目（文件 + 目录）
QStringList all = dir.entryList();
// [".", "..", "file1.cpp", "file2.h", "subdir"]

// 只列文件
QStringList files = dir.entryList(QDir::Files);

// 只列目录（排除 . 和 ..）
QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

// 按通配符过滤
QStringList sources = dir.entryList({"*.cpp", "*.h"}, QDir::Files);

// 排序
QStringList sorted = dir.entryList(
    QDir::Files,
    QDir::Name | QDir::IgnoreCase  // 按名称，忽略大小写
);

// ===== 列出详细信息（QFileInfo 列表） =====

QFileInfoList fileInfos = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
for (const QFileInfo &fi : fileInfos) {
    qDebug() << fi.fileName()
             << fi.size()
             << fi.lastModified().toString("yyyy-MM-dd");
}
```

### 5.4 过滤器（QDir::Filter）

| 过滤器 | 说明 |
|---|---|
| `QDir::Dirs` | 目录 |
| `QDir::Files` | 文件 |
| `QDir::Drives` | 驱动器（Windows） |
| `QDir::AllEntries` | 文件 + 目录 + 符号链接 |
| `QDir::NoDotAndDotDot` | 排除 `.` 和 `..` |
| `QDir::NoDot` | 只排除 `.` |
| `QDir::NoDotDot` | 只排除 `..` |
| `QDir::Hidden` | 包含隐藏文件 |
| `QDir::System` | 包含系统文件 |
| `QDir::Readable` | 可读的 |
| `QDir::Writable` | 可写的 |
| `QDir::Executable` | 可执行的 |
| `QDir::NoSymLinks` | 排除符号链接 |
| `QDir::AllDirs` | 列出所有目录（不受通配符影响） |
| `QDir::CaseSensitive` | 通配符区分大小写 |

### 5.5 排序标志（QDir::SortFlag）

| 排序 | 说明 |
|---|---|
| `QDir::Name` | 按名称 |
| `QDir::Time` | 按修改时间 |
| `QDir::Size` | 按大小 |
| `QDir::Type` | 按扩展名 |
| `QDir::Unsorted` | 不排序（最快） |
| `QDir::DirsFirst` | 目录排在文件前 |
| `QDir::DirsLast` | 目录排在文件后 |
| `QDir::Reversed` | 反序 |
| `QDir::IgnoreCase` | 名称排序时忽略大小写 |

```cpp
// 综合示例：列出所有 C++ 源文件，按修改时间倒序
QStringList files = dir.entryList(
    {"*.cpp", "*.h", "*.hpp"},
    QDir::Files | QDir::Readable,
    QDir::Time | QDir::Reversed
);
```

### 5.6 创建与删除目录

```cpp
QDir dir;

// 创建单级目录
bool ok = dir.mkdir("newdir");  // 父目录必须存在

// 创建多级目录（递归，类似 mkdir -p）
bool ok = dir.mkpath("path/to/deep/dir");

// 删除空目录
bool ok = dir.rmdir("emptydir");  // 目录必须为空

// 递归删除目录及其所有内容（危险操作！）
bool ok = dir.removeRecursively();
// 等价于 rm -rf，删除 dir 指向的目录及所有子内容

// 安全使用：先确认路径
QDir targetDir("C:/temp/build_output");
if (targetDir.exists() && targetDir.absolutePath().contains("build_output")) {
    targetDir.removeRecursively();
}
```

### 5.7 路径操作

```cpp
QDir dir("/home/user");

// 进入子目录
dir.cd("documents");          // 现在指向 /home/user/documents
dir.cd("..");                 // 回到 /home/user
dir.cdUp();                   // 等价于 cd("..")

// 拼接路径（正确处理分隔符）
QString path = dir.filePath("report.pdf");
// "/home/user/report.pdf"

QString abs = dir.absoluteFilePath("report.pdf");
// "/home/user/report.pdf"（即使 dir 是相对路径，也返回绝对路径）

// 检查文件是否在目录中
bool has = dir.exists("report.pdf");

// 路径规范化
QString clean = QDir::cleanPath("/home/user/../user/./documents//file.txt");
// "/home/user/documents/file.txt"

// 获取相对路径
QDir base("/home/user");
QString rel = base.relativeFilePath("/home/user/documents/file.txt");
// "documents/file.txt"

// 转换为原生路径分隔符
QString native = QDir::toNativeSeparators("/home/user/file.txt");
// Windows: "\\home\\user\\file.txt"
// Unix: "/home/user/file.txt"（不变）

QString unified = QDir::fromNativeSeparators("C:\\Users\\me\\file.txt");
// "C:/Users/me/file.txt"（统一为 /）
```

### 5.8 驱动器列表（Windows）

```cpp
// 获取所有驱动器
QFileInfoList drives = QDir::drives();
for (const QFileInfo &drive : drives) {
    qDebug() << drive.absolutePath();
    // "C:/", "D:/", "E:/" ...
}
```

---

## 6. QStandardPaths：系统标准目录

提供跨平台的标准目录路径（配置、缓存、数据等）。

```cpp
#include <QStandardPaths>

// 桌面
QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
// Win: "C:/Users/me/Desktop"
// Mac: "/Users/me/Desktop"
// Lin: "/home/me/Desktop"

// 用户文档
QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
// Win: "C:/Users/me/Documents"

// 应用数据目录（持久化数据）
QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
// Win: "C:/Users/me/AppData/Roaming/<AppName>"
// Mac: "~/Library/Application Support/<AppName>"
// Lin: "~/.local/share/<AppName>"

// 应用缓存目录
QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
// Win: "C:/Users/me/AppData/Local/<AppName>/cache"

// 应用配置目录
QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
// Win: "C:/Users/me/AppData/Local/<AppName>"
// Lin: "~/.config/<AppName>"

// 临时目录
QStandardPaths::writableLocation(QStandardPaths::TempLocation);

// 下载目录
QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

// 图片、音乐、视频
QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);

// 查找可执行文件路径（类似 which/where）
QString git = QStandardPaths::findExecutable("git");
// "C:/Program Files/Git/cmd/git.exe" 或 "/usr/bin/git"
```

常用模式——确保应用数据目录存在：

```cpp
QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
QDir().mkpath(dataDir);  // 不存在则创建
```

---

## 7. QTemporaryFile：临时文件

自动生成唯一文件名，默认在系统临时目录中，析构时自动删除。

```cpp
#include <QTemporaryFile>

// 基本用法
QTemporaryFile tempFile;
if (tempFile.open()) {
    qDebug() << "Temp file:" << tempFile.fileName();
    // e.g. "C:/Users/me/AppData/Local/Temp/qt_temp.XXXXXX"

    tempFile.write("temporary data");
    // 对象析构时自动删除文件
}

// 自定义文件名模板
QTemporaryFile tempFile("myapp_XXXXXX.txt");
// XXXXXX 被替换为随机字符
// 文件名类似：myapp_aB3kQw.txt

// 自定义目录 + 后缀
QTemporaryFile tempFile(QDir::tempPath() + "/export_XXXXXX.csv");

// 保留文件（不在析构时删除）
tempFile.setAutoRemove(false);
```

---

## 8. QTemporaryDir：临时目录

```cpp
#include <QTemporaryDir>

QTemporaryDir tempDir;
if (tempDir.isValid()) {
    QString dirPath = tempDir.path();
    qDebug() << "Temp dir:" << dirPath;

    // 在临时目录中创建文件
    QFile file(dirPath + "/data.txt");
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();
}
// 析构时整个目录及内容被自动删除

// 保留临时目录
tempDir.setAutoRemove(false);
```

---

## 9. QSaveFile：安全写入

`QSaveFile` 先写入临时文件，调用 `commit()` 时原子性地替换目标文件。写入过程中崩溃不会损坏原文件。

```cpp
#include <QSaveFile>

bool safeSave(const QString &filePath, const QByteArray &data) {
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open:" << file.errorString();
        return false;
    }

    file.write(data);

    // commit() 原子性替换目标文件
    // 如果之前的写入有任何失败，commit() 会返回 false，不影响原文件
    if (!file.commit()) {
        qWarning() << "Commit failed:" << file.errorString();
        return false;
    }
    return true;
}

// 配合 QTextStream 使用
QSaveFile file("config.json");
file.open(QIODevice::WriteOnly | QIODevice::Text);
QTextStream out(&file);
out << jsonContent;
file.commit();  // 原子替换

// 取消写入（放弃变更，不影响原文件）
file.cancelWriting();
```

---

## 10. 实战案例

### 10.1 递归查找文件

```cpp
QStringList findFiles(const QDir &dir, const QStringList &nameFilters) {
    QStringList results;

    // 当前目录的匹配文件
    QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files);
    for (const QFileInfo &fi : files) {
        results.append(fi.absoluteFilePath());
    }

    // 递归子目录
    QFileInfoList subDirs = dir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (const QFileInfo &subDir : subDirs) {
        results.append(findFiles(QDir(subDir.absoluteFilePath()), nameFilters));
    }

    return results;
}

// 使用
QStringList cppFiles = findFiles(QDir("C:/project/src"), {"*.cpp", "*.h"});
for (const auto &f : cppFiles) {
    qDebug() << f;
}
```

### 10.2 使用 QDirIterator（推荐替代递归）

```cpp
#include <QDirIterator>

// 递归遍历（比手动递归更高效）
QDirIterator it("C:/project/src",
                {"*.cpp", "*.h"},                          // 名称过滤
                QDir::Files,                               // 类型过滤
                QDirIterator::Subdirectories);             // 递归子目录

while (it.hasNext()) {
    it.next();
    qDebug() << it.filePath() << it.fileInfo().size();
}

// 非递归（仅当前目录）
QDirIterator it("C:/project", QDir::Files | QDir::NoDotAndDotDot);
while (it.hasNext()) {
    qDebug() << it.next();
}

// 跟随符号链接
QDirIterator it("path", QDir::AllEntries,
                QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
```

### 10.3 计算目录总大小

```cpp
qint64 directorySize(const QString &path) {
    qint64 totalSize = 0;
    QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        totalSize += it.fileInfo().size();
    }
    return totalSize;
}

// 使用
qint64 size = directorySize("C:/project");
qDebug() << "Total size:" << size / (1024.0 * 1024.0) << "MB";
```

### 10.4 复制整个目录

```cpp
bool copyDirectory(const QString &srcPath, const QString &dstPath) {
    QDir srcDir(srcPath);
    if (!srcDir.exists()) return false;

    QDir dstDir(dstPath);
    if (!dstDir.exists()) {
        dstDir.mkpath(".");
    }

    // 复制文件
    for (const QFileInfo &fi : srcDir.entryInfoList(QDir::Files)) {
        QString srcFile = fi.absoluteFilePath();
        QString dstFile = dstPath + "/" + fi.fileName();
        if (!QFile::copy(srcFile, dstFile)) {
            qWarning() << "Failed to copy:" << srcFile;
            return false;
        }
    }

    // 递归复制子目录
    for (const QFileInfo &fi : srcDir.entryInfoList(
             QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (!copyDirectory(fi.absoluteFilePath(),
                           dstPath + "/" + fi.fileName())) {
            return false;
        }
    }

    return true;
}
```

### 10.5 文件大小格式化

```cpp
QString formatFileSize(qint64 bytes) {
    if (bytes < 1024)
        return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024)
        return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024LL * 1024 * 1024)
        return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

// 使用
QFileInfo info("video.mp4");
qDebug() << info.fileName() << formatFileSize(info.size());
// "video.mp4 1.3 GB"
```

### 10.6 日志文件写入

```cpp
class SimpleLogger {
public:
    SimpleLogger(const QString &logDir) {
        QDir().mkpath(logDir);
        QString filename = QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".log";
        m_file.setFileName(logDir + "/" + filename);
        m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        m_stream.setDevice(&m_file);
    }

    ~SimpleLogger() {
        m_file.close();
    }

    void log(const QString &level, const QString &message) {
        m_stream << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
                 << " [" << level << "] " << message << "\n";
        m_stream.flush();  // 确保立即写入
    }

    void info(const QString &msg)  { log("INFO",  msg); }
    void warn(const QString &msg)  { log("WARN",  msg); }
    void error(const QString &msg) { log("ERROR", msg); }

private:
    QFile m_file;
    QTextStream m_stream;
};

// 使用
SimpleLogger logger(QStandardPaths::writableLocation(
    QStandardPaths::AppDataLocation) + "/logs");
logger.info("Application started");
logger.error("Failed to connect to database");
```

---

## 11. QStorageInfo：磁盘信息

```cpp
#include <QStorageInfo>

// 当前文件系统所在存储设备
QStorageInfo storage = QStorageInfo::root();
qDebug() << "Root volume:" << storage.rootPath();
qDebug() << "Name:"        << storage.name();
qDebug() << "File system:" << storage.fileSystemType();   // "NTFS", "ext4", "APFS"
qDebug() << "Total:"       << storage.bytesTotal() / (1024*1024) << "MB";
qDebug() << "Available:"   << storage.bytesAvailable() / (1024*1024) << "MB";
qDebug() << "Free:"        << storage.bytesFree() / (1024*1024) << "MB";
qDebug() << "Read-only:"   << storage.isReadOnly();

// 指定路径的存储设备
QStorageInfo dataStore("D:/Data");
qDebug() << dataStore.rootPath();  // "D:/"

// 列出所有挂载卷
for (const QStorageInfo &vol : QStorageInfo::mountedVolumes()) {
    if (vol.isValid() && vol.isReady()) {
        qDebug() << vol.rootPath()
                 << vol.fileSystemType()
                 << vol.bytesAvailable() / (1024*1024*1024) << "GB free";
    }
}
```

---

## 12. 路径处理最佳实践

### 12.1 跨平台路径写法

```cpp
// ❌ 硬编码分隔符
QString path = "C:\\Users\\me\\file.txt";      // 仅 Windows
QString path = "/home/user/file.txt";          // 仅 Unix

// ✅ 用 / 或 QDir::separator()，Qt 内部统一处理
QString path = "C:/Users/me/file.txt";          // Qt 在所有平台接受 /
QString path = QDir::homePath() + "/file.txt";  // 跨平台

// ✅ 用 QDir 拼接
QDir dir = QDir::home();
QString path = dir.filePath("documents/file.txt");
```

### 12.2 路径规范化

```cpp
// 消除冗余
QDir::cleanPath("C:/Users/me/../me/./docs//file.txt");
// "C:/Users/me/docs/file.txt"

// 绝对路径
QFileInfo info("relative/path.txt");
QString abs = info.absoluteFilePath();

// 解析符号链接
QString real = info.canonicalFilePath();  // 空字符串表示文件不存在
```

### 12.3 路径中的特殊字符

```cpp
// 路径含空格：Qt 正常处理，无需特殊转义
QFile file("C:/Program Files/My App/config.txt");  // OK

// URL 与路径互转
QUrl url = QUrl::fromLocalFile("C:/path/to/file.txt");
// "file:///C:/path/to/file.txt"

QString path = url.toLocalFile();
// "C:/path/to/file.txt"

// 从用户输入安全地解析路径
QString userInput = "~/documents/file.txt";
// Qt 不自动展开 ~，需手动处理
if (userInput.startsWith("~")) {
    userInput.replace(0, 1, QDir::homePath());
}
```

---

## 13. 常见问题

**问题 1：QFile::copy 失败（目标已存在）**

```cpp
// QFile::copy 不覆盖已存在的文件
// ❌
QFile::copy("src.txt", "dst.txt");  // 如果 dst.txt 存在，返回 false

// ✅ 先删除
if (QFile::exists("dst.txt")) {
    QFile::remove("dst.txt");
}
QFile::copy("src.txt", "dst.txt");
```

**问题 2：文本文件换行符混乱**

```cpp
// ❌ 二进制模式写文本，Windows 上只有 \n
QFile file("out.txt");
file.open(QIODevice::WriteOnly);  // 无 QIODevice::Text
file.write("line1\nline2\n");    // Windows 记事本显示为一行

// ✅ 加 Text 模式，Qt 自动转换
file.open(QIODevice::WriteOnly | QIODevice::Text);
file.write("line1\nline2\n");    // Windows 上实际写入 \r\n
```

**问题 3：读取中文乱码**

```cpp
// ❌ Qt 5 默认用 Latin1 编码读取
QFile file("chinese.txt");
file.open(QIODevice::ReadOnly);
QString text = file.readAll();  // 乱码（QByteArray → QString 默认 Latin1）

// ✅ 方法 1：用 QTextStream 指定编码
QTextStream in(&file);
in.setCodec("UTF-8");  // Qt 5
QString text = in.readAll();

// ✅ 方法 2：手动转换
QByteArray raw = file.readAll();
QString text = QString::fromUtf8(raw);

// Qt 6 默认 UTF-8，通常无此问题
```

**问题 4：QDir::removeRecursively() 删除了意外内容**

```cpp
// ❌ 危险：路径为空或根目录时会删除大量文件
QDir dir(configPath);  // 如果 configPath 为空，dir 指向当前工作目录！
dir.removeRecursively();

// ✅ 安全检查
QString path = configPath;
if (path.isEmpty() || path == "/" || path == "C:/" ||
    !QDir(path).exists()) {
    qWarning() << "Refusing to delete:" << path;
    return;
}
QDir(path).removeRecursively();
```

**问题 5：文件被占用无法删除/重命名**

```cpp
// Windows 上文件被其他进程打开时无法删除
// 确保自己的 QFile 已关闭
{
    QFile file("data.txt");
    file.open(QIODevice::ReadOnly);
    // ... 使用文件 ...
}  // file 析构，自动关闭

// 现在可以安全删除
QFile::remove("data.txt");
```

---

## 14. 最佳实践

| 实践 | 说明 |
|---|---|
| 路径用 `/` 分隔 | Qt 在所有平台统一处理 `/`，不要用 `\\` |
| 用 `QDir::filePath()` 拼接 | 避免手动拼接 `/` 导致双斜杠 |
| 用 `QStandardPaths` 定位目录 | 永远不要硬编码系统目录路径 |
| 用 `QSaveFile` 写配置 | 原子写入，防止崩溃导致文件损坏 |
| 检查 `open()` 返回值 | 每次打开文件都要检查是否成功 |
| 大文件分块读取 | 不要对大文件调用 `readAll()`，用 `read(blockSize)` |
| 用 `QDirIterator` 递归遍历 | 比手动递归更高效、更简洁 |
| 文本文件加 `Text` 标志 | 确保换行符在跨平台下正确转换 |
| 及时关闭文件 | 利用 RAII 作用域或显式 `close()`，避免文件锁 |
| `removeRecursively` 前验证路径 | 防止误删根目录或工作目录 |
