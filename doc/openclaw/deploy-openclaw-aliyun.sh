#!/bin/bash

# OpenClaw 一键部署脚本 - 阿里云版本
# 用法: bash deploy-openclaw-aliyun.sh

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}OpenClaw 阿里云一键部署脚本${NC}"
echo -e "${GREEN}========================================${NC}"

# 1. 检查环境
echo -e "\n${YELLOW}[1/6] 检查系统环境...${NC}"

if ! command -v docker &> /dev/null; then
    echo -e "${RED}❌ Docker 未安装${NC}"
    echo "请先安装 Docker: curl -fsSL https://get.docker.com | bash"
    exit 1
fi

if ! command -v docker-compose &> /dev/null && ! docker compose version &> /dev/null; then
    echo -e "${RED}❌ Docker Compose 未安装${NC}"
    echo "请先安装 Docker Compose"
    exit 1
fi

DOCKER_VERSION=$(docker --version | grep -oP '\d+' | head -1)
if [ "$DOCKER_VERSION" -lt 20 ]; then
    echo -e "${RED}❌ Docker 版本过低，需要 20+ 版本${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Docker 已安装: $(docker --version)${NC}"
echo -e "${GREEN}✓ Docker Compose 已安装${NC}"

# 2. 创建目录
echo -e "\n${YELLOW}[2/6] 创建 OpenClaw 目录...${NC}"
mkdir -p /opt/openclaw/data/postgres
cd /opt/openclaw
echo -e "${GREEN}✓ 目录创建完成: /opt/openclaw${NC}"

# 3. 创建 docker-compose.yml
echo -e "\n${YELLOW}[3/6] 生成 docker-compose.yml...${NC}"

cat > /opt/openclaw/docker-compose.yml << 'EOF'
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
EOF

echo -e "${GREEN}✓ docker-compose.yml 生成完成${NC}"

# 4. 拉取镜像并启动
echo -e "\n${YELLOW}[4/6] 拉取镜像并启动容器...${NC}"
docker compose pull
docker compose up -d

echo -e "${GREEN}✓ 容器启动完成${NC}"

# 5. 等待容器就绪
echo -e "\n${YELLOW}[5/6] 等待服务启动...${NC}"
sleep 10

# 检查容器状态
echo -e "\n${YELLOW}当前容器状态:${NC}"
docker ps --filter "name=openclaw"

# 6. 输出访问信息
echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}✓ OpenClaw 部署成功！${NC}"
echo -e "${GREEN}========================================${NC}"

# 获取服务器 IP
SERVER_IP=$(hostname -I | awk '{print $1}')

echo -e "\n${YELLOW}📝 访问信息:${NC}"
echo -e "Web UI: ${GREEN}http://${SERVER_IP}:3000${NC}"
echo -e "PostgreSQL: ${GREEN}${SERVER_IP}:5433${NC}"
echo -e "Redis: ${GREEN}${SERVER_IP}:6380${NC}"

echo -e "\n${YELLOW}⚠️  需要做的事：${NC}"
echo -e "1. 在阿里云安全组开放以下端口:"
echo -e "   - 3000 (OpenClaw Web UI)"
echo -e "   - 18789, 18791 (可选)"
echo ""
echo -e "2. 首次访问 Web UI 后:"
echo -e "   - 创建 Workspace"
echo -e "   - 创建 Agent"
echo -e "   - 配置 LLM 模型 (OpenAI/DeepSeek/Claude)"
echo ""
echo -e "3. 建议配置 HTTPS 和域名"
echo -e "   - 使用 Nginx 反向代理"
echo -e "   - 申请免费 SSL 证书"
echo ""
echo -e "4. 配置飞书机器人集成"
echo -e "   - 创建企业自建应用"
echo -e "   - 添加 Webhook Skill"

echo -e "\n${YELLOW}📋 常用命令:${NC}"
echo -e "查看日志:     ${GREEN}docker compose logs -f openclaw${NC}"
echo -e "停止服务:     ${GREEN}docker compose down${NC}"
echo -e "重启服务:     ${GREEN}docker compose restart${NC}"
echo -e "更新镜像:     ${GREEN}docker compose pull && docker compose up -d${NC}"

echo -e "\n${GREEN}========================================${NC}"
