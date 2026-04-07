# 15 · DataModel 分层设计 — Timeline / Track / Clip 体系

> **面试价值**：⭐⭐⭐⭐⭐  **优先级**：P0
> **相关文件**：
> - `modules/DataModel/Timeline/DmTimeline.h/.cpp`
> - `modules/DataModel/Timeline/DmTrack.h`
> - `modules/DataModel/Timeline/Clip/DmBaseClip.hpp/.cpp`
> - `modules/DataModel/Timeline/Clip/DmVideoClip.hpp`, `DmAudioClip.hpp`, `DmTextClip.hpp`, `DmCompositeClip.cpp`
> - `modules/DataModel/Timeline/Effect/DmEffect.hpp/.cpp`
> - `Interface/DataModel/IDmClipFactory.h`
> - `tests/modules/DataModel/DmClip_test.cpp`, `DmTimeline_test.cpp`

---

## 1. 概念介绍

DataModel 是 VBL 的核心数据层，负责维护视频编辑的所有数据结构。其设计参考了非线性编辑（NLE）领域的经典三层模型：

```
IDmTimeline（时间线）
    └── IDmTrack（轨道）× N
            └── IDmClip（片段）× M
                    ├── IDmEffect（效果）× K
                    └── Keyframe（关键帧）× P
```

**设计原则**：
- 面向接口编程（IDm* 前缀的纯虚接口）
- 对象通过工厂（IDmClipFactory）创建，禁止直接 new
- 生命周期由 RefCnt + SafePtr 管理
- 数据变更通知通过 Delegate 回调和 EventBus 两条路径传播

---

## 2. 项目实现

### 2.1 三层数据结构

**DmTimeline（时间线）**
- 文件：`modules/DataModel/Timeline/DmTimeline.h`
- 类：`VBL::DmTimeline : virtual public IDmTimeline, virtual public IDmTimelineBrandFeater, public DmBaseMedia, public DmDescription, virtual public IDmTimelineInternal`
- 内部存储：`struct _Impl { std::vector<TrackInfo> tracks; ... }` 通过 PIMPL 隐藏

```cpp
// modules/DataModel/Timeline/DmTimeline.h（节选）
class DmTimeline : virtual public IDmTimeline, virtual public IDmTimelineBrandFeater,
                   virtual public DmBaseMedia, public DmDescription,
                   virtual public IDmTimelineInternal {
public:
    VBLInt  trackCount() override;
    Result  addTrack(VBLUInt type, VBLInt idx) override;
    Result  moveTrack(VBLInt from_idx, VBLInt to_idx) override;
    Result  removeTrack(VBLInt idx) override;
    IDmTrack* track(VBLInt idx) override;

    VBLInt  clipCount(VBLInt track_idx) override;
    Result  addClip(VBLInt track_idx, IDmClip* pClip) override;
    Result  removeClip(IDmClip* pClip) override;
    IDmClip* clip(VBLInt track_idx, VBLInt clip_idx) override;
};
```

**DmTrack（轨道）**
- 文件：`modules/DataModel/Timeline/DmTrack.h`
- 类：`VBL::DmTrack : public IDmTrack, public IDmTrackInternal, public DmDescription, public RefCnt<DmTrack>`

```cpp
// modules/DataModel/Timeline/DmTrack.h（节选）
class DmTrack : public IDmTrack, public IDmTrackInternal,
                public DmDescription, public RefCnt<DmTrack> {
public:
    TrackType  type() override;
    VBLInt     index() override;
    VBLConstPChar alias() override;
    VBLBool    clipCanAddToTrack(ClipType clipType) override;
};
```

---

### 2.2 Clip 类型继承体系

所有 Clip 子类均位于 `modules/DataModel/Timeline/Clip/` 目录：

```
IDmClip（纯虚接口）
    └── DmBaseClip（公共实现基类）
            ├── DmVideoClip         ← 视频片段（含文件路径/MediaInfo）
            ├── DmAudioClip         ← 音频片段
            ├── DmTextClip          ← 文字片段（字幕/花字）
            ├── DmCompositeClip     ← 合成片段（内嵌子 Timeline）
            ├── DmSplitScreenClip   ← 分屏片段
            ├── DmSubtitleClip      ← 字幕片段
            ├── DmLayerClip         ← 图层片段
            ├── DmBorderClip        ← 边框片段
            ├── DmBufferClip        ← 缓冲片段
            ├── DmRecordClip        ← 录制片段
            └── DmMultiCameraVideoAudioClip ← 多机位片段
```

DmBaseClip 提供所有 Clip 子类共享的能力：
- Effect 管理（appendEffect/removeEffect/moveEffect/getAdditionalEffect）
- Keyframe 相关（通过 IDmEffect 间接管理）
- Clone/深拷贝（IDmClip* clone() override）
- 属性读写（setClipProperty/getClipProperty）
- 与 Adapter 层绑定（getClipAdapter()）

```cpp
// modules/DataModel/Timeline/Clip/DmBaseClip.hpp（Effect 方法节选）
class DmBaseClip : public IDmClip, public IDmClipInternal, public DmDescription {
public:
    // Effect 附加管理
    Result appendEffect(IDmEffect* pClip) override;
    Result removeEffect(IDmEffect* pClip) override;
    Result moveEffect(VBLInt from_idx, VBLInt to_idx) override;
    Result replaceAddlEffect(IDmEffect* pSrcIEffect, IDmEffect* pDesIEffect) override;
    VBLInt additionalEffectCount() override;
    IDmEffect* getAdditionalEffect(VBLInt idx) override;

    // Clone / 深拷贝
    IDmClip* clone() override;

    // 工具
    Result copyData(DmBaseClip* pClone);
    Result copyDataEx(DmBaseClip* pClone, VAL::IClipAdapter* pCloneAdapter);
};
```

---

### 2.3 Effect 与 Keyframe

**Effect 附加到 Clip**
- Effect 分为"默认效果"（DefaultEffect，如基础变换/颜色/音频参数）和"附加效果"（AdditionalEffect，如特效/滤镜/贴纸）
- 通过 `appendEffect(IDmEffect*)` 附加，`removeEffect` 移除

```cpp
// tests/modules/DataModel/DmClip_test.cpp（真实测试用例）
TEST(IDmClip, appendEffect)
{
    removeAllAdditionEffect();
    EXPECT_TRUE(pDmVideoClip->additionalEffectCount() == 0);
    EXPECT_TRUE(pDmVideoClip->appendEffect(pWaterEffect) == rOk);
    EXPECT_TRUE(pDmVideoClip->additionalEffectCount() == 1);
}
```

**Keyframe（关键帧）**
- 关键帧与 Effect 参数绑定，通过 IDmEffect 的 keyframe 接口管理

```cpp
// modules/DataModel/Timeline/Effect/DmEffect.hpp（Keyframe 接口节选）
class DmEffect : public IDmEffect, ... {
public:
    VBLInt  keyFrameCount(VBLInt idx_param) override;
    VBLInt  addKeyFrame(VBLInt idx_param, VBLLonglong keyframe_time,
                        const Property& value) override;
    Result  removeKeyFrame(VBLInt idx_param, VBLInt idx_keyframe) override;
    Result  keyFrameParam(VBLInt idx_param, VBLInt idx_keyframe,
                         Property& value) override;
    Result  changeKeyFrameParam(VBLInt idx_param, VBLInt idx_keyframe,
                                const Property& value) override;
    Result  keyFrameTime(VBLInt idx_param, VBLInt idx_keyframe,
                        VBLLonglong& value) override;
    IDmEffectKeyframeUserData* keyframeUserData() override;
};
```

---

### 2.4 Clone / 深拷贝实现

每个 Clip 子类重写 `clone()`，其中 DmCompositeClip 需要深拷贝内嵌 Timeline：

```cpp
// modules/DataModel/Timeline/Clip/DmCompositeClip.cpp
VBL::IDmClip* DmCompositeClip::clone()
{
    auto pDmClone = new DmCompositeClip(type());
    VBL_SAFE_ADDREF(pDmClone)   // 引用计数置 1

    copyData(pDmClone);         // 拷贝基类数据（属性/效果列表等）

    // CompositeClip 特有：深拷贝内嵌子 Timeline
    if (!isTimelineSharedClip() &&
        getClipAdapter()->typeFromTlbType() == ctVideoAudioTimeline)
    {
        auto pTimeline = timelinePointer();
        SafePtr<IDmTimeline> pCloneTm;
        pCloneTm.Attach(pTimeline->clone());       // 递归 clone 子 Timeline
        pDmClone->setTimelinePointer(pCloneTm.get());
    }
    return pDmClone;
}
```

`copyData()` 负责拷贝基类公共数据（属性 map、Effect 列表等），由基类统一实现，各子类调用后再处理自身特有数据。

---

### 2.5 IDmClipFactory — 工厂接口

所有 DataModel 对象通过工厂创建，禁止直接 new：

```cpp
// Interface/DataModel/IDmClipFactory.h（接口节选）
class IDmClipFactory {
public:
    virtual IDmTimeline* createTimeline() = 0;
    virtual IDmVideoClip* createVideoClip(ClipType type, VBLConstPChar filePath,
                                          ParentClipType pType = depBaseTimeline) = 0;
    virtual IDmAudioClip* createAudioClip(VBLConstPChar filePath) = 0;
    virtual IDmTextClip*  createTextClip(VBLConstPChar text,
                                         VBLConstPChar szAspect = nullptr,
                                         VBLBool bHideChar = 0) = 0;
    virtual IDmEffect*    createEffect(VBLConstPChar configPath) = 0;
    virtual IDmMediaInfo* createMediaInfo(VBLConstPChar filePath,
                                          VBLConstPChar fileMd5 = "") = 0;
    virtual IDmAnimation* createAnimation(VBLConstPChar configPath) = 0;
};
```

全局工厂获取：`getDmFactoryInstance()` 返回 `IDmClipFactory*`（在 `VblDataModelLib.h` 声明）

---

### 2.6 DmTimeline::addClip 实现流程

`addClip` 是 DataModel 层最复杂的操作之一，需要同步 DataModel 层和 Adapter 层：

```cpp
// modules/DataModel/Timeline/DmTimeline.cpp（精简版流程）
Result DmTimeline::addClip(VBLInt track_idx, IDmClip* pClip)
{
    // 1. 前置检查：adapter 是否就绪，track 索引是否合法
    VBL_CHECK_FAILED_RETURN(checkTimelineAdapter(), rFailed)

    // 2. 获取 clip 的底层 adapter
    auto clipAdapter = baseClip->getClipAdapter();
    VBLInt adapterIdx = toAdapterTrackIndex(track_idx);

    // 3. 同步到底层引擎（Adapter 层）
    auto result = timelineAdapter()->addClip(adapterIdx, clipAdapter);
    if (result != rOk) {
        // 回滚：若 video/audio 联动，需回滚 audio adapter
        return rFailed;
    }

    // 4. 在 DataModel 本地维护 clip 引用
    if (insertClip(baseClip.get(), track_idx) != rOk) {
        return rFailed;
    }

    // 5. 通知 Delegate 和资源管理器
    if (m_delegate != nullptr && !m_disablCallback) {
        m_delegate->clipAdded(pClip, track_idx);
    }
    if (getTimelineResInfos()) {
        m_timelineResInfos->addClip(pClip);
    }
    return rOk;
}
```

---

## 3. 关键代码片段

### 片段1：测试用例展示 Timeline 基本操作

```cpp
// tests/modules/DataModel/DmTimeline_test.cpp
TEST(IDmTimeline, addTrack)
{
    pTimeline->addTrack(ttVideo, 0);
    pTimeline->addTrack(ttVideoAudio, 1);
    pTimeline->addTrack(ttVideo, 2);
    EXPECT_TRUE(pTimeline->trackCount() == 3);

    // 工厂创建 TextClip
    auto pTextClip = pDmFactory->createTextClip("hello");
    // 设置时间轴属性
    pTextClip->setClipProperty(clipKey::timelineEnd,   end);
    pTextClip->setClipProperty(clipKey::timelineBegin, begin);
    pTextClip->setClipProperty(clipKey::markOut, end);
    pTextClip->setClipProperty(clipKey::markIn,  begin);
    // 添加并验证
    EXPECT_TRUE(pTimeline->addClip(2, pTextClip) == rOk);
    EXPECT_TRUE(pTimeline->clip(2, 0) == pTextClip);
    pTextClip->Release();
}
```

### 片段2：Effect 操作测试

```cpp
// tests/modules/DataModel/DmClip_test.cpp
TEST(IDmClip, moveEffect) {
    removeAllAdditionEffect();
    pDmVideoClip->appendEffect(pWaterEffect);
    pDmVideoClip->appendEffect(pMosaicEffect);
    EXPECT_TRUE(pDmVideoClip->getAdditionalEffect(0) == pWaterEffect);

    pDmVideoClip->moveEffect(0, 1);  // water 移到索引1
    EXPECT_TRUE(pDmVideoClip->getAdditionalEffect(0) == pMosaicEffect);
    EXPECT_TRUE(pDmVideoClip->getAdditionalEffect(1) == pWaterEffect);
}
```

---

## 4. 面试要点

1. **三层解耦，各司其职**：Timeline 管理 Track 集合，Track 持有 Clip 集合，Clip 持有 Effect 集合。每层只通过接口（IDmTimeline/IDmTrack/IDmClip）向外暴露，上层不感知实现细节。

2. **addClip 需要同时维护 DataModel 和 Adapter 两套数据**：DataModel 层维护 `_Impl->tracks[i].clips`（SafePtr 数组），Adapter 层通过 `timelineAdapter()->addClip(adapterIdx, clipAdapter)` 同步到底层引擎（WES/NLE）。两者必须保持一致，失败时需回滚。

3. **Clone 使用递归深拷贝策略**：`DmCompositeClip::clone()` 先调用基类 `copyData()` 拷贝公共数据，再对内嵌子 Timeline 执行 `pTimeline->clone()` 递归深拷贝，确保每个 CompositeClip 持有独立的数据。

4. **Effect 分两类：DefaultEffect 和 AdditionalEffect**：DefaultEffect 是每个 Clip 固有的效果（基础变换/颜色），随 Clip 类型确定；AdditionalEffect 是用户叠加的特效滤镜，支持多个，可 appendEffect/removeEffect/moveEffect。

5. **全面向接口编程 + 工厂创建**：所有对象通过 `IDmClipFactory::createXxx()` 创建，业务代码只依赖 `IDmClip*`/`IDmEffect*` 等接口，不持有具体实现类型，便于 mock 测试和未来替换实现。

---

## 5. 可能被追问的问题

**Q1：为什么 addClip 需要同时维护 DataModel 和 Adapter 两套数据，而不是只维护一套？**
> DataModel 层是 VBL 的数据模型，负责业务逻辑（历史记录、资源管理、事件通知等）；Adapter 层是引擎适配层，负责将数据同步到底层 WES/NLE 引擎实际渲染。两套数据必须同步：DataModel 存业务数据（属性/效果参数），Adapter 存渲染数据（引擎原生对象）。分离的好处是：可以换底层引擎（从 NLE 切换到 WES），DataModel 层不用改；也可以单独序列化 DataModel，不依赖引擎状态。

**Q2：CompositeClip 内嵌子 Timeline，有没有循环引用风险？**
> DmCompositeClip 持有 SafePtr<IDmTimeline>（强引用），而子 Timeline 通过 Delegate 回调父 Clip，Delegate 持有的是弱引用或裸指针（不增加引用计数）。因此不构成 SafePtr 循环引用。

**Q3：Clip 子类很多，如何保证 clone 语义正确？**
> 通过基类 copyData() 统一拷贝公共数据，子类 clone() 先调 copyData()，再追加子类特有数据的拷贝。CompositeClip 需要特别处理内嵌 Timeline 的深拷贝。这是模板方法模式的应用：基类定义 clone 的骨架，子类在 copyData 后添加扩展。

**Q4：Effect 支持多少层叠加？有性能限制吗？**
> AdditionalEffect 是 vector/list，理论上无限叠加。但受限于：① 底层引擎渲染性能（每个 Effect 对应一次 GPU pass）；② 内存（每个 Effect 有自己的参数 map 和关键帧数据）。实际上通过 DmEffect.hpp 中的 isAddlEffectsEnabled() 可以禁用效果（不删除，只跳过渲染），提供性能逃生门。

**Q5：IDmClipFactory 是如何被注入的？是否支持多个 DataModel 实例？**
> 全局入口 `getDmFactoryInstance()` 返回的工厂是每个 DataModel 库实例全局唯一的（通过静态指针管理）。如果需要多个 DataModel 实例（如预览用 timeline 和编辑用 timeline），它们共享同一个工厂实例，各自创建独立的 IDmTimeline 对象。工厂本身是无状态的（只负责 new 对象），不存在多实例冲突。
