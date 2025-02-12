## 1. 检查是否已有 SSH 密钥
在 macOS 中检查是否已有 SSH 密钥

```
ls -al ~/.ssh
```

在 Windows 中检查是否已有 SSH 密钥
通过 Git Bash 检查
```
ls -al ~/.ssh
# 或通过 Windows 文件资源管理器检查，打开 文件资源管理器,进入到 C:\Users\你的用户名\\.ssh 目录（例如 C:\Users\We\\.ssh）。
```

能够看到类似 id_rsa 和 id_rsa.pub 这样的文件。
id_rsa 是你的私钥文件。
id_rsa.pub 是你的公钥文件。
如果这些文件存在，就说明你已经有了 SSH 密钥。

## 生成 SSH 密钥
```
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
# 此命令会生成一个新的 SSH 密钥对。执行时，它会提示你选择保存密钥的文件位置。默认是 ~/.ssh/id_rsa，可以直接按回车键接受默认设置。
# 如果已有相同的密钥文件名，可以选择覆盖它，或将新的密钥保存为不同的文件名。
```

## 将 SSH 公钥添加到 Git 服务器
需要将生成的公钥 id_rsa.pub 添加到 Git 服务（比如 GitHub、GitLab 或 Bitbucket）中
- GitHub: 登录 GitHub，进入 Settings > SSH and GPG keys > New SSH key，粘贴你的公钥内容，然后保存。
- GitLab: 登录 GitLab，进入 Profile > Preferences > SSH Keys，粘贴公钥内容，然后保存。