# Claude Code 快速参考指南

> 基于 Claude Code 官方文档整理的命令速查手册，涵盖斜杠命令、快捷键、CLI 参数、配置与 MCP。

---

## 目录

- [1. Claude Code 是什么](#1-claude-code-是什么)
- [2. 斜杠命令（Slash Commands）](#2-斜杠命令slash-commands)
- [3. 键盘快捷键](#3-键盘快捷键)
- [4. CLI 启动参数](#4-cli-启动参数)
- [5. 配置文件与 Settings](#5-配置文件与-settings)
- [6. Hooks 自动化钩子](#6-hooks-自动化钩子)
- [7. MCP（Model Context Protocol）](#7-mcpmodel-context-protocol)
- [8. 最佳实践](#8-最佳实践)

---

## 1. Claude Code 是什么

Claude Code 是 Anthropic 推出的**智能命令行工具**，以 Claude AI 为内核，将对话式 AI、文件读写、终端执行、插件/Skills、SubAgents、MCP 工具接入整合为可自动化的开发助理。

**核心能力：**
- 直接读写项目文件、执行终端命令
- 理解整个代码库的上下文
- 执行多步骤复杂任务（搜索 → 分析 → 修改 → 验证）
- 与 Git、构建系统、测试框架深度集成
- 通过 Hooks / Skills / MCP 与外部工具集成

**与传统 AI Chat 的区别：**

| 特性 | 传统 AI Chat | Claude Code |
|------|-------------|-------------|
| 文件访问 | 手动粘贴代码 | 直接读写项目文件 |
| 命令执行 | 不支持 | 可运行终端命令 |
| 上下文范围 | 单次对话 | 整个代码库 |
| 多步骤任务 | 需手动衔接 | 自动规划并执行 |
| 结果验证 | 人工检查 | 可自动编译/测试验证 |

---

## 2. 斜杠命令（Slash Commands）

> 在交互模式中输入 `/` 可查看全部可用命令。

### 会话管理

| 命令 | 说明 |
|------|------|
| `/help` | 显示帮助与可用命令列表 |
| `/clear` | 清空当前会话上下文 |
| `/compact [指令]` | 压缩对话上下文，可附带聚焦指令（节省 token） |
| `/rename [名称]` | 重命名当前会话 |
| `/resume` | 从历史会话列表中选择恢复 |
| `/new` | 开启新会话 |
| `/exit` / `/quit` | 退出 Claude Code |

### 上下文 / 记忆 / 设置

| 命令 | 说明 |
|------|------|
| `/memory` | 查看/编辑 CLAUDE.md 与自动记忆条目 |
| `/config` / `/settings` | 打开设置界面（主题、模型、输出风格等） |
| `/context` | 可视化当前上下文占用情况 |
| `/cost` | 查看当前会话的 token 用量和费用 |
| `/model` | 切换当前使用的模型 |
| `/status` | 查看当前状态信息 |

### 工具 / 权限 / 调试

| 命令 | 说明 |
|------|------|
| `/permissions` | 查看或更新工具权限 |
| `/hooks` | 查看当前配置的自动化钩子 |
| `/debug` | 启用调试模式 |
| `/doctor` | 检查安装与配置状态 |
| `/vim` | 切换 Vim 编辑模式 |

### 开发工作流

| 命令 | 说明 |
|------|------|
| `/diff` | 查看未提交的代码修改 |
| `/pr-comments [PR号]` | 拉取 GitHub PR 的评论（需要 gh CLI） |
| `/statusline` | 配置/显示终端状态行 |
| `/init` | 自动分析项目并生成 CLAUDE.md |

### 插件 / Skills / Agents

| 命令 | 说明 |
|------|------|
| `/skills` | 列出所有可用技能（Skills） |
| `/agents` | 管理/列出 SubAgents |
| `/mcp` | 管理 MCP 服务器连接 |
| `/plugin` | 管理插件（安装/卸载/启用/禁用/更新） |

---

## 3. 键盘快捷键

### 全局控制

| 快捷键 | 功能 |
|--------|------|
| `Ctrl+C` | 中断当前模型生成 / 取消输入 |
| `Ctrl+D` | 退出 Claude Code（EOF） |
| `Ctrl+L` | 清屏（保留会话历史） |
| `Ctrl+O` | 切换 verbose / 显示工具执行细节 |

### 输入编辑

| 快捷键 | 功能 |
|--------|------|
| `Enter` | 发送消息 |
| `Shift+Enter` | 插入换行（多行输入） |
| `Ctrl+K` | 删除至行尾 |
| `Ctrl+U` | 删除整行 |
| `Ctrl+Y` | 粘贴最近删除的内容 |

### 命令与历史

| 快捷键 | 功能 |
|--------|------|
| `/`（输入斜杠） | 打开命令/技能菜单 |
| `Ctrl+R` | 反向搜索命令历史 |
| `↑` / `↓` | 浏览历史命令/消息 |
| `Esc` | 中断生成（Vim 模式下切换 NORMAL） |
| `Esc Esc`（双击） | 中断并回退到上一步 |

### 自定义快捷键

快捷键配置文件位置（Windows）：
```
C:\Users\<用户名>\.claude\keybindings.json
```

使用 `/keybindings` 命令打开或创建该配置文件。

---

## 4. CLI 启动参数

### 基本启动

```bash
# 交互式模式
claude

# 带初始问题启动
claude "帮我分析这个项目的架构"

# 单次执行后退出（适合脚本集成）
claude -p "生成这段代码的单元测试"
```

### 会话控制

```bash
# 继续上次会话
claude -c
claude --continue

# 恢复指定会话
claude --resume <会话ID或名称>
claude -r <会话名称>

# 指定会话显示名
claude --name "我的项目会话"
claude -n "我的项目会话"

# 在隔离的 git worktree 中启动
claude --worktree <名称>
claude -w <名称>
```

### 模型与推理

```bash
# 指定模型
claude --model claude-sonnet-4-20250514
claude --model opus

# 设置思考深度
claude --effort low
claude --effort medium
claude --effort high
claude --effort max
```

### System Prompt

```bash
# 替换整个 system prompt
claude --system-prompt "你是一个 C++ 专家"

# 在默认 system prompt 后追加（推荐）
claude --append-system-prompt "代码注释请使用英文"

# 从文件读取
claude --append-system-prompt-file ./my-prompt.txt
```

### 权限与工具

```bash
# 设置权限模式
claude --permission-mode plan
claude --permission-mode auto

# 限制可用工具
claude --tools "Bash,Edit,Read"

# 允许特定工具
claude --allowedTools "Bash(git *),Read,Edit"

# 禁用特定工具
claude --disallowedTools "Bash(rm *)"
```

### 调试与日志

```bash
# 启用调试日志
claude --debug
claude --debug "api,mcp"

# 显示详细输出
claude --verbose

# 查看版本
claude --version
claude -v
```

### MCP 与配置

```bash
# 指定 MCP 配置文件
claude --mcp-config ./mcp.json

# 只使用指定的 MCP 配置（忽略其他来源）
claude --strict-mcp-config

# 指定额外的 settings 文件
claude --settings ./my-settings.json

# 限制 token 花费（非交互模式）
claude -p "..." --max-budget-usd 0.5

# 要求结构化 JSON 输出
claude -p "..." --json-schema '{"type":"object"}'
```

---

## 5. 配置文件与 Settings

### 配置文件位置与优先级

| 作用域 | 位置 | 说明 |
|--------|------|------|
| 用户级 | `~/.claude/settings.json` | 全局偏好，对所有项目生效 |
| 项目级 | `.claude/settings.json` | 项目规范，建议提交到 git 与团队共享 |
| 本地私有 | `.claude/settings.local.json` | 个人设置，加入 `.gitignore` |
| 受管理 | 系统管理员下发 | 企业/组织强制策略 |

> Windows 用户路径示例：`C:\Users\<用户名>\.claude\settings.json`

### 常用配置项

```json
{
  "model": "claude-sonnet-4-20250514",
  "permissions": {
    "allow": [
      "Read",
      "Edit",
      "Bash(git *)",
      "Bash(npm *)",
      "Bash(cmake *)"
    ],
    "deny": [
      "Bash(rm -rf *)",
      "Bash(sudo *)"
    ]
  },
  "ignore": [
    ".env",
    "*.pem",
    "*.key",
    "secrets/"
  ],
  "autoCompact": true,
  "compactThreshold": 80
}
```

### CLAUDE.md 加载层级

```
~/.claude/CLAUDE.md              ← 全局偏好（语言、风格）
项目根目录/CLAUDE.md              ← 项目级上下文（最重要）
项目根目录/.claude/CLAUDE.md      ← 等同于根目录
子目录/CLAUDE.md                  ← 模块级补充（进入该目录时加载）
```

---

## 6. Hooks 自动化钩子

Hooks 让你在特定事件发生时**自动执行脚本或逻辑**，实现自动化工作流。

### 常用事件类型

| 事件 | 触发时机 | 是否可阻止 |
|------|----------|-----------|
| `SessionStart` | 会话开始/恢复/清除/compact 时 | 否 |
| `InstructionsLoaded` | CLAUDE.md 等加载完成后 | 否 |
| `PreToolUse` | 工具（Bash/Edit/Read 等）调用前 | **是** |
| `PermissionRequest` | 即将提示用户权限时 | **是** |
| `PostToolUse` | 工具成功执行后 | 否 |
| `PostToolUseFailure` | 工具执行失败后 | 否 |
| `Stop` | Claude 停止输出时 | 否 |

### Handler 类型

| 类型 | 说明 |
|------|------|
| `command` | 运行本地脚本/可执行文件（stdin 接收事件 JSON） |
| `http` | POST 到指定 URL（用于远程决策或审计） |
| `prompt` / `agent` | 内部调用 LLM/SubAgent 作为 Hook |

### 配置示例

在 `.claude/settings.json` 中配置：

```json
{
  "hooks": {
    "PreToolUse": [
      {
        "matcher": "Bash",
        "hooks": [
          {
            "type": "command",
            "command": ".claude/hooks/block-dangerous.sh",
            "timeout": 5
          }
        ]
      }
    ],
    "SessionStart": [
      {
        "hooks": [
          {
            "type": "command",
            "command": ".claude/hooks/init-env.sh"
          }
        ]
      }
    ]
  }
}
```

### 常见 Hook 用途

- **SessionStart**：初始化环境变量、写入 CLAUDE_ENV_FILE
- **PreToolUse**：拦截危险命令（如 `rm -rf`），返回 `permissionDecision: "deny"`
- **PermissionRequest**：自动允许安全命令，避免重复确认
- **PostToolUse**：记录操作日志、触发通知

### 阻止工具执行（PreToolUse 脚本示例）

脚本通过 `exit 0` + stdout 返回 JSON 控制行为：

```bash
#!/bin/bash
# .claude/hooks/block-dangerous.sh
COMMAND=$(cat | jq -r '.tool_input.command // ""')
if echo "$COMMAND" | grep -qE 'rm -rf|format|del /f'; then
  jq -n '{
    hookSpecificOutput: {
      hookEventName: "PreToolUse",
      permissionDecision: "deny",
      permissionDecisionReason: "已阻止危险命令"
    }
  }'
else
  exit 0
fi
```

> **注意**：`exit 2` 表示阻止事件；脚本 stdout 必须只输出 JSON，否则解析会失败。

---

## 7. MCP（Model Context Protocol）

### 概念

MCP 是一个扩展协议，将 Claude Code 与外部**工具服务器**互联，让模型可以直接调用：
- 内部数据库查询
- 公司内部 API
- 第三方服务（Sentry、CI、监控等）
- 自定义业务逻辑

MCP Server 可以定义：**工具集合**（带输入 Schema）、**交互式 Prompts**、**OAuth 登录流程** 等。

### 配置与使用

```bash
# 使用指定 MCP 配置文件启动
claude --mcp-config ./mcp.json

# 只使用指定配置（忽略其他来源）
claude --strict-mcp-config --mcp-config ./mcp.json

# 在交互模式中管理 MCP 连接
/mcp
```

### MCP 配置文件示例（mcp.json）

```json
{
  "mcpServers": {
    "my-internal-api": {
      "command": "node",
      "args": ["./mcp-server/index.js"],
      "env": {
        "API_BASE_URL": "https://api.example.com"
      }
    },
    "database-tools": {
      "command": "python",
      "args": ["-m", "my_mcp_server"],
      "timeout": 30000
    }
  }
}
```

### MCP Prompts（动态斜杠命令）

MCP Server 暴露的 Prompts 会自动以 `/mcp__<服务器名>__<prompt名>` 的形式成为斜杠命令，可在交互模式中直接使用。

### 安全建议

- 使用 `allowedMcpServers` / `deniedMcpServers` 控制访问
- 敏感业务逻辑放在服务器端，不在客户端暴露
- 结合 Hooks 对 MCP 工具调用做二次审查

---

## 8. 最佳实践

### CLAUDE.md 维护

- 把团队约定、构建命令、代码风格写入 `CLAUDE.md`
- 使用 `/init` 命令自动生成初始 `CLAUDE.md`
- 避免在 CLAUDE.md 中包含大段源代码或敏感信息
- 文件建议控制在 2000 行以内

### 上下文管理

- 定期使用 `/compact` 压缩上下文，减少 token 费用
- 用 `/cost` 关注当前会话的花费
- 使用 `--append-system-prompt` 追加指令，避免替换默认能力

### 配置管理

- 项目共享配置放 `.claude/settings.json`（提交到 git）
- 个人私有设置放 `.claude/settings.local.json`（加入 `.gitignore`）
- 用 Hooks 实现自动化，而不是把规则依赖 AI 记忆

### 安全习惯

- 审查每次文件修改，不盲目接受
- 不在提示中包含 API Key、密码等敏感信息
- 配置合理的 `deny` 规则（特别是 `rm -rf`、`sudo` 等危险命令）
- 使用 `git diff` 审查所有改动后再提交

### 提示撰写

- 提供足够的上下文（文件路径、函数名、错误信息）
- 明确约束（不要改哪些文件、保持哪些兼容性）
- 使用参考实例（"参照 xxx 模块的结构"）
- 复杂任务分步执行，每步完成后验证

---

## 附录：官方文档链接

| 文档 | 链接 |
|------|------|
| 斜杠命令完整列表 | https://code.claude.com/docs/en/commands.md |
| CLI 参数参考 | https://code.claude.com/docs/en/cli-reference.md |
| 交互式模式与快捷键 | https://code.claude.com/docs/en/interactive-mode.md |
| 键位绑定配置 | https://code.claude.com/docs/en/keybindings.md |
| 配置与权限（Settings） | https://code.claude.com/docs/en/settings.md |
| Hooks 详细文档 | https://code.claude.com/docs/en/hooks.md |
| Hooks 使用指南 | https://code.claude.com/docs/en/hooks-guide.md |
| MCP 协议 | https://code.claude.com/docs/en/mcp.md |

---

> 本文档基于 Claude Code 实际使用经验整理，随版本更新持续维护。
