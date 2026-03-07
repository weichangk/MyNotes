# 使用 WinDbg 分析 .dmp 文件完整指南

## 目录

- [一、WinDbg 简介](#一windbg-简介)
- [二、环境准备](#二环境准备)
- [三、生成 Dump 文件](#三生成-dump-文件)
- [四、打开 Dump 文件](#四打开-dump-文件)
- [五、符号（Symbol）配置](#五符号symbol配置)
- [六、核心分析命令](#六核心分析命令)
- [七、常见崩溃场景分析实战](#七常见崩溃场景分析实战)
- [八、高级技巧](#八高级技巧)
- [九、常见问题与排错](#九常见问题与排错)
- [十、附录：命令速查表](#十附录命令速查表)

---

## 一、WinDbg 简介

WinDbg 是微软提供的免费调试器，支持用户态和内核态调试，是 Windows 平台下分析崩溃转储文件（`.dmp`）的首选工具。

### 1.1 WinDbg 版本

| 版本 | 说明 |
|------|------|
| **WinDbg (Classic)** | 传统版本，包含在 Windows SDK 中 |
| **WinDbg Preview** | 新版 UI，可从 Microsoft Store 安装，推荐使用 |

### 1.2 Dump 文件类型

| 类型 | 说明 | 大小 |
|------|------|------|
| **Mini Dump** | 仅包含线程栈、加载模块列表等基本信息 | 较小（几百 KB ~ 几 MB） |
| **Full Dump** | 包含进程完整内存空间 | 较大（与进程内存占用相当） |
| **Kernel Dump** | 内核态转储，用于蓝屏分析 | 视类型而定 |

---

## 二、环境准备

### 2.1 安装 WinDbg

**方式一：通过 Microsoft Store 安装 WinDbg Preview（推荐）**

在 Microsoft Store 搜索 "WinDbg Preview" 并安装。

**方式二：通过 Windows SDK 安装**

1. 下载 [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/)
2. 安装时仅勾选 **Debugging Tools for Windows**
3. 安装完成后在以下路径找到 WinDbg：
   ```
   C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\windbg.exe   (64位)
   C:\Program Files (x86)\Windows Kits\10\Debuggers\x86\windbg.exe   (32位)
   ```

> ⚠️ **注意**：分析 64 位进程的 Dump 文件必须使用 64 位的 WinDbg，32 位同理。

### 2.2 准备符号文件（.pdb）

确保拥有与 Dump 文件对应版本的 `.pdb` 符号文件。编译项目时建议：

- **Release 版本也生成 PDB**（CMake 中配置方式见下文）
- 将 PDB 文件与版本号对应归档保存

**CMake 中为 Release 生成 PDB：**

```cmake
# 为所有配置生成调试信息
if(MSVC)
    # 编译器选项：生成完整调试信息
    add_compile_options(/Zi)
    # 链接器选项：生成 PDB 文件
    add_link_options(/DEBUG /OPT:REF /OPT:ICF)
endif()
```

### 2.3 准备源代码

如果需要源码级调试，准备好与 Dump 文件对应版本的源代码。

---

## 三、生成 Dump 文件

### 3.1 程序崩溃时自动生成

**方式一：通过代码注册异常处理**

```cpp
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")

LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo)
{
    HANDLE hFile = CreateFileW(
        L"crash.dmp",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION mei;
        mei.ThreadId = GetCurrentThreadId();
        mei.ExceptionPointers = pExceptionInfo;
        mei.ClientPointers = FALSE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            // MiniDumpNormal       — 最小转储
            // MiniDumpWithFullMemory — 完整内存转储
            MiniDumpWithFullMemory,
            &mei,
            NULL,
            NULL
        );

        CloseHandle(hFile);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

int main()
{
    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
    // ... 程序逻辑
}
```

**方式二：配置 Windows 注册表（全局 WER）**

```reg
Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps]
"DumpFolder"="C:\\CrashDumps"
"DumpType"=dword:00000002
"DumpCount"=dword:0000000a
```

| DumpType 值 | 含义 |
|-------------|------|
| 0 | Custom Dump |
| 1 | Mini Dump |
| 2 | Full Dump |

### 3.2 手动抓取正在运行进程的 Dump

**方式一：任务管理器**

1. 打开任务管理器 → 详细信息
2. 右键目标进程 → **创建转储文件**
3. 默认保存到 `%LOCALAPPDATA%\Temp\`

**方式二：使用 procdump（推荐）**

```powershell
# 下载 procdump：https://docs.microsoft.com/en-us/sysinternals/downloads/procdump

# 进程崩溃时自动抓取
procdump -e -ma <PID> C:\dumps\

# 立即抓取 Full Dump
procdump -ma <PID> C:\dumps\crash.dmp

# CPU 超过 90% 持续 5 秒时抓取
procdump -ma -c 90 -s 5 <PID> C:\dumps\
```

---

## 四、打开 Dump 文件

### 4.1 基本打开方式

1. 启动 WinDbg
2. `File` → `Open Crash Dump`（快捷键 `Ctrl+D`）
3. 选择 `.dmp` 文件
4. WinDbg 会自动加载并显示初始分析信息

### 4.2 命令行打开

```powershell
windbg -z "C:\dumps\crash.dmp"
```

### 4.3 打开后的初始输出

打开 Dump 后，WinDbg 会自动输出以下信息：

```
Loading Dump File [C:\dumps\crash.dmp]
User Mini Dump File with Full Memory: Only application data is available

Windows 10 Version 19041 MP (8 procs) Free x64
Product: WinNt, suite: SingleUserTS
...
This dump file has an exception of interest stored in it.
The stored exception information can be accessed via .ecxr.
```

---

## 五、符号（Symbol）配置

### 5.1 设置符号路径

符号配置是 Dump 分析的**最关键步骤**。在 WinDbg 命令窗口中输入：

```
.sympath srv*C:\Symbols*https://msdl.microsoft.com/download/symbols
```

这条命令的含义：
- `srv*` — 使用符号服务器
- `C:\Symbols` — 本地符号缓存目录
- `https://msdl.microsoft.com/download/symbols` — 微软公共符号服务器

### 5.2 添加自己项目的符号路径

```
.sympath+ C:\MyProject\build\Release
```

> `+` 号表示追加路径，不覆盖已有配置。

### 5.3 常用符号相关命令

```
.sympath                    ; 查看当前符号路径
.sympath+ <path>            ; 追加符号路径
.symfix                     ; 设置微软符号服务器（快捷方式）
.symfix+ C:\Symbols         ; 设置微软符号服务器并指定缓存
.reload                     ; 重新加载符号
.reload /f <module.dll>     ; 强制重新加载指定模块符号
ld <module>                 ; 加载指定模块的符号
lm                          ; 列出所有加载的模块
lm vm <module>              ; 查看模块详细信息（含符号状态）
!sym noisy                  ; 开启符号加载详细日志（调试符号问题时用）
!sym quiet                  ; 关闭符号加载详细日志
!chksym <module>            ; 检查模块符号匹配情况
```

### 5.4 设置源代码路径

```
.srcpath C:\MyProject\src
.srcpath+ C:\MyProject\include
```

### 5.5 在环境变量中永久配置

可以通过设置环境变量来永久配置符号路径：

```
变量名：_NT_SYMBOL_PATH
变量值：srv*C:\Symbols*https://msdl.microsoft.com/download/symbols;C:\MyProject\build\Release
```

---

## 六、核心分析命令

### 6.1 自动分析（首选）

```
!analyze -v
```

这是最重要的命令，会自动分析崩溃原因，输出包括：

- **EXCEPTION_RECORD** — 异常记录
- **FAULTING_IP** — 出错的指令地址
- **EXCEPTION_CODE** — 异常代码
- **STACK_TEXT** — 崩溃时的调用栈
- **MODULE_NAME** — 出错所在模块
- **FOLLOWUP_IP** — 最可能出错的位置

### 6.2 查看调用栈

```
k           ; 简略调用栈
kb          ; 调用栈 + 前 3 个参数
kp          ; 调用栈 + 完整参数（需符号）
kv          ; 调用栈 + FPO 信息和调用约定
kn          ; 调用栈 + 帧编号
kf          ; 调用栈 + 栈帧大小
kL          ; 隐藏源码行号
k 20        ; 显示前 20 帧
~*k         ; 所有线程的调用栈
~0k         ; 0 号线程的调用栈
~#k         ; 当前崩溃线程的调用栈
```

### 6.3 切换线程与上下文

```
~           ; 列出所有线程
~<N>s       ; 切换到第 N 号线程
~*          ; 显示所有线程信息
.ecxr       ; 切换到异常上下文（分析崩溃必用）
.cxr        ; 恢复默认上下文
```

### 6.4 查看寄存器

```
r               ; 查看所有寄存器
r eax           ; 查看单个寄存器
r eax=0         ; 修改寄存器值（活动调试时）
```

### 6.5 内存查看

```
db <address>        ; 以字节显示内存
dw <address>        ; 以字（2字节）显示
dd <address>        ; 以双字（4字节）显示
dq <address>        ; 以四字（8字节）显示
da <address>        ; 以 ASCII 字符串显示
du <address>        ; 以 Unicode 字符串显示
dp <address>        ; 以指针大小显示
dps <address>       ; 显示内存中的指针及其对应的符号

db <address> L100   ; 显示 256 字节
```

### 6.6 查看变量与数据结构

```
dt <TypeName>                       ; 显示类型结构定义
dt <TypeName> <address>             ; 以指定类型解析地址处的数据
dt -r <TypeName> <address>          ; 递归展开子结构
dv                                  ; 显示当前帧的局部变量
dv /V                               ; 显示变量的地址和类型
?? <expression>                     ; C++ 表达式求值
? <expression>                      ; MASM 表达式求值
```

### 6.7 模块相关

```
lm                      ; 列出所有已加载模块
lm vm <name>            ; 查看指定模块详细信息
!lmi <module>           ; 模块详细信息（含时间戳、校验和）
!dh <module_base>       ; 显示模块 PE 头信息
.chain                  ; 查看调试器扩展加载链
```

### 6.8 搜索

```
s -a <start> L<length> "string"     ; 在内存中搜索 ASCII 字符串
s -u <start> L<length> "string"     ; 在内存中搜索 Unicode 字符串
s -d <start> L<length> <DWORD>      ; 在内存中搜索 DWORD 值
x <module>!<pattern>                ; 搜索符号（支持通配符）
x MyApp!*Init*                      ; 搜索 MyApp 模块中含 Init 的符号
```

---

## 七、常见崩溃场景分析实战

### 7.1 空指针访问（Access Violation）

**典型异常代码**：`0xC0000005`

```
0:000> !analyze -v
...
EXCEPTION_CODE: c0000005 (Access violation)
EXCEPTION_RECORD: ... - Read using address 0000000000000000
...
```

**分析步骤**：

1. 执行 `!analyze -v` 查看自动分析
2. 执行 `.ecxr` 切换到异常上下文
3. 执行 `k` 查看调用栈
4. 根据调用栈定位到源代码中的空指针

```
0:000> .ecxr
0:000> k
Child-SP          RetAddr           Call Site
00000000`0014f6a0 00007ff6`12345678 MyApp!CMyClass::DoSomething+0x3a
00000000`0014f6e0 00007ff6`12345abc MyApp!CMyClass::ProcessData+0x12
...
```

在这个例子中，崩溃发生在 `CMyClass::DoSomething` 偏移 `0x3a` 处。如果有对应的 PDB 和源码，可以直接看到行号。

### 7.2 栈溢出（Stack Overflow）

**典型异常代码**：`0xC00000FD`

```
0:000> !analyze -v
EXCEPTION_CODE: c00000fd (Stack overflow)
```

**分析步骤**：

1. 查看调用栈 `k 200`，寻找递归调用模式
2. 通常能看到大量重复的函数调用帧

```
0:000> k 200
...
MyApp!CParser::Parse+0x55
MyApp!CParser::ParseNode+0x120
MyApp!CParser::Parse+0x55          ; ← 递归
MyApp!CParser::ParseNode+0x120
MyApp!CParser::Parse+0x55          ; ← 递归
...
```

### 7.3 堆损坏（Heap Corruption）

**典型异常代码**：`0xC0000374`

```
0:000> !analyze -v
EXCEPTION_CODE: c0000374 (Heap corruption)
```

**分析步骤**：

1. 执行 `!heap -s` 查看堆状态摘要
2. 执行 `!heap -a <heap_address>` 查看特定堆的详细信息
3. 开启页堆验证以便更早发现问题：

```powershell
# 使用 gflags 开启页堆（需要重新运行程序复现）
gflags /p /enable MyApp.exe /full
```

### 7.4 死锁（Deadlock / Hang）

Dump 来自卡死/无响应的程序：

```
0:000> !locks                  ; 查看临界区信息
0:000> !cs -l                  ; 列出所有活跃的临界区
0:000> ~*kvn                   ; 查看所有线程调用栈

; 分析步骤：
; 1. 找到等待锁的线程
; 2. 找到持有锁的线程
; 3. 检查是否存在循环等待
```

### 7.5 C++ 异常未捕获

**典型异常代码**：`0xE06D7363`（MSVC C++ 异常）

```
0:000> .ecxr
0:000> k
0:000> !analyze -v

; 查看 C++ 异常对象
0:000> .exr -1
```

### 7.6 非法指令（Illegal Instruction）

**典型异常代码**：`0xC000001D`

```
0:000> .ecxr
0:000> u @rip          ; 反汇编当前指令指针处的代码
0:000> ub @rip         ; 反汇编当前指令之前的代码
```

---

## 八、高级技巧

### 8.1 Time Travel Debugging (TTD)

WinDbg Preview 支持时间旅行调试，可以录制程序执行过程并回放：

```powershell
# 录制（在 WinDbg Preview 中）
# File → Start debugging → Launch executable (advanced) → 勾选 "Record with Time Travel Debugging"
```

录制后可以使用：
```
!tt 0           ; 跳到录制起点
!tt 100         ; 跳到录制终点
!tt 50          ; 跳到 50% 的位置
g-              ; 反向执行
p-              ; 反向单步
```

### 8.2 使用脚本自动化分析

**WinDbg 脚本文件（.wds）：**

```
$$ auto_analyze.wds
$$ 自动分析脚本

.symfix+ C:\Symbols
.reload

!analyze -v

.echo "========== All Thread Stacks =========="
~*kv

.echo "========== Loaded Modules =========="
lm

.echo "========== Locks =========="
!locks

.logclose
```

运行脚本：
```
$$><C:\scripts\auto_analyze.wds
```

**导出分析日志：**
```
.logopen C:\analysis\result.log      ; 开始记录
!analyze -v
~*kv
.logclose                            ; 停止记录
```

### 8.3 使用 SOS 分析 .NET 混合应用

如果程序包含 .NET 组件（如 C++/CLI）：

```
.loadby sos clr                      ; .NET Framework
.loadby sos coreclr                  ; .NET Core / .NET 5+
!clrstack                            ; 查看托管调用栈
!dumpheap -stat                      ; 堆统计
!dumpobj <address>                   ; 查看托管对象
```

### 8.4 比较两个 Dump 文件

当需要对比正常与异常两种状态时，可以打开两个 WinDbg 实例分别加载 Dump 进行比较。

### 8.5 使用扩展命令

```
!peb                    ; 查看进程环境块
!teb                    ; 查看线程环境块
!handle                 ; 显示句柄信息
!handle 0 f             ; 显示所有句柄的详细信息
!address                ; 显示进程虚拟地址空间摘要
!address -summary       ; 内存使用摘要
!vprot <address>        ; 查看虚拟内存保护属性
!runaway                ; 查看线程 CPU 占用时间（排查高 CPU）
!threadpool             ; 查看线程池信息（.NET）
```

### 8.6 反汇编与断点

```
u <address>             ; 反汇编（从指定地址向后）
ub <address>            ; 反汇编（从指定地址向前）
uf <function>           ; 反汇编整个函数
u @rip L20              ; 从当前指令指针开始反汇编 20 条
```

---

## 九、常见问题与排错

### 9.1 符号加载失败

**症状**：调用栈只显示模块名和偏移，没有函数名

```
MyApp!+0x12345
ntdll!+0x6789a
```

**排查步骤**：

```
!sym noisy              ; 开启符号加载诊断
.reload /f MyApp.exe    ; 强制重载
lm vm MyApp             ; 查看模块的符号加载状态
!chksym MyApp           ; 检查符号匹配
```

**常见原因**：
- PDB 文件与 EXE/DLL 版本不匹配（时间戳或 GUID 不一致）
- PDB 路径未正确添加到 `.sympath`
- 编译时未生成 PDB 文件

### 9.2 Dump 位数不匹配

**症状**：打开 Dump 后出现异常或调用栈不正确

**解决**：
- 32 位进程的 Dump → 使用 32 位 WinDbg
- 64 位进程的 Dump → 使用 64 位 WinDbg

如果使用 WinDbg Preview 打开 32 位 Dump，需要切换上下文：
```
.load wow64exts
!sw               ; 切换到 WoW64 (32位) 模式
```

### 9.3 调用栈不完整或混乱

**可能原因**：
- 栈被破坏（缓冲区溢出）
- 缺少符号导致帧无法正确回溯
- FPO（Frame Pointer Omission）优化导致

**尝试修复**：
```
.ecxr               ; 先切换到异常上下文
kb                   ; 尝试不同的调用栈命令
kv
kP
!uniqstack           ; 去重后显示所有线程调用栈
```

### 9.4 分析 Mini Dump 时信息不足

Mini Dump 信息有限，如果需要更多信息：
- 改用 Full Dump 重新抓取
- 如果只有 Mini Dump，核心分析仍然可行：

```
.ecxr
k
!analyze -v
lm          ; 模块列表通常都包含在 Mini Dump 中
```

### 9.5 "No export xxx found" 错误

```
; 确保加载了正确的扩展
.chain              ; 查看已加载的扩展
.load <ext.dll>     ; 手动加载扩展
.extpath+ <path>    ; 添加扩展搜索路径
```

---

## 十、附录：命令速查表

### 基础流程命令

| 命令 | 说明 |
|------|------|
| `!analyze -v` | 🔥 **自动分析崩溃（最常用）** |
| `.ecxr` | 切换到异常上下文 |
| `k` / `kb` / `kp` / `kv` | 查看调用栈 |
| `~*k` | 查看所有线程调用栈 |
| `r` | 查看寄存器 |
| `lm` | 列出所有模块 |
| `dv` | 查看局部变量 |

### 符号相关

| 命令 | 说明 |
|------|------|
| `.sympath` | 查看/设置符号路径 |
| `.sympath+ <path>` | 追加符号路径 |
| `.symfix` | 设置微软符号服务器 |
| `.reload` | 重新加载符号 |
| `.reload /f <mod>` | 强制加载指定模块符号 |
| `!sym noisy` | 开启符号诊断日志 |

### 内存与数据

| 命令 | 说明 |
|------|------|
| `db` / `dw` / `dd` / `dq` | 按不同粒度查看内存 |
| `da` / `du` | 查看 ASCII / Unicode 字符串 |
| `dps` | 显示内存中的符号指针 |
| `dt <Type> <addr>` | 按类型解析内存 |
| `s -a <start> L<len> "str"` | 搜索字符串 |

### 线程与同步

| 命令 | 说明 |
|------|------|
| `~` | 列出所有线程 |
| `~<N>s` | 切换到第 N 号线程 |
| `!locks` | 查看临界区锁信息 |
| `!runaway` | 查看线程 CPU 时间 |
| `!cs -l` | 列出活跃的临界区 |

### 堆与内存

| 命令 | 说明 |
|------|------|
| `!heap -s` | 堆摘要 |
| `!heap -a <addr>` | 指定堆的详细信息 |
| `!address -summary` | 虚拟内存摘要 |
| `!vprot <addr>` | 内存页保护属性 |

### 异常代码速查

| 异常代码 | 含义 |
|----------|------|
| `0xC0000005` | Access Violation（访问违规） |
| `0xC0000008` | Invalid Handle（无效句柄） |
| `0xC000001D` | Illegal Instruction（非法指令） |
| `0xC0000094` | Integer Division by Zero（整数除零） |
| `0xC00000FD` | Stack Overflow（栈溢出） |
| `0xC0000374` | Heap Corruption（堆损坏） |
| `0xE06D7363` | C++ Exception（MSVC C++ 异常） |
| `0xC0000409` | Stack Buffer Overrun（栈缓冲区溢出，/GS 检测） |
| `0x80000003` | Breakpoint（断点异常） |
| `0xC0000602` | Unknown Exception（未知异常） |

---

## 参考资料

- [WinDbg 官方文档](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/)
- [Debugging Tools for Windows](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools)
- [Microsoft Public Symbol Server](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/microsoft-public-symbols)
- [Common WinDbg Commands](https://learn.microsoft.com/en-us/windows-hardware/drivers/debuggercmds/)
