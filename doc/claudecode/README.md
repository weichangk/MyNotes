# Claude Code 使用知识文档

> 基于实际项目经验整理的 Claude Code 全面指南，涵盖安装配置、核心用法、项目定制、高效工作流与实战技巧。

---

## 目录

- [1. 什么是 Claude Code](#1-什么是-claude-code)
- [2. 安装与配置](#2-安装与配置)
- [3. CLAUDE.md — 项目记忆文件](#3-claudemd--项目记忆文件)
- [4. 核心命令与快捷操作](#4-核心命令与快捷操作)
- [5. 日常开发工作流](#5-日常开发工作流)
- [6. 提示词工程（Prompt Engineering）](#6-提示词工程prompt-engineering)
- [7. 多文件与大型项目实战](#7-多文件与大型项目实战)
- [8. 调试与问题排查](#8-调试与问题排查)
- [9. 代码审查与重构](#9-代码审查与重构)
- [10. Git 集成工作流](#10-git-集成工作流)
- [11. 测试驱动开发](#11-测试驱动开发)
- [12. 成本控制与性能优化](#12-成本控制与性能优化)
- [13. 安全与隐私](#13-安全与隐私)
- [14. VS Code 集成（Copilot Chat + Claude）](#14-vs-code-集成copilot-chat--claude)
- [15. 实战案例](#15-实战案例)
- [16. 常见问题与排错](#16-常见问题与排错)
- [17. 最佳实践清单](#17-最佳实践清单)

---

## 1. 什么是 Claude Code

Claude Code 是 Anthropic 推出的 **命令行 AI 编程助手**，可以：

- 直接读写项目文件、执行终端命令
- 理解整个代码库的上下文
- 执行多步骤复杂任务（搜索 → 分析 → 修改 → 验证）
- 与 Git、构建系统、测试框架深度集成

**与传统 AI Chat 的区别：**

| 特性 | 传统 AI Chat | Claude Code |
|------|-------------|-------------|
| 文件访问 | 手动粘贴代码 | 直接读写项目文件 |
| 命令执行 | 不支持 | 可运行终端命令 |
| 上下文范围 | 单次对话 | 整个代码库 |
| 多步骤任务 | 需手动衔接 | 自动规划并执行 |
| 结果验证 | 人工检查 | 可自动编译/测试验证 |

**运行模式：**
- **CLI 模式**：在终端中直接使用 `claude` 命令
- **VS Code 集成模式**：通过 GitHub Copilot Chat 选择 Claude 模型

---

## 2. 安装与配置

### 2.1 CLI 安装

```bash
# 使用 npm 全局安装
npm install -g @anthropic-ai/claude-code

# 验证安装
claude --version

# 首次运行，进行认证
claude
```

### 2.2 认证方式

```bash
# 方式1：交互式登录（推荐）
claude login

# 方式2：环境变量
export ANTHROPIC_API_KEY=sk-ant-xxxxx

# 方式3：.env 文件（项目根目录）
echo "ANTHROPIC_API_KEY=sk-ant-xxxxx" > .env
```

### 2.3 全局配置

配置文件位置：`~/.claude/settings.json`

```json
{
  "permissions": {
    "allow": [
      "Read",
      "Edit",
      "Bash(git *)",
      "Bash(npm *)",
      "Bash(cmake *)"
    ],
    "deny": [
      "Bash(rm -rf /)",
      "Bash(sudo *)"
    ]
  },
  "model": "claude-sonnet-4-20250514"
}
```

### 2.4 项目级配置

在项目根目录创建 `.claude/settings.json`：

```json
{
  "permissions": {
    "allow": [
      "Bash(cd * && cmake *)",
      "Bash(msbuild *)"
    ]
  }
}
```

### 2.5 全局记忆文件

在 `~/.claude/CLAUDE.md` 中写入全局偏好：

```markdown
# 全局偏好

始终用中文回复用户。
代码注释使用英文。
优先使用 C++17 特性。
```

---

## 3. CLAUDE.md — 项目记忆文件

### 3.1 什么是 CLAUDE.md

`CLAUDE.md` 是 Claude Code 在每次会话开始时自动读取的**项目上下文文件**，放在项目根目录。它相当于给 AI 一份"项目说明书"，让它在不问你的情况下就理解项目结构、构建方式和编码规范。

### 3.2 CLAUDE.md 的加载层级

Claude Code 会按优先级加载多个 CLAUDE.md：

```
~/.claude/CLAUDE.md          ← 全局偏好（语言、风格）
项目根目录/CLAUDE.md          ← 项目级上下文（最重要）
项目根目录/.claude/CLAUDE.md  ← 等同于根目录
子目录/CLAUDE.md              ← 模块级补充（进入该目录时加载）
```

### 3.3 CLAUDE.md 模板（推荐结构）

```markdown
# CLAUDE.md

本文件为 Claude Code 在此仓库中工作时提供指导。

## 项目概述
[一两句话说明项目是什么、用什么技术栈]

## 构建系统
### 构建命令
[列出关键的构建/运行/测试命令]

### 构建输出
[列出输出目录]

## 架构
### 目录结构
[列出核心目录及职责]

### 关键模块
[列出模块间依赖关系]

### 设计模式
[说明项目采用的架构模式]

## 编码规范
[命名约定、格式要求、注释风格等]

## 第三方依赖
[列出重要的第三方库及其用途]

## 常见任务
[新增功能、修复 Bug 的标准流程]
```

### 3.4 编写 CLAUDE.md 的实战要点

**DO — 应该写的：**

```markdown
## 构建命令
​```bash
# 生成 VS2019 工程
CmakeVS2019.bat

# 编译 Debug
cmake --build VSProject2019 --config Debug

# 运行
Bin/x64/Debug/DemoCreatorD.exe
​```

## 关键约定
- 模块公共头文件导出到 `include/inc_模块名/`
- 功能开关宏在 `include/GDefineFunc.h`，使用 FUNC_OPEN / FUNC_CLOSE
- CN/EN 变体通过 `MY_PROJECT_VERSION_TYPE` 控制
```

**DON'T — 不应该写的：**

- ❌ 大段粘贴源代码（Claude 可以自己读）
- ❌ 写过于笼统的描述（"这是一个好项目"）
- ❌ 文件超过 2000 行（会浪费 token）
- ❌ 包含敏感信息（API Key、密码）

### 3.5 CLAUDE.md 维护策略

```bash
# 在会话结束时，让 Claude 帮你更新
claude "根据我们今天的工作，更新 CLAUDE.md 中的构建说明"

# 使用 /init 命令自动生成初始 CLAUDE.md
claude /init
```

---

## 4. 核心命令与快捷操作

### 4.1 CLI 基本命令

```bash
# 启动交互式会话
claude

# 单次命令（不进入交互模式）
claude -p "解释一下这个函数的作用" < src/main.cpp

# 指定模型
claude --model claude-sonnet-4-20250514

# 恢复上次会话
claude --continue

# 从最近的会话列表中选择恢复
claude --resume
```

### 4.2 交互模式中的斜杠命令

| 命令 | 说明 |
|------|------|
| `/init` | 自动生成 CLAUDE.md |
| `/compact` | 压缩对话历史，释放上下文窗口 |
| `/clear` | 清除当前对话 |
| `/cost` | 查看当前会话的 token 用量和费用 |
| `/model` | 切换模型 |
| `/permissions` | 查看/修改权限设置 |
| `/help` | 查看帮助 |
| `/vim` | 切换 vim 编辑模式 |
| `/login` | 切换账户/重新登录 |
| `/status` | 查看当前状态信息 |

### 4.3 快捷键（交互模式）

| 快捷键 | 功能 |
|--------|------|
| `Esc` | 中断当前生成 |
| `Esc Esc`（双击） | 中断并回退到上一步 |
| `Tab` | 接受建议的命令 |
| `Shift+Tab` | 切换文件引用 |
| `Ctrl+C` | 取消当前操作 |

### 4.4 文件引用语法

在提示中可以直接引用文件路径，Claude 会自动读取：

```
看看 src/main.cpp 有什么问题
修改 include/config.h 中的版本号
把 src/utils/ 目录下所有文件的日志调用统一改成新的 API
```

---

## 5. 日常开发工作流

### 5.1 理解陌生代码

```
# 让 Claude 解读整个项目
这个项目的整体架构是怎样的？画一个模块依赖图

# 追踪调用链
从用户点击"录制"按钮开始，追踪代码执行流程到录制开始

# 解释复杂逻辑
解释 src/recorder/capture_engine.cpp 中 CaptureFrame() 函数的工作流程
```

### 5.2 实现新功能

**推荐的多步骤提示模式：**

```
# 第一步：让 Claude 先做调研
我要在录制器中添加"定时录制"功能，先帮我分析：
1. 现有的录制启动流程是怎样的
2. 应该在哪些文件中添加代码
3. 给出实现方案（先不要写代码）

# 第二步：确认方案后再实现
方案 A 看起来不错，请按这个方案实现，注意：
- 保持现有代码风格
- 添加必要的头文件引用
- 需要修改的文件逐个列出改动

# 第三步：验证
编译一下看看有没有错误
```

### 5.3 Bug 修复工作流

```
# 描述问题（越具体越好）
录制完成后点击"停止"按钮，程序偶尔会崩溃。
崩溃堆栈显示在 RecorderEngine::StopCapture() 第 342 行。
看起来是多线程竞争问题。请分析并修复。

# 提供崩溃日志
[粘贴崩溃堆栈]
分析这个崩溃，找出根因并修复
```

### 5.4 代码生成

```
# 生成样板代码
帮我创建一个新的 Qt 对话框类 TimerSettingsDialog，包含：
- 时间选择器（小时、分钟、秒）
- 开始/取消按钮
- 遵循项目现有的 UI 风格

# 生成测试
为 src/utils/string_helper.cpp 中的所有公共函数生成单元测试
```

---

## 6. 提示词工程（Prompt Engineering）

### 6.1 有效提示的结构

```
[上下文] + [具体任务] + [约束条件] + [输出格式]
```

**示例对比：**

❌ **模糊提示：**
```
帮我改一下那个 bug
```

✅ **优秀提示：**
```
在 src/recorder/audio_capture.cpp 中，AudioCapture::ProcessBuffer() 
函数在处理 48kHz 采样率时会产生杂音。
请分析 PCM 数据处理逻辑，找出采样率转换的问题并修复。
要求保持对 44.1kHz 和 96kHz 的兼容。
```

### 6.2 高效提示技巧

**1. 分步拆解复杂任务：**
```
我要重构日志系统，分三步进行：
1. 先分析当前日志系统的所有调用点
2. 设计新的日志接口
3. 逐模块替换
现在先执行第 1 步
```

**2. 提供参考示例：**
```
参照 src/conv/ 模块的 MVP 结构，为新的 watermark 功能创建
同样的目录结构和文件模板
```

**3. 限定范围避免过度修改：**
```
只修改 ProcessFrame() 函数内部的逻辑，不要改动函数签名和类结构
```

**4. 让 Claude 先理解再行动：**
```
先阅读 src/editor/ 目录下所有文件，总结架构后再开始修改
```

**5. 迭代式优化：**
```
# 第一轮
给这段代码做性能优化

# 第二轮（基于结果反馈）
优化后比较慢的部分是 for 循环中的 map 查找，改用 unordered_map
```

### 6.3 特殊指令模式

```bash
# 只分析不修改
分析 src/export/encoder.cpp 的问题，但不要修改任何文件

# 批量操作
把 src/ 下所有 .cpp 文件中的 qDebug() 替换为 LOG_DEBUG()

# 条件式指令
如果这个类有超过 500 行，就拆成子类；否则保持原样优化
```

---

## 7. 多文件与大型项目实战

### 7.1 大型项目的上下文策略

Claude Code 的上下文窗口有限，在大型项目中需要注意：

**方法1：用 CLAUDE.md 提供路线图**
```markdown
## 模块导航
- 录制相关问题 → 看 Src/PRecorder/
- 编辑器问题 → 看 Src/PEditor/
- IPC 通信 → 看 Src/BrokerService/ 和 PCore/PBroker*.cpp
- UI 控件 → 看 Src/PWidgets/
```

**方法2：主动引导搜索范围**
```
在 Src/PRecorder/ 目录下搜索摄像头初始化的代码
```

**方法3：使用 /compact 释放上下文**
```
# 当对话变长时
/compact
# Claude 会压缩历史，保留关键信息
```

### 7.2 跨文件修改

```
我要给所有 Broker 消息添加时间戳字段：
1. 先找出所有 Broker 消息的定义位置
2. 在基类中添加 timestamp 字段
3. 在所有发送点设置时间戳
4. 在所有接收点打印时间戳
请按顺序执行，每步完成后告诉我改了哪些文件
```

### 7.3 CMake 项目操作

```
# 添加新模块
帮我在 Src/ 下创建一个新模块 PScreenAnnotation，包含：
- CMakeLists.txt（参照现有模块的写法）
- 基本的头文件和源文件骨架
- 在根 CMakeLists.txt 中注册

# 依赖管理
为 PRecorder 模块添加对 OpenCV 的依赖
```

---

## 8. 调试与问题排查

### 8.1 编译错误修复

```
# 直接粘贴错误信息
编译出错了：
error C2039: 'StartTimer' is not a member of 'RecorderEngine'
帮我修复

# 批量修复
编译有 23 个错误，都是类似的头文件找不到问题，帮我统一修复
```

### 8.2 运行时问题分析

```
# 崩溃分析
程序在以下位置崩溃：
[粘贴堆栈跟踪]
分析可能的原因，检查相关代码

# 内存泄漏
valgrind 报告以下泄漏：
[粘贴报告]
找出泄漏点并修复

# 性能问题
这个函数处理 1080p 视频帧需要 50ms，目标是 16ms。
分析瓶颈并优化
```

### 8.3 利用 Claude 做日志分析

```
# 分析日志模式
查看 logs/ 目录下最新的日志文件，找出所有 ERROR 级别的日志，
分析是否有规律性的错误模式

# 对比分析
对比正常运行的日志和崩溃时的日志，找出区别
```

---

## 9. 代码审查与重构

### 9.1 代码审查

```
# 审查单个文件
审查 src/capture/screen_capture.cpp，关注：
- 线程安全性
- 资源泄漏
- 错误处理
- 性能问题

# 审查 PR 改动
审查最近一次 commit 的改动，给出改进建议
```

### 9.2 重构模式

```
# 提取函数
ProcessFrame() 太长了（200行），帮我拆成逻辑清晰的子函数

# 设计模式改造
把 src/export/ 中的 if-else 链改成策略模式

# 现代化改造
把 src/utils/ 中的原始指针改成智能指针，
旧式 for 循环改成 range-based for
```

### 9.3 安全的重构策略

```
# 让 Claude 先评估影响范围
我想重命名 CaptureManager 类为 ScreenCaptureManager，
先分析所有引用点，评估影响范围

# 分步重构
先只改头文件声明和直接引用，编译通过后再改间接引用
```

---

## 10. Git 集成工作流

### 10.1 提交信息生成

```bash
# 自动生成 commit 信息
claude -p "查看 git diff --staged 的内容，生成一条规范的 commit 信息"

# 在交互模式中
帮我生成这次改动的 commit 信息，使用 conventional commit 格式
```

### 10.2 分支管理

```
# 分析分支差异
比较 feature/timer-recording 分支和 main 分支的差异，给出总结

# 解决合并冲突
当前有合并冲突，帮我解决，优先保留 feature 分支的改动
```

### 10.3 PR 工作流

```
# 生成 PR 描述
根据当前分支与 main 的差异，生成 PR 描述，包含：
- 改动摘要
- 影响的模块
- 测试建议

# PR 审查
审查本 PR 中的所有改动文件，给出审查意见
```

### 10.4 Git Hooks 集成

```bash
# 在 .git/hooks/pre-commit 中集成 Claude 审查
#!/bin/bash
git diff --cached | claude -p "审查这些改动，如果有严重问题输出 REJECT 并说明原因"
```

---

## 11. 测试驱动开发

### 11.1 生成测试

```
# 为现有代码生成测试
为 src/utils/string_helper.cpp 中的所有函数生成 Google Test 单元测试

# 为特定场景生成测试
为 AudioCapture 类生成以下场景的测试：
1. 正常采集 48kHz PCM 数据
2. 设备断开时的错误处理
3. 多次 Start/Stop 循环
```

### 11.2 TDD 工作流

```
# 先写测试
为"定时录制"功能写测试用例，测试以下场景：
- 设置 5 秒后开始录制
- 设置录制 10 秒后自动停止
- 取消定时
然后再实现代码让测试通过
```

### 11.3 测试覆盖率分析

```
分析 src/recorder/ 模块的测试覆盖情况，找出缺少测试的关键路径
```

---

## 12. 成本控制与性能优化

### 12.1 Token 消耗意识

**高成本操作（尽量避免）：**
- 让 Claude 读取整个大型代码库
- 反复修改同一文件（每次都要重新读取）
- 不明确的提示导致多轮试错

**低成本操作（推荐）：**
- 先用精确搜索定位，再读取特定文件
- 一次提示包含完整上下文
- 使用 CLAUDE.md 减少每次解释项目结构

### 12.2 优化策略

```bash
# 查看费用
/cost

# 压缩历史，减少重复上下文传输
/compact

# 开启自动压缩（在设置中）
# ~/.claude/settings.json
{
  "autoCompact": true,
  "compactThreshold": 80  # 上下文使用率达到 80% 时自动压缩
}
```

### 12.3 模型选择策略

| 任务类型 | 推荐模型 | 理由 |
|----------|----------|------|
| 简单修改、格式化 | Sonnet | 快速、便宜 |
| 复杂架构设计 | Opus | 推理能力强 |
| 代码补全、生成 | Sonnet | 性价比高 |
| 深度 debug、安全审计 | Opus | 准确度高 |

```bash
# 在会话中切换模型
/model claude-sonnet-4-20250514
/model claude-opus-4-20250514
```

---

## 13. 安全与隐私

### 13.1 权限管理

Claude Code 的权限分三级：

```
读取文件         → 默认允许
修改文件         → 需确认（或预授权）
执行终端命令     → 需确认（或预授权）
```

**推荐的权限配置：**

```json
{
  "permissions": {
    "allow": [
      "Read",
      "Edit",
      "Bash(git status)",
      "Bash(git diff*)",
      "Bash(git log*)",
      "Bash(cmake --build*)",
      "Bash(npm run*)",
      "Bash(python *.py)"
    ],
    "deny": [
      "Bash(rm -rf *)",
      "Bash(sudo *)",
      "Bash(curl * | bash)",
      "Bash(git push *)"
    ]
  }
}
```

### 13.2 敏感文件保护

在 `.claude/settings.json` 中排除敏感文件：

```json
{
  "ignore": [
    ".env",
    "*.pem",
    "*.key",
    "secrets/",
    "credentials.*"
  ]
}
```

### 13.3 安全准则

- **不要** 在提示中包含 API Key、密码
- **不要** 让 Claude 执行你不理解的 shell 命令
- **审查** Claude 的每次文件修改（尤其是配置文件）
- **限制** 网络相关命令的权限
- 使用 `git diff` 审查所有改动后再提交

---

## 14. VS Code 集成（Copilot Chat + Claude）

### 14.1 配置 Claude 作为 Copilot Chat 后端

在 VS Code 中：
1. 安装 GitHub Copilot Chat 扩展
2. 在 Copilot Chat 面板中选择 Claude 模型
3. 使用 `Agent` 模式让 Claude 可以编辑文件和执行命令

### 14.2 VS Code 中的高效用法

```
# 选中代码后提问
[选中一段代码] → 右键 → Copilot → 解释/优化/修复

# 在 Chat 中引用文件
@workspace 找出所有使用了废弃 API 的文件

# 引用终端错误
@terminal 刚才的编译错误是什么原因
```

### 14.3 Agent 模式 vs Ask 模式

| 模式 | 能力 | 适用场景 |
|------|------|----------|
| **Ask** | 只回答问题 | 代码解释、知识查询 |
| **Agent** | 可读写文件、执行命令 | 实现功能、修复 Bug |

**Agent 模式下 Claude 可以：**
- 搜索项目文件
- 读取/修改代码
- 运行终端命令
- 查看编译错误
- 管理任务列表

---

## 15. 实战案例

### 15.1 案例：为 Qt 项目添加新功能模块

```
提示：
我要在 EasyPhoto 项目中添加"图片水印"功能模块。
参照现有的 conv/ 模块结构（MVP 模式），创建：
1. src/watermark/ 目录，包含 view、presenter、repository
2. 在 navbarview 中添加新的导航项
3. 在 funcareaview 中注册新模块
4. 遵循现有的代码风格和命名规范

请先分析 conv/ 模块的结构作为参考，再开始实现。
```

### 15.2 案例：跨模块重构

```
提示：
DemoCreator 项目中 PCore 模块的 Logger 类需要重构：
1. 当前是单例模式，改为依赖注入
2. 分析所有模块对 Logger 的引用
3. 设计新接口，保持向后兼容
4. 分模块逐步替换，每修改一个模块后编译验证
```

### 15.3 案例：调试多进程 IPC

```
提示：
DemoCreator 的录制进程和编辑器进程之间的 Broker 消息传递有时会丢失。
1. 先帮我理解 BrokerService 的消息路由机制
2. 在 PBroker 相关文件中找到消息发送和接收的关键路径
3. 分析可能导致消息丢失的场景
4. 添加诊断日志帮助定位问题
```

### 15.4 案例：性能优化

```
提示：
EasyPhoto 的图片压缩功能在处理 4K 图片时很慢（>10秒），分析 
src/comp/ 模块的处理流程，找出性能瓶颈，考虑：
1. 是否可以用多线程处理
2. 是否有不必要的内存拷贝
3. ImageMagick 的调用是否可以优化参数
```

### 15.5 案例：CMake 构建问题

```
提示：
编译 DemoCreator 时在链接阶段报错：
LNK2019: unresolved external symbol "public: void __cdecl 
CaptureManager::StartCapture(void)" 
分析 CMakeLists.txt 中的链接配置，找出缺少的库依赖并修复
```

---

## 16. 常见问题与排错

### Q1: Claude 说"我无法访问该文件"

**原因：** 权限配置不允许读取，或文件不在工作区内。

**解决：**
```json
// .claude/settings.json
{
  "permissions": {
    "allow": ["Read"]
  }
}
```

### Q2: 上下文窗口满了

**症状：** Claude 回复变短、忘记前面的对话内容。

**解决：**
```
/compact              # 压缩对话历史
/clear                # 清空重来
claude --continue     # 新建会话但保持目录状态
```

### Q3: Claude 的修改引入了新错误

**预防：**
```
每次修改后运行编译，确保没有新错误。
如果有错误，先修复再继续下一步。
```

**修复：**
```bash
# 用 git 回退
git checkout -- src/problematic_file.cpp

# 或者让 Claude 修复
编译报错了：[粘贴错误]，请修复你刚才的修改
```

### Q4: Claude 不了解项目特有的框架/SDK

**解决：** 在 CLAUDE.md 中说明：
```markdown
## 针对 VBL SDK 的说明
VBL 是私有视频处理引擎，不要在网上搜索其文档。
参考 3rdparty/vbl/include/ 下的头文件了解接口。
主要类：VBLEngine, VBLTrack, VBLClip
```

### Q5: 修改大文件时 Claude 只改了一部分

**解决：**
```
# 明确指出要修改的范围
继续修改 src/editor/timeline.cpp，
上次改到了 UpdateTrack() 函数，
接着修改 RemoveTrack() 和 MoveTrack() 函数

# 或分段指示
这个文件有 2000 行，先改 1-500 行中的旧 API 调用
```

### Q6: Claude 给出的方案不符合项目风格

**解决：** 加强 CLAUDE.md 中的规范描述：
```markdown
## 编码规范
- 类名使用 PascalCase，如 `CaptureManager`
- 成员变量前缀 `m_`，如 `m_isRecording`
- 使用 Q_OBJECT 宏的类必须放在单独的头文件中
- 信号槽使用新式语法 connect(sender, &Sender::signal, ...)
```

---

## 17. 最佳实践清单

### 启动项目时
- [ ] 创建或更新 CLAUDE.md
- [ ] 配置项目级 `.claude/settings.json`
- [ ] 明确构建和测试命令

### 每次会话
- [ ] 先用简短提示确认 Claude 理解了项目上下文
- [ ] 复杂任务分步执行
- [ ] 每步完成后验证（编译/测试）
- [ ] 使用 `/compact` 管理上下文
- [ ] 用 `/cost` 关注费用

### 提示撰写
- [ ] 提供足够的上下文（文件路径、函数名、错误信息）
- [ ] 明确约束（不要改哪些文件、保持哪些兼容性）
- [ ] 使用参考实例（"参照 xxx 模块"）
- [ ] 指定输出格式（"给出 diff"、"列出改动清单"）

### 安全习惯
- [ ] 审查每次文件修改
- [ ] 不在提示中包含敏感信息
- [ ] 配置合理的命令执行权限
- [ ] 使用 Git 追踪所有改动

### 长期维护
- [ ] 定期更新 CLAUDE.md
- [ ] 记录有效的提示模式复用
- [ ] 团队共享 CLAUDE.md 和权限配置

---

## 附录 A：常用提示模板

### 功能开发
```
在 [模块/目录] 中实现 [功能描述]。
参考 [参考模块] 的结构。
要求：
- [约束1]
- [约束2]
先分析现有代码，给出方案后再实现。
```

### Bug 修复
```
[问题描述]
错误信息/堆栈：[粘贴]
相关文件：[文件列表]
请分析根因并修复，确保修复不影响 [相关功能]。
```

### 代码审查
```
审查 [文件/PR] 的代码质量，关注：
- 线程安全
- 内存管理
- 错误处理
- 性能
- 可读性
给出具体改进建议和代码示例。
```

### 重构
```
重构 [目标代码] ：
- 当前问题：[说明]
- 目标状态：[说明]
- 约束：不改变公共 API / 保持向后兼容
分步进行，每步编译验证。
```

---

## 附录 B：Claude Code vs 其他 AI 编程工具

| 特性 | Claude Code | GitHub Copilot | Cursor | Windsurf |
|------|------------|----------------|--------|----------|
| 终端命令执行 | ✅ | ❌ | ✅ | ✅ |
| 项目级上下文 | ✅（CLAUDE.md） | 部分 | ✅ | ✅ |
| 多文件编辑 | ✅ | ❌ | ✅ | ✅ |
| 自主任务执行 | ✅ | ❌ | 部分 | 部分 |
| CLI 工作流 | ✅（原生） | ❌ | ❌ | ❌ |
| 自定义权限 | ✅ | N/A | 部分 | 部分 |
| 会话持久性 | ✅ | ❌ | ✅ | ✅ |
| 成本可见性 | ✅（/cost） | 订阅制 | 订阅制 | 订阅制 |

---

> **持续更新中** — 随着 Claude Code 的版本迭代，本文档会不断补充新特性和实战经验。
