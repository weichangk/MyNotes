# compound-engineering 插件完整使用指南

> Claude Code 的工程团队专属插件，将规划、并行执行、代码审查、研究、发布等工程任务封装为可调用的 Skills 和 Agents。

---

## 目录

- [1. 插件概述与设计理念](#1-插件概述与设计理念)
- [2. 全部斜杠命令](#2-全部斜杠命令)
- [3. Review Agents（代码审查专家）](#3-review-agents代码审查专家)
- [4. Research Agents（研究专家）](#4-research-agents研究专家)
- [5. 典型工作流（端到端）](#5-典型工作流端到端)
- [6. Hooks 自动化配合](#6-hooks-自动化配合)
- [7. Agent-Native 设计理念](#7-agent-native-设计理念)
- [8. 最佳实践](#8-最佳实践)

---

## 1. 插件概述与设计理念

**compound-engineering** 是一个面向工程团队的**复合能力插件**，把常见工程任务（规划、并行执行、代码审查、研究、发布说明生成）封装为可调用的 Skills 和 Agents。

### 核心用途

- 快速把模糊需求转成可执行计划（`/brainstorm`、`/plan`）
- 并行化与编排实现（`/lfg`、`/slfg`、`/work`、`/batch`）
- 自动化代码审查与架构评审（`/review` 系列、各类 Review Agents）
- 生成对外文档（变更日志 `/changelog`、功能演示 `/feature-video`）
- 复现、排查并上报 Bug（`/reproduce-bug`、`/report-bug`）

### 设计理念

| 理念 | 说明 |
|------|------|
| **拆分** | 把复杂工作拆成 研究→规划→实现→审查→部署 的可复用任务 |
| **并行化** | 通过多个 SubAgents + Worktrees 并行处理可分片工作 |
| **可审计** | 每步输出结构化结果（评分、引用、审查报告） |
| **最小权限** | 通过 settings.json / Hooks 控制工具可用性与自动化审批 |
| **可扩展** | 以 Skills + Agents + Hooks + MCP 插件化方式扩展团队能力 |

---

## 2. 全部斜杠命令

### 🧠 规划 & 头脑风暴

| 命令 | 说明 |
|------|------|
| `/compound-engineering:\workflows:brainstorm` | 探索需求、讨论多种实现方案，**实现前先用** |
| `/compound-engineering:\workflows:plan` | 把选定方案细化为任务列表、里程碑、验收标准 |
| `/compound-engineering:deepen-plan` | 启动并行 Research Agents 深化计划（加入最佳实践和细节） |
| `/compound-engineering:plan_review` | 多个专家 Agent 并行评审计划 |

### ⚡ 并行执行 & 任务管理

| 命令 | 说明 |
|------|------|
| `/compound-engineering:lfg` | **Let's Go!** 把大任务拆分给多个 SubAgents 并行执行（全自动工程工作流） |
| `/compound-engineering:slfg` | **Swarm 模式**的 lfg，更强并行化（适合 5-30 个并行 Worktrees） |
| `/compound-engineering:\workflows:work` | 执行某个具体任务，生成改动并提交 PR 草稿 |
| `/compound-engineering:\workflows:review` | 对变更执行全面多 Agent 并行代码审查 |

**`/lfg` vs `/slfg` 区别：**

| | `/lfg` | `/slfg` |
|-|--------|---------|
| 适用场景 | 中等规模并行任务（2-10 个子任务） | 大批量作业（5-30+ 个并行 Worktrees） |
| 模式 | 标准多 Agent 协作 | Swarm 协作模式 |
| 推荐场景 | 跨文件大规模改动 | 批量迁移、大规模重构 |

### 🔍 PR & TODO 并行处理

| 命令 | 说明 |
|------|------|
| `/compound-engineering:resolve_pr_parallel` | 并行处理 PR 上的所有 Review 评论 |
| `/compound-engineering:resolve_parallel` | 并行处理代码中所有 TODO 注释 |
| `/compound-engineering:resolve_todo_parallel` | 并行处理所有待办 CLI todos |

### 📚 研究 & 知识管理

| 命令 | 说明 |
|------|------|
| `/compound-engineering:\workflows:compound` | 把已解决的问题记录为团队知识文档 |
| `/compound-engineering:triage` | 对发现的问题进行分类和优先级排序 |
| `/compound-engineering:compound-docs` | 把解决方案记录为带 YAML frontmatter 的分类文档 |

### 🐛 Bug 相关

| 命令 | 说明 |
|------|------|
| `/compound-engineering:report-bug` | 把问题描述转化为可执行的 bug ticket（含复现步骤、优先级） |
| `/compound-engineering:reproduce-bug` | 自动尝试复现问题（运行脚本、抓日志、截图） |

### 📦 发布 & 文档

| 命令 | 说明 |
|------|------|
| `/compound-engineering:changelog` | 基于最近 commits/PRs 生成变更日志 |
| `/compound-engineering:feature-video` | 录制功能演示视频并添加到 PR 描述 |
| `/compound-engineering:deploy-docs` | 验证并准备文档部署到 GitHub Pages |
| `/compound-engineering:release-docs` | 构建并更新文档站点 |

### 🧩 Skill & Agent 开发

| 命令 | 说明 |
|------|------|
| `/compound-engineering:create-agent-skill` | 创建新的 Claude Code Skill（含 SKILL.md 模板和最佳实践） |
| `/compound-engineering:skill-creator` | 专家引导创建/改进 Skill |
| `/compound-engineering:create-agent-skills` | 批量创建多个 Skills |
| `/compound-engineering:heal-skill` | 修复 SKILL.md 中错误的指令或过时的 API 引用 |
| `/compound-engineering:generate_command` | 按规范生成新的自定义斜杠命令 |

### 🔒 审计 & 测试

| 命令 | 说明 |
|------|------|
| `/compound-engineering:agent-native-audit` | 全面的 Agent 原生架构评审（含量化评分） |
| `/compound-engineering:test-browser` | 对当前 PR 或分支影响的页面运行浏览器测试 |
| `/compound-engineering:xcode-test` | 在模拟器上构建并测试 iOS 应用 |
| `/compound-engineering:agent-browser` | 浏览器自动化操作（填表单、截图、抓取数据） |

### 🎨 设计相关

| 命令 | 说明 |
|------|------|
| `/compound-engineering:design:design-iterator` | 对设计进行 N 次迭代改进（默认循环截图→分析→改进） |
| `/compound-engineering:design:design-implementation-reviewer` | 验证 UI 实现是否匹配 Figma 设计规范 |
| `/compound-engineering:design:figma-design-sync` | 自动检测并修复实现与 Figma 设计的视觉差异 |

---

## 3. Review Agents（代码审查专家）

每个 Review Agent 专注于不同的审查维度，可单独调用或通过 `/review` 批量启动：

### 架构 & 代码质量

| Agent | 职责 | 检查重点 | 典型使用场景 |
|-------|------|----------|-------------|
| `architecture-strategist` | 架构合理性审查 | 服务边界、耦合度、可扩展性、部署复杂度 | 大型重构、新增模块 |
| `code-simplicity-reviewer` | 简化与可读性 | 过度抽象、重复代码、命名一致性 | 功能开发完成后的轻量审查 |
| `pattern-recognition-specialist` | 设计模式与反模式 | Anti-pattern、命名规范、代码重复 | 全局代码质量检查 |

### 安全 & 性能

| Agent | 职责 | 检查重点 | 典型使用场景 |
|-------|------|----------|-------------|
| `security-sentinel` | 全面安全审查 | 硬编码密钥、注入漏洞、权限边界、OWASP | 支付、认证相关改动 |
| `performance-oracle` | 性能与可扩展性 | O(N²) 算法、N+1 查询、锁竞争、缓存策略 | 高并发模块、批处理 |

### 数据 & 部署

| Agent | 职责 | 检查重点 | 典型使用场景 |
|-------|------|----------|-------------|
| `data-integrity-guardian` | 数据持久层审查 | 迁移安全、事务边界、数据丢失风险 | Schema 变更、数据迁移 |
| `data-migration-expert` | 数据迁移专项 | ID 映射验证、枚举迁移、数据转码 | ID 重命名、枚举变化 |
| `deployment-verification-agent` | 部署验证清单 | Pre/Post SQL 验证、监控、回滚策略 | 生产部署前的 Go/No-Go |
| `schema-drift-detector` | Schema 漂移检测 | schema.rb 中无关的变更（来自其他分支） | 合并含 DB 变更的 PR 前 |

### 语言 & 框架专家

| Agent | 职责 | 典型使用场景 |
|-------|------|-------------|
| `dhh-rails-reviewer` | Rails/DHH 风格合规 | Rails 项目审查 |
| `kieran-rails-reviewer` | 严格 Rails 代码质量 | Rails 项目高标准审查 |
| `kieran-python-reviewer` | 严格 Python 代码质量 | Python 项目审查 |
| `kieran-typescript-reviewer` | TypeScript 严格质量 | TS/React 项目审查 |
| `julik-frontend-races-reviewer` | 前端竞争条件检测 | Stimulus/JS 控制器 |

### Agent-Native 审查

| Agent | 职责 | 典型使用场景 |
|-------|------|-------------|
| `agent-native-reviewer` | 用户能做的 Agent 是否也能做 | 开放 API 或开发自动化功能前 |

---

## 4. Research Agents（研究专家）

负责聚合外部/内部资料、做事实校验、生成引用与背景分析：

| Agent | 职责 | 输入 | 输出 |
|-------|------|------|------|
| `best-practices-researcher` | 收集外部最佳实践（官方文档、开源示例） | 研究主题 | 优先级排序的实践清单 + 参考链接 |
| `framework-docs-researcher` | 聚焦框架/库文档 | 框架名 + 版本 + 目标 | 官方示例摘录、注意事项、迁移建议 |
| `learnings-researcher` | 搜索仓库内历史经验（docs/solutions/） | 问题描述或关键字 | 匹配的知识条目、以往决策 |
| `repo-research-analyst` | 分析仓库结构、规范、贡献模式 | 文件/模块/路径 | 代码演化摘要、规范指南 |
| `git-history-analyzer` | 分析代码变更历史 | 文件/模块 | 演化摘要、关键提交、设计决策来源 |
| `bug-reproduction-validator` | 系统性验证 Bug 是否可复现 | Bug 描述、复现步骤 | 复现证明或失败原因 |

---

## 5. 典型工作流（端到端）

### 标准工程工作流

```
需求描述
    │
    ▼
/brainstorm         ── 探索方案，确定思路
    │
    ▼
/plan               ── 细化任务、里程碑、验收标准
    │
    ▼
/lfg 或 /slfg      ── 拆分任务 → 多 SubAgents 并行实现
    │
    ▼
/review             ── 架构 + 安全 + 性能 + 简洁性 并行审查
    │
    ▼
/resolve_pr_parallel ── 并行处理所有 PR 评论
    │
    ▼
/changelog          ── 生成 Release Notes
    │
    ▼
deployment-verification-agent ── 生成 Pre/Post 验证清单
    │
    ▼
/compound           ── 记录经验到团队知识库
```

### 场景示例：批量代码迁移

```bash
# 1. 头脑风暴迁移方案
/brainstorm 把所有模块的 qDebug() 替换为统一的 LOG_DEBUG() 宏

# 2. 生成迁移计划
/plan 分析影响范围、分模块拆分任务、评估风险

# 3. 并行执行迁移
/slfg 用 Swarm 模式对 10 个模块并行执行替换

# 4. 批量审查变更
/review 确认每个模块的改动无副作用

# 5. 生成变更日志
/changelog 为本次迁移生成 release notes
```

### 场景示例：新功能开发

```bash
# 1. 探索实现思路
/brainstorm 为录制器添加定时录制功能

# 2. 细化实现方案
/plan 确定接口设计、模块划分、测试策略

# 3. 深化计划（可选）
/deepen-plan 让 Research Agents 补充最佳实践和实现细节

# 4. 执行实现
/work 在 RecorderEngine 中实现定时逻辑

# 5. 多维度审查
/review 覆盖架构、安全、性能三个维度
```

### 场景示例：Bug 修复

```bash
# 1. 复现问题
/reproduce-bug 录制停止后偶发崩溃，崩溃堆栈在 StopCapture() 第 342 行

# 2. 分析根因（由 research agents 辅助）
# 系统会自动启动 git-history-analyzer 查看历史变更

# 3. 修复
/work 修复多线程竞争导致的崩溃

# 4. 审查修复
/review 专注 data-integrity 和 performance 维度

# 5. 记录经验
/compound 把此次多线程问题的解决方案记录到知识库
```

---

## 6. Hooks 自动化配合

compound-engineering 常与 Hooks 配合，将审查和验证集成到自动化流程中：

### 典型 Hook 配置

```json
{
  "hooks": {
    "PreToolUse": [
      {
        "matcher": "Bash",
        "hooks": [{
          "type": "command",
          "command": ".claude/hooks/security-check.sh",
          "timeout": 10
        }]
      }
    ],
    "SessionStart": [
      {
        "hooks": [{
          "type": "command",
          "command": ".claude/hooks/init-project-env.sh"
        }]
      }
    ]
  }
}
```

### 常见 Hook 用途

| 事件 | 与插件配合的用途 |
|------|----------------|
| `SessionStart` | 注入项目环境变量、预加载代码库上下文 |
| `PreToolUse` | 在写入敏感文件前自动触发 `security-sentinel` 审查 |
| `PermissionRequest` | 自动批准 CI 流程中的安全操作，减少手动确认 |
| `PostToolUse` | 文件修改后自动触发格式化检查、自动运行测试 |
| `Stop` | 会话结束时自动触发 `/changelog` 或生成工作报告 |

---

## 7. Agent-Native 设计理念

### 核心原则

**"用户能做什么，Agent 也能做什么；用户能看到什么，Agent 也能看到什么。"**

| 原则 | 说明 |
|------|------|
| **对等性（Parity）** | 每个 UI 操作都应有对应的 API 供 Agent 调用 |
| **可观测性（Observability）** | Agent 需要结构化访问系统状态（日志、事件、快照） |
| **最小权限（Least Privilege）** | 给 Agent 最少必要权限；通过审批 Hook 控制敏感能力 |
| **幂等性（Idempotency）** | Agent 操作可以安全重试和并行执行 |
| **结构化输出（Structured Output）** | Agent 输出使用 Schema，便于自动化合并和审计 |
| **人工审批（Human-in-the-loop）** | 高风险步骤（生产迁移、功能开关）保留人工确认 |

### 落地建议

1. **为 UI 操作实现对应 API**，并通过 MCP 注册为 Agent 可调用的工具
2. **提供只读快照接口**，保证 Agent 操作的可复现性
3. **设计审计日志**，记录 Agent 的每次动作（谁/何时/为什么）
4. **高风险区域强制审批**，在 `PermissionRequest` Hook 中处理
5. **区分 Agent 和人类的提交**，便于后续审计与回溯

### agent-native-audit 评分维度

运行 `/compound-engineering:agent-native-audit` 会对以下维度评分：

- 工具可访问性（UI 操作是否都有对应 API）
- 上下文可见性（Agent 能否读取用户可见的信息）
- 权限粒度（权限是否过于宽泛）
- 输出结构化程度（是否方便自动化处理）
- 幂等性（操作是否可安全重试）

---

## 8. 最佳实践

### 选择合适的命令

```
简单单文件改动   → /work
复杂多文件改动   → /lfg
大批量批量操作   → /slfg 或 /batch
需要先探索方案   → /brainstorm → /plan → /work 或 /lfg
有 PR 评论待处理 → /resolve_pr_parallel
```

### Review Agent 使用建议

- **常规 PR**：`code-simplicity-reviewer` + `architecture-strategist`
- **涉及安全**：必加 `security-sentinel`
- **涉及数据库**：必加 `data-integrity-guardian` + `schema-drift-detector`
- **高并发模块**：必加 `performance-oracle`
- **生产部署前**：必跑 `deployment-verification-agent`

### 知识积累

- 每解决一个复杂问题后运行 `/compound` 记录经验
- 用 `learnings-researcher` 在开始新任务前先搜索历史方案
- 在 `docs/solutions/` 中维护带 YAML frontmatter 的解决方案文档

### 工作流集成

- 把 `/review` 配置为 PR merge gate（通过 Hooks + CI 集成）
- 用 `deployment-verification-agent` 生成的清单作为生产部署的 SOP
- 把 `/changelog` 集成到 release 流程中自动生成发版说明

---

## 附录：快速速查卡

```
规划阶段:   /brainstorm → /plan → /deepen-plan
实现阶段:   /work (单任务) | /lfg (多任务并行) | /slfg (大批量)
审查阶段:   /review → /resolve_pr_parallel
发布阶段:   /changelog → deployment-verification-agent
维护阶段:   /reproduce-bug → /report-bug → /compound

Review 专家: architecture | security | performance | data-integrity
              simplicity  | deployment | schema-drift | agent-native
Research 专家: best-practices | framework-docs | learnings
               repo-analyst   | git-history    | bug-reproduction
```

---

> 官方文档：https://code.claude.com/docs/en/claude_code_docs_map.md
