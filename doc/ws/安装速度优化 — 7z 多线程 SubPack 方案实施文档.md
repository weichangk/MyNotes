帮我评估以下下面的技术方案
# 安装速度优化 — 7z 多线程 SubPack 方案实施文档

> 项目：Wondershare UniConverter（VideoConvertUltimate-Windows）
> 分支：`InstallSpeed`

---

## 一、背景与问题

**核心发现：Inno Setup 解压本身不是瓶颈。** 真正的耗时来源于 IS5 运行时对每个文件的逐一处理（I/O 写入、注册表记录等单线程串行操作）。

### 已排除的优化方向

| 方向 | 实测结果 | 结论 |
|------|---------|------|
| lzma/ultra → lzma2/fast | 节省 25 秒 | 不是主要瓶颈 |
| Inno Setup 5 → Inno Setup 6 | 反而慢了 2 分 05 秒 | 排除 |

### 新思路：绕过 IS 运行时，用 7z 直接多线程解压

- 将所有安装文件预先打成 `.7z` 压缩包（SubPack）
- IS 安装包只负责将 SubPack 和 7z.exe 释放到临时目录
- 安装时调用 `7z.exe x ... -mmt=on` 多线程解压到目标目录
- **完全绕过 IS5 单线程文件提取逻辑**，充分利用多核 CPU 和高速 I/O

---

## 二、方案架构

### 整体流程

```
构建阶段（开发机）                    安装阶段（用户机）
─────────────────────                ──────────────────────────────────
Temp\（VS 构建输出）                  IS 运行时（单线程，仅 4 个文件）
    │                                    │
    ▼                                    ▼
Build_7z_SubPacks.bat            释放 7z.exe + 7z.dll → {tmp}\7z\
    │                                    │
    ├─ SubPack1.7z  ◄── staging 采集     ├─ 释放 SubPack1.7z → {tmp}\
    └─ SubPack2.7z  ◄── staging 采集     └─ 释放 SubPack2.7z → {tmp}\
                                         │
                                         │  AfterInstall 回调
                                         │
                                    7z.exe x SubPack1.7z → {app}\               （多线程）
                                    7z.exe x SubPack2.7z → {commonappdata}\Wondershare\  （多线程）
```

### SubPack 分包设计

| 包名 | 解压目标 | 内容 |
|------|---------|------|
| `UniConverter SubPack1.7z` | `{app}` | 主程序所有可执行文件、DLL、驱动、转码引擎等 |
| `UniConverter SubPack2.7z` | `{commonappdata}\Wondershare` | 语言包、示例媒体、字幕预设、DVD 菜单、浏览器扩展、下载服务等 |

---

## 三、文件清单

### 新增/修改文件

| 文件 | 说明 |
|------|------|
| `Setup\Build_7z_SubPacks.bat` | 生成 CBS / Full 两套 SubPack1.7z 和 SubPack2.7z |
| `Setup\WsVideoConverterUltimate_CBS_7z.iss` | CBS 7z 版安装脚本 |
| `Setup\WsVideoConverterUltimate_Full_7z.iss` | Full 7z 版安装脚本 |
| `Setup\Build_7z_ExcludeTempExtras.txt` | Pass B xcopy 排除列表（CBS 不安装的文件）|
| `Setup\Build_7z_ExcludeTransferFiles.txt` | Pass B xcopy 排除列表（Transfer 子目录文件）|
| `Setup\zip\7z\7z.exe` | 7-Zip 命令行工具 |
| `Setup\zip\7z\7z.dll` | 7-Zip 运行时库 |

---

## 四、构建脚本说明

### Build_7z_SubPacks.bat

**职责：** 生成两个 SubPack 压缩包，支持 CBS（默认）和 Full 两种模式。

```bat
Build_7z_SubPacks.bat        -- CBS 模式
Build_7z_SubPacks.bat Full   -- Full 模式
```

**SubPack1 打包逻辑：**

分两个 Pass 向 staging 目录采集文件，再一次性压缩：

- **Pass A**：从 Library/Tools/Resource/Resources 等源目录按 CBS.iss [Files] 条目逐一复制
- **Pass B**：将 `Temp\`（VS 构建 + Dotfuscator 输出）合并进 staging，使用排除列表过滤不需要的文件

```bat
"%SEVENZIP%" a -bsp1 -bso1 -y -mmt=on -mx=1 "SubPack1.7z" "%STAGING_APP%\*"
```

- `-mx=1`：最快压缩，安装时解压速度最快
- `-mmt=on`：多线程压缩

**SubPack2 采集结构：**

```
%TEMP%\VCU_SubPack2_Staging\
├── UniConverter 17\
│   ├── Lng\                  ← Resources\Wondershare\Language\*
│   ├── Samples\              ← Resource\Samples\（CBS 仅水印去除，Full 全部）
│   ├── SubtitlePresets\      ← Resource\SubtitlePresets\*
│   ├── MenuRes\              ← Resource\DVDTemplate\InstallTemplate\*
│   ├── GlobalConfig.dat      ← Resources\Common\VCGlobalConfigForV17-4.json（改名）
│   ├── CustomEncodeParamInfos.dat
│   └── PrivateSpaceDomain.json
└── extensions\
    ├── downloadHelper\
    ├── uniconverter_firefox.xpi
    └── Service\
        ├── WUL.*.dll
        ├── UniConvertDownloadService.exe / .config
        ├── NativeHost.exe / .config
        ├── SensorsAnalytics *.dll
        └── installer.json
```

**注意事项：**

- 所有 `xcopy` 调用必须加 `/i` 标志，避免单文件复制时弹出交互提示卡死
- 排除列表文件（`Build_7z_Exclude*.txt`）必须使用 **CRLF 行尾**，否则 xcopy 无法正确解析每行
- 每次打包前删除旧 .7z 文件，避免 `7z a` 追加模式保留历史文件
- `.exe.config` 单文件复制使用 `copy` 命令而非 `xcopy`

### ISS 关键修改说明

`_CBS_7z.iss` / `_Full_7z.iss` 基于对应原版 ISS 修改，原 `[Files]` 第二段（约 274 行文件条目）完全替换为：

```ini
Source: "zip\7z\7z.exe"; DestDir: "{tmp}\7z"; Flags: ignoreversion
Source: "zip\7z\7z.dll"; DestDir: "{tmp}\7z"; Flags: ignoreversion
Source: "Output\SubPack\UniConverter SubPack1.7z"; DestDir: "{tmp}"; AfterInstall: AfterSubPack1Install
Source: "Output\SubPack\UniConverter SubPack2.7z"; DestDir: "{tmp}"; AfterInstall: AfterSubPack2Install
```

压缩配置改为 `Compression=none`，原因：SubPack 已是 7z 压缩包，二次压缩无收益；IS5 不支持逐文件 `dontcompress`，只能全局关闭。

`[Code]` 段新增 Pascal 解压函数：

```pascal
function UnCompress7zip(const AArchivePath, ADestDir: string): Integer;
// 调用 {tmp}\7z\7z.exe x <archive> -o<dest> -mmt=on -y

procedure AfterSubPack1Install;
// 解压 SubPack1.7z → {app}

procedure AfterSubPack2Install;
// 解压 SubPack2.7z → {commonappdata}\Wondershare
```

`WSUtilities.dll` 条目加 `uninsneveruninstall` 标志，确保卸载程序初始化阶段能找到该 DLL（由 `DelTree` 在卸载末尾统一清理）。

---

## 五、构建操作步骤

```
步骤 1：确认前置条件
  ✅ Setup\Temp\ 已由 VS 构建流水线填充
  ✅ Setup\zip\7z\7z.exe 和 7z.dll 已放置

步骤 2：生成 SubPack 压缩包
  运行：Build_7z_SubPacks.bat          → CBS 模式
  运行：Build_7z_SubPacks.bat Full     → Full 模式
  产出：Output\SubPack\UniConverter SubPack1.7z
        Output\SubPack\UniConverter SubPack2.7z

步骤 3：编译安装包（正式打包机）
  WsVideoConvertUltimateBuild.bat 会自动调用 Build_7z_SubPacks.bat
  产出：Output\UniConverterSetup.exe（CBS）
        Output\UniConverterSetup_Full.exe（Full）
```

---

*文档生成工具：Claude Code*
