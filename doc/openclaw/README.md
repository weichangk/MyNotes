# OpenClaw 阿里云部署脚本 - 快速参考

## 📦 生成的文件

已为您生成以下部署文件：

1. **deploy-openclaw-aliyun.sh** - Linux/Mac 一键部署脚本
2. **deploy-openclaw-aliyun.ps1** - Windows PowerShell 部署脚本  
3. **DEPLOYMENT_GUIDE.md** - 详细部署指南
4. **README.md** - 本文件

---

## 🚀 快速开始

### 选项 A：在 Linux 服务器上部署（推荐用于阿里云）

#### 步骤 1：将脚本上传到服务器

```bash
# 在服务器上创建目录
mkdir -p /opt/openclaw

# 上传脚本（使用 SCP 或直接 vi 编辑）
# scp deploy-openclaw-aliyun.sh root@YOUR_SERVER_IP:/opt/openclaw/
```

#### 步骤 2：执行部署

```bash
cd /opt/openclaw
chmod +x deploy-openclaw-aliyun.sh
sudo bash deploy-openclaw-aliyun.sh
```

#### 步骤 3：查看输出

脚本会自动输出访问地址，例如：
```
Web UI: http://YOUR_SERVER_IP:3000
```

---

### 选项 B：在 Windows 上部署

```powershell
# 以管理员身份打开 PowerShell

# 进入脚本目录
cd "C:\path\to\deploy-scripts"

# 执行部署
powershell -ExecutionPolicy Bypass -File deploy-openclaw-aliyun.ps1
```

---

## ⚙️ 脚本功能

两个部署脚本都自动执行以下操作：

- ✓ 检查 Docker 和 Docker Compose 环境
- ✓ 创建 OpenClaw 目录结构
- ✓ 生成 docker-compose.yml 配置文件
- ✓ 拉取最新的 Docker 镜像
- ✓ 启动所有必需的容器：
  - OpenClaw 应用服务
  - PostgreSQL 数据库
  - Redis 缓存
- ✓ 等待服务就绪
- ✓ 输出访问信息和后续步骤

---

## 🔧 部署后的必要配置

### 1️⃣ 配置阿里云安全组

在阿里云控制台：

1. 进入 **安全组** → **入站规则**
2. 添加以下规则：
   - **3000** (TCP) - OpenClaw Web UI
   - **18789** (TCP) - 可选
   - **18791** (TCP) - 可选

### 2️⃣ 访问 OpenClaw

打开浏览器访问：
```
http://YOUR_SERVER_IP:3000
```

### 3️⃣ 创建 Workspace 和 Agent

1. 创建新的 Workspace
2. 创建 Agent
3. 配置 LLM 模型（OpenAI / DeepSeek / Claude）

### 4️⃣ 配置 HTTPS（强烈推荐）

```bash
# 安装 Nginx
sudo apt-get update
sudo apt-get install nginx -y

# 申请 SSL 证书（需要域名）
sudo apt-get install certbot -y
sudo certbot certonly --standalone -d your-domain.com
```

详见 **DEPLOYMENT_GUIDE.md** 中的 Nginx 配置部分。

### 5️⃣ 配置飞书集成（可选）

1. 创建飞书企业自建应用
2. 在 OpenClaw 中创建 Webhook Skill
3. 配置消息转发 URL

详见 **DEPLOYMENT_GUIDE.md** 中的飞书集成部分。

---

## 📊 容器配置

默认配置：

| 容器 | 内存 | CPU | 端口 |
|------|------|-----|------|
| openclaw | 512MB | 1.0 | 3000 |
| openclaw-postgres | 256MB | 0.5 | 5433 |
| openclaw-redis | 128MB | 0.25 | 6380 |

**最低服务器配置**: 2 vCPU, 2GB RAM, 20GB 存储

---

## 🛠️ 常用操作

### 查看日志

```bash
cd /opt/openclaw
docker compose logs -f openclaw
```

### 停止服务

```bash
cd /opt/openclaw
docker compose down
```

### 重启服务

```bash
cd /opt/openclaw
docker compose restart
```

### 更新到最新版本

```bash
cd /opt/openclaw
docker compose pull
docker compose up -d
```

### 备份数据库

```bash
cd /opt/openclaw
docker compose exec openclaw-postgres pg_dump -U openclaw openclaw > backup-$(date +%Y%m%d).sql
```

### 查看数据库大小

```bash
cd /opt/openclaw
docker compose exec openclaw-postgres psql -U openclaw openclaw -c "SELECT pg_size_pretty(pg_database_size('openclaw'));"
```

---

## 🐛 故障排查

### 脚本执行失败

```bash
# 检查脚本日志
bash deploy-openclaw-aliyun.sh 2>&1 | tee deploy.log

# 检查 Docker 是否运行
docker ps

# 检查磁盘空间
df -h
```

### 容器无法启动

```bash
cd /opt/openclaw

# 查看详细日志
docker compose logs

# 尝试重建容器
docker compose down -v
docker compose up -d
```

### 无法访问 Web UI

1. 检查防火墙和安全组设置
2. 检查容器是否运行: `docker ps`
3. 检查端口是否被占用: `netstat -tlnp | grep 3000`
4. 查看日志: `docker compose logs openclaw`

---

## 📈 性能优化

### 增加容器资源

编辑 `docker-compose.yml`：

```yaml
openclaw:
  mem_limit: 1024m  # 增加到 1GB
  cpus: 2.0         # 增加到 2 核
```

然后重启：
```bash
cd /opt/openclaw
docker compose up -d
```

### 优化数据库性能

```bash
# 清理旧日志
cd /opt/openclaw
docker compose exec openclaw-postgres vacuumdb -U openclaw -d openclaw

# 分析表
docker compose exec openclaw-postgres psql -U openclaw -d openclaw -c "ANALYZE;"
```

---

## 🔒 安全建议

- ✓ 修改默认数据库密码
- ✓ 启用 HTTPS
- ✓ 限制 IP 访问（在安全组中）
- ✓ 定期更新 Docker 镜像
- ✓ 设置自动备份

详见 **DEPLOYMENT_GUIDE.md** 的安全部分。

---

## 💰 成本估算

在阿里云上运行 OpenClaw：

- **服务器**: 2 vCPU, 2GB RAM (~¥50/月)
- **存储**: 20GB 云盘 (~¥5/月)
- **带宽**: 按需计费 (~¥20-50/月)

**估计总费用**: ¥75-105/月（最低配置）

---

## 📚 进阶话题

### 集群部署

如需多节点部署，参考 **DEPLOYMENT_GUIDE.md** 中的性能优化部分。

### 自定义配置

编辑 `docker-compose.yml` 中的环境变量：

```yaml
environment:
  DATABASE_URL: postgres://user:pass@host:5432/db
  REDIS_URL: redis://redis-host:6379
  NODE_ENV: production
```

### CI/CD 集成

可将部署脚本集成到 CI/CD 流程中：

```bash
#!/bin/bash
# 在 GitHub Actions 或 GitLab CI 中使用
bash deploy-openclaw-aliyun.sh
```

---

## 📞 获取帮助

- **OpenClaw 官方**: https://github.com/openclaw/openclaw
- **Docker 文档**: https://docs.docker.com
- **阿里云帮助**: https://help.aliyun.com
- **本地文档**: 查看 `DEPLOYMENT_GUIDE.md`

---

## 📝 更新日志

### v1.0 (2024-04-11)
- ✓ 初版发布
- ✓ 支持 Linux Bash 脚本
- ✓ 支持 Windows PowerShell 脚本
- ✓ 完整的部署指南

---

**祝部署顺利！如有问题，请参考详细的 DEPLOYMENT_GUIDE.md 文件。**
