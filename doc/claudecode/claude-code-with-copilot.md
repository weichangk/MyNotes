# Claude Code + GitHub Copilot 完整配置指南

> 本文介绍如何利用 GitHub Copilot 订阅获得完整的 Claude Agent 编程能力，提供三条路线供选择。

---

## 先看这里：选择适合你的路线

### 路线对比

| | **路线一** | **路线二** | **路线三** |
|---|---|---|---|
| **方式** | VS Code Copilot Chat 直接使用 Claude | Claude Code 扩展 + copilot-api 代理 | Claude Code CLI + copilot-api 代理 |
| **配置难度** | ⭐ 最简单，开箱即用 | ⭐⭐ 需要配置 | ⭐⭐ 需要配置 |
| **需要额外工具** | ❌ 不需要 | copilot-api | copilot-api |
| **Claude 模型版本** | Sonnet 4.6 / Opus 4.6 / Haiku 4.5 等最新版 | 取决于 copilot-api 暴露的模型 | 取决于 copilot-api 暴露的模型 |
| **Agent 能力** | ✅ 读写文件、执行终端命令、多步任务 | ✅ 同左，且更自主 | ✅ 同左，且更自主 |
| **CLAUDE.md 支持** | ❌ | ✅ | ✅ |
| **Checkpoint 回滚** | ❌ | ✅ | ✅ |
| **Plan 模式** | ❌ | ✅ | ✅ |
| **CLI 工作流** | ❌ | 可用集成终端 | ✅ 原生 |
| **推荐场景** | 日常编程辅助，快速上手 | 在 VS Code 中使用 Claude Code 全部功能 | 深度 Agent 任务、脚本自动化 |

### 如何选择

```
只想在 VS Code 里用 Claude 写代码，不折腾
    → 路线一（Copilot Chat 直接选 Claude 模型）

想要 CLAUDE.md / Checkpoint / Plan 模式，习惯图形界面
    → 路线二（Claude Code 扩展 + copilot-api）

习惯终端，需要 CLI 工作流或脚本自动化
    → 路线三（Claude Code CLI + copilot-api）
```

---

## 目录

- [一、路线一：VS Code Copilot Chat 直接使用 Claude](#一路线一vs-code-copilot-chat-直接使用-claude)
- [二、路线二 / 三 的原理说明](#二路线二--三的原理说明)
- [三、前置要求](#三前置要求)
- [四、安装工具](#四安装工具)
- [五、GitHub OAuth 认证](#五github-oauth-认证)
- [六、路线二：VS Code 扩展 + copilot-api](#六路线二vs-code-扩展--copilot-api)
- [七、路线三 A：mycc 一键启动（CLI，推荐）](#七路线三-amycc-一键启动cli推荐)
- [八、路线三 B：手动启动（CLI）](#八路线三-b手动启动cli)
- [九、验证 Agent 能力](#九验证-agent-能力)
- [十、常见问题排查](#十常见问题排查)
- [十一、可用模型列表](#十一可用模型列表)

---

## 一、路线一：VS Code Copilot Chat 直接使用 Claude

VS Code **1.98+** 起，GitHub Copilot Chat 已原生内置 Claude 模型支持。你的 Copilot 订阅直接包含，**无需安装任何额外工具，无需 API Key，无需代理**。

### Copilot 订阅可用的 Claude 模型（截至 2026 年）

| 模型 | 说明 |
|------|------|
| `Claude Sonnet 4` / `4.5` / `4.6` | 主力模型，日常编程首选 |
| `Claude Opus 4.5` / `4.6` | 最强推理，适合复杂架构和深度分析 |
| `Claude Haiku 4.5` | 轻量快速，简单任务 |

### 使用步骤

1. 确认 VS Code 版本 ≥ 1.98（`帮助 → 关于`）
2. 确认已安装 **GitHub Copilot** 和 **GitHub Copilot Chat** 扩展（新版 VS Code 已内置）
3. 按 `Ctrl+Alt+I` 打开 Copilot Chat 面板
4. 点击输入框旁的**模型选择器**，选择任意 Claude 模型
5. 点击输入框旁的**模式切换**，选择 **Agent 模式**

### Agent 模式具备的能力

- 搜索整个代码库
- 读取 / 修改文件
- 执行终端命令（需你确认）
- 多步骤自主任务规划与执行

> **这就是你要的 Agent 能力**，且完全不需要 `copilot-api`、`mycc` 等任何额外工具。

### 局限性

- 不读取 `CLAUDE.md` 项目记忆文件
- 没有 Checkpoint 回滚功能
- 没有 Plan 模式（先看计划再执行）
- 没有 Claude Code 的 CLI 工作流

如果以上功能对你重要，继续阅读路线二 / 三。

---

## 二、路线二 / 三 的原理说明

```
你的代码 / VS Code / 终端
          ↓
Claude Code（CLI 或 VS Code 扩展）
          ↓  向 ANTHROPIC_BASE_URL 发送 Anthropic 格式请求
copilot-api（本地代理 :4141）
          ↓  协议转换：Anthropic API → GitHub Copilot API
GitHub Copilot（你已订阅的 AI 服务）
```

**关键点：**
- Claude Code 不知道后端换成了 Copilot，它只认 `ANTHROPIC_BASE_URL` 指向的地址
- `copilot-api` 是核心中间层，负责协议转换和 OAuth token 管理
- VS Code 扩展与 CLI 底层共用同一套配置（`~/.claude/settings.json`），对 CLI 生效的代理设置，对扩展同样生效

---

## 三、前置要求

| 要求 | 版本 / 说明 |
|------|------------|
| **Node.js** | 16 或更高版本 |
| **npm** | 随 Node.js 一同安装 |
| **GitHub 账号** | 需有有效的 Copilot 订阅（Individual / Business / Enterprise 均可） |
| **VS Code**（可选） | 1.98.0 或更高，仅使用扩展模式时需要 |
| **网络** | 能访问 `github.com`（国内用户见 [认证网络问题](#国内网络代理问题)） |

### 检查 Node.js

```powershell
node --version    # 应输出 v16.x.x 或更高
npm --version
```

如未安装，前往 https://nodejs.org 下载 LTS 版本。

---

## 四、安装工具

### 4.1 安装 Claude Code CLI

```powershell
npm install -g @anthropic-ai/claude-code

# 验证
claude --version
```

### 4.2 安装 copilot-api

```powershell
npm install -g copilot-api

# 验证
copilot-api --version
```

### 4.3 国内 npm 加速（如果安装很慢）

```powershell
# 切换淘宝镜像源
npm config set registry https://registry.npmmirror.com

# 验证
npm config get registry

# 再执行安装
npm install -g @anthropic-ai/claude-code copilot-api
```

### 4.4 安装 Claude Code VS Code 扩展（仅路线二需要）

在 VS Code 中按 `Ctrl+Shift+X` 打开扩展市场，搜索 **Claude Code**（发布者：Anthropic），点击安装。

或在浏览器打开：https://marketplace.visualstudio.com/items?itemName=anthropic.claude-code

> VS Code 最低版本要求：**1.98.0**，可通过 帮助 → 关于 查看。

---

## 五、GitHub OAuth 认证

无论使用哪种启动方式，都需要先完成一次 GitHub 认证。

### 5.1 执行认证

```powershell
copilot-api auth
```

执行后：
1. 终端显示一个 **8 位设备码**（如 `ABCD-1234`）
2. 自动打开浏览器访问 `https://github.com/login/device`
3. 在网页上输入设备码并授权
4. 认证成功，token 自动保存到本地

**token 存储位置：**

```
Windows: %USERPROFILE%\.copilot-api\auth.json
         %USERPROFILE%\.local\share\copilot-api\github_token
macOS:   ~/.copilot-api/auth.json
```

### 5.2 国内网络代理问题

`copilot-api` 使用 `undici` 发请求，**不走系统代理**，需手动指定 `HTTPS_PROXY`：

```powershell
# 认证前设置代理（替换为你的端口）
$env:HTTPS_PROXY = "http://127.0.0.1:7890"   # Clash
# $env:HTTPS_PROXY = "http://127.0.0.1:10809" # V2Ray/Xray
# $env:HTTPS_PROXY = "http://127.0.0.1:1080"  # Shadowsocks
$env:HTTP_PROXY = $env:HTTPS_PROXY

copilot-api auth
```

---

## 六、路线二：VS Code 扩展 + copilot-api

VS Code 扩展底层调用 Claude Code CLI 进程，与 CLI 共享 `~/.claude/settings.json`，因此只需将环境变量写入该文件，扩展启动时会自动读取。

### 步骤 1：安装 Claude Code VS Code 扩展

> 若已在 [四、安装工具](#四安装工具) 中完成，跳过此步。

`Ctrl+Shift+X` → 搜索 **Claude Code**（发布者：Anthropic）→ 安装。

### 步骤 2：安装 copilot-api 并完成认证

> 若已在 [四](#四安装工具) 和 [五](#五github-oauth-认证) 中完成，跳过此步。

```powershell
npm install -g copilot-api
copilot-api auth   # 国内网络需先设置 HTTPS_PROXY，见第五节
```

### 步骤 3：写入 `~/.claude/settings.json`

打开（或创建）`%USERPROFILE%\.claude\settings.json`（Windows）或 `~/.claude/settings.json`（macOS），添加以下内容：

```json
{
  "env": {
    "ANTHROPIC_BASE_URL": "http://localhost:4141",
    "ANTHROPIC_AUTH_TOKEN": "dummy",
    "ANTHROPIC_MODEL": "gpt-4o",
    "ANTHROPIC_DEFAULT_SONNET_MODEL": "gpt-4o",
    "ANTHROPIC_SMALL_FAST_MODEL": "gpt-4o-mini",
    "ANTHROPIC_DEFAULT_HAIKU_MODEL": "gpt-4o-mini",
    "DISABLE_NON_ESSENTIAL_MODEL_CALLS": "1",
    "CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC": "1"
  }
}
```

> 如果文件已有其他内容（如 `permissions`），将 `"env": { ... }` 合并进去，不要覆盖整个文件。

### 步骤 4：在 VS Code 中禁用登录提示

```
Ctrl+,（打开设置）→ 搜索 "Claude Code login"
→ 勾选 Disable Login Prompt
```

### 步骤 5：启动 copilot-api 代理

```powershell
# 新开终端窗口，保持运行
copilot-api start
```

### 步骤 6：打开 Claude Code 面板

点击 VS Code 编辑器右上角的 **✦ 火花图标**，或点击底部状态栏的 **✱ Claude Code**。

### VS Code 扩展相比 Copilot Chat 多出的能力

| 功能 | 说明 |
|------|------|
| **CLAUDE.md** | 自动读取项目记忆文件，理解项目上下文 |
| **可视化 Diff** | 文件修改时显示左右对比，点击接受 / 拒绝 |
| **Plan 模式** | Claude 先输出计划文档，你批注后再执行 |
| **Checkpoint 回滚** | 每条消息打快照，可随时 rewind 到任意步骤 |
| **IDE 诊断集成** | 自动读取 VS Code Problems 面板的错误和警告 |
| **多会话并行** | 多标签页同时运行多个 Agent 任务 |

---

## 七、路线三 A：mycc 一键启动（CLI，推荐）

本仓库 `mycc/` 目录提供封装脚本，**自动完成全部步骤**（环境检查 → 认证 → 启动代理 → 设置环境变量 → 启动 Claude Code CLI）。

### Windows

```powershell
cd mycc

.\mycc.ps1                              # 正常启动
.\mycc.ps1 -ReAuth                      # 强制重新认证
.\mycc.ps1 -Proxy http://127.0.0.1:7890 # 指定代理（国内网络）
.\mycc.ps1 -NoProxy                     # 跳过启动代理（代理已在运行时）
```

**PowerShell 执行策略报错时：**

```powershell
powershell -ExecutionPolicy Bypass -File .\mycc.ps1

# 或永久设置（当前用户）
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

**全局安装（任意目录可用）：**

```powershell
.\install.ps1   # 安装后可在任意目录直接运行 mycc
```

### macOS / Linux

```bash
cd mycc
chmod +x mycc.sh
./mycc.sh               # 正常启动
./mycc.sh -r            # 重新认证
./mycc.sh --no-proxy    # 跳过代理
```

---

## 八、路线三 B：手动启动（CLI）

不使用 mycc 时，手动分三步操作。

### 步骤 1：启动 copilot-api 代理（新开一个终端窗口，保持运行）

```powershell
# 直接启动
copilot-api start

# 带代理启动（国内网络）
$env:HTTPS_PROXY = "http://127.0.0.1:7890"
copilot-api start

# 验证代理正常运行
Invoke-RestMethod "http://localhost:4141/"
```

### 步骤 2：设置环境变量

```powershell
$env:ANTHROPIC_BASE_URL                       = "http://localhost:4141"
$env:ANTHROPIC_AUTH_TOKEN                     = "dummy"
$env:ANTHROPIC_MODEL                          = "gpt-4o"
$env:ANTHROPIC_DEFAULT_SONNET_MODEL           = "gpt-4o"
$env:ANTHROPIC_SMALL_FAST_MODEL               = "gpt-4o-mini"
$env:ANTHROPIC_DEFAULT_HAIKU_MODEL            = "gpt-4o-mini"
$env:DISABLE_NON_ESSENTIAL_MODEL_CALLS        = "1"
$env:CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC = "1"
```

### 步骤 3：启动 Claude Code

```powershell
claude
```

---

## 九、验证 Agent 能力

启动后，在 Claude Code 中输入以下内容验证各项能力是否正常：

```
# 文件读写
读取当前目录下所有 .cpp 文件，列出文件名

# 终端命令执行
运行 git status，看看当前仓库状态

# 多步骤自主任务
找出 src/ 目录下所有包含 TODO 注释的文件，列出文件名和行号
```

**验证代理和模型是否正常：**

```powershell
Invoke-RestMethod "http://localhost:4141/v1/models" | Select-Object -ExpandProperty data | Select-Object id
```

---

## 十、常见问题排查

### ❌ `API Error 400 model_not_supported`

**原因**：`ANTHROPIC_MODEL` 使用了 `claude-*` 格式，copilot-api 无法识别。

**解决**：必须使用 copilot-api 暴露的模型名（如 `gpt-4o`），在 `settings.json` 或环境变量中确认：

```json
"ANTHROPIC_MODEL": "gpt-4o",
"DISABLE_NON_ESSENTIAL_MODEL_CALLS": "1"
```

---

### ❌ `copilot-api auth` 连接超时（github.com:443）

**原因**：copilot-api 不走系统代理。

**解决**：

```powershell
$env:HTTPS_PROXY = "http://127.0.0.1:7890"
copilot-api auth
```

---

### ❌ VS Code 扩展仍弹出 Anthropic 登录框

**原因**：未勾选 Disable Login Prompt，或 `settings.json` 的 `env` 字段未正确写入。

**解决**：
1. 确认 `Ctrl+,` → 搜索 `Claude Code login` → 已勾选 **Disable Login Prompt**
2. 检查 `~/.claude/settings.json` 中 `env.ANTHROPIC_BASE_URL` 是否存在
3. 重启 VS Code（`Ctrl+Shift+P` → `Developer: Reload Window`）

---

### ❌ 端口 4141 被占用

```powershell
netstat -ano | findstr :4141
taskkill /PID <PID> /F
```

---

### ❌ 认证后仍提示未认证（token 损坏）

```powershell
Remove-Item "$env:USERPROFILE\.copilot-api\auth.json" -ErrorAction SilentlyContinue
Remove-Item "$env:USERPROFILE\.local\share\copilot-api\github_token" -ErrorAction SilentlyContinue

$env:HTTPS_PROXY = "http://127.0.0.1:7890"   # 如需代理
copilot-api auth
```

---

### ❌ Claude Code 无响应 / connection refused

**原因**：copilot-api 代理未启动或已崩溃。

```powershell
# 检查代理状态
Invoke-RestMethod "http://localhost:4141/" -TimeoutSec 2

# 重新启动
copilot-api start
```

---

### ❌ npm 全局安装失败（权限错误）

```powershell
# 推荐：使用 nvm-windows 管理 Node.js（彻底避免权限问题）
# 下载：https://github.com/coreybutler/nvm-windows
nvm install lts
nvm use lts
npm install -g copilot-api @anthropic-ai/claude-code
```

---

## 十一、可用模型列表（路线二 / 三）

运行以下命令查询当前代理支持的模型：

```powershell
Invoke-RestMethod "http://localhost:4141/v1/models" | Select-Object -ExpandProperty data | Select-Object id
```

常见可用模型（取决于你的 Copilot 订阅等级）：

| 模型 ID | 说明 | 推荐用途 |
|---------|------|----------|
| `gpt-4o` | GPT-4o（默认） | 日常编程、代码生成、重构 |
| `gpt-4o-mini` | GPT-4o Mini（快速） | 简单任务、格式化、快速回答 |
| `claude-3.5-sonnet` | Claude Sonnet（部分账号可用） | 复杂推理、架构设计 |
| `gemini-2.5-pro` | Gemini Pro（部分账号可用） | 大上下文分析 |

> **注意**：`ANTHROPIC_MODEL` 等环境变量必须填写上表中的模型 ID，不能填 `claude-opus-4` 等 Anthropic 官方命名格式。

---

## 附：settings.json 完整示例

`%USERPROFILE%\.claude\settings.json`（Windows）/ `~/.claude/settings.json`（macOS）：

```json
{
  "env": {
    "ANTHROPIC_BASE_URL": "http://localhost:4141",
    "ANTHROPIC_AUTH_TOKEN": "dummy",
    "ANTHROPIC_MODEL": "gpt-4o",
    "ANTHROPIC_DEFAULT_SONNET_MODEL": "gpt-4o",
    "ANTHROPIC_SMALL_FAST_MODEL": "gpt-4o-mini",
    "ANTHROPIC_DEFAULT_HAIKU_MODEL": "gpt-4o-mini",
    "DISABLE_NON_ESSENTIAL_MODEL_CALLS": "1",
    "CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC": "1"
  },
  "permissions": {
    "allow": [
      "Read",
      "Edit",
      "Bash(git *)",
      "Bash(npm *)"
    ]
  }
}
```
