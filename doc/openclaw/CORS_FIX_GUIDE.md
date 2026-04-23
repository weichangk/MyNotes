# OpenClaw CORS 配置指南

## 🔒 CORS 问题解决方案

### 问题描述
当您从公网IP访问OpenClaw时，可能会遇到以下错误：
```
origin not allowed (open the Control UI from the gateway host or allow it in gateway.controlUi.allowedOrigins)
```

这是因为OpenClaw默认只允许特定的域名/IP访问Control UI。

### 🚀 解决方案

#### 方法1：修改配置文件（推荐）

1. **进入OpenClaw容器**
```bash
docker exec -it --user root openclaw bash
```

2. **编辑配置文件**
```bash
# 配置文件位置
nano /home/node/.openclaw/openclaw.json
```

3. **修改 allowedOrigins 配置**
```json
{
  "gateway": {
    "controlUi": {
      "allowedOrigins": [
        "http://localhost:18789",
        "http://127.0.0.1:18789",
        "http://47.116.165.13:18789",
        "http://openclaw.lilikk.com"
      ]
    }
  }
}
```

4. **重启服务**
```bash
# 退出容器
exit

# 重启OpenClaw服务
docker compose restart openclaw
```

#### 方法2：通过环境变量配置

修改 `docker-compose.yml` 文件：

```yaml
services:
  openclaw:
    environment:
      - GATEWAY_CONTROLUI_ALLOWED_ORIGINS=http://localhost:18789,http://127.0.0.1:18789,http://47.116.165.13:18789
    # ... 其他配置
```

然后重启服务：
```bash
docker compose up -d
```

#### 方法3：使用域名访问（推荐）

1. **配置域名**
```bash
# 编辑 /etc/hosts 或配置DNS
47.116.165.13 your-openclaw-domain.com
```

2. **修改配置允许域名**
```json
{
  "gateway": {
    "controlUi": {
      "allowedOrigins": [
        "http://localhost:18789",
        "http://127.0.0.1:18789",
        "http://openclaw.lilikk.com:18789"
      ]
    }
  }
}
```

### 🔍 验证配置

#### 1. 检查当前配置
```bash
# 进入容器查看配置
docker exec -it openclaw cat /home/node/.openclaw/openclaw.json
```

#### 2. 测试访问
```bash
# 测试Control UI访问
curl -H "Origin: http://47.116.165.13:18789" http://127.0.0.1:18791/
```

#### 3. 查看日志确认
```bash
# 查看启动日志
docker compose logs openclaw | grep -i "allowedOrigins"
```

### 🌐 完整的访问配置

#### 推荐的完整配置

```json
{
  "gateway": {
    "controlUi": {
      "allowedOrigins": [
        "http://localhost:18789",
        "http://127.0.0.1:18789",
        "http://47.116.165.13:18789",
        "http://your-domain.com:18789",
        "https://your-domain.com:18789"
      ]
    },
    "auth": {
      "token": "your-generated-token"
    }
  }
}
```

### 🔐 安全建议

1. **只允许必要的域名/IP**
   - 不要使用 `*` 通配符
   - 明确指定允许的origin

2. **启用HTTPS**
   ```bash
   # 配置SSL证书后使用HTTPS
   "allowedOrigins": ["https://your-domain.com:18789"]
   ```

3. **定期更新token**
   ```bash
   # 生成新token
   docker exec openclaw openssl rand -hex 32
   ```

### 🚀 快速修复脚本

```bash
#!/bin/bash
# 快速修复CORS问题

echo "修复OpenClaw CORS配置..."

# 备份原配置
docker exec openclaw cp /home/node/.openclaw/openclaw.json /home/node/.openclaw/openclaw.json.backup

# 创建新配置
docker exec openclaw bash -c "
cat > /home/node/.openclaw/openclaw.json << 'EOF'
{
  \"gateway\": {
    \"controlUi\": {
      \"allowedOrigins\": [
        \"http://localhost:18789\",
        \"http://127.0.0.1:18789\",
        \"http://47.116.165.13:18789\"
      ]
    }
  }
}
EOF
"

# 重启服务
docker compose restart openclaw

echo "CORS配置已更新，请等待服务重启完成..."
sleep 10

echo "修复完成！现在可以从公网IP访问Control UI了。"
```

### 📋 访问地址总结

修复CORS后，您可以访问：

- **主Web UI**: `http://47.116.165.13:3000`
- **Control UI**: `http://47.116.165.13:18789`
- **Canvas界面**: `http://47.116.165.13:18789`

### ❓ 常见问题

**Q: 修改配置后仍然报错？**
A: 确保重启服务后等待10-20秒让配置生效。

**Q: 如何查看当前允许的origins？**
A: 查看启动日志：`docker compose logs openclaw | grep allowedOrigins`

**Q: 可以允许所有域名吗？**
A: 不推荐，但可以通过设置 `"allowedOrigins": ["*"]` 实现（不安全）。

**Q: 配置不生效？**
A: 检查JSON格式是否正确，可以用 `jq` 验证：`docker exec openclaw jq . /home/node/.openclaw/openclaw.json`