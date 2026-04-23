# DataModel 模块详细设计文档

> 所属层：DataModel
> 接口头：`Interface/DataModel/`（106 个头文件）
> 初始化入口：`Interface/DataModel/VblDataModelLib.h`
> 命名空间：`VBL::`

---

## 1. 模块职责

DataModel 是 VBL 的**全局核心数据层**，负责：
- 定义工程的所有数据结构（工程 / 时间线 / 轨道 / Clip / 媒体项 / 效果 / 转场等）
- 提供 IDmClipFactory 工厂创建所有 Dm 对象
- 管理数据的序列化/反序列化（`project.xml`）
- 对接底层 Adapter（PRIVATE 依赖）获取媒体信息、渲染能力
- 通过 ListenerCenter 广播数据变更事件（PUBLIC 依赖）

---

## 2. 初始化接口（VblDataModelLib.h）

```cpp
// C 导出全局函数
extern "C" {
    VBLLIB_API Result initDataModel();
    VBLLIB_API Result uninitDataModel();
    VBLLIB_API IDmClipFactory* getDmFactoryInstance();

    VBLLIB_API Result initDecodeManager(VBLConstPChar pluginPath);
    VBLLIB_API Result addEffectResPath(VBLConstPChar path);
    VBLLIB_API Result setPerformanceMode(PerformanceMode mode);
    VBLLIB_API Result setRenderPlatform(RenderPlatform platform);
}
```

---

## 3. 核心数据结构体系

### 3.1 工程层（IDmProject）

```
IDmProject
  ├── IDmTimeline*         主时间线
  ├── IDmMediaFolder*      媒体区根目录
  │     ├── IDmMediaFolder（子文件夹）
  │     └── IDmMediaItem*（各类媒体项）
  ├── IDmFontLibrary*      字体库
  ├── TimelineConfig       工程配置（分辨率/帧率/背景色）
  └── serialize()/deserialize()  工程序列化
```

### 3.2 时间线层（IDmTimeline）

```
IDmTimeline
  ├── addTrack(type, idx)
  ├── removeTrack(idx)
  ├── getTrack(idx) → IDmTrack*
  ├── trackCount()
  ├── duration()              时间线总时长
  ├── currentPosition()       当前播放位置
  ├── beginEdit() / endEdit() 批量编辑事务
  ├── serialize()/deserialize()
  └── getFrame(pos, width, height, buffer)  帧提取
```

### 3.3 轨道层（IDmTrack）

```
IDmTrack
  ├── type()   TrackType（ttVideoAudio / ttAudio / ttText / ttEffect…）
  ├── addClip(pos, clip)
  ├── removeClip(clip)
  ├── getClip(idx) → IDmClip*
  ├── clipCount()
  └── visible / locked / height（UI 显示属性）
```

### 3.4 Clip 类型体系

```
IDmClip（基类）
  ├── type()          ClipType
  ├── duration()      时长
  ├── alias()         别名
  ├── isComposite()   是否复合 clip
  ├── effect()        挂载滤镜
  ├── transition()    挂载转场
  ├── variableSpeed() 变速参数
  ├── markersManager()标记点管理
  │
  ├── IDmVideoClip         视频片段
  ├── IDmAudioClip         音频片段
  ├── IDmVideoAudioClip    音视频片段（主轨）
  ├── IDmTextClip          文字/字幕片段
  ├── IDmLayerClip         特效/贴纸/画中画片段
  ├── IDmBufferClip        缓冲(预渲染)片段
  ├── IDmCompositeClip     复合片段（多 clip 合并）
  └── IDmVideoAudioTimelineClip  嵌套时间线片段
       └── IDmMultiCameraVideoAudioClip  多机位片段
```

### 3.5 媒体项类型体系

```
IDmBaseMedia（基类）
  ├── IDmMediaItem（带 UI 选中状态）
  │    ├── IDmSourceMedia      用户导入媒体（本地文件）
  │    ├── IDmResourceMedia    内置资源（特效包/模板）
  │    ├── IDmElementMedia     画中画/贴纸资源
  │    ├── IDmMusicMedia       内置音乐
  │    ├── IDmTemplateMedia    模板资源
  │    ├── IDmVideoScenceMedia 场景检测片段
  │    ├── IDmBannerResourceMedia 横幅资源
  │    └── IDmCloudDiskResourceItem 云盘文件
  └── 其他内部媒体类型
```

### 3.6 效果与转场（IDmEffect / IDmTransition）

```
IDmEffect
  ├── effectId()          特效 ID（对应资源包）
  ├── paramCount()        参数个数
  ├── getParam(idx)       获取参数值
  ├── setParam(idx, val)  设置参数值
  └── enabled()

IDmTransition
  ├── transitionId()
  ├── duration()          转场时长
  ├── type()              TransitionType（头部/尾部/中间）
  └── getParam() / setParam()
```

### 3.7 变速（IDmVariableSpeed）

```
IDmVariableSpeed
  ├── segmentCount()              片段数（冻帧会增加片段）
  ├── getSegment(idx)             获取速度段
  │    ├── speed()               速率（1.0=正常）
  │    ├── duration()            段时长
  │    └── isFreezeFrame()       是否冻帧段
  ├── addCurvePoint(pos, speed)   添加曲线控制点
  ├── removeCurvePoint(idx)
  └── type()   VariableSpeedType（均匀/曲线）
```

---

## 4. IDmClipFactory 工厂接口

```cpp
class IDmClipFactory {
    // 时间线/工程创建
    virtual IDmTimeline*  createTimeline(const TimelineConfig&) = 0;
    virtual IDmProject*   createProject() = 0;

    // Clip 创建
    virtual IDmVideoClip*        createVideoClip(IDmSourceMedia*, ...) = 0;
    virtual IDmAudioClip*        createAudioClip(IDmSourceMedia*, ...) = 0;
    virtual IDmVideoAudioClip*   createVideoAudioClip(IDmSourceMedia*, ...) = 0;
    virtual IDmTextClip*         createTextClip(...) = 0;
    virtual IDmLayerClip*        createLayerClip(IDmResourceMedia*, ...) = 0;
    virtual IDmTransition*       createTransition(IDmResourceMedia*) = 0;
    virtual IDmEffect*           createEffect(IDmResourceMedia*) = 0;

    // 媒体项创建
    virtual IDmSourceMedia*    createSourceMedia(VBLConstPChar filePath) = 0;
    virtual IDmResourceMedia*  createResourceMedia(VBLConstPChar resId) = 0;
    virtual IDmMediaFolder*    createMediaFolder(VBLConstPChar alias) = 0;
};
```

---

## 5. 依赖关系

```
DataModel
  ├── PUBLIC  → ListenerCenter   （事件广播，对上层可见）
  ├── PUBLIC  → BsCommonSetting  （公共配置路径）
  ├── PUBLIC  → BsCloudResource  （内置云资源元数据）
  ├── PUBLIC  → BsCloudConfig    （云端配置）
  ├── PRIVATE → Adapter          （媒体解码/帧提取/渲染）
  ├── PRIVATE → BsNet            （在线资源请求）
  ├── PRIVATE → BsGpuCheck       （GPU 能力检测）
  ├── PRIVATE → BsPreset         （预设文件管理）
  ├── PRIVATE → BsPluginManager  （OpenFX 插件）
  ├── PRIVATE → BsCloudDisk      （云盘文件引用）
  └── PRIVATE → TBB              （多线程资源管理，非 Web/Mobile）
```

---

## 6. 时序图

### 6.1 DataModel 初始化

```
UI / BusinessLayer         DataModel(initDataModel)     Adapter
        │                          │                       │
        │ initAdapter(libPath)      │                       │
        ├──────────────────────────────────────────────────►│
        │                          │   底层引擎初始化        │
        │ initDataModel()          │                       │
        ├─────────────────────────►│                       │
        │                          │ initDecodeManager(pluginPath)
        │                          ├──────────────────────►│
        │                          │ 注册解码插件           │
        │ addEffectResPath(path)   │                       │
        ├─────────────────────────►│                       │
        │                          │ 注册特效资源路径       │
        │ setPerformanceMode(mode) │                       │
        ├─────────────────────────►│                       │
        │ getDmFactoryInstance()   │                       │
        ├─────────────────────────►│                       │
        │  IDmClipFactory*         │                       │
        │◄─────────────────────────┤                       │
```

### 6.2 工程序列化（保存）

```
ProjectEditor       IDmProject       IDmTimeline    IDmClip(×N)   文件系统
     │                   │                │               │             │
     │ project->serialize(path)           │               │             │
     ├──────────────────►│                │               │             │
     │                   │ 生成 XML 根节点 │               │             │
     │                   │ timeline->serialize()          │             │
     │                   ├───────────────►│               │             │
     │                   │               │ 遍历 tracks    │             │
     │                   │               │ clip->serialize()            │
     │                   │               ├──────────────► │             │
     │                   │               │ <clip type="video" .../>     │
     │                   │               │◄──────────────┤             │
     │                   │               │ effect->serialize()          │
     │                   │               ├──────────────► │             │
     │                   │  写入 project.xml              │             │
     │                   ├────────────────────────────────────────────── ►│
     │                   │  写入 tracks_info.json         │             │
     │                   ├────────────────────────────────────────────── ►│
     │ 保存完成          │                │               │             │
     │◄──────────────────┤                │               │             │
```

### 6.3 工程反序列化（加载）

```
ProjectEditor       IDmProject       IDmTimeline    IDmClipFactory   Adapter
     │                   │                │                │              │
     │ project->deserialize(path)         │                │              │
     ├──────────────────►│                │                │              │
     │                   │ 解析 project.xml                │              │
     │                   │ factory->createTimeline(config) │              │
     │                   ├───────────────────────────────── ►│            │
     │                   │  IDmTimeline*  │                │              │
     │                   │◄───────────────────────────────┤              │
     │                   │ 遍历 <track>   │                │              │
     │                   │ timeline->addTrack(type, idx)   │              │
     │                   ├───────────────►│               │              │
     │                   │ 遍历 <clip>    │               │              │
     │                   │ factory->createXxxClip(attrs)  │              │
     │                   ├───────────────────────────────── ►│            │
     │                   │ clip->deserialize(xml_node)     │              │
     │                   │ clip->resolveMediaRef(mediaId)  │              │
     │                   │ Adapter::getMediaInfo(mediaPath)│              │
     │                   ├─────────────────────────────────────────────── ►│
     │                   │  MediaInfo（宽高/帧率/时长）    │              │
     │                   │◄──────────────────────────────────────────────┤
     │                   │ timeline->addClip(track, clip)  │              │
     │                   ├───────────────►│               │              │
     │ deserialize 完成  │                │               │              │
     │◄──────────────────┤                │               │              │
```

### 6.4 Clip 数据变更事件广播

```
TimelineEditor    IDmTimeline    IDmClip     ListenerCenter    UI 层
     │                │              │              │              │
     │ beginEdit()    │              │              │              │
     ├───────────────►│              │              │              │
     │ clip->setProperty(key, val)   │              │              │
     ├──────────────────────────────►│              │              │
     │ endEdit()      │              │              │              │
     ├───────────────►│              │              │              │
     │                │ 汇总变更列表  │              │              │
     │                │ EventBus->postEvent("timeline.clip.changed", changeList)
     │                ├──────────────────────────── ►│            │
     │                │              │              │ notify subscribers
     │                │              │              ├─────────────►│
     │                │              │              │  UI 刷新对应 Clip 属性面板
```

---

## 7. 对象生命周期

### 7.1 引用计数机制

所有 IDm* 对象继承 `IDmBaseObj`，实现引用计数：

```cpp
// 创建后引用计数 = 1
IDmClip* clip = factory->createVideoClip(media);

// 交给时间线持有（引用计数 +1 = 2）
timeline->addClip(trackIdx, pos, clip);

// 业务层不再持有（引用计数 -1 = 1，不销毁）
clip->Release();

// 从时间线移除（引用计数 -1 = 0，对象销毁）
timeline->removeClip(clip);
```

### 7.2 智能指针

```cpp
// VblSafePtr 自动 AddRef/Release
VblSafePtr<IDmClip> clipPtr = factory->createVideoClip(media);

// VblWeakPtr 弱引用（不增加引用计数）
VblWeakPtr<IDmTimeline> weakTl = timeline;
if (auto tl = weakTl.lock()) {
    tl->addClip(...);
}
```

---

## 8. 设计要点

| 要点 | 说明 |
|---|---|
| 工厂模式 | 所有 Dm 对象通过 IDmClipFactory 创建，外部不 new，保证内存管理一致性 |
| 编辑事务 | `beginEdit/endEdit` 合并批量变更，避免中间态广播事件导致 UI 多次刷新 |
| PRIVATE 依赖 Adapter | DataModel 封装所有底层调用，上层业务层无需直接访问 Adapter |
| 序列化版本兼容 | project.xml 含版本号，旧版工程加载时做向前兼容处理 |
| TBB 并发 | 多轨道渲染帧提取使用 TBB 并行执行（移动端/Web 降级为串行）|
| ClipKey 属性 | Clip 属性通过 `clipKey::enable/locked/timelineBegin` 等字符串 key 统一存取，便于扩展 |
