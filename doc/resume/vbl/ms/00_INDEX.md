# VBL 项目面试技术点文档 · 总目录

> 项目：VBL（Video Business Layer）— Filmora 视频编辑软件业务中间层  
> 语言：C++17 | 平台：Windows / macOS / iOS / Android / Web-WASM  
> 版本：v7.17.x | 文档更新：2026-03-28

---

## 一、文档使用说明

本目录下的每份文档对应一个**可被面试考查的技术专题**，统一结构如下：

| 章节 | 内容 |
|------|------|
| 概念介绍 | 技术点的定义与通用原理 |
| 项目实现 | VBL 中的具体方案、类名、文件路径 |
| 关键代码片段 | 带文件路径的代码节选（可直接背诵/演示）|
| 面试要点 | 3~5 条浓缩核心结论，开口即能说清楚 |
| 可能被追问的问题 | 高频追问 + 参考答案方向 |

**复习策略**：  
- 面试前 1 天：通读"面试要点"和"可能被追问的问题"列  
- 面试前 3 天：通读"项目实现"和"关键代码片段"  
- 日常积累：完整阅读全文  

---

## 二、文档总目录

### ① 设计模式专题（高频，必背）

| 编号 | 文件名 | 技术点 | ⭐ 面试价值 | 优先级 |
|------|--------|--------|-----------|--------|
| 01 | [01_设计模式_单例模式.md](01_设计模式_单例模式.md) | Singleton：三种实现对比 | ⭐⭐⭐⭐⭐ | P0 |
| 02 | [02_设计模式_工厂模式.md](02_设计模式_工厂模式.md) | Factory：接口工厂 + 多引擎适配 | ⭐⭐⭐⭐ | P0 |
| 03 | [03_设计模式_命令模式与UndoRedo.md](03_设计模式_命令模式与UndoRedo.md) | Command + Undo/Redo框架 | ⭐⭐⭐⭐⭐ | P0 |
| 04 | [04_设计模式_观察者Builder等.md](04_设计模式_观察者Builder等.md) | Observer / Builder / Strategy / PIMPL / Proxy | ⭐⭐⭐⭐ | P1 |

### ② 内存与智能指针专题（C++ 核心，必考）

| 编号 | 文件名 | 技术点 | ⭐ 面试价值 | 优先级 |
|------|--------|--------|-----------|--------|
| 05 | [05_智能指针体系_SafePtr_RefCnt_WeakPtr.md](05_智能指针体系_SafePtr_RefCnt_WeakPtr.md) | 自定义 intrusive 智能指针 vs std::shared_ptr | ⭐⭐⭐⭐⭐ | P0 |
| 08 | [08_内存管理与对象池.md](08_内存管理与对象池.md) | 对象池 / RAII / 引用计数 | ⭐⭐⭐⭐ | P1 |

### ③ 多线程与异步专题（高频考点）

| 编号 | 文件名 | 技术点 | ⭐ 面试价值 | 优先级 |
|------|--------|--------|-----------|--------|
| 06 | [06_事件总线_MsEventBus.md](06_事件总线_MsEventBus.md) | 多线程事件总线设计 | ⭐⭐⭐⭐⭐ | P0 |
| 07 | [07_多线程与并发.md](07_多线程与并发.md) | 线程池 / condition_variable / atomic / future | ⭐⭐⭐⭐⭐ | P0 |

### ④ 跨平台与工程化专题

| 编号 | 文件名 | 技术点 | ⭐ 面试价值 | 优先级 |
|------|--------|--------|-----------|--------|
| 09 | [09_多平台适配与CMake构建.md](09_多平台适配与CMake构建.md) | 条件编译 / 动态库 / CMake模块化 / WASM | ⭐⭐⭐⭐ | P1 |
| 12 | [12_多语言Wrapper层.md](12_多语言Wrapper层.md) | C/ObjC++/Swift/C++CLI/WASM wrapper | ⭐⭐⭐⭐ | P1 |

### ⑤ 序列化 & 错误处理

| 编号 | 文件名 | 技术点 | ⭐ 面试价值 | 优先级 |
|------|--------|--------|-----------|--------|
| 10 | [10_序列化与JSON.md](10_序列化与JSON.md) | rapidjson vs jsoncpp 选型 | ⭐⭐⭐ | P2 |
| 11 | [11_错误处理设计.md](11_错误处理设计.md) | Result枚举 / X-Macro / 无异常设计 | ⭐⭐⭐⭐ | P1 |

### ⑥ 接口规范 & 测试

| 编号 | 文件名 | 技术点 | ⭐ 面试价值 | 优先级 |
|------|--------|--------|-----------|--------|
| 13 | [13_接口设计规范与COM风格.md](13_接口设计规范与COM风格.md) | 纯虚接口 / COM生命周期 / 接口隔离 | ⭐⭐⭐⭐ | P1 |
| 14 | [14_单元测试与GoogleTest.md](14_单元测试与GoogleTest.md) | GTest / 参数化测试 / CMake集成 | ⭐⭐⭐ | P2 |

### ⑦ 核心数据与业务架构专题（新增）

| 编号 | 文件名 | 技术点 | ⭐ 面试价值 | 优先级 |
|------|--------|--------|-----------|--------|
| 15 | [15_DataModel分层设计.md](15_DataModel分层设计.md) | Timeline/Track/Clip 三层 / Effect / Keyframe / Clone | ⭐⭐⭐⭐⭐ | P0 |
| 17 | [17_Adapter适配层设计.md](17_Adapter适配层设计.md) | 抽象工厂 / 多引擎切换 / MediaInfo缓存 | ⭐⭐⭐⭐⭐ | P0 |
| 18 | [18_业务层架构分层.md](18_业务层架构分层.md) | BusinessLayer 子模块 / 依赖关系 / EncodeManager / AIManager | ⭐⭐⭐⭐ | P1 |
| 16 | [16_WASM_Emscripten深度解析.md](16_WASM_Emscripten深度解析.md) | 内存配置 / pthreads in WASM / JS 互操作 / idbfs | ⭐⭐⭐⭐ | P1 |

---

## 三、优先级说明

| 优先级 | 含义 | 复习顺序 |
|--------|------|----------|
| **P0** | 必考核心，几乎每次面试都会问 | 最先背，重点准备 |
| **P1** | 高概率被问，有较强区分度 | 第二轮复习 |
| **P2** | 加分项，有深度追问时用 | 时间充裕再复习 |

---

## 四、面试价值评级标准

| ⭐ 数量 | 含义 |
|---------|------|
| ⭐⭐⭐⭐⭐ | 核心考点，答好可直接加分，答差会减分 |
| ⭐⭐⭐⭐ | 重要考点，有一定区分度 |
| ⭐⭐⭐ | 加分项，有亮点 |

---

## 五、VBL 整体架构速记（面试时可以主动介绍）

```
┌──────────────────────────────────────────────────────────────┐
│         UI 层 / 上层调用方（Filmora 主进程）                    │
└──────────────────────┬───────────────────────────────────────┘
                       │ wrapper 层（C/ObjC++/Swift/C++CLI/WASM）
┌──────────────────────▼───────────────────────────────────────┐
│                 VBL 业务层（本项目 C++）                        │
│                                                              │
│  BusinessLayer ──→ DataModel ──→ BaseService                 │
│      │                               │                       │
│  ListenerCenter(EventBus)        Adapter(VAL::)              │
│                                       │                      │
└───────────────────────────────────────┼──────────────────────┘
                                        ▼
                          底层引擎（WES / NLE SDK）
```

**一句话描述**：VBL 是 Filmora 的 C++ 业务中间层，屏蔽底层多引擎差异（WES/NLE），向上通过多语言 wrapper 提供统一 API，内部采用事件总线解耦、Command 模式实现 Undo/Redo、自定义 intrusive 智能指针管理对象生命周期。

---

## 六、技术关键词速查表

| 关键词 | 对应文档 |
|--------|----------|
| `SafePtr<T>` / `RefCnt<T>` / `WeakPtr<T>` | 05 |
| `MsEventBus` / `postMainThreadEvent` / sticky event | 06 |
| `ThreadPool` / `condition_variable` / `std::future` | 07 |
| `BsUndoAutoTemplateCommand` / `BsUndoCombCommand` | 03 |
| `DmClipFactory` / `IDmClipFactory` | 02 |
| Singleton / Meyers单例 | 01 |
| PIMPL / struct Impl + unique_ptr | 04 |
| `VBL::Result` / X-Macro / `VBL_CHECK_NULL_RETURN` | 11 |
| `rapidjson` / `jsoncpp` | 10 |
| `extern "C"` / `.mm` / `ref class` / emscripten | 12 |
| `AppendModulesBasic()` / CMake模块化 | 09 |
| `IDmBaseObj` / `AddRef/Release` / 纯虚接口 | 13 |
| `TEST_P` / `INSTANTIATE_TEST_SUITE_P` | 14 |
| Observer / SafelyObserverBase / `weak_ptr` | 04 |
| Builder / `TimelineDataBuilder` | 04 / 15 |
| 对象池 / `MediaInfoAdapterObjectPool` / `shared_mutex` | 08 |
| `DmTimeline` / `DmTrack` / `DmBaseClip` / Timeline三层 | 15 |
| `IDmEffect` / keyframe / `appendEffect` / clone | 15 |
| `IAdapterFactory` / `DmWesAdapterFactory` / 多引擎切换 | 17 |
| `VBL_IMPLEMENT_LEVEL` / wes vs nle 切换 | 17 |
| `MediaInfoAdapterObjectPool` / 对象池 + rapidjson 缓存 | 17 |
| `VbProjectEditor` / loadProject / saveProject | 18 |
| `EncodeManager` / `GenerateTimelineTask` / Builder导出 | 18 |
| `VbAIManagerThread` / loopfunc / AI异步线程 | 18 |
| WASM / `-sWASM=1` / emscripten / pthreads in WASM | 16 |
| `-sALLOW_MEMORY_GROWTH` / `-sPTHREAD_POOL_SIZE` | 16 |
| `webProject2ExportProject` / C API / idbfs | 16 |

