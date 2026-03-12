## 确认服务器环境
```
docker -v
docker compose version
```
需要满足
| 软件             | 版本  |
| -------------- | --- |
| Ubuntu         | 20+ |
| Docker         | 20+ |
| Docker Compose | v2  |

## 创建 OpenClaw 目录
```
mkdir -p /opt/openclaw
cd /opt/openclaw
```

## 创建 docker-compose.yml
```
services:

  openclaw-postgres:
    image: postgres:16-alpine
    container_name: openclaw-postgres
    restart: always
    environment:
      POSTGRES_USER: openclaw
      POSTGRES_PASSWORD: openclaw
      POSTGRES_DB: openclaw
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
    environment:
      DATABASE_URL: postgres://openclaw:openclaw@openclaw-postgres:5432/openclaw
      REDIS_URL: redis://openclaw-redis:6379
      NODE_ENV: production
    mem_limit: 512m
    cpus: 1.0
	
```

## 启动 OpenClaw
```
docker compose up -d
docker ps
```
	
## 更新
```
拉取最新镜像
docker compose pull
重新启动容器
docker compose up -d
```

## 访问 OpenClaw
安全组开放端口
```
http://服务器IP:3000
http://47.xxx.xxx.xxx:3000
```

## 创建 OpenClaw Agent
进入 Web UI：
1.创建 Workspace
2.创建 Agent

配置模型，例如：
- OpenAI
- DeepSeek （推荐）
- Claude

## 创建飞书机器人
进入https://open.feishu.cn
创建企业自建应用
添加机器人能力

需要拿到：
App ID
App Secret

开启事件订阅
message.receive_v1

## 飞书接入 OpenClaw
在 OpenClaw 创建 Webhook Skill
示例：
```
POST /api/agent/chat
```

飞书消息转发到：
```
http://服务器IP:3000/api/chat
```

机器人逻辑：
```
飞书群
   ↓
Webhook
   ↓
OpenClaw
   ↓
LLM
   ↓
返回飞书
```

## 建议加一个域名（非常重要）
飞书 必须 HTTPS

所以建议：
```
ai.lilikk.com
```

反向代理：
```
Nginx
```

结构：
```
https://ai.lilikk.com
      ↓
OpenClaw:3000
```

## 最终效果
```
飞书群
  @AI
     ↓
OpenClaw Agent
     ↓
调用 Skills
     ↓
回复飞书
```