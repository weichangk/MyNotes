# VBL 模块详细设计文档总目录

> 版本：v7.17.x | 更新日期：2026-03-28

---

## 文档列表

| 编号 | 文件名 | 模块 | 所属层 |
|---|---|---|---|
| 01 | [01_BusinessLayer_ProjectEditor.md](01_BusinessLayer_ProjectEditor.md) | ProjectEditor | BusinessLayer |
| 02 | [02_BusinessLayer_SourceManager.md](02_BusinessLayer_SourceManager.md) | SourceManager | BusinessLayer |
| 03 | [03_BusinessLayer_TimelineUX_TimelineEditor.md](03_BusinessLayer_TimelineUX_TimelineEditor.md) | TimelineUX / TimelineEditor | BusinessLayer |
| 04 | [04_BusinessLayer_AIManager.md](04_BusinessLayer_AIManager.md) | AIManager | BusinessLayer |
| 05 | [05_BusinessLayer_MaterialAnalysis_TemplateMatch.md](05_BusinessLayer_MaterialAnalysis_TemplateMatch.md) | MaterialAnalysis / TemplateMatch | BusinessLayer |
| 06 | [06_DataModel.md](06_DataModel.md) | DataModel | DataModel |
| 07 | [07_ListenerCenter.md](07_ListenerCenter.md) | ListenerCenter (IMsEventBus) | ListenerCenter |
| 08 | [08_BaseService_BackgroundTaskManager.md](08_BaseService_BackgroundTaskManager.md) | BackgroundTaskManager | BaseService |
| 09 | [09_BaseService_PlayManager_EncodeManager.md](09_BaseService_PlayManager_EncodeManager.md) | PlayManager / EncodeManager | BaseService |
| 10 | [10_BaseService_UndoManager_BsNet_BsCloudResource_BsPreset.md](10_BaseService_UndoManager_BsNet_BsCloudResource_BsPreset.md) | UndoManager / BsNet / BsCloudResource / BsPreset / BSWsid | BaseService |
| 11 | [11_BaseService_VblLogger_VblUtils_BsDataStructure_BsDiskFolder.md](11_BaseService_VblLogger_VblUtils_BsDataStructure_BsDiskFolder.md) | VblLogger / VblUtils / BsDataStructure / BsDiskFolder | BaseService（基础设施）|
| 12 | [12_Adapter.md](12_Adapter.md) | Adapter (VAL::) | Adapter |

---

## 架构层级速查

```
┌─────────────────────────────────────────────────────────────┐
│                    BusinessLayer（业务逻辑层）                 │
│  01 ProjectEditor │ 02 SourceManager │ 03 TimelineUX/Editor  │
│  04 AIManager     │ 05 MaterialAnalysis/TemplateMatch        │
└──────────────────────────────┬──────────────────────────────┘
                               │
┌──────────────────────────────▼──────────────────────────────┐
│                     DataModel（数据模型层）                    │
│  06 DataModel（IDmProject / IDmTimeline / IDmClip ...）      │
└────────────┬─────────────────────────────────────┬──────────┘
             │  PUBLIC 依赖                          │  PRIVATE 依赖
┌────────────▼────────────┐            ┌────────────▼──────────┐
│  07 ListenerCenter      │            │   BaseService         │
│  （IMsEventBus 事件总线）│            │  08 BackgroundTaskMgr │
└─────────────────────────┘            │  09 Play/EncodeManager│
                                       │  10 Undo/Net/Cloud... │
                                       │  11 Logger/Utils/...  │
                                       └────────────┬──────────┘
                                                    │
                                       ┌────────────▼──────────┐
                                       │  12 Adapter (VAL::)   │
                                       │  wes/ 实现 | nle/ 实现 │
                                       └────────────┬──────────┘
                                                    │
                                       ┌────────────▼──────────┐
                                       │  底层引擎 WES/NLE SDK  │
                                       └───────────────────────┘
```

---

## 各文档包含内容说明

每份模块文档统一包含以下章节：

1. **模块职责** — 该模块解决什么问题，边界在哪里
2. **核心接口说明** — 关键接口方法的签名与说明
3. **依赖关系** — 模块的上下游依赖（PUBLIC/PRIVATE）
4. **时序图** — 覆盖主要业务场景的 ASCII 时序图（每模块 3~8 张）
5. **设计要点** — 重要的设计决策、注意事项、约束条件

---

## 时序图覆盖场景汇总

| 模块 | 覆盖的时序图场景 |
|---|---|
| ProjectEditor | 新建工程、加载工程、保存工程、自动保存/恢复、媒体重定位 |
| SourceManager | 导入媒体、移动媒体项、替换媒体 |
| TimelineUX/Editor | 添加Clip、分割Clip、添加转场、变速曲线、J/L Cut |
| AIManager | 本地AI任务（STT）、云端AI任务（超清修复）、取消任务 |
| MaterialAnalysis | 素材分析完整流程、模板匹配一键成片、取消分析 |
| DataModel | 初始化、工程序列化、工程反序列化、Clip变更事件广播 |
| ListenerCenter | 注册订阅、主线程事件、后台任务通知、粘性事件、取消订阅 |
| BackgroundTaskManager | 音频波形提取、任务分组切换、并发限制示意 |
| PlayManager/EncodeManager | 时间线播放、截图、导出视频、Trim预览 |
| UndoManager | Undo/Redo完整流程、Macro合并操作 |
| BsNet | 断点续传下载 |
| BsCloudResource | 资源下载与使用 |
| BsPreset | 保存并应用预设 |
| BSWsid | AI功能授权校验 |
| VblLogger | 日志格式化与输出 |
| BsDiskFolder | 工程目录初始化 |
| Adapter | 系统初始化、媒体信息读取、时间线渲染帧、Clip添加、WES↔NLE切换 |
