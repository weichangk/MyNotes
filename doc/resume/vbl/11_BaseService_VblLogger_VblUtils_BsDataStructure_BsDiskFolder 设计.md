# VblLogger / VblUtils / BsDataStructure / BsDiskFolder 模块详细设计文档

> 所属层：BaseService（基础设施子层）
> 命名空间：`VBL::`

---

## 一、VblLogger（日志系统）

### 接口头
`Interface/BaseService/VblLogger/`

### 1.1 职责
- 提供统一的**多级日志输出**接口（Debug/Info/Warning/Error/Fatal）
- 支持多后端输出：文件通道（`IBsFileChannel`）、控制台通道（`IBsConsoleChannel`）
- 异步写文件，不阻塞主线程
- 日志格式：`[Level][ThreadId][Timestamp][File:Line] Message`
- 全局日志级别过滤（低于设定级别的日志被过滤丢弃）

### 1.2 核心接口

```cpp
class IBsLogger : virtual public IDmBaseObj {
    virtual Result addChannel(IBsLogChannel* channel) = 0;
    virtual Result removeChannel(IBsLogChannel* channel) = 0;
    virtual Result setLogLevel(LogLevel level) = 0;     // DEBUG/INFO/WARN/ERROR/FATAL
    virtual Result log(LogLevel level, VBLConstPChar file, VBLInt line,
                       VBLConstPChar func, VBLConstPChar msg) = 0;
    virtual Result flush() = 0;
};

class IBsFileChannel : virtual public IBsLogChannel {
    virtual Result setFilePath(VBLConstPChar path) = 0;
    virtual Result setMaxFileSize(VBLLonglong bytes) = 0;   // 单文件最大字节数
    virtual Result setMaxBackupCount(VBLInt count) = 0;     // 日志文件滚动数量
    virtual Result setAsyncWrite(VBLBool async) = 0;        // 异步写入开关
};

class IBsConsoleChannel : virtual public IBsLogChannel {
    virtual Result setColorOutput(VBLBool enable) = 0;      // 彩色终端输出
};
```

### 1.3 宏定义（全模块使用）

```cpp
// 各模块通过宏调用，自动注入文件名/行号
#define VBL_LOG_DEBUG(msg)   VblLogger::log(DEBUG, __FILE__, __LINE__, __FUNC__, msg)
#define VBL_LOG_INFO(msg)    VblLogger::log(INFO,  __FILE__, __LINE__, __FUNC__, msg)
#define VBL_LOG_WARN(msg)    VblLogger::log(WARN,  __FILE__, __LINE__, __FUNC__, msg)
#define VBL_LOG_ERROR(msg)   VblLogger::log(ERROR, __FILE__, __LINE__, __FUNC__, msg)
```

### 1.4 时序图

```
任意模块        VblLogger          IBsFileChannel     IBsConsoleChannel
    │               │                    │                    │
    │ VBL_LOG_INFO("project loaded")     │                    │
    ├──────────────►│                    │                    │
    │               │ level >= INFO? → 通过过滤               │
    │               │ formatLog:         │                    │
    │               │ "[INFO][tid=1][2024-01-01 10:00:00]"    │
    │               │ "[ProjectEditor.cpp:70] project loaded" │
    │               │                    │                    │
    │               │ asyncWrite(log)    │                    │
    │               ├───────────────────►│                    │
    │               │                   │ 放入异步队列        │
    │               │                   │ 后台线程写文件       │
    │               │                   │ (不阻塞调用方)       │
    │               │ directWrite(log)   │                    │
    │               ├────────────────────────────────────────►│
    │               │                   │                    │ 彩色输出到控制台
```

### 1.5 日志滚动策略

```
vbl_log.txt         → 当前日志文件（超过 maxFileSize 触发滚动）
vbl_log.1.txt       → 第1备份
vbl_log.2.txt       → 第2备份（最多保留 maxBackupCount 份）
...
```

---

## 二、VblUtils（工具集）

### 接口头
`Interface/BaseService/VblUtils/` 及 `Include/Common/`

### 2.1 线程池（ThreadPool）

```cpp
class IVblThreadPool {
    virtual Result submitTask(IVblRunnable* task) = 0;
    virtual Result submitTask(IVblRunnable* task, VBLInt priority) = 0;
    virtual Result shutdown(VBLBool waitForFinish = true) = 0;
    virtual VBLInt pendingCount() = 0;
    virtual VBLInt runningCount() = 0;
};
```

### 2.2 定时器（IVblTimer）

```cpp
// Include/Common/IVblTimer.h
class IVblTimer {
    virtual Result start(VBLInt intervalMs, VBLBool repeat = true) = 0;
    virtual Result stop() = 0;
    virtual Result setCallback(IVblTimerCallback* cb) = 0;
};
// 典型用途：自动保存定时器（ISyncAutoSaveTimer 继承此接口）
```

### 2.3 文件流（FileStream）

```cpp
class IVblFileStream {
    virtual Result open(VBLConstPChar path, OpenMode mode) = 0;
    virtual Result close() = 0;
    virtual Result read(VBLByte* buf, VBLInt size, VBLInt& bytesRead) = 0;
    virtual Result write(VBLConstPChar data, VBLInt size) = 0;
    virtual Result seek(VBLLonglong pos) = 0;
    virtual VBLLonglong size() = 0;
};
```

### 2.4 字符串列表（IVblString / StringList）

```cpp
class IVblString {
    virtual VBLConstPChar c_str() = 0;
    virtual VBLInt length() = 0;
    virtual Result append(VBLConstPChar) = 0;
    virtual Result replace(VBLConstPChar from, VBLConstPChar to) = 0;
};

// VBLPCharList：字符串数组（C 接口 compatible）
// IVblList<T>：泛型列表基类，支持 add/remove/get/count
```

### 2.5 AES 加解密

```cpp
// 用于工程文件/预设文件的加密存储
class IVblAES {
    virtual Result encrypt(VBLConstPChar key, VBLByte* in, VBLInt inLen, VBLByte* out, VBLInt& outLen) = 0;
    virtual Result decrypt(VBLConstPChar key, VBLByte* in, VBLInt inLen, VBLByte* out, VBLInt& outLen) = 0;
};
```

---

## 三、BsDataStructure（公共数据结构）

### 接口头
`Interface/BaseService/BsDataStructure/`

### 3.1 职责
提供全模块共用的**基础数据容器接口**，所有模块通过 `group_private_link_libs` 隐式链接。

### 3.2 核心接口

```cpp
// 通用列表（所有 IDmXxxList 的基类）
template<typename T>
class IVblList {
    virtual Result add(T* item) = 0;
    virtual Result remove(T* item) = 0;
    virtual Result insert(VBLInt idx, T* item) = 0;
    virtual T* get(VBLInt idx) = 0;
    virtual VBLInt count() = 0;
    virtual Result clear() = 0;
};

// 具体列表类型（由 IDmClipList / IDmTrackList / VBLPCharList 等继承）
IVblList<IDmClip>      → IDmClipList
IVblList<IDmTrack>     → IDmTrackList
IVblList<IDmMediaItem> → IDmMediaItemList
IVblList<VBLPChar>     → VBLPCharList
IVblList<VBLInt>       → VBLIntList
IVblList<VBLLonglong>  → VBLLonglongList

// 字符串接口
class IVblString {
    virtual VBLConstPChar c_str() = 0;
    virtual VBLInt length() = 0;
};

// 参数接口（Key-Value 容器）
class IVblParam {
    virtual Result setProperty(VBLConstPChar key, const Property& val) = 0;
    virtual Result getProperty(VBLConstPChar key, Property& val) = 0;
};
```

### 3.3 Property 类型

`Property` 是 VBL 的通用属性值类型，采用 union 或 variant 存储多种基本类型：

| 类型 | 说明 |
|---|---|
| `bool` | 布尔值 |
| `int` | 整型 |
| `long long` | 长整型（时间戳 us）|
| `float/double` | 浮点数 |
| `const char*` | 字符串 |
| `RectF / PointF / SizeF` | 几何类型 |
| `Rational` | 分数（帧率）|

---

## 四、BsDiskFolder（磁盘目录工具）

### 接口头
`Interface/BaseService/BsDiskFolder/`

### 4.1 职责
- 工程目录结构的**创建与管理**
- 应用数据目录（AppData/Cache/Temp）的标准化访问
- 目录遍历（列举子目录/文件）
- 文件复制/移动/删除
- 路径合法性校验

### 4.2 核心功能

```cpp
class IBsDiskFolder {
    virtual Result createFolder(VBLConstPChar path) = 0;
    virtual Result deleteFolder(VBLConstPChar path) = 0;
    virtual Result copyFolder(VBLConstPChar src, VBLConstPChar dst) = 0;
    virtual Result listFiles(VBLConstPChar path, VBLPCharList* files) = 0;
    virtual Result listSubFolders(VBLConstPChar path, VBLPCharList* folders) = 0;
    virtual VBLBool exists(VBLConstPChar path) = 0;
    virtual Result getAppDataDir(VBLConstPChar appName, IVblString* path) = 0;
    virtual Result getCacheDir(IVblString* path) = 0;
    virtual Result getTempDir(IVblString* path) = 0;
};
```

### 4.3 时序图：工程目录初始化

```
ProjectEditor      BsDiskFolder       文件系统
    │                   │                │
    │ createNewProject() │               │
    ├──────────────────►│                │
    │ getAppDataDir("Filmora") → baseDir │
    │◄──────────────────┤                │
    │                   │ createFolder(baseDir/projects/newId/)
    │                   ├───────────────►│
    │                   │ createFolder(.../newId/Medias/)
    │                   ├───────────────►│
    │                   │ createFolder(.../newId/Project/)
    │                   ├───────────────►│
    │ 目录创建完毕       │                │
    │◄──────────────────┤                │
    │ 开始写 project.xml │                │
```

---

## 五、基础设施依赖关系总结

```
所有模块（通过 group_private_link_libs 全局注入）
  ├── VblLogger       日志
  ├── VblUtils        工具集（ThreadPool/Timer/FileStream）
  ├── BsDataStructure 数据容器（IVblList/IVblString/Property）
  └── BsDiskFolder    磁盘目录操作
```

这四个模块构成 VBL 的**基础设施层**，无需在各模块 CMakeLists.txt 中显式声明，通过 `modules/CMakeLists.txt` 中的 `group_private_link_libs` 自动注入到所有子模块。
