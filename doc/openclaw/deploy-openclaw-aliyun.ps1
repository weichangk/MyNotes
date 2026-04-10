# OpenClaw 阿里云一键部署脚本 - PowerShell 版本
# 用法: powershell -ExecutionPolicy Bypass -File deploy-openclaw-aliyun.ps1

# 配置
$DeployPath = "/opt/openclaw"
$DockerComposePath = "$DeployPath/docker-compose.yml"

# 颜色输出
function Write-Success { Write-Host $args -ForegroundColor Green }
function Write-Error { Write-Host "❌ $args" -ForegroundColor Red }
function Write-Info { Write-Host "ℹ️  $args" -ForegroundColor Cyan }
function Write-Warning { Write-Host "⚠️  $args" -ForegroundColor Yellow }

Write-Host ""
Write-Success "========================================"
Write-Success "OpenClaw 阿里云一键部署脚本"
Write-Success "========================================"
Write-Host ""

# 1. 检查环境
Write-Warning "[1/6] 检查系统环境..."

# 检查 Docker
try {
    $dockerVersion = docker --version
    Write-Success "✓ Docker 已安装: $dockerVersion"
} catch {
    Write-Error "Docker 未安装"
    Write-Info "请先安装 Docker Desktop 或在服务器上安装 Docker"
    exit 1
}

# 检查 Docker Compose
try {
    docker compose version | Out-Null
    Write-Success "✓ Docker Compose 已安装"
} catch {
    Write-Error "Docker Compose 未安装"
    exit 1
}

# 2. 创建目录
Write-Warning "[2/6] 创建 OpenClaw 目录..."

if (-not (Test-Path $DeployPath)) {
    New-Item -ItemType Directory -Path $DeployPath -Force | Out-Null
    New-Item -ItemType Directory -Path "$DeployPath/data/postgres" -Force | Out-Null
    Write-Success "✓ 目录创建完成: $DeployPath"
} else {
    Write-Info "✓ 目录已存在: $DeployPath"
}

# 3. 创建 docker-compose.yml
Write-Warning "[3/6] 生成 docker-compose.yml..."

$dockerComposeContent = @"
services:

  openclaw-postgres:
    image: postgres:16-alpine
    container_name: openclaw-postgres
    restart: always
    environment:
      POSTGRES_USER: openclaw
      POSTGRES_PASSWORD: openclaw
      POSTGRES_DB: openclaw
      LISTEN_HOST: 0.0.0.0
    volumes:
      - ./data/postgres:/var/lib/postgresql/data
    ports:
      - "5433:5432"
    mem_limit: 256m
    cpus: 0.50

  openclaw-redis:
    image: redis:7-alpine
    container_name: openclaw-redis
    restart: always
    command: redis-server --maxmemory 128mb --maxmemory-policy allkeys-lru
    ports:
      - "6380:6379"
    environment:
      LISTEN_HOST: 0.0.0.0
    mem_limit: 128m
    cpus: 0.25

  openclaw:
    image: ghcr.io/openclaw/openclaw:latest
    container_name: openclaw
    restart: always
    depends_on:
      - openclaw-postgres
      - openclaw-redis
    ports:
      - "3000:3000"
      - "18789:18789"
      - "18791:18791"
    environment:
      DATABASE_URL: postgres://openclaw:openclaw@openclaw-postgres:5432/openclaw
      REDIS_URL: redis://openclaw-redis:6379
      NODE_ENV: production
      LISTEN_HOST: 0.0.0.0
    mem_limit: 512m
    cpus: 1.0
"@

# 将内容写入文件
Set-Content -Path $DockerComposePath -Value $dockerComposeContent -Encoding UTF8
Write-Success "✓ docker-compose.yml 生成完成"

# 4. 拉取镜像并启动
Write-Warning "[4/6] 拉取镜像并启动容器..."

Push-Location $DeployPath
try {
    Write-Info "正在拉取最新镜像..."
    docker compose pull
    
    Write-Info "正在启动容器..."
    docker compose up -d
    
    Write-Success "✓ 容器启动完成"
} catch {
    Write-Error "启动容器失败: $_"
    exit 1
} finally {
    Pop-Location
}

# 5. 等待容器就绪
Write-Warning "[5/6] 等待服务启动..."
Write-Info "等待 10 秒..."
Start-Sleep -Seconds 10

# 检查容器状态
Write-Info "当前容器状态:"
docker ps --filter "name=openclaw"

# 6. 输出访问信息
Write-Host ""
Write-Success "========================================"
Write-Success "✓ OpenClaw 部署成功！"
Write-Success "========================================"

# 获取 IP 地址
$ipAddress = (Test-Connection -ComputerName (hostname) -Count 1).IPV4Address.IPAddressToString
if (-not $ipAddress) {
    $ipAddress = "YOUR_SERVER_IP"
}

Write-Host ""
Write-Warning "📝 访问信息:"
Write-Host "Web UI:      " -NoNewline
Write-Success "http://$ipAddress:3000"
Write-Host "PostgreSQL:  " -NoNewline
Write-Success "$ipAddress:5433"
Write-Host "Redis:       " -NoNewline
Write-Success "$ipAddress:6380"

Write-Host ""
Write-Warning "⚠️  需要做的事:"
Write-Host "1. 在阿里云安全组开放以下端口:"
Write-Host "   - 3000 (OpenClaw Web UI)"
Write-Host "   - 18789, 18791 (可选)"
Write-Host ""
Write-Host "2. 首次访问 Web UI 后:"
Write-Host "   - 创建 Workspace"
Write-Host "   - 创建 Agent"
Write-Host "   - 配置 LLM 模型 (OpenAI/DeepSeek/Claude)"
Write-Host ""
Write-Host "3. 建议配置 HTTPS 和域名"
Write-Host "   - 使用 Nginx 反向代理"
Write-Host "   - 申请免费 SSL 证书"
Write-Host ""
Write-Host "4. 配置飞书机器人集成"
Write-Host "   - 创建企业自建应用"
Write-Host "   - 添加 Webhook Skill"

Write-Host ""
Write-Warning "📋 常用命令:"
Write-Host "查看日志:     " -NoNewline
Write-Success "docker compose logs -f openclaw"
Write-Host "停止服务:     " -NoNewline
Write-Success "docker compose down"
Write-Host "重启服务:     " -NoNewline
Write-Success "docker compose restart"
Write-Host "更新镜像:     " -NoNewline
Write-Success "docker compose pull && docker compose up -d"

Write-Host ""
Write-Success "========================================"
