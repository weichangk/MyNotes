# 17 · Adapter 适配层设计 — 多引擎封装与抽象工厂

> **面试价值**：⭐⭐⭐⭐⭐  **优先级**：P0
> **相关文件**：
> - `Interface/Adapter/VblAdapterLib.h`（initAdapter/getAdapterFactory 接口声明）
> - `Interface/Adapter/VblAdapterFactory.h`（IAdapterFactory 接口）
> - `modules/Adapter/wes/VblAdapterLib.cpp`（WES 实现入口）
> - `modules/Adapter/wes/DmWesAdapterFactory.h/.cpp`（WES 工厂实现）
> - `modules/Adapter/wes/MediaInfo/DmWesMediaInfoAdapter.hpp/.cpp`
> - `modules/DataModel/DmCommons.h`（g_adapterFactory 全局变量）
> - `CMakeLists.txt`（VBL_IMPLEMENT_LEVEL 选项）

---

## 1. 概念介绍

Adapter（适配层）模式将一个类的接口转换成客户希望的另一个接口，使原本由于接口不兼容而不能在一起工作的类可以协同工作。

VBL 的 Adapter 层（VAL，Video Adapter Layer）解决的核心问题：
- Filmora 底层有多个渲染引擎（WES、NLE），接口各不相同
- DataModel 层需要一套**统一的接口**操作底层引擎
- 未来可能新增引擎，不能让 DataModel 层依赖具体引擎

**设计模式**：抽象工厂（Abstract Factory）+ Adapter

```
DataModel 层
    ↓ 只依赖接口
IAdapterFactory（接口）
    ├── DmWesAdapterFactory（WES 实现）
    └── DmNleAdapterFactory（NLE 实现）

ITimelineAdapter、IVideoClipAdapter、IEffectAdapter...（产品族接口）
    ├── DmWesTimelineAdapter（WES 实现）
    └── DmNleTimelineAdapter（NLE 实现）
```

---

## 2. 引擎切换机制（编译期多态）

通过 CMake 选项 `VBL_IMPLEMENT_LEVEL` 在**编译期**切换引擎：

```cmake
# CMakeLists.txt（顶层）
set(VBL_IMPLEMENT_LEVEL wes CACHE STRING "底层实现引擎（wes/nle）")
set_property(CACHE VBL_IMPLEMENT_LEVEL PROPERTY STRINGS wes nle)

# modules/Adapter/CMakeLists.txt
message(STATUS "VBL_IMPLEMENT_LEVEL is ${VBL_IMPLEMENT_LEVEL}")
add_subdirectory(${VBL_IMPLEMENT_LEVEL})  # 编译 wes 或 nle 子目录
```

**切换方法**：
```bash
cmake -DVBL_IMPLEMENT_LEVEL=nle -S . -B build   # 切换为 NLE 引擎
cmake -DVBL_IMPLEMENT_LEVEL=wes -S . -B build   # 默认 WES 引擎
```

DataModel 层通过 `IAdapterFactory` 接口调用，**不知道也不关心**底层是 WES 还是 NLE。

---

## 3. 工厂初始化流程

### 3.1 initAdapter — 底层引擎初始化

```cpp
// modules/Adapter/wes/VblAdapterLib.cpp
VALLIB_API void initAdapter(VBLConstPChar mediaLibraryPath)
{
    VBL_LOG_INFO() << "VAL::initAdapter begin";
    initWesLibrary(mediaLibraryPath);   // 加载 WES 动态库，初始化全局资源
    VBL_LOG_INFO() << "VAL::initAdapter end";
}
```

### 3.2 getAdapterFactory — 懒初始化单例工厂

```cpp
// modules/Adapter/wes/VblAdapterLib.cpp
VALLIB_API IAdapterFactory* getAdapterFactory(VBLConstPChar szPath)
{
    static DmWesAdapterFactory* fct = nullptr;
    if (fct == nullptr && szPath) {
        fct = new DmWesAdapterFactory(szPath);
        fct->AddRef();  // 引用计数 +1，由调用方在关闭时 Release()
    }
    return fct;
}
```

**设计亮点**：
- 静态局部指针实现懒初始化单例
- 返回 `IAdapterFactory*`（接口指针），DataModel 层不需要知道是 `DmWesAdapterFactory`
- 全局工厂存储在 `modules/DataModel/DmCommons.cpp` 的 `g_adapterFactory` 变量中

---

## 4. IAdapterFactory — 抽象工厂接口

`IAdapterFactory` 是典型的**抽象工厂**模式，定义了整个适配层的"产品族"：

```cpp
// Interface/Adapter/VblAdapterFactory.h（节选）
class IAdapterFactory : public VBL::IDmBaseObj {
public:
    // 时间线
    virtual ITimelineAdapter* createTimelineAdapter() = 0;

    // 媒体信息
    virtual IMediaInfoAdapter* createMediaInfoAdapter(
        VBLConstPChar filePath, VBLConstPChar fileMd5 = "") = 0;

    // 各类 Clip 适配器
    virtual IVideoClipAdapter*  createVideoClipAdapter(
        ClipType type, VBLConstPChar filePath,
        VBLConstPChar md5, ParentClipType pType) = 0;
    virtual IAudioClipAdapter*  createAudioClipAdapter(VBLConstPChar filePath) = 0;
    virtual ITextClipAdapter*   createTextClipAdapter(...) = 0;
    virtual IEffectAdapter*     createEffectAdapter(VBLConstPChar configPath) = 0;
    virtual ILayerClipAdapter*  createLayerClipAdapter(...) = 0;
    virtual IRecordClipAdapter* createRecordClipAdapter(...) = 0;

    // 效果资源路径注册
    virtual Result addEffectResPath(VBLConstPChar path) = 0;
    // 为 Clip 自动附加配置的默认效果
    virtual Result addConfigEnabledDefaultEffects(IClipAdapter* pClip) = 0;
};
```

---

## 5. DmWesAdapterFactory — WES 具体实现

```cpp
// modules/Adapter/wes/DmWesAdapterFactory.h
class DmWesAdapterFactory
    : public VBL::RefCnt<void>    // 引用计数
    , public IAdapterFactory      // 抽象工厂接口
{
public:
    explicit DmWesAdapterFactory(VBLConstPChar szResPath);
    ~DmWesAdapterFactory() override;

    IMediaInfoAdapter* createMediaInfoAdapter(
        VBLConstPChar filePath, VBLConstPChar fileMd5 = "") override;
    IVideoClipAdapter* createVideoClipAdapter(...) override;
    // ... 其余接口实现
};
```

### 5.1 createMediaInfoAdapter — 对象池复用

```cpp
// modules/Adapter/wes/DmWesAdapterFactory.cpp
IMediaInfoAdapter* DmWesAdapterFactory::createMediaInfoAdapterInternal(
    VBLConstPChar filePath, VBLConstPChar fileMd5, const std::string_view& prefix)
{
    std::string wesFilePath;
    // 步骤1：本地路径 → WES 路径（加前缀，处理协议差异）
    if (localFileToWesFilePath(filePath, wesFilePath, prefix) == rFailed) { ... }

    // 步骤2：从对象池取出（基于文件 md5 缓存，避免重复解析）
    DmWesMediaInfoAdapter* holder =
        MediaInfoAdapterObjectPool::getInstance().create(wesFilePath.c_str(), fileMd5);
    holder->AddRef();
    return holder;
}
```

### 5.2 createVideoClipAdapter — 封装底层引擎对象

```cpp
// modules/Adapter/wes/DmWesAdapterFactory.cpp（精简）
IVideoClipAdapter* DmWesAdapterFactory::createVideoClipAdapterInternal(...)
{
    // 步骤1：本地路径 → WES 路径
    localFileToWesFilePath(filePath, wesFilePath, prefix);

    // 步骤2：通过底层 TLB 工厂创建引擎原生 Clip 对象
    auto pClipFactory = clipFactory();
    auto clip = pClipFactory->CreateVideoClip(wesFilePath.c_str());

    // 步骤3：设置 Clip 的默认效果类型（根据 ParentClipType）
    DmWesBaseClipAdapter::SetDefEffectType(clip, pType, ...);

    // 步骤4：用 Adapter 包装引擎对象（桥接 WES 接口 → VBL 接口）
    auto ptr = new DmWesVideoClipAdapter;
    ptr->init(clip, md5);
    ptr->AddRef();

    // 步骤5：自动附加配置文件中启用的默认效果
    addConfigEnabledDefaultEffects(ptr);
    return ptr;
}
```

---

## 6. MediaInfo 适配器 — 带缓存的解析

### 6.1 对象池设计

```cpp
// modules/Adapter/wes/MediaInfo/DmWesMediaInfoAdapter.hpp
class MediaInfoAdapterObjectPool {
public:
    static MediaInfoAdapterObjectPool& getInstance() {
        static MediaInfoAdapterObjectPool instance;
        return instance;
    }

    // 创建或从池中取出（基于文件 md5 缓存）
    DmWesMediaInfoAdapter* create(const char* wesPath, const char* md5);
    // 清理池
    void clearPoolObj();

private:
    std::map<std::string, ValueInfo> m_pool;
    std::shared_mutex m_pMtx;  // 读写锁，支持并发读取
};
```

### 6.2 序列化/反序列化（rapidjson）

MediaInfo 解析耗时（需要读取文件头），结果序列化为 JSON 缓存到磁盘：

```cpp
// modules/Adapter/wes/MediaInfo/DmWesMediaInfoAdapter.cpp（序列化片段）
rapidjson::Document doc;
rapidjson::SetValueByPointer(doc, "/version", gMediaVersion.c_str());
rapidjson::SetValueByPointer(doc, "/file_name", localFilePath.c_str());

// 写入基础信息
rapidjson::SetValueByPointer(doc, "/sourceInfo/basicInfo/streamType",
                              m_basicInfo.stream_type_);

// 写入视频流数组
for (uint32_t i = 0; i < m_VideoInfos.size(); ++i) {
    std::string keyPrefix("/sourceInfo/vidStreamInfos/" + std::to_string(i) + "/");
    rapidjson::SetValueByPointer(doc,
        rapidjson::Pointer((keyPrefix + "vidStreamId").c_str()),
        m_VideoInfos[i].vid_stream_id_);
    // ... 更多字段
}
```

反序列化从缓存恢复：

```cpp
// modules/Adapter/wes/MediaInfo/DmWesMediaInfoAdapter.cpp（反序列化片段）
if (auto basicInfoValue = rapidjson::GetValueByPointer(doc, "/sourceInfo/basicInfo")) {
    VBL_CHECK_FAILED_RETURN(parseBasicInfo(basicInfoValue->GetObject()), rFailed)
}
if (auto vidStreamValue = rapidjson::GetValueByPointer(doc, "/sourceInfo/vidStreamInfos")) {
    VBL_CHECK_FAILED_RETURN(parseVideoStreamInfos(vidStreamValue->GetArray()), rFailed)
}
// 将缓存注入底层引擎（避免引擎再次读文件）
if (sync && !tlb::GetSrcManager()->SetCacheInfo(m_initWesPath.c_str(), m_basicInfo, ...)) {
    // 缓存注入失败，回退到重新解析
}
```

---

## 7. 面试要点

1. **抽象工厂 = 产品族的接口**：IAdapterFactory 定义了 VBL 所有 Adapter 产品（Timeline/Clip/Effect/MediaInfo）的创建接口。DataModel 层通过 `g_adapterFactory->createXxx()` 创建，不感知具体引擎，这是解耦 DataModel 和引擎的核心机制。

2. **编译期多态实现引擎切换**：通过 CMake 的 `VBL_IMPLEMENT_LEVEL` 选择编译 `modules/Adapter/wes/` 还是 `modules/Adapter/nle/`，运行时只有一套实现。切换引擎只需一个 CMake 参数，DataModel 层代码零修改。

3. **createMediaInfoAdapter 用了对象池缓存**：MediaInfo 解析（读取视频文件元数据）是耗时操作，通过 `MediaInfoAdapterObjectPool`（基于 md5 的 map + shared_mutex）缓存已解析的结果，同一文件的第二次 createMediaInfoAdapter 直接从池中返回，避免重复解析。

4. **createVideoClipAdapter 的五步流程**：路径转换 → TLB 工厂创建引擎原生对象 → 设置默认效果类型 → Adapter 包装 → 自动附加配置效果。每一步都通过组合而非继承扩展功能（Adapter + 工厂 + 配置驱动）。

5. **MediaInfo 序列化用 rapidjson**：MediaInfo 解析结果以 rapidjson Document 序列化到 media.json 缓存文件，跨进程/跨次启动可以直接读缓存，进一步加速。rapidjson 的 `SetValueByPointer` / `GetValueByPointer` 提供了 JSON Pointer 语法的高效读写。

---

## 8. 可能被追问的问题

**Q1：为什么用编译期切换（CMake）而不是运行期切换（策略模式/配置文件）？**
> 编译期切换的好处：① 零运行时开销（无虚函数分发开销）；② 二进制只包含一套实现，包体更小；③ 编译期检查，减少运行时错误。代价：不能在同一个 binary 中同时支持两套引擎。若需要在同一 binary 中 AB 测试两套引擎，才需要运行期切换（当前项目不需要）。

**Q2：MediaInfoAdapterObjectPool 的 shared_mutex 如何保证线程安全？**
> `create()` 操作先用 `shared_lock`（读锁）检查 map 中是否存在。若存在，直接返回；若不存在，升级为 `unique_lock`（写锁），再次检查（double-check locking），再插入。这是读多写少场景的标准模式：多个线程可以同时查询已缓存的 MediaInfo，不互相阻塞。

**Q3：DmWesAdapterFactory 的 createVideoClipAdapter 为什么最后调用 addConfigEnabledDefaultEffects？**
> `addConfigEnabledDefaultEffects` 根据配置文件（effect config）为 Clip 自动附加一组"启用的默认效果"（如视频默认附加基础变换效果、颜色效果）。这样可以在不修改代码的情况下通过配置文件控制默认效果，是配置驱动设计的体现。

**Q4：如果底层 WES 引擎接口发生变化（如 API 升级），影响范围有多大？**
> 只影响 `modules/Adapter/wes/` 目录下的 Adapter 实现文件。DataModel 层（IDmTimeline/IDmClip 等）完全不受影响，因为 DataModel 只依赖 `IAdapterFactory`/`ITimelineAdapter` 等接口。这是适配层设计的核心价值：将变化隔离在适配层内部。

**Q5：IAdapterFactory 对象是 Singleton，但 getAdapterFactory 需要传入路径参数，如何保证路径一致？**
> 第一次调用时传入路径创建工厂并缓存（静态局部指针）。后续调用如果 `szPath == nullptr`，返回已创建的工厂；如果传入不同路径，仍返回第一次创建的工厂（忽略后续路径），因此调用方必须保证第一次调用传入正确路径。初始化顺序由 `initDataModel()` 统一管理，确保工厂初始化只发生一次。
