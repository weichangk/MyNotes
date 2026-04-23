# OpenClaw 阿里云一键部署指南

## 快速开始

### 方式一：使用 Bash 脚本（推荐）

```bash
# 下载脚本
curl -O https://your-server/deploy-openclaw-aliyun.sh

# 或直接上传脚本到服务器

# 赋予执行权限
chmod +x deploy-openclaw-aliyun.sh

# 执行部署
sudo bash deploy-openclaw-aliyun.sh
```

### 方式二：手动部署

#### 1. 检查系统要求

```bash
docker -v           # 需要 Docker 20+
docker compose version  # 需要 Docker Compose v2
```

#### 2. 创建目录和配置

```bash
mkdir -p /opt/openclaw/data/postgres
cd /opt/openclaw
```

#### 3. 创建 docker-compose.yml

将 `docker-compose.yml` 文件放入 `/opt/openclaw/` 目录

#### 4. 启动服务

```bash
docker compose up -d
```

#### 5. 检查服务状态

```bash
docker ps
docker compose logs -f openclaw
```

---

## 部署后配置

### 1. 配置阿里云安全组

在阿里云控制台中配置入站规则：

| 端口   | 协议 | 用途           |
|--------|------|----------------|
| 3000   | TCP  | OpenClaw Web UI |
| 18789  | TCP  | 可选           |
| 18791  | TCP  | 可选           |

### 2. 首次访问

访问 `http://YOUR_SERVER_IP:3000`

- 创建 Workspace
- 创建 Agent
- 配置 LLM 模型

### 3. 配置 HTTPS（强烈推荐）

使用 Nginx 反向代理：

```bash
# 安装 Nginx
sudo apt-get install nginx -y

# 创建配置文件
sudo cat > /etc/nginx/sites-available/openclaw << 'EOF'
server {
    listen 80;
    server_name openclaw.lilikk.com;
    
    location / {
        proxy_pass http://127.0.0.1:18789;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
EOF

# 启用配置
sudo ln -s /etc/nginx/sites-available/openclaw /etc/nginx/sites-enabled/
sudo systemctl restart nginx

# 申请 Let's Encrypt SSL 证书
sudo apt-get install certbot python3-certbot-nginx -y
sudo certbot --nginx -d ai.yourdomain.com
```

### 4. 配置飞书机器人集成

1. 访问 https://open.feishu.cn
2. 创建企业自建应用
3. 在 OpenClaw 创建 Webhook Skill
4. 配置飞书消息转发到 OpenClaw

---

## 常用命令

```bash
# 查看日志
docker compose logs -f openclaw

# 查看所有容器
docker ps

# 停止服务
docker compose down

# 重启服务
docker compose restart

# 删除所有容器（谨慎）
docker compose down -v

# 更新到最新版本
cd /opt/openclaw
docker compose pull
docker compose up -d
```

---

## 数据备份

### 备份数据库

```bash
docker compose exec openclaw-postgres pg_dump -U openclaw openclaw > backup-$(date +%Y%m%d).sql
```

### 恢复数据库

```bash
cat backup-20240411.sql | docker compose exec -T openclaw-postgres psql -U openclaw openclaw
```

---

## 故障排查

### 容器无法启动

```bash
# 查看详细日志
docker compose logs openclaw

# 检查 Docker 磁盘空间
docker system df

# 清理未使用的镜像
docker image prune -a
```

### 数据库连接失败

```bash
# 检查 Postgres 容器
docker compose logs openclaw-postgres

# 重建数据库
docker compose down -v
docker compose up -d
```

### 端口被占用

```bash
# 查看占用的端口
sudo lsof -i :3000

# 修改 docker-compose.yml 中的端口映射
# 例如改为 3001:3000
```

---

## 性能优化建议

### 1. 增加资源限制

编辑 docker-compose.yml：

```yaml
openclaw:
  mem_limit: 1024m  # 增加内存
  cpus: 2.0         # 增加 CPU
```

### 2. 优化数据库

```bash
# 使用 PostgreSQL 12+ 获得更好性能
# 在 docker-compose.yml 中修改:
# image: postgres:15-alpine
```

### 3. 启用 Redis 缓存

已在配置中默认启用。根据需求调整：

```yaml
openclaw-redis:
  command: redis-server --maxmemory 256mb  # 增加缓存大小
```

---

## 安全建议

1. ✓ 修改默认密码
   ```bash
   # 编辑 docker-compose.yml
   # 更改 POSTGRES_PASSWORD
   ```

2. ✓ 限制网络访问
   - 仅在安全组中开放必要的端口
   - 使用防火墙限制访问

3. ✓ 启用 HTTPS
   - 配置 SSL 证书
   - 强制 HTTPS 重定向

4. ✓ 定期备份
   - 设置自动备份任务
   - 测试恢复流程

5. ✓ 更新镜像
   ```bash
   docker compose pull
   docker compose up -d
   ```

---

## 成本优化

在阿里云上运行 OpenClaw 的最低配置：

- **服务器**: 2 vCPU, 2GB RAM
- **存储**: 20GB 云盘
- **带宽**: 按需计费

估计月成本：¥50-100（取决于具体配置）

---

## 获取帮助

- OpenClaw 官方文档: https://github.com/openclaw/openclaw
- Docker 文档: https://docs.docker.com
- 阿里云帮助: https://help.aliyun.com

---

**最后更新**: 2024年4月11日
