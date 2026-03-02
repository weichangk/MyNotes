# myCC - Claude Code with GitHub Copilot

一键启动工具，将 GitHub Copilot API 作为后端，让 Claude Code 可以使用 Copilot 服务。

## ✨ 特性

- 🚀 **一键启动** - 自动处理所有依赖和配置
- 🔐 **自动认证** - GitHub OAuth 流程，无需手动配置
- 🌐 **跨平台支持** - Windows (PowerShell/CMD) 和 macOS (Shell)
- 📦 **自动安装** - 自动安装 copilot-api 和 claude-code
- 🔄 **代理管理** - 自动启动和健康检查
- 💡 **智能提示** - 友好的错误消息和解决建议

## 🚀 快速开始

### Windows 用户

**方式 1: 直接运行（推荐）**
```powershell
# 克隆项目
git clone <repository-url>
cd mycc

# 直接运行（首次会自动安装依赖）
.\mycc.ps1
```

**方式 2: 全局安装**
```powershell
# 全局安装后可以在任意目录运行 mycc
.\install.ps1
# 或
.\install.cmd

# 然后在任意目录
mycc
```

### macOS 用户

**方式 1: 直接运行（推荐）**
```bash
# 克隆项目
git clone <repository-url>
cd mycc

# 添加可执行权限
chmod +x mycc.sh

# 直接运行（首次会自动安装依赖）
./mycc.sh
```

**方式 2: 全局安装**
```bash
# 全局安装后可以在任意目录运行 mycc
chmod +x install.sh
./install.sh

# 然后在任意目录
mycc  # 或 mycc.sh（取决于安装方式）
```

## 📋 前置要求

- **Node.js** 16+ (用于安装 npm 包)
- **Git** (用于克隆项目)
- **终端** (PowerShell 5.1+ / Bash 3.2+ / Zsh 5.x+)

## 🎯 工作原理

```
┌─────────────────┐
│  Claude Code    │  (AI 编辑器)
└────────┬────────┘
         │ API 调用
         ↓
┌─────────────────┐
│  copilot-api    │  (本地代理服务器 :4141)
└────────┬────────┘
         │ 转发请求
         ↓
┌─────────────────┐
│ GitHub Copilot  │  (AI 服务)
└─────────────────┘
```

mycc 做了什么：
1. ✅ 检查 Node.js 环境
2. ✅ 安装 copilot-api（如果缺失）
3. ✅ 安装 @anthropic-ai/claude-code（如果缺失）
4. ✅ 引导 GitHub OAuth 认证
5. ✅ 启动 copilot-api 代理服务器
6. ✅ 设置环境变量
7. ✅ 启动 Claude Code

## 📖 命令行参数

### mycc.ps1 (Windows)

```powershell
.\mycc.ps1              # 正常启动
.\mycc.ps1 -ReAuth      # 强制重新认证
.\mycc.ps1 -NoProxy     # 跳过代理启动（代理已在运行时）
```

### mycc.sh (macOS)

```bash
./mycc.sh               # 正常启动
./mycc.sh -r            # 强制重新认证
./mycc.sh --reauth      # 强制重新认证（完整形式）
./mycc.sh -n            # 跳过代理启动
./mycc.sh --no-proxy    # 跳过代理启动（完整形式）
./mycc.sh -h            # 显示帮助信息
```

## 🌍 全局安装

全局安装后，可以在任意目录直接运行 `mycc`。

### Windows

运行 `install.ps1` 或 `install.cmd`，选择安装方式：

1. **添加到 PATH**（推荐）- 文件保持原位，仅修改环境变量
2. **复制到用户 bin** - 复制文件到 `%USERPROFILE%\bin`
3. **npm 全局安装** - 使用 npm 包管理器管理

### macOS

运行 `install.sh`，选择安装方式：

1. **添加到 PATH**（推荐）- 修改 `.bashrc` 或 `.zshrc`
2. **符号链接到 /usr/local/bin** - 系统级安装（需要 sudo）
3. **复制到用户 bin** - 用户级安装，无需 sudo

## 🔧 故障排除

### 问题：PowerShell 执行策略错误

```powershell
# 临时绕过（仅当前窗口）
powershell -ExecutionPolicy Bypass -File .\mycc.ps1

# 或永久设置（需要管理员权限）
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### 问题：npm 权限错误（macOS/Linux）

```bash
# 方法 1: 使用 nvm（推荐）
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.0/install.sh | bash
nvm install node

# 方法 2: 修复 npm 权限
mkdir ~/.npm-global
npm config set prefix '~/.npm-global'
echo 'export PATH=~/.npm-global/bin:$PATH' >> ~/.bashrc
source ~/.bashrc
```

### 问题：端口 4141 被占用

```bash
# Windows
netstat -ano | findstr :4141
taskkill /PID <pid> /F

# macOS/Linux
lsof -ti:4141 | xargs kill -9
```

### 问题：认证失败

```powershell
# 重新认证
.\mycc.ps1 -ReAuth  # Windows
./mycc.sh -r        # macOS
```

更多问题请查看 [docs/knowledge/lessons/](docs/knowledge/lessons/) 目录。

## 📁 项目结构

```
mycc/
├── mycc.ps1           # Windows PowerShell 启动脚本
├── mycc.cmd           # Windows CMD 入口
├── mycc.sh            # macOS/Linux Shell 启动脚本
├── install.ps1        # Windows 全局安装脚本
├── install.cmd        # Windows 安装入口
├── install.sh         # macOS/Linux 全局安装脚本
├── docs/              # 文档
│   ├── knowledge/     # 知识库（模式、经验、决策）
│   ├── plans/         # 功能计划
│   ├── reports/       # 完成报告
│   └── review/        # 代码审查
└── scripts/           # 开发工具
    └── test/          # 测试脚本
```

## 📚 文档

- [知识库](docs/knowledge/) - 设计模式、经验教训、技术决策
- [功能计划](docs/plans/) - 详细的功能开发计划
- [代码审查](docs/review/) - 质量保证和审查报告
- [Scripts 说明](scripts/README.md) - 开发和测试工具

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

在提交 PR 前：
1. 阅读 [docs/knowledge/](docs/knowledge/) 了解项目设计理念
2. 查看现有的 [plans](docs/plans/) 和 [review](docs/review/)
3. 遵循现有的代码风格和文档格式

## 📄 许可证

MIT License

## 🙏 致谢

- [copilot-gpt4-service](https://github.com/aaamoon/copilot-gpt4-service) - GitHub Copilot API 代理
- [Claude Code](https://code.claude.com) - Anthropic 的 AI 编辑器
- [GitHub Copilot](https://github.com/features/copilot) - GitHub 的 AI 服务
