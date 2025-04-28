https://www.runoob.com/docker/ubuntu-docker-install.html

docker: Error response from daemon: Get "https://registry-1.docker.io/v2/": context deadline exceeded
说明 Docker 客户端在访问 Docker Hub（registry-1.docker.io）时超时了，通常是 网络连接问题 引起的。

配置镜像加速器
https://cr.console.aliyun.com/cn-hangzhou/instances/mirrors

修改 /etc/docker/daemon.json 文件（如果没有这个文件就新建一个）：
```
{
  "registry-mirrors": [
    "https://<你的加速器地址>"
  ]
}
```

然后重启 Docker 服务：
```
sudo systemctl daemon-reexec
sudo systemctl restart docker
```
