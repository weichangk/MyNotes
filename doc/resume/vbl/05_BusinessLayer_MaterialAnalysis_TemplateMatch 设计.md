# MaterialAnalysis / TemplateMatch 模块详细设计文档

> 所属层：BusinessLayer
> 接口头（MaterialAnalysis）：`Interface/BusinessLayer/MaterialAnalysis/IVbMaterialAnalysis.h`
> 接口头（TemplateMatch）：`Interface/BusinessLayer/TemplateMatch/`
> 工厂函数：`createMaterialAnalysis()`
> 命名空间：`VBL::`

---

## 1. 模块职责

### MaterialAnalysis（素材分析）
对用户导入的媒体素材进行**多维度智能分析**，输出分析结果供 TemplateMatch 使用：
- 人脸检测（`AT_FACEDETECT`）：识别视频帧中人脸位置
- 场景检测（`AT_SCENE_DETECT`）：按镜头切换分段
- 模糊检测（`AT_BLUR`）：过滤模糊帧
- 关键帧提取（`AT_KEY_FRAME`）：提取代表性帧
- 熵值计算（`AT_ENTROPY_VALUE`）：评估画面信息量/质量
- 组合分析（`AT_SCENE_AND_KEYFRAME` / `AT_FACE_AND_ENTROPY`）

每段分析结果封装为 `VbMaterialInfo`（含得分、起止时间、最高分帧时间、人脸区域）。

### TemplateMatch（模板匹配）
接收 MaterialAnalysis 的分析结果，**自动将最优素材片段匹配到模板时间线对应位置**，生成完整的视频工程：
- 根据模板定义的镜头数量/时长要求选取最优片段
- 按模板布局创建时间线（含特效/转场/音乐）
- **仅桌面端可用**（Android/iOS 构建时忽略此模块）

---

## 2. 核心接口说明

### 2.1 MaterialAnalysis

```cpp
class IVbMaterialAnalysis : virtual public IVblBaseTask {
    // 配置分析算法组合
    virtual Result configAnalysis(VBLIntList* types) = 0;

    // 各算法权重（影响最终得分排序）
    virtual Result setAnalysisWeight(AnalysisType type, VBLInt weight) = 0;

    // 素材管理
    virtual Result insertMaterials(VBLBaseObjectList* pMedias, VBLInt idx) = 0;
    virtual Result removeMaterials(VBLBaseObjectList* pMedia) = 0;
    virtual Result removeMaterialsByIndex(VBLInt idx) = 0;
    virtual Result clearMaterials() = 0;

    // 分析参数
    virtual Result setAnalysisParam(VBLConstPChar key, const Property& value) = 0;
    // 参数 key：minScenceDuration / scenceDetectionThreshold /
    //          minNeedAnalysisDuartion / photoNeedAnalysis

    // 获取结果（继承自 IVblBaseTask）
    // start() / stop() / pause() / resume() / onProgress(cb) / onComplete(cb)
    virtual Result analysisResult(VBLVoidPtrList* pMaterialInfo) = 0;
};

struct VbMaterialInfo {
    VBLREAL score;              // 综合得分
    VBLLonglong begin;          // 片段起始时间（us）
    VBLLonglong end;            // 片段结束时间（us）
    VBLLonglong posOfMaxScore;  // 最高分帧时间
    IDmBaseMedia* pMedia;       // 对应媒体对象
    RectF faceRectOfPhoto;      // 照片人脸区域
};
```

### 2.2 分析参数 Key

| Key | 类型 | 默认值 | 说明 |
|---|---|---|---|
| `minScenceDuration` | int（秒）| 2 | 最小场景时长（过短场景不纳入结果）|
| `scenceDetectionThreshold` | float | 0.5 | 场景检测灵敏度（越高越敏感）|
| `minNeedAnalysisDuartion` | int（秒）| 5 | 最短需分析素材时长 |
| `photoNeedAnalysis` | bool | true | 是否对图片素材做人脸检测 |

---

## 3. AnalysisType 枚举

| 值 | 名称 | 说明 |
|---|---|---|
| 1 | `AT_FACEDETECT` | 人脸检测 |
| 2 | `AT_BLUR` | 模糊检测 |
| 3 | `AT_KEY_FRAME` | 关键帧提取 |
| 4 | `AT_SCENE_DETECT` | 场景检测 |
| 5 | `AT_ENTROPY_VALUE` | 熵值计算 |
| 6 | `AT_SCENE_AND_KEYFRAME` | 场景+关键帧组合 |
| 7 | `AT_FACE_AND_ENTROPY` | 人脸+熵值组合 |

---

## 4. 依赖关系

```
MaterialAnalysis
  ├── 依赖 → BackgroundTaskManager  （将分析任务提交后台线程执行）
  ├── 依赖 → Adapter（MediaAnalysisAdapter）  （底层算法调用）
  └── 输出 → TemplateMatch          （VbMaterialInfo 列表）

TemplateMatch
  ├── 输入 ← MaterialAnalysis       （分析结果）
  ├── 依赖 → DataModel              （创建时间线/Clip 对象）
  ├── 依赖 → SourceManager          （管理导入素材）
  └── 发布 → IMsEventBus            （project.created 事件）
```

---

## 5. 时序图

### 5.1 素材分析完整流程

```
UI 层    MaterialAnalysis   BackgroundTaskMgr   MediaAnalysisAdapter  EventBus
  │             │                  │                    │                  │
  │ createMaterialAnalysis()       │                    │                  │
  ├────────────►│                  │                    │                  │
  │ configAnalysis([SCENE, FACE])  │                    │                  │
  ├────────────►│                  │                    │                  │
  │ setAnalysisWeight(FACE, 60)    │                    │                  │
  ├────────────►│                  │                    │                  │
  │ insertMaterials(mediaList, 0)  │                    │                  │
  ├────────────►│                  │                    │                  │
  │ start()     │                  │                    │                  │
  ├────────────►│                  │                    │                  │
  │             │ addTask(sceneTask)│                   │                  │
  │             ├─────────────────►│                    │                  │
  │             │ addTask(faceTask) │                   │                  │
  │             ├─────────────────►│                    │                  │
  │             │                  │ 依次执行分析任务    │                  │
  │             │                  │ MediaAnalysisAdapter::analyzeScene()  │
  │             │                  ├────────────────────►│                 │
  │             │                  │                     │ 底层算法处理     │
  │             │                  │ onProgress(30%)     │                 │
  │             │                  │◄────────────────────┤                 │
  │             │ onProgress(30%)  │                     │                 │
  │◄────────────┤                  │                     │                 │
  │  进度条更新  │                  │                     │                 │
  │             │                  │ MediaAnalysisAdapter::analyzeFace()   │
  │             │                  ├────────────────────►│                 │
  │             │                  │ onProgress(80%)     │                 │
  │             │                  │◄────────────────────┤                 │
  │             │ onProgress(80%)  │                     │                 │
  │◄────────────┤                  │                     │                 │
  │             │                  │ onComplete(results) │                 │
  │             │                  │◄────────────────────┤                 │
  │             │ aggregateResults()│                    │                 │
  │             │ 按权重排序得分   │                     │                 │
  │             │ postEvent("task.analysis.done")        │                 │
  │             ├──────────────────────────────────────────────────────── ►│
  │ onEvent() 展示分析结果         │                    │                  │ → UI
  │◄──────────────────────────────────────────────────────────────────────────┤
```

### 5.2 模板匹配流程（一键成片）

```
UI 层      MaterialAnalysis  TemplateMatch    DataModel    SourceManager  EventBus
  │              │                 │               │              │           │
  │ 用户选择模板  │                 │               │              │           │
  │ startAnalysis(medias)          │               │              │           │
  ├─────────────►│                 │               │              │           │
  │   [等待分析完成，见 5.1]        │               │              │           │
  │              │                 │               │              │           │
  │ getResult()  │                 │               │              │           │
  ├─────────────►│                 │               │              │           │
  │  materialInfoList              │               │              │           │
  │◄─────────────┤                 │               │              │           │
  │              │                 │               │              │           │
  │ matchTemplate(infoList, templateMedia)         │              │           │
  ├─────────────────────────────── ►│              │              │           │
  │              │  模板解析：读取 slots 定义        │              │           │
  │              │  按 score 排序选取最优片段        │              │           │
  │              │  createProject()│               │              │           │
  │              │                 ├──────────────►│              │           │
  │              │  addSourceMedias(selected)      │              │           │
  │              │                 ├──────────────────────────────►│           │
  │              │  addClipsToTimeline(clips, template_layout)     │           │
  │              │                 ├──────────────►│              │           │
  │              │  addEffectsFromTemplate()        │              │           │
  │              │                 ├──────────────►│              │           │
  │              │  addBGMFromTemplate()            │              │           │
  │              │                 ├──────────────►│              │           │
  │              │  postEvent("project.created")   │              │           │
  │              │                 ├──────────────────────────────────────────►│
  │ onEvent() 打开生成的工程        │               │              │           │ → UI
  │◄──────────────────────────────────────────────────────────────────────────────┤
```

### 5.3 分析任务取消

```
UI 层      MaterialAnalysis   BackgroundTaskMgr   分析适配器
  │              │                  │                  │
  │ stop()       │                  │                  │
  ├─────────────►│                  │                  │
  │              │ 标记 cancelling  │                  │
  │              │ removeTask()/stop()                 │
  │              ├─────────────────►│                  │
  │              │                  │ task->cancel()   │
  │              │                  ├─────────────────►│
  │              │                  │                  │ 中断算法
  │              │                  │ onCancelled()    │
  │              │                  │◄─────────────────┤
  │              │ clearMaterials() │                  │
  │◄─────────────┤                  │                  │
  │ 分析已取消   │                  │                  │
```

---

## 6. 平台支持

| 功能 | Windows | macOS | iOS | Android | Web |
|---|:---:|:---:|:---:|:---:|:---:|
| MaterialAnalysis | ✅ | ✅ | ✅ | ✅ | ❌ |
| TemplateMatch | ✅ | ✅ | ❌ | ❌ | ❌ |

> TemplateMatch 在 Android/iOS 构建时通过 `IGNORE_LIBS` 跳过编译。

---

## 7. 设计要点

| 要点 | 说明 |
|---|---|
| 异步执行 | 分析任务通过 BackgroundTaskManager 在后台线程执行，不阻塞主线程 |
| 多算法组合 | `configAnalysis` 支持多个算法并行或顺序执行，`setAnalysisWeight` 控制综合得分权重 |
| 得分排序 | 最终 `VbMaterialInfo` 列表按综合得分降序排列，TemplateMatch 优先取高分片段 |
| 增量分析 | `insertMaterials/removeMaterials` 支持增量更新分析队列 |
| 模板 slot | 模板定义每个 slot 的时长范围和素材类型要求，TemplateMatch 按约束匹配 |
