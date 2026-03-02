# mycc 使用笔记

## npm 镜像源管理

### 问题：npm install 下载很慢

npm 默认使用官方源 `https://registry.npmjs.org/`，在国内访问很慢。
即使开了科学上网，npm 也**不会自动走系统代理**，需要手动配置。

---

### 方案一：切换为国内镜像源（推荐）

```powershell
# 切换为 npmmirror（淘宝）
npm config set registry https://registry.npmmirror.com

# 验证
npm config get registry
```

### 方案二：让 npm 走本地代理

```powershell
# Clash 默认端口 7890
npm config set proxy http://127.0.0.1:7890
npm config set https-proxy http://127.0.0.1:7890

# 其他常见端口：
# V2Ray/Xray: 10809
# Shadowsocks: 1080
```

---

### 恢复官方源

```powershell
# 恢复官方镜像
npm config set registry https://registry.npmjs.org/

# 删除代理设置（如果设置过）
npm config delete proxy
npm config delete https-proxy
```

---

### 查看当前配置

```powershell
npm config get registry
npm config get proxy
npm config get https-proxy
```

---

## copilot-api 认证连不上 GitHub

### 问题

`copilot-api auth` 报错 `Connect Timeout Error (github.com:443)`。

原因：`copilot-api` 使用 `undici` 发请求，**不走系统代理**，需要单独设置 `HTTPS_PROXY` 环境变量。

### 解决方案

**方式 1：启动时传入代理（推荐）**

```powershell
# 将 7890 替换为你的代理端口（Clash=7890, V2Ray=10809, SS=1080）
.\mycc.ps1 -Proxy http://127.0.0.1:7890
```

**方式 2：在当前终端设置环境变量后再运行**

```powershell
$env:HTTPS_PROXY = "http://127.0.0.1:7890"
$env:HTTP_PROXY  = "http://127.0.0.1:7890"
.\mycc.ps1
```

> mycc 会自动将当前终端的 `HTTPS_PROXY` / `HTTP_PROXY` 传给认证窗口，无需重复设置。
