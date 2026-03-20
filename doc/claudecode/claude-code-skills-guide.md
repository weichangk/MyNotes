# Claude Code Skills 完整使用指南

> Skills（技能）是 Claude Code 最强大的扩展机制：把你的团队工作流、编码规范、重复性任务封装成可复用的"斜杠命令"（`/skill-name`），一次编写，随处调用。

---

## 目录

- [一、什么是 Skills](#一什么是-skills)
- [二、Skills 文件结构](#二skills-文件结构)
- [三、SKILL.md frontmatter 完整字段](#三skillmd-frontmatter-完整字段)
- [四、在 SKILL.md 中使用变量](#四在-skillmd-中使用变量)
- [五、Skills 的存放位置与优先级](#五skills-的存放位置与优先级)
- [六、如何调用 Skills](#六如何调用-skills)
- [七、Skills vs Hooks vs MCP 的选择](#七skills-vs-hooks-vs-mcp-的选择)
- [八、日常开发实战：10 个开箱即用的 Skills](#八日常开发实战10-个开箱即用的-skills)
- [九、进阶技巧](#九进阶技巧)
- [十、Skills 工作流总结](#十skills-工作流总结)

---

## 一、什么是 Skills

Skills 是以 Markdown 文件为入口的"可调用能力单元"，本质是给 Claude 的**可复用 Prompt 模板 + 工作流说明**。

```
用户输入 /fix-issue 123
        ↓
Claude 加载对应 SKILL.md 的内容（注入参数 123）
        ↓
按 SKILL.md 中的指令执行完整工作流（读文件、分析、修改、提交…）
```

**Skills 能做什么：**
- 把"修复 GitHub Issue"、"生成 PR 摘要"、"代码审查"封装成一条命令
- 在 SKILL.md 里描述完整的多步骤工作流，让 Claude 自主执行
- 通过 `!`command`` 在发给模型前自动注入 git diff、PR 内容等动态上下文
- 在隔离的子 Agent 中运行（不污染主会话上下文）

**与其他功能的关系：**

| 功能 | 本质 | 触发方式 |
|------|------|----------|
| **Skills** | LLM 驱动的可复用工作流 | `/skill-name` 或 Claude 自动匹配 |
| **Hooks** | 生命周期事件上的确定性脚本 | 系统事件（工具调用前后、会话启动等） |
| **MCP** | 接入外部服务/工具的接口 | 模型通过工具调用 API 调用 |
| **CLAUDE.md** | 项目级持久上下文/偏好 | 每次会话自动加载 |

---

## 二、Skills 文件结构

每个 Skill 是一个**目录**，目录名即命令名：

```
.claude/skills/
├── fix-issue/
│   └── SKILL.md          # 必需：入口文件（frontmatter + 指令）
├── pr-summary/
│   ├── SKILL.md          # 入口
│   └── template.md       # 可选：输出模板
├── deploy/
│   ├── SKILL.md
│   └── scripts/
│       └── pre-deploy-check.sh   # 可选：辅助脚本
└── code-review/
    ├── SKILL.md
    ├── checklist.md      # 可选：审查清单（供 Claude 参考）
    └── examples/
        └── good-pr.md    # 可选：示例输出
```

### SKILL.md 基本格式

```markdown
---
name: skill-name
description: 一句话描述，Claude 根据它判断何时自动使用此技能
---

这里是给 Claude 的完整指令，可以是：
- 多步骤工作流
- 输出格式要求
- 约束条件
- 参考信息

使用参数：$ARGUMENTS
```

---

## 三、SKILL.md frontmatter 完整字段

```yaml
---
# ── 基础信息 ──────────────────────────────────────────
name: my-skill                    # 命令名，小写字母/数字/连字符，最多64字符
                                  # 省略则使用目录名
description: 简短描述技能用途       # 推荐填写，Claude 用它判断何时自动加载

argument-hint: "<issue-number>"   # 在 / 菜单中显示的参数提示

# ── 调用控制 ──────────────────────────────────────────
disable-model-invocation: false   # true = 禁止 Claude 自动触发，只能用户手动 /skill-name
                                  # 对有副作用的命令（deploy/commit）务必设为 true
user-invocable: true              # false = 从 / 菜单隐藏，仅 Claude 内部使用作为知识

# ── 执行环境 ──────────────────────────────────────────
context: fork                     # fork = 在隔离的子 Agent 中运行（不影响主会话上下文）
agent: Explore                    # 配合 context: fork，指定子 Agent 类型：
                                  #   Explore（只读探索）
                                  #   Plan（规划/架构）
                                  #   general-purpose（通用）
model: claude-sonnet-4-5          # 指定运行此技能使用的模型（可选）

# ── 权限控制 ──────────────────────────────────────────
allowed-tools:                    # 技能运行期间允许 Claude 使用的工具白名单
  - Read
  - Grep
  - Glob
  - Bash(git *)
  - Bash(gh *)
  - Edit
  - Write
---
```

### 字段速查

| 字段 | 默认值 | 作用 |
|------|--------|------|
| `name` | 目录名 | 斜杠命令名称 |
| `description` | — | 触发判断依据，**强烈建议填写** |
| `disable-model-invocation` | `false` | 防止 Claude 自动触发有副作用的命令 |
| `user-invocable` | `true` | 是否出现在 `/` 菜单 |
| `context: fork` | — | 隔离执行，避免污染主会话 |
| `agent` | — | 配合 fork 指定子 Agent 类型 |
| `allowed-tools` | 继承全局 | 精细控制技能可用工具 |

---

## 四、在 SKILL.md 中使用变量

### 4.1 参数变量

```markdown
---
name: fix-issue
---

# 修复 GitHub Issue

修复 issue $ARGUMENTS（完整参数字符串）

或按位置取参数：
- 第一个参数：$ARGUMENTS[0]（等同于 $0）
- 第二个参数：$ARGUMENTS[1]（等同于 $1）
```

**调用示例：**
```
/fix-issue 123              → $ARGUMENTS = "123"
/fix-issue 123 --urgent     → $ARGUMENTS[0] = "123", $ARGUMENTS[1] = "--urgent"
```

### 4.2 内置变量

```markdown
---
name: log-session
---

会话 ID：${CLAUDE_SESSION_ID}
技能目录：${CLAUDE_SKILL_DIR}
```

| 变量 | 说明 |
|------|------|
| `$ARGUMENTS` | 调用时传入的全部参数 |
| `$ARGUMENTS[N]` / `$N` | 第 N 个参数（0 开始） |
| `${CLAUDE_SESSION_ID}` | 当前会话唯一 ID |
| `${CLAUDE_SKILL_DIR}` | 技能所在目录的绝对路径 |

### 4.3 预处理命令插值（动态上下文注入）

用 `` !`command` `` 语法在**发给模型前**先执行 Shell 命令，把输出注入到 prompt 中：

```markdown
---
name: pr-review
allowed-tools:
  - Bash(gh *)
---

## 当前 PR 信息

变更文件：
!`gh pr diff --name-only`

完整 diff：
!`gh pr diff`

PR 描述与评论：
!`gh pr view --comments`

## 任务
根据以上信息进行代码审查...
```

> **注意**：`` !`command` `` 是预处理，在 prompt 送达模型**之前**执行，模型看到的是命令输出，而不是命令本身。

---

## 五、Skills 的存放位置与优先级

```
优先级（高 → 低）

企业管理配置（managed）
        ↓
~/.claude/skills/<skill-name>/SKILL.md     ← 个人全局（所有项目可用）
        ↓
.claude/skills/<skill-name>/SKILL.md       ← 项目级（当前项目可用，推荐团队共享）
        ↓
插件中的 skills/（通过 plugin:skill-name 调用）
```

### 推荐目录规划

```
~/.claude/skills/              # 个人通用技能
├── explain-code/
├── write-commit/
└── debug-help/

<your-project>/.claude/skills/ # 项目专用技能（提交到 git 共享给团队）
├── fix-issue/
├── pr-summary/
├── deploy/
└── code-review/
```

---

## 六、如何调用 Skills

### 6.1 手动调用（最常用）

```
# 在 Claude Code 会话中直接输入
/skill-name                        # 无参数调用
/skill-name arg1                   # 带参数
/skill-name arg1 arg2              # 多个参数
/fix-issue 123                     # 示例：修复 issue 123
/pr-summary feature/login          # 示例：生成 PR 摘要
```

### 6.2 Claude 自动触发

只要 `disable-model-invocation` 不为 `true`，Claude 会根据技能的 `description` 字段，在认为合适的时机自动加载技能内容来辅助回答。

```yaml
# Claude 在你提问"帮我解释这段代码"时会自动加载此技能
description: 解释代码逻辑，包含类比、ASCII 图和逐步讲解
```

### 6.3 在 / 菜单中发现技能

输入 `/` 会列出所有可用技能（受 `user-invocable` 控制）。

### 6.4 定时/循环调用

```
/loop 30m /pr-summary              # 每 30 分钟执行一次 pr-summary
```

---

## 七、Skills vs Hooks vs MCP 的选择

```
我想…                              → 使用
─────────────────────────────────────────────────────
封装一个可重复的多步骤工作流          → Skill
让 Claude 学会"如何做某件事"          → Skill（user-invocable: false）
在工具调用前强制做安全检查            → Hook（PreToolUse）
自动格式化保存的文件                  → Hook（PostToolUse）
接入外部数据库/API                   → MCP
访问 GitHub / Slack 等外部服务        → MCP
防止执行危险的 shell 命令             → Hook（exit 2 阻断）
```

---

## 八、日常开发实战：10 个开箱即用的 Skills

以下技能可直接复制到 `.claude/skills/<目录名>/SKILL.md`。

---

### Skill 1：修复 GitHub Issue（`/fix-issue`）

`.claude/skills/fix-issue/SKILL.md`

```markdown
---
name: fix-issue
description: 读取并修复指定编号的 GitHub Issue，包含测试和提交
argument-hint: "<issue-number>"
disable-model-invocation: true
allowed-tools:
  - Read
  - Edit
  - Write
  - Bash(git *)
  - Bash(gh *)
  - Grep
  - Glob
---

# 修复 GitHub Issue #$ARGUMENTS

## 工作流程

1. **读取 Issue**
   - 运行 `gh issue view $ARGUMENTS` 获取详情
   - 理解问题描述、复现步骤和期望行为

2. **分析代码**
   - 根据 Issue 描述定位相关文件
   - 理解现有实现逻辑

3. **实现修复**
   - 在合适的位置做最小化改动
   - 遵循项目现有的代码风格

4. **验证**
   - 如果存在测试文件，为修复添加对应测试
   - 确保现有测试不被破坏

5. **提交**
   - 生成符合项目规范的 commit 信息
   - 格式：`fix: <简洁描述> (close #$ARGUMENTS)`

完成后输出：改动文件列表、修复摘要、commit hash。
```

---

### Skill 2：生成 PR 摘要（`/pr-summary`）

`.claude/skills/pr-summary/SKILL.md`

```markdown
---
name: pr-summary
description: 为当前或指定 PR 生成结构化摘要，包含变更说明和测试建议
argument-hint: "[pr-number]"
context: fork
agent: Explore
allowed-tools:
  - Bash(gh *)
  - Read
---

# PR 摘要生成

## PR 上下文

变更文件：
!`gh pr diff --name-only 2>/dev/null || git diff main --name-only`

PR diff：
!`gh pr diff 2>/dev/null || git diff main`

PR 详情：
!`gh pr view --comments 2>/dev/null || echo "（无 PR 信息，基于 git diff 分析）"`

## 输出格式

请生成以下结构的 PR 摘要：

### 变更摘要
（2-3 句话概括本次变更的目的）

### 主要改动
- 文件/模块：改动内容描述
- ...

### 潜在风险
（列出需要重点关注的地方，若无则写"无明显风险"）

### 测试建议
- [ ] 测试场景 1
- [ ] 测试场景 2

### 备注
（其他需要 reviewer 了解的信息）
```

---

### Skill 3：代码审查（`/code-review`）

`.claude/skills/code-review/SKILL.md`

```markdown
---
name: code-review
description: 对最近的变更或指定文件进行全面代码审查
argument-hint: "[file-or-pr]"
context: fork
agent: Explore
allowed-tools:
  - Read
  - Grep
  - Glob
  - Bash(git *)
  - Bash(gh *)
---

# 代码审查

## 审查目标
$ARGUMENTS（若未指定，审查 `git diff HEAD~1` 的变更）

## 获取变更内容

!`git diff HEAD~1 --stat 2>/dev/null || echo "无法获取 git diff"`

## 审查维度

请按以下维度逐一审查，发现问题时引用具体文件和行号：

### 1. 正确性
- 逻辑是否有 bug？
- 边界条件是否处理？
- 错误处理是否完善？

### 2. 安全性
- 是否存在注入风险？
- 是否有硬编码的密钥/密码？
- 权限检查是否到位？

### 3. 性能
- 是否有 N+1 查询？
- 循环内是否有不必要的重复计算？
- 内存/资源是否正确释放？

### 4. 可维护性
- 函数/方法是否过长（>50行）？
- 命名是否清晰？
- 是否有重复代码可以抽取？

### 5. 测试覆盖
- 核心逻辑是否有测试？
- 边界条件是否有测试？

## 输出格式

- 🔴 **必须修复**：影响功能/安全的问题
- 🟡 **建议改进**：影响质量的问题
- 🟢 **可以考虑**：优化建议
- ✅ **做得好**：值得肯定的地方

最后给出整体评分（1-5分）和一句话总结。
```

---

### Skill 4：智能提交（`/smart-commit`）

`.claude/skills/smart-commit/SKILL.md`

```markdown
---
name: smart-commit
description: 分析 staged 变更，生成规范的 commit 信息并提交
disable-model-invocation: true
allowed-tools:
  - Bash(git *)
---

# 智能提交

## 当前 Staged 变更

!`git diff --staged --stat`

## 变更详情

!`git diff --staged`

## 任务

根据以上变更内容：

1. **分析变更类型**，从以下选择最合适的 prefix：
   - `feat:` 新功能
   - `fix:` Bug 修复
   - `refactor:` 重构（无功能变化）
   - `style:` 格式/样式
   - `test:` 测试相关
   - `docs:` 文档
   - `chore:` 构建/工具/依赖

2. **生成 commit 信息**，格式：
   ```
   <type>: <简洁的中文描述>（50字以内）

   <可选：详细说明，说明"为什么"而不是"做了什么">
   ```

3. **执行提交**：`git commit -m "<生成的信息>"`

4. 输出：最终使用的 commit 信息和 commit hash。

如果没有 staged 变更，提示用户先运行 `git add`。
```

---

### Skill 5：解释代码（`/explain`）

`.claude/skills/explain/SKILL.md`

```markdown
---
name: explain
description: 深度解释代码逻辑，包含类比、流程图和常见陷阱。在用户问"这段代码做什么"时自动使用。
argument-hint: "<file-or-function>"
allowed-tools:
  - Read
  - Grep
---

# 代码解释：$ARGUMENTS

请用以下结构解释代码，让没有上下文的人也能理解：

## 1. 一句话总结
（这段代码做什么，用最简单的语言）

## 2. 生活类比
（用一个生活中的例子类比它的工作方式）

## 3. 执行流程
（ASCII 流程图或有序步骤）

```
输入 → [步骤1] → [步骤2] → 输出
```

## 4. 关键细节逐行讲解
（聚焦在最复杂/最重要的部分）

## 5. 常见陷阱
（使用这段代码时容易踩的坑）

## 6. 相关文件/依赖
（这段代码依赖什么、被什么依赖）
```

---

### Skill 6：生成测试（`/gen-tests`）

`.claude/skills/gen-tests/SKILL.md`

```markdown
---
name: gen-tests
description: 为指定文件或函数生成完整的单元测试，覆盖正常路径、边界条件和异常处理
argument-hint: "<file-path>"
disable-model-invocation: true
allowed-tools:
  - Read
  - Write
  - Glob
  - Grep
---

# 生成单元测试：$ARGUMENTS

## 工作流程

1. **读取目标文件**：`$ARGUMENTS`
   - 理解所有公共函数/方法的签名和行为

2. **查找现有测试**
   - 搜索 `__tests__/`、`*.test.*`、`*_test.*` 等测试文件
   - 避免重复，遵循现有测试风格

3. **生成测试，覆盖：**
   - ✅ 正常路径（Happy Path）
   - ✅ 边界条件（空值、零、最大值等）
   - ✅ 异常/错误处理
   - ✅ 重要的业务逻辑分支

4. **测试文件命名**：
   - 与源文件同名，加 `.test.` 或 `_test.` 后缀
   - 放在 `__tests__/` 目录或与源文件同级

5. **写入测试文件**

完成后输出：生成的测试文件路径、测试用例数量和覆盖的函数列表。
```

---

### Skill 7：查找相关代码（`/find`）

`.claude/skills/find/SKILL.md`

```markdown
---
name: find
description: 在代码库中搜索与关键词相关的所有实现，包括定义、调用和测试
argument-hint: "<keyword-or-function-name>"
context: fork
agent: Explore
allowed-tools:
  - Read
  - Grep
  - Glob
---

# 代码搜索：$ARGUMENTS

请全面搜索代码库中与 `$ARGUMENTS` 相关的所有内容：

## 搜索范围

1. **定义位置**：函数/类/变量/常量在哪里定义？
2. **调用位置**：被哪些文件调用？传入了什么参数？
3. **测试位置**：有没有对应的测试？测试覆盖了哪些场景？
4. **文档位置**：有没有相关注释或文档说明？

## 输出格式

### 定义
- `路径:行号` - 简要说明

### 主要调用位置（最多10个）
- `路径:行号` - 调用方式

### 测试文件
- `路径` - 测试了哪些场景

### 总结
- 功能用途一句话
- 使用时注意事项
```

---

### Skill 8：重构建议（`/refactor`）

`.claude/skills/refactor/SKILL.md`

```markdown
---
name: refactor
description: 分析指定代码并给出可落地的重构建议，按优先级排序
argument-hint: "<file-or-directory>"
disable-model-invocation: true
allowed-tools:
  - Read
  - Grep
  - Glob
  - Edit
---

# 重构分析：$ARGUMENTS

## 分析维度

阅读 `$ARGUMENTS` 后，从以下维度分析问题：

### 代码异味（Code Smells）
- 过长的函数（>50行）
- 过深的嵌套（>3层）
- 重复代码（DRY 原则）
- 过大的类（>300行）
- 魔法数字/字符串
- 注释说明"做了什么"而不是"为什么"

### 设计问题
- 单一职责原则违反
- 不必要的耦合
- 可以用策略模式替换的 if/switch 链
- 可以提取的公共逻辑

## 输出格式

按优先级列出建议：

### 🔴 高优先级（影响可维护性）
| 文件:行号 | 问题描述 | 重构方案 |

### 🟡 中优先级（代码质量改进）
| 文件:行号 | 问题描述 | 重构方案 |

### 🟢 低优先级（可选优化）
| 文件:行号 | 问题描述 | 重构方案 |

询问用户：是否需要自动应用某项重构？
```

---

### Skill 9：快速文档（`/docgen`）

`.claude/skills/docgen/SKILL.md`

```markdown
---
name: docgen
description: 为指定文件或函数生成或更新代码注释和文档
argument-hint: "<file-path>"
disable-model-invocation: true
allowed-tools:
  - Read
  - Edit
  - Glob
---

# 生成文档：$ARGUMENTS

## 任务

为 `$ARGUMENTS` 中所有**缺少或注释不完整**的公共函数/方法/类生成文档注释。

## 文档规范

根据文件语言选择对应格式：

- **TypeScript/JavaScript**：JSDoc 格式
  ```js
  /**
   * 函数作用的一句话描述
   * @param {type} paramName - 参数说明
   * @returns {type} 返回值说明
   * @throws {ErrorType} 何时抛出错误
   * @example
   * // 使用示例
   */
  ```

- **Python**：Google Style Docstring
- **Go**：godoc 格式
- **C++/C**：Doxygen 格式
- **Java**：Javadoc 格式

## 原则

- 只写"为什么"和"使用场景"，不写"做了什么"（代码本身说明做了什么）
- 对复杂参数或返回值给出示例
- 对抛出的异常说明触发条件
- 不改动任何业务逻辑代码

完成后输出：修改了多少个函数/方法的文档。
```

---

### Skill 10：项目健康检查（`/health-check`）

`.claude/skills/health-check/SKILL.md`

```markdown
---
name: health-check
description: 对当前项目做全面健康检查，输出待改进项的优先级列表
disable-model-invocation: true
context: fork
agent: Explore
allowed-tools:
  - Read
  - Grep
  - Glob
  - Bash(git *)
---

# 项目健康检查

## 检查范围

### 1. 代码质量
- 找出超过 200 行的文件（过大类/模块）
- 找出超过 50 行的函数
- 找出含有 TODO/FIXME/HACK 注释的位置
- 找出重复代码块

### 2. 依赖健康
- 检查 `package.json` / `requirements.txt` / `go.mod` 等依赖文件
- 标注明显过时的依赖
- 找出未使用的依赖（如有 lock 文件）

### 3. 测试覆盖
- 有测试的模块 vs 没有测试的模块
- 找出完全没有测试的核心业务文件

### 4. 安全隐患
- 搜索硬编码的 token/password/secret 关键词
- 检查 .gitignore 是否遗漏了敏感文件

### 5. Git 卫生
!`git log --oneline -20`
!`git status`

## 输出格式

### 整体评分：X/10

| 类别 | 状态 | 关键问题 |
|------|------|----------|
| 代码质量 | 🔴/🟡/🟢 | ... |
| 依赖健康 | 🔴/🟡/🟢 | ... |
| 测试覆盖 | 🔴/🟡/🟢 | ... |
| 安全 | 🔴/🟡/🟢 | ... |
| Git 卫生 | 🔴/🟡/🟢 | ... |

### Top 5 优先改进项
1. ...
2. ...
```

---

## 九、进阶技巧

### 9.1 用 `context: fork` 隔离大型任务

对于需要读取大量文件的任务，在独立子 Agent 中执行，避免占满主会话的上下文窗口：

```yaml
---
name: deep-analysis
context: fork
agent: Explore
allowed-tools:
  - Read
  - Grep
  - Glob
---
```

### 9.2 `disable-model-invocation: true` 保护有副作用的命令

任何会修改文件、执行部署、发起网络请求的技能，都应防止 Claude 自动触发：

```yaml
---
name: deploy
disable-model-invocation: true    # ← 必须手动 /deploy 才能触发
---
```

### 9.3 用 `user-invocable: false` 创建"隐式知识"

把编码规范、项目约定写成 Skill，让 Claude 在相关对话中自动参考，但不暴露为斜杠命令：

```markdown
---
name: coding-standards
description: 本项目的编码规范：命名约定、注释风格、错误处理模式
user-invocable: false             # ← 不出现在 / 菜单，Claude 根据 description 自动参考
---

## 命名规范
- 变量/函数：camelCase
- 类：PascalCase
- 常量：UPPER_SNAKE_CASE

## 错误处理
- 所有异步函数必须有 try/catch
...
```

### 9.4 用 `allowed-tools` 做最小权限原则

只读型技能应限制工具权限：

```yaml
allowed-tools:
  - Read
  - Grep
  - Glob
  # 注意：不包含 Edit/Write/Bash，防止只读任务意外修改文件
```

### 9.5 把参考资料分离到独立文件

```
.claude/skills/code-review/
├── SKILL.md          # 保持简短（<100行）
├── checklist.md      # 详细审查清单（Claude 按需读取）
└── examples/
    ├── good-review.md
    └── bad-review.md
```

在 `SKILL.md` 中引用：

```markdown
详细审查清单见 ${CLAUDE_SKILL_DIR}/checklist.md，
好的审查示例见 ${CLAUDE_SKILL_DIR}/examples/good-review.md。
```

---

## 十、Skills 工作流总结

### 创建一个新 Skill 的完整步骤

```
1. 确定任务类型
   ├── 有副作用（提交/部署/发请求）→ disable-model-invocation: true
   ├── 需要大量读取代码库        → context: fork, agent: Explore
   └── 只是提供知识/规范         → user-invocable: false

2. 创建目录
   mkdir -p .claude/skills/<skill-name>

3. 写 SKILL.md
   ├── frontmatter（至少填 name + description）
   └── 指令正文（清晰的步骤 + 输出格式要求）

4. 测试
   # 在 Claude Code 会话中
   /skill-name [args]

5. 迭代优化
   ├── 如果 Claude 不按预期执行 → 把指令写得更具体
   ├── 如果上下文太长          → 用 context: fork 或拆分文件
   └── 如果被误触发            → 加 disable-model-invocation: true
```

### 日常开发推荐的 Skills 配置

```
~/.claude/skills/（个人全局）
├── explain/          # /explain - 解释任何代码
├── smart-commit/     # /smart-commit - 智能提交
└── find/             # /find - 全库搜索

<project>/.claude/skills/（项目团队共享，提交到 git）
├── fix-issue/        # /fix-issue <n> - 修复 Issue
├── pr-summary/       # /pr-summary - 生成 PR 摘要
├── code-review/      # /code-review - 代码审查
├── gen-tests/        # /gen-tests <file> - 生成测试
├── refactor/         # /refactor <file> - 重构建议
├── docgen/           # /docgen <file> - 生成文档
├── health-check/     # /health-check - 项目健康检查
└── coding-standards/ # 隐式知识（user-invocable: false）
```

### 典型工作日使用场景

```
早上开始工作
  → /health-check                    快速了解项目状态

接到 Issue
  → /fix-issue 123                   分析并修复

改完代码
  → /code-review                     自我审查
  → /gen-tests src/auth.ts           补全测试
  → /smart-commit                    规范提交

提 PR 前
  → /pr-summary                      生成 PR 描述

遇到不熟悉的代码
  → /explain src/utils/crypto.ts     深度解释
  → /find parseJWT                   找所有相关代码
```
