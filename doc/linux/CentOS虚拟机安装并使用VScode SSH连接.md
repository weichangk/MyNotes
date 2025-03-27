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