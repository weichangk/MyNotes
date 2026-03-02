#!/usr/bin/env pwsh
# mycc.ps1 - 一键启动 Claude Code with GitHub Copilot
#
# 参见: docs/knowledge/patterns/01_one_click_launcher.md
# 陷阱: docs/knowledge/lessons/03_powershell_profile_interference.md
#       如果遇到语法错误，首先尝试: powershell -NoProfile -File mycc.ps1
param([switch]$ReAuth, [switch]$NoProxy, [string]$Proxy)
$ErrorActionPreference = "Stop"

# 步骤计数器（用于改进用户体验）
$script:TotalSteps = 6
$script:CurrentStep = 0

function Write-Info { param($m) Write-Host $m -ForegroundColor Cyan }
function Write-Success { param($m) Write-Host "+ $m" -ForegroundColor Green }
function Write-Warn { param($m) Write-Host "! $m" -ForegroundColor Yellow }
function Write-Err { param($m) Write-Host "X $m" -ForegroundColor Red }
function Write-Step {
    <#
    .SYNOPSIS
    显示带步骤计数的进度信息
    
    .PARAMETER Message
    步骤描述信息
    
    .DESCRIPTION
    自动递增步骤计数并显示 [当前步骤/总步骤] 格式的进度提示。
    这样可以避免硬编码步骤数字，提高可维护性。
    
    .EXAMPLE
    Write-Step "Checking environment..."
    输出: [1/6] Checking environment...
    #>
    param([string]$Message)
    $script:CurrentStep++
    Write-Info "[$script:CurrentStep/$script:TotalSteps] $Message"
}
function Show-Banner {
    Write-Host "`n=========================================" -ForegroundColor Cyan
    Write-Host "   Claude Code + GitHub Copilot CLI      " -ForegroundColor Cyan
    Write-Host "=========================================`n" -ForegroundColor Cyan
}
function Test-ProxyRunning {
    <#
    .SYNOPSIS
    Tests if the copilot-api proxy server is running and responsive.
    
    .PARAMETER Url
    The health check endpoint URL. Default: http://localhost:4141/
    
    .PARAMETER TimeoutSec
    Request timeout in seconds. Default: 2
    
    .OUTPUTS
    Boolean. Returns $true if proxy is running, $false otherwise.
    #>
    param(
        [string]$Url = "http://localhost:4141/",
        [int]$TimeoutSec = 2
    )
    try {
        $null = Invoke-WebRequest $Url -TimeoutSec $TimeoutSec -ErrorAction Stop
        return $true
    } catch {
        return $false
    }
}
function Test-NodeJs {
    Write-Step "Checking environment..."
    if (-not (Get-Command node -EA SilentlyContinue)) { Write-Err "Node.js not found"; exit 1 }
    Write-Success "Node.js $(node --version)"
}
function Test-CopilotApi {
    Write-Step "Checking copilot-api..."
    if (-not (Get-Command copilot-api -EA SilentlyContinue)) {
        Write-Warn "Installing copilot-api (first time may take 1-3 minutes)..."
        npm install -g copilot-api --loglevel=info
        if ($LASTEXITCODE -ne 0) { Write-Err "Install failed"; exit 1 }
        # 刷新 PATH，使新安装的命令立即可用
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" +
                    [System.Environment]::GetEnvironmentVariable("Path","User")
    }
    Write-Success "copilot-api ready"
}
function Test-ClaudeCode {
    <#
    .SYNOPSIS
    检测 Claude Code CLI 是否已安装，如未安装则自动安装
    
    .DESCRIPTION
    检测 claude-code 或 claude 命令是否可用。
    如果未找到，提示用户并自动执行 npm 全局安装。
    
    .OUTPUTS
    None. Exits script on installation failure.
    #>
    Write-Step "Checking Claude Code CLI..."
    
    # 检测 claude-code 或 claude 命令
    $cmd = Get-Command claude-code -EA SilentlyContinue
    if (-not $cmd) { $cmd = Get-Command claude -EA SilentlyContinue }
    
    if ($cmd) {
        Write-Success "Claude Code ready ($($cmd.Source))"
        return
    }
    
    # 未检测到，提示安装
    Write-Warn "Claude Code CLI not found"
    Write-Host "`n=========================================" -ForegroundColor Yellow
    Write-Host "  Claude Code CLI Installation" -ForegroundColor Yellow
    Write-Host "=========================================`n" -ForegroundColor Yellow
    Write-Host "Will install: @anthropic-ai/claude-code" -ForegroundColor Cyan
    Write-Host "Command: npm install -g @anthropic-ai/claude-code`n" -ForegroundColor Gray
    Write-Host "Press any key to start installation..." -ForegroundColor Cyan
    Write-Host "(or Ctrl+C to cancel)`n" -ForegroundColor Gray
    
    # 等待用户确认
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    
    # 执行安装（带超时机制）
    Write-Info "`nInstalling Claude Code CLI (this may take 30-60 seconds)..."
    $job = Start-Job { npm install -g @anthropic-ai/claude-code }
    $timeout = 120  # 2 minutes
    
    for ($i = 0; $i -lt $timeout; $i++) {
        if ($job.State -eq 'Completed') { break }
        if ($i % 10 -eq 0) { Write-Host "." -NoNewline -ForegroundColor Yellow }
        Start-Sleep 1
    }
    
    if ($job.State -ne 'Completed') {
        Write-Host ""
        Write-Err "Installation timeout (network issue?)"
        Stop-Job $job
        Remove-Job $job
        Write-Host "`nTo retry manually:" -ForegroundColor Yellow
        Write-Host "  npm install -g @anthropic-ai/claude-code" -ForegroundColor Cyan
        exit 1
    }
    
    Receive-Job $job | Out-Null
    Remove-Job $job
    
    # 检查安装结果
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Err "Installation failed"
        Write-Host "`nTo clean up and retry:" -ForegroundColor Yellow
        Write-Host "  npm uninstall -g @anthropic-ai/claude-code" -ForegroundColor Cyan
        Write-Host "  npm cache clean --force" -ForegroundColor Cyan
        Write-Host "  npm install -g @anthropic-ai/claude-code" -ForegroundColor Cyan
        Write-Host "`nOr visit: https://code.claude.com`n" -ForegroundColor Cyan
        exit 1
    }
    
    Write-Host ""
    Write-Success "Claude Code CLI installed successfully"
    
    # 验证命令是否真正可用
    $cmd = Get-Command claude-code -EA SilentlyContinue
    if (-not $cmd) { $cmd = Get-Command claude -EA SilentlyContinue }
    
    if (-not $cmd) {
        Write-Warn "Installation succeeded but command not found in PATH"
        Write-Host "You may need to restart your terminal or run:" -ForegroundColor Yellow
        Write-Host "  refreshenv  # or restart terminal" -ForegroundColor Cyan
    }
}
function Test-Authentication {
    Write-Step "Checking authentication..."
    $paths = @(
        "$env:USERPROFILE\.copilot-api\auth.json",
        "$env:USERPROFILE\.copilot\auth.json",
        "$env:USERPROFILE\.local\share\copilot-api\github_token"
    )
    $auth = $false
    foreach ($p in $paths) {
        if ((Test-Path $p) -and ((Get-Content $p -Raw -EA SilentlyContinue).Length -gt 10)) {
            $auth = $true; Write-Success "Authenticated"; break
        }
    }
    if (-not $auth -or $ReAuth) {
        Write-Warn "Authentication required"
        Write-Host "`n=========================================" -ForegroundColor Yellow
        Write-Host "  GitHub OAuth Required" -ForegroundColor Yellow
        Write-Host "=========================================`n" -ForegroundColor Yellow
        Write-Host "Press any key to start auth..." -ForegroundColor Cyan
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        Write-Info "`nAuthenticating..."
        Write-Host "Starting GitHub OAuth flow..." -ForegroundColor Yellow
        Write-Host "Browser will open. Follow GitHub instructions." -ForegroundColor Yellow
        Write-Host ""
        
        # Run copilot-api auth command (dedicated for authentication only)
        # Start the auth process in a new PowerShell window with better visibility
        try {
            Write-Host "Opening authentication in new window..." -ForegroundColor Cyan
            Write-Host ""
            Write-Host "⚠ SECURITY NOTE:" -ForegroundColor Yellow
            Write-Host "  • Do not share or record the authentication window" -ForegroundColor Yellow
            Write-Host "  • The window may display sensitive authentication codes" -ForegroundColor Yellow
            Write-Host "  • Close the window after completing authentication" -ForegroundColor Yellow
            Write-Host ""
            
            # 确定代理：优先用 -Proxy 参数，其次用已有环境变量
            $proxyUrl = if ($Proxy) { $Proxy } elseif ($env:HTTPS_PROXY) { $env:HTTPS_PROXY } elseif ($env:HTTP_PROXY) { $env:HTTP_PROXY } else { "" }
            $proxyBlock = if ($proxyUrl) {
                "`$env:HTTPS_PROXY = '$proxyUrl'; `$env:HTTP_PROXY = '$proxyUrl'"
            } else { "" }
            if ($proxyUrl) {
                Write-Host "  Using proxy: $proxyUrl" -ForegroundColor Gray
            }

            # Start auth process and capture output
            $authProcess = Start-Process powershell -ArgumentList "-NoExit", "-Command", @"
Write-Host '=========================================' -ForegroundColor Green
Write-Host '  GitHub Copilot Authentication' -ForegroundColor Green
Write-Host '=========================================' -ForegroundColor Green
Write-Host ''
$proxyBlock
if ('$proxyUrl') { Write-Host "Proxy: $proxyUrl" -ForegroundColor Gray; Write-Host '' }
Write-Host 'Starting authentication...' -ForegroundColor Cyan
Write-Host 'Browser should open automatically.' -ForegroundColor Yellow
Write-Host 'If not, follow the instructions below.' -ForegroundColor Yellow
Write-Host ''
copilot-api auth
Write-Host ''
Write-Host 'Authentication window - You can close this after completion' -ForegroundColor Gray
"@ -PassThru -WindowStyle Normal
            
            # Wait a bit for auth to complete
            Write-Host "Waiting for authentication (check new window)..." -ForegroundColor Yellow
            Write-Host "Press any key when authentication is complete..." -ForegroundColor Cyan
            $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
            
            # Verify authentication
            $authFound = $false
            foreach ($p in $paths) {
                if ((Test-Path $p) -and ((Get-Content $p -Raw -EA SilentlyContinue).Length -gt 10)) {
                    $authFound = $true
                    break
                }
            }
            
            if ($authFound) {
                Write-Host ""
                Write-Success "Authentication completed!"
            } else {
                Write-Host ""
                Write-Err "Authentication not completed or failed"
                Write-Host "Please run the authentication again with: mycc -ReAuth" -ForegroundColor Yellow
                Write-Host ""
                
                # 清理认证窗口进程（如果仍在运行）
                if ($authProcess -and -not $authProcess.HasExited) {
                    Write-Info "Closing authentication window..."
                    Stop-Process -Id $authProcess.Id -EA SilentlyContinue
                }
                
                exit 1  # 停止脚本，不继续启动代理
            }
        } catch {
            Write-Host ""
            Write-Err "Authentication process interrupted"
            
            # 清理认证窗口进程
            if ($authProcess -and -not $authProcess.HasExited) {
                Write-Info "Cleaning up authentication window..."
                Stop-Process -Id $authProcess.Id -EA SilentlyContinue
            }
            
            exit 1
        }
        
        Start-Sleep 2
    }
    return $auth
}
function Start-ProxyServer {
    param([bool]$IsNew)
    Write-Step "Starting proxy..."
    if (Test-ProxyRunning) {
        Write-Success "Proxy running"
        return $true
    }
    Write-Host "Starting..." -ForegroundColor Yellow
    # 确定代理：优先用 -Proxy 参数，其次继承环境变量
    $proxyUrl = if ($Proxy) { $Proxy } elseif ($env:HTTPS_PROXY) { $env:HTTPS_PROXY } elseif ($env:HTTP_PROXY) { $env:HTTP_PROXY } else { "" }
    if ($proxyUrl) { Write-Host "  Proxy: $proxyUrl" -ForegroundColor Gray }
    # Start-Job 不继承环境变量，需要显式传入
    $j = Start-Job -ArgumentList $proxyUrl -ScriptBlock {
        param($proxy)
        if ($proxy) {
            $env:HTTPS_PROXY = $proxy
            $env:HTTP_PROXY  = $proxy
        }
        copilot-api start 2>&1
    }
    for ($i=0; $i -lt 30; $i++) {
        Start-Sleep 1
        Write-Host "." -NoNewline -ForegroundColor Yellow
        if (Test-ProxyRunning -TimeoutSec 1) {
            Write-Host ""
            Write-Success "Proxy started"
            return $true
        }
    }
    
    # Cleanup: Stop and remove job on timeout
    Write-Host ""
    Write-Err "Proxy startup timeout (30 seconds)"
    Write-Info "Cleaning up background job..."
    
    Stop-Job -Job $j -EA SilentlyContinue
    Remove-Job -Job $j -EA SilentlyContinue -Force
    
    Write-Host "`nPlease check if port 4141 is already in use:" -ForegroundColor Yellow
    Write-Host "  netstat -ano | findstr 4141" -ForegroundColor Cyan
    Write-Host "`nIf you find a process using the port, you can stop it with:" -ForegroundColor Yellow
    Write-Host "  # Find and stop by process name:" -ForegroundColor Cyan
    Write-Host "  Get-Process | Where-Object {`$_.MainWindowTitle -match 'copilot'} | Stop-Process" -ForegroundColor Cyan
    Write-Host "  # Or find by port and stop:" -ForegroundColor Cyan
    Write-Host "  `$pid = (netstat -ano | findstr 4141 | ForEach-Object {`$_ -split ' +' | Select-Object -Last 1})[0]; Stop-Process -Id `$pid -Force" -ForegroundColor Cyan
    
    return $false
}
function Set-ClaudeEnv {
    $env:ANTHROPIC_BASE_URL   = "http://localhost:4141"
    $env:ANTHROPIC_AUTH_TOKEN = "dummy"
    # copilot-api 展示的可用模型均为非 claude 名称，由 copilot-api 负责 API 格式转换
    $env:ANTHROPIC_MODEL                  = "gpt-4o"
    $env:ANTHROPIC_DEFAULT_SONNET_MODEL   = "gpt-4o"
    $env:ANTHROPIC_SMALL_FAST_MODEL       = "gpt-4o-mini"
    $env:ANTHROPIC_DEFAULT_HAIKU_MODEL    = "gpt-4o-mini"
    # 禁止 Claude Code 发起额外的模型查询请求
    $env:DISABLE_NON_ESSENTIAL_MODEL_CALLS         = "1"
    $env:CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC  = "1"
    # 代理：优先用 -Proxy 参数，其次继承已有环境变量
    $proxyUrl = if ($Proxy) { $Proxy } elseif ($env:HTTPS_PROXY) { $env:HTTPS_PROXY } elseif ($env:HTTP_PROXY) { $env:HTTP_PROXY } else { "" }
    if ($proxyUrl) {
        $env:HTTPS_PROXY = $proxyUrl
        $env:HTTP_PROXY  = $proxyUrl
    }
}
function Start-ClaudeCode {
    <#
    .SYNOPSIS
    启动 Claude Code CLI
    
    .DESCRIPTION
    设置环境变量并启动 Claude Code。
    假设 Claude Code 已由 Test-ClaudeCode 确保安装。
    
    .OUTPUTS
    None. Launches Claude Code CLI.
    #>
    Write-Step "Starting Claude Code..."
    Set-ClaudeEnv
    Write-Success "Environment set"
    
    # 获取命令（应该已经存在）
    $c = Get-Command claude-code -EA SilentlyContinue
    if (-not $c) { $c = Get-Command claude -EA SilentlyContinue }
    
    if (-not $c) {
        # 理论上不应该到这里（Test-ClaudeCode 应该已确保安装）
        Write-Err "Unexpected error: Claude Code command not found"
        Write-Host "Please report this issue.`n" -ForegroundColor Yellow
        exit 1
    }
    
    Write-Host "`n=========================================" -ForegroundColor Green
    Write-Host "  Launching Claude Code..." -ForegroundColor Green
    Write-Host "=========================================`n" -ForegroundColor Green
    
    & $c.Source
}

function Test-Environment {
    <#
    .SYNOPSIS
    检查运行环境（Node.js、copilot-api 和 Claude Code CLI）
    
    .OUTPUTS
    Boolean. Returns $true if environment is ready, $false otherwise.
    #>
    try {
        Test-NodeJs
        Test-CopilotApi
        Test-ClaudeCode
        return $true
    } catch {
        Write-Err "Environment check failed: $_"
        return $false
    }
}

function Ensure-Authentication {
    <#
    .SYNOPSIS
    验证认证状态，如果需要则触发认证流程
    
    .OUTPUTS
    Boolean. Returns $true if authentication is valid, $false otherwise.
    #>
    try {
        $authResult = Test-Authentication
        return $true
    } catch {
        Write-Err "Authentication check failed: $_"
        return $false
    }
}

function Ensure-ProxyRunning {
    <#
    .SYNOPSIS
    确保代理服务器正在运行，如果未运行则启动它
    
    .OUTPUTS
    Boolean. Returns $true if proxy is running, $false otherwise.
    #>
    try {
        $auth = Test-Authentication
        $isNew = (-not $auth -or $ReAuth)
        
        $result = Start-ProxyServer -IsNew $isNew
        if (-not $result) {
            Write-Err "Proxy server startup failed"
            return $false
        }
        
        return $true
    } catch {
        Write-Err "Proxy check failed: $_"
        return $false
    }
}

function Invoke-ClaudeCode {
    <#
    .SYNOPSIS
    启动 Claude Code 编辑器
    
    .OUTPUTS
    None. Calls Start-ClaudeCode function.
    #>
    Start-ClaudeCode
}

function Main {
    Clear-Host
    Show-Banner
    
    # 检查是否需要全局安装提示
    $scriptDir = Split-Path -Parent $MyInvocation.ScriptName
    $inPath = $env:PATH -split ';' | Where-Object { $_ -eq $scriptDir }
    
    if (-not $inPath -and (Test-Path "$scriptDir\install.ps1")) {
        Write-Host "💡 提示：可以将 mycc 安装为全局命令，在任意目录直接运行 " -NoNewline -ForegroundColor Yellow
        Write-Host "mycc" -NoNewline -ForegroundColor Cyan
        Write-Host "" -ForegroundColor Yellow
        Write-Host "   运行 " -NoNewline -ForegroundColor Gray
        Write-Host ".\install.ps1" -NoNewline -ForegroundColor Cyan
        Write-Host " 或 " -NoNewline -ForegroundColor Gray
        Write-Host ".\install.cmd" -NoNewline -ForegroundColor Cyan
        Write-Host " 进行全局安装" -ForegroundColor Gray
        Write-Host ""
    }
    
    try {
        # 阶段 1: 环境检查
        if (-not (Test-Environment)) {
            exit 1
        }
        
        # 阶段 2: 认证验证
        if (-not (Ensure-Authentication)) {
            exit 1
        }
        
        # 阶段 3: 启动代理（如果需要）
        if (-not $NoProxy) {
            if (-not (Ensure-ProxyRunning)) {
                Write-Err "Failed to start proxy server"
                exit 1
            }
        } else {
            Write-Warn "Skipping proxy server (NoProxy mode)"
        }
        
        # 阶段 4: 启动 Claude Code
        Invoke-ClaudeCode
        
    } catch {
        Write-Host ""
        Write-Err "Unexpected error: $_"
        Write-Host $_.ScriptStackTrace -ForegroundColor Red
        exit 1
    }
}
Main