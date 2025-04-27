## VMware 安装
VMware-workstation_full_12.1.1.6932 下载，傻瓜式安装
vmware 12 序列号：5A02H-AU243-TZJ49-GTC7K-3C61N

## CentOS-7 虚拟机安装
CentOS-7-x86_64-DVD-2207-02.iso 镜像下载：https://www.runoob.com/linux/linux-install.html
VMware 新建虚拟机（典型）傻瓜式安装

## 虚拟机设置静态ip
进入 VMware - 编辑 - 虚拟网络编辑器 - 选中NAT模式的VMnet - 查看NAT设置网关IP - 查看DHCP设置IP地址范围

VMware 进入虚拟机使用 vi 查看 `/etc/sysconfig/network-scripts/ifcfg-ens33` 配置
```
TYPE=Ethernet
PROXY_METHOD=none
BROWSER_ONLY=no
BOOTPROTO=dhcp
DEFROUTE=yes
IPV4_FAILURE_FATAL=no
IPV6INIT=yes
IPV6_AUTOCONF=yes
IPV6_DEFROUTE=yes
IPV6_FAILURE_FATAL=no
IPV6_ADDR_GEN_MODE=stable-privacy
NAME=ens33
UUID=a553e244-3548-4a18-a518-d8604a50f8bc
DEVICE=ens33
ONBOOT=yes
```
添加配置
```
IPV6_PRIVACY=no
IPADDR0=192.168.43.128 // DHCP设置IP地址范围
PREFIXO0=24
GATEWAY0=192.168.43.2 //NAT设置网关IP
DNS1=8.8.8.8
DNS2=8.8.4.4
```
保存重启

## 使用VScode SSH 远程
安装插件 `Remote - SSH`
新建远程 `eg:用户名@ip地址，如 root@192.168.43.128`
或进入配置`C:\Users\weichangk\.ssh`添加
```
Host 192.168.43.128
  HostName 192.168.43.128
  User root
```
即可使用SSH输入密码连接

## 远程主机不满足运行VS Code服务器的先决条件
https://code.visualstudio.com/docs/remote/faq#_can-i-run-vs-code-server-on-older-linux-distributions

也就是从 1.86 版本开始，vscode server 要求服务器操作系统版本必须是 ubuntu 20.04 或者 centos 8 或者 debian 10 以上了。否则会导致 vscode 连接不上远程。
因此服务器是 centos 7 或者 ubuntu 18.04 最简便的方法就是下载 vscode 1.85 版本。

1.85 版本的官方介绍链接：https://code.visualstudio.com/updates/v1_85，这个页面上有 exe 的下载链接，构造一下 zip 包的下载链接：https://update.code.visualstudio.com/1.85.2/win32-x64-archive/stable。
解压缩上面的 zip 包，使用 1.85 版本的 vscode 即可解决连接不上远程的问题。

## Ubuntu 虚拟机安装
官网下载镜像 https://releases.ubuntu.com/
VMware 新建虚拟机（自定义（高级）），安装来源选稍后安装操作系统，使用网路地址转换（NAT），配置完成后，先在CD/DVD配置iso镜像再开启虚拟机，在首次进入虚拟机时提示配置账号密码配置即可。

## 配置Ubuntu 虚拟机ssh服务
```
sudo apt update
sudo apt install openssh-server
sudo apt install vim
sudo vim /etc/ssh/sshd_config
#找到PermitRootLogin设置为yes保存退出，主机和虚拟机互ping一下看网络连通情况
```

## 使用VScode SSH 远程
和上面一样

## Ubuntu 虚拟机科学上网问题

最常见的原因是虚拟机的网络连接方式没有正确设置。通常虚拟机有几种网络模式，比如：
- NAT模式（默认）：虚拟机通过主机上网。
- 桥接模式：虚拟机直接接入和主机同一局域网，像是单独的一台机器。
- 仅主机模式：虚拟机只能跟主机通信，不能访问外网。

如果你的虚拟机是NAT模式，但科学上网软件只作用在主机本身的应用层（比如只代理浏览器，而不是系统全局），那么虚拟机是感知不到科学上网的。

很多科学上网软件，比如Clash、V2Ray、Trojan等，是本地代理（HTTP代理、Socks5代理），只代理了主机本地发出的请求。
另外，科学上网软件默认不允许局域网设备连接，虚拟机属于局域网设备。

方案一：在科学上网软件里开启“允许局域网连接”，然后虚拟机走主机代理
- 在主机上打开科学上网软件，比如：
  - Clash for Windows（设置里打开Allow LAN）
  - V2RayN（设置里打开允许局域网连接）
- 记下主机的局域网IP地址（比如 192.168.1.6）。
- 在虚拟机里设置网络代理：
  - HTTP Proxy：主机的IP地址，比如 192.168.1.6
  - Port（端口）：你的 V2RayN 的 HTTP代理端口，比如 10809
  - 如果有 HTTPS Proxy，也一起填上，通常端口一样。
