# Wondershare UniConverter（VideoConverter Ultimate）Windows 版
## 整体设计架构文档

> 文档版本：v1.0
> 对应分支：`UniConverter16`
> 更新日期：2026-03-27

---

## 目录

1. [项目概述](#1-项目概述)
2. [整体目录结构](#2-整体目录结构)
3. [解决方案与工程文件](#3-解决方案与工程文件)
4. [主工程架构（VCU10）](#4-主工程架构vcu10)
   - 4.1 [功能模块划分](#41-功能模块划分)
   - 4.2 [UI 层架构](#42-ui-层架构)
   - 4.3 [启动流程](#43-启动流程)
5. [公共模块层（CommonModule）](#5-公共模块层commonmodule)
6. [第三方库依赖](#6-第三方库依赖)
7. [构建与打包体系](#7-构建与打包体系)
8. [关键设计模式与约定](#8-关键设计模式与约定)
9. [数据流与任务调度](#9-数据流与任务调度)
10. [多语言与皮肤系统](#10-多语言与皮肤系统)
11. [AI 能力集成](#11-ai-能力集成)
12. [崩溃上报与数据采集](#12-崩溃上报与数据采集)
13. [国内版 / 国际版差异（ZH 编译宏）](#13-国内版--国际版差异zh-编译宏)
14. [关键常量速查](#14-关键常量速查)

---

## 1. 项目概述

**Wondershare UniConverter**（商品名，原名 VideoConverter Ultimate）是 Wondershare 公司推出的 Windows 平台视频处理综合软件。

| 属性 | 值 |
|------|----|
| 开发语言 | C#（主体 WinForms）、Delphi（部分工具模块） |
| 目标框架 | .NET Framework 2.0（VCU10.csproj 当前配置） |
| 主输出类型 | WinExe（VideoConverterUltimate.exe） |
| 平台目标 | x86 / AnyCPU（视构建配置） |
| UI 框架 | Wondershare UI Library（WUL）自定义皮肤控件框架 |
| 崩溃上报 | BugSplat（数据库：`Wondershare_UniConverter2019`） |
| 当前分支 | `UniConverter16`（主分支 `master`） |

### 核心功能

- **视频格式转换**：支持数百种视频/音频格式互转，含硬件加速
- **视频压缩**：智能压缩，保留画质
- **视频下载**：从主流网站解析/下载
- **屏幕录制**：多种录制模式
- **视频编辑**：水印、字幕、批量编辑、智能剪辑
- **AI 工具箱**：人声提取、噪声消除、GIF 制作、图像增强、语音识别（STT）、文字转语音（TTS）、视频水印去除、人像抠图、视频防抖、智能裁剪等
- **DVD/BD 刻录**：支持 DVD5/DVD9/BD25 等规格
- **文件传输**：移动设备文件传输管理

---

## 2. 整体目录结构

```
VideoConvertUltimate-Windows/         ← 仓库根目录
├── Src/                              ← 所有源码与依赖
│   ├── Solution/                     ← Visual Studio 解决方案文件
│   ├── ProductUI/                    ← 产品 UI 层（C# WinForms）
│   │   └── VideoConverterUltimate10/
│   │       ├── Source/               ← 主工程源码（VCU10.csproj）
│   │       └── Setup/                ← 打包脚本与 Inno Setup 配置
│   ├── CommonModule/                 ← 公共模块（各功能共享基础库）
│   ├── DelphiModule/                 ← Delphi 工具模块
│   ├── Bin/                          ← 编译输出（Release/Debug）
│   ├── Library/                      ← x86 / AnyCPU 第三方库
│   └── Library(x64)/                 ← x64 专用第三方库（AI、转码引擎等）
├── Document/                         ← 设计文档（含本文档）
├── Tools/                            ← 辅助工具
├── docs/                             ← GitHub Pages / 在线文档
├── reports/                          ← 报告产物
├── CLAUDE.md                         ← AI 辅助开发配置说明
└── Readme.md
```

---

## 3. 解决方案与工程文件

### 解决方案列表

| 解决方案文件 | 用途 | 构建配置 |
|-------------|------|---------|
| `Src/Solution/KvProducts.sln` | **主工程**（含 VCU10 项目及依赖） | `CBSRelease`（正式包） |
| `Src/Solution/Uninstaller.sln` | 卸载程序 | — |
| `Src/Solution/SubtitleEditor.sln` | 字幕编辑器独立工程 | — |

### VCU10.csproj 构建配置矩阵

| 配置名 | 特点 |
|--------|------|
| `Debug` | 调试版，含调试符号 |
| `Release` | 标准发布版 |
| `CBSRelease` | 正式打包配置（含混淆/签名触发） |
| `ZHDebug` / `ZHRelease` | 国内版（定义 `ZH` 宏） |
| `ISRelease` / `KVRelease` | 特定渠道发布版 |
| `AIRelease` | AI 功能版本 |
| `FVCRelease` | FVC 渠道版本 |
| `STEAMGAMES` | Steam 渠道版本 |

---

## 4. 主工程架构（VCU10）

主工程源码位于 `Src/ProductUI/VideoConverterUltimate10/Source/`，组织结构如下：

### 4.1 功能模块划分

```
Source/
├── Program.cs                  ← 程序入口、Mutex 防多开、初始化流程
├── MainForm.cs                 ← 主窗体（导航、页面切换）
├── ProductConst.cs             ← 全局常量（格式ID、注册Key、产品信息等）
│
├── Home/                       ← 主界面（首页、导航菜单）
│
├── Convert/                    ← 视频转换与压缩
│   ├── ConvertPage.cs              主转换页
│   ├── VideoConverterPage.cs       视频转换子页
│   ├── VideoCompressorPage.cs      视频压缩子页
│   ├── VideoEditPage.cs            转换前编辑页
│   ├── MergePage.cs                视频合并页
│   ├── Components/                 转换相关 UI 控件
│   └── DeviceMediaForm/            设备媒体导入
│
├── Download/                   ← 视频下载
│
├── MediaEdit/                  ← 视频编辑
│   ├── 水印编辑
│   ├── 字幕编辑
│   └── 批量编辑等
│
├── ToolBox/                    ← AI 与工具箱（详见 §11）
│   ├── GifMaker/                   GIF 制作
│   ├── AutoReFrame/                智能重新构图
│   ├── FixVideoShake/              视频防抖
│   ├── ImageConverter/             图片格式转换
│   ├── NewAI/                      AI 视频增强
│   ├── STT/                        语音转文字
│   ├── TTS/                        文字转语音
│   ├── NoiseRemover/               噪声消除
│   ├── VocalRemover/               人声提取
│   └── WaterMarker/                水印去除等
│
├── AIPortait/                  ← AI 人像抠图、AI 图像处理
│
├── Burn/                       ← DVD/BD 刻录
│
├── RecorderV17/                ← 屏幕录制（V17 版本）
├── Recorder/                   ← 屏幕录制（旧版本）
│
├── Transfer/                   ← 移动设备文件传输
│
├── MediaPlayer/                ← 媒体播放器页面
├── Player/                     ← 播放器底层组件
│
├── SmartTrim/                  ← 智能剪辑
├── SegmentCutter/              ← 分段截取
├── Trim/                       ← 视频裁剪
├── SubtitleEditor/             ← 字幕编辑器（嵌入）
├── ImageTool/                  ← 图像工具
│
├── Account/                    ← 账户管理
├── Register/                   ← 产品注册
├── Payment/ / Payment2/        ← 支付流程
├── FeedBack/                   ← 反馈与建议
├── LiveUpdate/                 ← 在线更新
├── GuardSystem/                ← 防盗版守护
│
├── BaseUI/                     ← 通用 UI 基类与控件库
│   ├── BasePage.cs                 页面基类
│   └── Components/                 公共控件（播放器组件等）
│
├── Dialogs/                    ← 公共对话框
├── Format/                     ← 格式相关 UI（格式选择器等）
├── FileManager/                ← 文件管理工具
├── Util/                       ← 通用工具函数
├── OSS/                        ← 阿里云 OSS 集成
├── WGP/                        ← WGP 相关
└── Resources/                  ← 静态资源（图片、图标等）
```

### 4.2 UI 层架构

整体采用 **WinForms + WUL（Wondershare UI Library）** 方案：

```
┌─────────────────────────────────────────────────────────┐
│                      MainForm                           │
│  ┌──────────┐  ┌──────────────────────────────────────┐ │
│  │ 导航侧栏  │  │           页面区域（Page）              │ │
│  │ (NavMenu)│  │  ConvertPage / DownloadPage / ...    │ │
│  └──────────┘  │  各页面继承 BasePage 基类              │ │
│                └──────────────────────────────────────┘ │
│                       WUL 皮肤层（自定义渲染）            │
└─────────────────────────────────────────────────────────┘
```

- 每个功能页面继承 `BasePage` 基类
- 子控件遵循 **WinForms Designer 模式**（`.cs` + `.Designer.cs` + `.resx` 三件套）
- 部分工具子模块采用 **MVC-ish 分层**：
  ```
  ToolBox/{功能}/
  ├── Controller/   ← 业务逻辑控制器
  ├── Model/        ← 数据模型
  ├── View/         ← 窗体/控件（WinForms）
  └── Component/   ← 可复用 UI 子控件
  ```

### 4.3 启动流程

```
Program.Main(args)
    │
    ├─ IsInstanceRunning()     ─── 创建 Mutex 防多开
    │       ├─ "Wondershare UniConverter 14"
    │       ├─ "Wondershare UniConverter 15"
    │       ├─ "{CompanyName} {ProductName}"（当前版本）
    │       └─ "VCUDownloadUpdaterMutex"
    │       （若已有实例：WM_COPYDATA 传参 → 激活已有窗口 → 退出）
    │
    ├─ LogAPI.LogInit("MultiMedia.log")     初始化日志
    ├─ CheckPlayWithProduct()               命令行参数解析
    ├─ InitGuardSystemConfig()              防盗版系统初始化（非ZH非DEBUG）
    ├─ InitConfigFile()                     配置文件加载
    ├─ LoadSkinAndLanguages()               皮肤与语言包加载
    ├─ GPUAccForAISDKHelper.Init()          GPU/AI SDK 初始化
    ├─ InitRemoteConfig()                   远程配置拉取
    ├─ InitCustomize()                      产品定制化初始化
    ├─ UserDataCollectHelper.Init()         数据采集初始化
    ├─ InitApp()                            应用核心初始化
    │       ├─ 文化/区域设置
    │       ├─ 首次启动判定
    │       └─ InitBugSplat()              崩溃上报初始化
    ├─ WSProductReg.Init()                  注册模块初始化
    │
    ├─ new MainForm() → Application.Run()   主窗体运行
    │
    └─ 退出清理
            ├─ EncodeParamMgr 保存
            ├─ SystemStateHelper.SetExcutionState(false)
            ├─ RecorderComFactory.ReleaseAllComObject()
            ├─ ExitDataCollect()
            ├─ LogAPI.LogUninit()
            └─ ReleaseMutex()
```

---

## 5. 公共模块层（CommonModule）

位于 `Src/CommonModule/`，以源码链接方式（`<Compile Include=... Link=...>`）引入主工程，实现跨模块能力复用。

### 模块清单

| 模块目录 | 职责 |
|---------|------|
| `BaseClass/` | 基础类库：`CommonBaseClass`、`BaseForm`、自定义控件（`CustomizeEditControl` 等）、`WSReg`（注册）、`ProcessEx`、`JsCallManager` |
| `BaseDll/` | 基础 DLL：数据采集（`DataCollect/`）、文件状态、播放列表管理、系统状态、VOB 修复 API |
| `TaskManager/` | 转换任务调度：`EncoderTasks`、任务队列管理 |
| `DownloadHelperService/` | 下载底层：`ProtocolHandler`、协议解析 |
| `DownloadMgr/` | 下载管理器 |
| `TransferProcess/` | 设备文件传输处理 |
| `TransferManager/` | 传输管理 |
| `MediaBatchConv/` | 批量转换 |
| `FormatManager/` | 格式 ID 管理与格式参数解析 |
| `VCPlayer/` | 视频播放器内核 |
| `WsMutimedias/` | 多媒体处理（音视频信息读取、处理） |
| `UniMessageCenter/` | 应用内消息中心（模块间通信） |
| `DVDMenu/` | DVD 菜单管理与资源 |
| `DRMConverter/` / `DRMManager/` | DRM 加密视频处理 |
| `FeedbackOnline/` | 在线反馈（`FeedbackConst` 等） |
| `WsUpdateHelper/` | 在线更新辅助 |
| `WsPushHelper/` | 推送通知辅助 |
| `ImageMetadateRW/` | 图片元数据读写 |
| `LoadMedias/` | 媒体文件加载 |
| `PluginInstallHelper/` | 插件安装辅助 |

### 数据采集子模块（BaseDll/DataCollect/）

| 子组件 | 说明 |
|--------|------|
| `GATracker.cs` | Google Analytics 事件追踪 |
| `Sparrow/` | Wondershare 内部数据平台上报 |
| `sensors_analytics/` | 神策数据分析 SDK 集成 |

---

## 6. 第三方库依赖

### 6.1 x86 / AnyCPU 库（`Src/Library/`）

| 库 / 目录 | 说明 |
|-----------|------|
| `WUL/` | Wondershare UI Library：`WUL.Core.dll`、`WUL.Ctrls.dll`、`WUL.Localization.dll`、`WUL.Zip.dll`（皮肤/控件/多语言框架） |
| `WULN/` | WUL 新版本组件 |
| `JSON/` | `Newtonsoft.Json.dll`、`AVILibrary.dll`、`MKVLibrary.dll`、`WMVLibrary.dll`、`ID3.dll`、`VideoConverter.PList.dll` 等 |
| `AgileTrans/Bin/Release/` | 核心音视频转码引擎（x86 版本）：`DecoderMgr.dll`、`EffectPlugin.dll`、`MediaInfo.dll` 等 |
| `AdditionLib/` | 附加工具库：捕获模块（`WSCapture.dll`）、转换（`CmdConverter.exe`、`WS_BatchConv.dll`、`EncodeParamCheck.dll`）、DRM、DVD、工具（`GetMediaInfo.exe`、`MetadataConvert.exe`）等 |

### 6.2 x64 专用库（`Src/Library(x64)/`）

| 库 / 目录 | 说明 |
|-----------|------|
| `AgileTrans/Bin/Release/` | x64 核心转码引擎（`CaptureEngine.dll`、`AVCHDParser.dll`、`D3DCompiler_43.dll` 等） |
| `AIResouces/` | AI 推理框架与模型：OpenVINO（`inference_engine.dll`、`libprotobuf.dll`）、OpenCV（`opencv_*.dll`）、TBB（`tbb*.dll`）、AI 模型文件（`.xml` + `.bin` 格式，如 `HumanSegV2.3.7_H.bin`） |
| `GifSDK/` | GIF 制作 SDK：`OpenCvSharp.dll` |
| `AntiPiracy/` | 防盗版模块 |
| `BugSplat/` | 崩溃上报 SDK |
| `DownloadRes/` | 下载底层（含 ffmpeg） |
| `markdig/` | Markdown 解析：`Markdig.dll` |

### 6.3 NuGet / 外部库引用（csproj 引用节选）

| 库 | 路径 | 用途 |
|----|------|------|
| `Aliyun.OSS` | `Tools/Aliyun.OSS.dll` | 阿里云对象存储 |
| `Magick.NET-Q16-AnyCPU` | `Tools/Magick.NET-Q16-AnyCPU.dll` | 图像处理 |
| `qrcodecreator` | `Tools/qrcodecreator.dll` | 二维码生成 |
| `OpenCvSharp` | `Library(x64)/GifSDK/OpenCvSharp.dll` | OpenCV C# 封装 |
| `Newtonsoft.Json` | `Library/JSON/` | JSON 序列化 |

---

## 7. 构建与打包体系

### 7.1 构建流程

```
WsVideoConvertUltimateBuild.bat
    │
    ├─ 调用 Visual Studio 2019
    │   devenv.exe KvProducts.sln /Build CBSRelease
    │   → 输出到 Src/Bin/Release/
    │
    ├─ 调用 Dotfuscator（混淆，仅正式包）
    │   dotfuscator.exe ...
    │
    ├─ 调用 Inno Setup 5
    │   ISCC.exe WsVideoConverterUltimate_Full.iss
    │   → 生成安装包到 Setup/Output/
    │
    └─ DigiCert 代码签名（正式包）
```

### 7.2 打包脚本说明

| 脚本 | 用途 |
|------|------|
| `WsVideoConvertUltimateBuild.bat` | 正式打包（国际版，含混淆+签名） |
| `WsVideoConvertUltimateBuild_zh.bat` | 正式打包（国内版） |
| `WsVideoConvertUltimateBuild_Test_zh.bat` | 测试打包（国内版，跳过部分步骤） |
| `UniConverterBuild.bat` | 本地验证打包（不混淆，无签名） |
| `PluginPackageBuild.bat` | 插件包单独打包（下载插件、特效插件、DVD 菜单等） |

### 7.3 Inno Setup 安装脚本

| ISS 文件 | 用途 |
|---------|------|
| `WsVideoConverterUltimate_Full.iss` | 国际版完整安装包 |
| `WsVideoConverterUltimate_Full_zh.iss` | 国内版完整安装包 |
| `WsVideoConverterUltimate_CBS.iss` | CBS（渠道）版本安装包 |
| `ProductUpdatePackge_zh.iss` | 国内版增量更新包 |
| `DVDMenuUpdateBuild_zh.iss` | DVD 菜单插件更新包 |

### 7.4 构建依赖工具

| 工具 | 版本 / 路径 | 说明 |
|------|------------|------|
| Visual Studio | 2019（`%VS160COMNTOOLS%`） | 主编译器 |
| Inno Setup | 5（`Setup/Inno Setup 5/ISCC.exe`） | 安装包打包 |
| Dotfuscator | `Setup/Dotfuscator/dotfuscator.exe` | 代码混淆（仅正式包） |
| DigiCert 证书 | — | 代码签名（仅正式包） |

---

## 8. 关键设计模式与约定

### 8.1 编译条件宏（国内/国际版分离）

代码中大量使用 `#if ZH` / `#else` 区分国内版与国际版：

```csharp
#if ZH
    public static string GoogleTrackingID = "UA-127156838-8";
    public static int ProductId = 4981;
#else
    public static string GoogleTrackingID = "UA-127156838-12";
    public static int ProductId = 14204;
#endif
```

影响范围：产品名称、ProductId、注册 Key、追踪 ID、支付渠道、更新地址等核心常量。

### 8.2 格式 ID 体系

所有媒体格式通过整数 ID 唯一标识，定义在 `ProductConst.cs`：

```
格式 ID 示例：
  MP4   = 1003001
  MKV   = 1005001
  ...
```

多个字典管理不同场景的格式映射：

| 字典名 | 用途 |
|--------|------|
| `DefaultOutputFormats` | 默认输出格式列表 |
| `compressOutputFormatdic` | 压缩功能可用格式 |
| `segmentCutterVideoOutputFormatdic` | 分段截取视频格式 |
| `segmentCutterDefaultOutputFormatdic` | 分段截取默认格式 |

`FormatManager` 模块（CommonModule）负责格式参数的运行时解析。

### 8.3 多语言文本获取

统一使用 WUL 本地化 API：

```csharp
WUL.Localization.ML.GetText(key, defaultValue)
```

字符串资源由 WUL 框架的 `.resx` 资源文件管理，支持运行时切换语言。

### 8.4 公共模块引入方式

CommonModule 源码以**文件链接**方式引入 VCU10.csproj，而非 DLL 引用：

```xml
<Compile Include="..\..\..\CommonModule\DownloadHelperService\ProtocolHandler.cs">
    <Link>Download\ProtocolHandler.cs</Link>
</Compile>
```

此方式保持单一编译产物，同时实现代码复用。

---

## 9. 数据流与任务调度

### 9.1 视频转换任务流

```
用户添加文件（UI 层 ConvertPage）
    │
    ↓
TaskManager（CommonModule）
    ├─ 任务队列管理（EncoderTasks）
    ├─ 格式参数解析（FormatManager）
    └─ 调度到 AgileTrans 转码引擎
            │
            ├─ x64/AgileTrans/Bin/Release/*.dll（底层编解码）
            └─ GPU 加速（GPUAccForAISDKHelper）

转换完成 → 结果回调 → UI 刷新（ConvertTotalProgressControl）
```

### 9.2 下载任务流

```
用户输入 URL（Download 页面）
    │
    ↓
ProtocolHandler（CommonModule/DownloadHelperService）
    → 协议解析（HTTP/RTMP/HLS 等）
    │
    ↓
DownloadMgr（CommonModule）
    → 调度到 Library(x64)/DownloadRes/（含 ffmpeg）
    │
    ↓
文件写入 → 进度回调 → UI 更新
```

### 9.3 AI 工具任务流

```
用户触发 AI 功能（ToolBox 页面）
    │
    ↓
AIPreAnalysis（Controller 层）
    → GPUAccForAISDKHelper 检测硬件
    │
    ↓
OpenVINO 推理（Library(x64)/AIResouces/inference_engine.dll）
    → 加载模型（.xml + .bin）
    → 推理执行（CPU/GPU）
    │
    ↓
结果后处理 → UI 渲染（预览/输出）
```

---

## 10. 多语言与皮肤系统

### 皮肤系统（WUL）

- **框架**：Wondershare UI Library（`WUL.Core.dll`、`WUL.Ctrls.dll`）
- **皮肤常量**（`ProductConst.cs`）：`SkinStart`、`SkinPause` 等控制皮肤状态
- **加载时机**：`Program.Main → LoadSkinAndLanguages()`
- 皮肤资源打包在安装包内，支持动态切换

### 多语言系统

- **API**：`WUL.Localization.ML.GetText(key, defaultValue)`
- **资源管理**：WUL Localization 框架管理语言包
- **支持语言**：国际版支持英语等多国语言，国内版（ZH）独立语言包
- **热键本地化**：热键文本同样走多语言通道

---

## 11. AI 能力集成

AI 功能集中在 `Source/ToolBox/` 和 `Source/AIPortait/` 模块，底层依赖 OpenVINO 推理框架。

### AI 工具箱功能矩阵

| 工具名 | 模块目录 | 核心技术 |
|--------|---------|---------|
| AI 视频增强 | `ToolBox/NewAI/` | OpenVINO 超分辨率 |
| 人像抠图 | `AIPortait/` | HumanSeg 语义分割模型 |
| 视频防抖 | `ToolBox/FixVideoShake/` | 光流稳定算法 |
| 智能重新构图 | `ToolBox/AutoReFrame/` | 目标检测 + 跟踪 |
| 人声提取 | `ToolBox/VocalRemover/` | 音频分离模型 |
| 噪声消除 | `ToolBox/NoiseRemover/` | 音频降噪模型 |
| 语音转文字（STT） | `ToolBox/STT/` | 语音识别模型 |
| 文字转语音（TTS） | `ToolBox/TTS/` | 语音合成模型 |
| 水印去除 | `ToolBox/WaterMarker/` | 图像修复（Inpainting） |
| GIF 制作 | `ToolBox/GifMaker/` | OpenCvSharp 帧处理 |
| 图片格式转换 | `ToolBox/ImageConverter/` | Magick.NET |

### AI 推理基础设施

```
Library(x64)/AIResouces/
├── inference_engine.dll          ← OpenVINO 推理引擎核心
├── libprotobuf.dll               ← Protocol Buffers
├── opencv_*.dll                  ← OpenCV 计算机视觉
├── tbb*.dll                      ← TBB 并行计算
└── models/
    ├── humansegmentation/
    │   ├── HumanSegV2.3.7_H.xml  ← 模型结构
    │   └── HumanSegV2.3.7_H.bin  ← 模型权重
    └── ...（其他 AI 模型）
```

---

## 12. 崩溃上报与数据采集

### 崩溃上报（BugSplat）

- **SDK**：`Library(x64)/BugSplat/`
- **数据库名**：`Wondershare_UniConverter2019`
- **初始化**：`Program.InitBugSplat()` 在主窗体创建前调用
- **钩子**：注册 `Application.ThreadException` 和 `AppDomain.UnhandledException` 事件

### 数据采集体系

多个采集渠道并行上报，受用户隐私协议开关控制：

| 采集组件 | 文件 | 平台 |
|---------|------|------|
| Google Analytics | `GATracker.cs` | Google（国际版） |
| Sparrow | `BaseDll/DataCollect/Sparrow/` | Wondershare 内部平台 |
| 神策数据 | `BaseDll/DataCollect/sensors_analytics/` | 国内版数据平台 |

- **初始化**：`UserDataCollectHelper.Init()`（含隐私开关检查）
- **退出清理**：`ExitDataCollect()`
- **隐私控制**：`UserPrivacyPolicyMgr` 根据 Setup 参数决定是否启用采集

---

## 13. 国内版 / 国际版差异（ZH 编译宏）

| 差异项 | 国际版（默认） | 国内版（`#if ZH`） |
|--------|--------------|------------------|
| Google Tracking ID | `UA-127156838-12` | `UA-127156838-8` |
| Sparrow ID | `UA-127156838-11` | `UA-127156838-8` |
| ProductId | `14204` | `4981` |
| 注册 Key（keyn） | `1463282851` | 不同值 |
| 注册 Key（keyd） | `149831635` | 不同值 |
| GUID | `{C5C29D92-759E-41B3-8CD7-87ED305AEC83}` | 不同值 |
| 数据采集 | Google Analytics + Sparrow | 神策数据 + Sparrow |
| 打包脚本 | `WsVideoConvertUltimateBuild.bat` | `WsVideoConvertUltimateBuild_zh.bat` |
| ISS 安装脚本 | `_Full.iss` | `_Full_zh.iss` |

---

## 14. 关键常量速查

> 位于 `Src/ProductUI/VideoConverterUltimate10/Source/ProductConst.cs`

| 常量名 | 值（国际版） | 说明 |
|--------|------------|------|
| `bugsplatDatabaseName` | `"Wondershare_UniConverter2019"` | BugSplat 崩溃数据库 |
| `GoogleTrackingID` | `"UA-127156838-12"` | Google 分析追踪 ID |
| `SparrowID` | `"UA-127156838-11"` | Sparrow 平台追踪 ID |
| `ProductId` | `14204` | 产品 ID |
| `keyn` | `1463282851` | 注册验证密钥 N |
| `keyd` | `149831635` | 注册验证密钥 D |
| `defaultGuid` | `{C5C29D92-759E-41B3-8CD7-87ED305AEC83}` | 默认产品 GUID |
| `DVD5Size` | — | DVD5 容量常量（约 4.7 GB） |
| `DVD9Size` | — | DVD9 容量常量（约 8.5 GB） |
| `BD25Size` | — | 蓝光 BD25 容量常量 |

### Mutex 名称（防多开）

| Mutex 名称 | 作用 |
|-----------|------|
| `"Wondershare UniConverter 14"` | 与 V14 版本互斥 |
| `"Wondershare UniConverter 15"` | 与 V15 版本互斥 |
| `"{CompanyName} {ProductName}"` | 当前版本自身互斥 |
| `"VCUDownloadUpdaterMutex"` | 更新下载器互斥 |

---

*文档由 Claude Code 辅助生成，基于项目源码静态分析。如有疑问或需补充，请联系项目架构负责人。*
