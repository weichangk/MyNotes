#!/usr/bin/env pwsh
# install-global.ps1 - 将 mycc 安装为全局命令

$ErrorActionPreference = "Stop"

function Write-Info { param($msg) Write-Host $msg -ForegroundColor Cyan }
function Write-Success { param($msg) Write-Host "✓ $msg" -ForegroundColor Green }
function Write-Warning { param($msg) Write-Host "⚠ $msg" -ForegroundColor Yellow }
function Write-Error { param($msg) Write-Host "✗ $msg" -ForegroundColor Red }

Write-Host ""
Write-Host "═══════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  安装 mycc 为全局命令" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

# 获取当前脚本所在目录
$currentDir = Split-Path -Parent $MyInvocation.MyCommand.Path

Write-Info "当前目录: $currentDir"
Write-Host ""

# 方案选择
Write-Host "请选择安装方式：" -ForegroundColor Yellow
Write-Host ""
Write-Host "1. 添加当前目录到 PATH（推荐）" -ForegroundColor White
Write-Host "   优点: 文件保持在原位置，易于管理" -ForegroundColor Gray
Write-Host "   路径: $currentDir" -ForegroundColor Gray
Write-Host ""
Write-Host "2. 复制到用户 bin 目录" -ForegroundColor White
Write-Host "   优点: 统一管理全局脚本" -ForegroundColor Gray
Write-Host "   路径: $env:USERPROFILE\bin" -ForegroundColor Gray
Write-Host ""
Write-Host "3. 使用 npm 全局安装（推荐给 Node.js 用户）" -ForegroundColor White
Write-Host "   优点: 跨平台，包管理器管理" -ForegroundColor Gray
Write-Host ""

$choice = Read-Host "请输入选择 (1/2/3)"

switch ($choice) {
    "1" {
        # 方案 1: 添加到 PATH
        Write-Info "`n[方案 1] 添加到 PATH 环境变量"
        
        # 获取用户 PATH
        $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
        
        # 检查是否已存在
        if ($userPath -split ";" | Where-Object { $_ -eq $currentDir }) {
            Write-Warning "目录已在 PATH 中"
        } else {
            # 添加到 PATH
            $newPath = "$userPath;$currentDir"
            [Environment]::SetEnvironmentVariable("Path", $newPath, "User")
            Write-Success "已添加到 PATH"
        }
        
        # 更新当前会话的 PATH
        $env:Path = [Environment]::GetEnvironmentVariable("Path", "User") + ";" + [Environment]::GetEnvironmentVariable("Path", "Machine")
        
        Write-Host ""
        Write-Success "安装完成！"
        Write-Host ""
        Write-Host "现在可以在任何位置运行：" -ForegroundColor Green
        Write-Host "  mycc" -ForegroundColor Cyan
        Write-Host "  或" -ForegroundColor White
        Write-Host "  mycc.cmd" -ForegroundColor Cyan
        Write-Host ""
        Write-Warning "注意: 新打开的终端窗口才会生效"
        Write-Host "当前窗口可以直接使用：mycc" -ForegroundColor Gray
    }
    
    "2" {
        # 方案 2: 复制到 bin 目录
        Write-Info "`n[方案 2] 复制到用户 bin 目录"
        
        $binDir = "$env:USERPROFILE\bin"
        
        # 创建 bin 目录
        if (-not (Test-Path $binDir)) {
            New-Item -Path $binDir -ItemType Directory -Force | Out-Null
            Write-Success "创建目录: $binDir"
        }
        
        # 复制文件
        $filesToCopy = @("mycc.ps1", "mycc.cmd")
        foreach ($file in $filesToCopy) {
            $source = Join-Path $currentDir $file
            $dest = Join-Path $binDir $file
            if (Test-Path $source) {
                Copy-Item -Path $source -Destination $dest -Force
                Write-Success "已复制: $file"
            }
        }
        
        # 检查 bin 目录是否在 PATH 中
        $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
        if (-not ($userPath -split ";" | Where-Object { $_ -eq $binDir })) {
            Write-Info "添加 bin 目录到 PATH..."
            $newPath = "$userPath;$binDir"
            [Environment]::SetEnvironmentVariable("Path", $newPath, "User")
            Write-Success "已添加到 PATH"
        }
        
        # 更新当前会话的 PATH
        $env:Path = [Environment]::GetEnvironmentVariable("Path", "User") + ";" + [Environment]::GetEnvironmentVariable("Path", "Machine")
        
        Write-Host ""
        Write-Success "安装完成！"
        Write-Host ""
        Write-Host "文件已复制到: $binDir" -ForegroundColor Gray
        Write-Host ""
        Write-Host "现在可以在任何位置运行：" -ForegroundColor Green
        Write-Host "  mycc" -ForegroundColor Cyan
        Write-Host ""
        Write-Warning "注意: 新打开的终端窗口才会生效"
        Write-Host "当前窗口可以直接使用：mycc" -ForegroundColor Gray
        Write-Host ""
        Write-Warning "更新提示: 如需更新，请重新运行此安装脚本"
    }
    
    "3" {
        # 方案 3: npm 全局安装
        Write-Info "`n[方案 3] 使用 npm 全局安装"
        
        # 创建 package.json
        $packageJson = @{
            name = "mycc"
            version = "1.0.0"
            description = "Claude Code with GitHub Copilot - One-click launcher"
            bin = @{
                mycc = "./mycc-cli.js"
            }
            keywords = @("claude", "copilot", "ai", "cli")
            author = ""
            license = "MIT"
        } | ConvertTo-Json -Depth 10
        
        $packageJson | Out-File -FilePath "$currentDir\package.json" -Encoding UTF8 -Force
        Write-Success "创建 package.json"
        
        # 创建 CLI 入口文件
        $cliContent = @"
#!/usr/bin/env node
const { spawn } = require('child_process');
const path = require('path');
const os = require('os');

const scriptPath = path.join(__dirname, 'mycc.ps1');
const args = process.argv.slice(2);

const isWindows = os.platform() === 'win32';

if (isWindows) {
    // Windows: 使用 PowerShell
    const ps = spawn('powershell.exe', [
        '-ExecutionPolicy', 'Bypass',
        '-File', scriptPath,
        ...args
    ], {
        stdio: 'inherit',
        shell: true
    });
    
    ps.on('exit', (code) => {
        process.exit(code || 0);
    });
} else {
    // Linux/Mac: 使用 pwsh
    const ps = spawn('pwsh', [
        '-File', scriptPath,
        ...args
    ], {
        stdio: 'inherit'
    });
    
    ps.on('exit', (code) => {
        process.exit(code || 0);
    });
}
"@
        
        [System.IO.File]::WriteAllText("$currentDir\mycc-cli.js", $cliContent, [System.Text.UTF8Encoding]::new($false))
        Write-Success "创建 mycc-cli.js"
        
        # 全局安装
        Write-Info "正在全局安装..."
        Push-Location $currentDir
        npm install -g .
        Pop-Location
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host ""
            Write-Success "安装完成！"
            Write-Host ""
            Write-Host "现在可以在任何位置运行：" -ForegroundColor Green
            Write-Host "  mycc" -ForegroundColor Cyan
            Write-Host ""
            Write-Host "卸载命令：" -ForegroundColor Gray
            Write-Host "  npm uninstall -g mycc" -ForegroundColor Gray
            Write-Host ""
            Write-Host "更新命令：" -ForegroundColor Gray
            Write-Host "  cd $currentDir" -ForegroundColor Gray
            Write-Host "  npm install -g ." -ForegroundColor Gray
        } else {
            Write-Error "安装失败"
            exit 1
        }
    }
    
    default {
        Write-Error "无效选择"
        exit 1
    }
}

Write-Host ""
Write-Host "═══════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  测试运行" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""
Write-Host "在新的终端窗口中运行：" -ForegroundColor Yellow
Write-Host "  mycc" -ForegroundColor Cyan
Write-Host ""
Write-Host "或在当前窗口测试（PATH 已更新）：" -ForegroundColor Yellow

$test = Read-Host "是否现在测试？(y/n)"
if ($test -eq "y" -or $test -eq "Y") {
    Write-Host ""
    Write-Info "启动 mycc..."
    Start-Sleep -Seconds 1
    & mycc
}
