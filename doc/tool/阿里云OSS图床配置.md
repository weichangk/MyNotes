# 阿里云 OSS + PicGo 图床配置超详细教程

## 一、前言

在博客写作或文档编辑中，图片的管理和引用是一个常见的需求。
使用图床可以将图片上传到云端，并通过链接引用，避免本地图片路径变动导致的问题。
本文将详细介绍如何使用阿里云 OSS 与 PicGo 搭建图床，实现图片的稳定存储和快速访问。

## 二、准备工作

### 1. 注册并开通阿里云 OSS 服务

1. 访问 [阿里云官网](https://www.aliyun.com/)，注册并登录账号。
2. 进入控制台，搜索并进入“对象存储 OSS”服务。
3. 点击“创建 Bucket”，填写以下信息：
   - **Bucket 名称**：自定义名称，注意不能包含大写字母。
   - **地域**：选择距离您较近的地域。
   - **存储类型**：选择“标准存储”。
   - **读写权限**：选择“公共读”。
4. 点击“确定”完成创建。

### 2. 获取 AccessKey

1. 在控制台右上角点击头像，选择“AccessKey 管理”。
2. 点击“创建 AccessKey”，获取 `AccessKey ID` 和 `AccessKey Secret`。请妥善保存这两个信息，后续配置 PicGo 时需要使用。、
3. 记得配置用户OSS权限否则无法上传，勾选中用户，点击添加权限，再点击下面两项权限，加到已选择框中。
   - AliyunOSSFullAccess——管理对象存储服务（OSS）权限。
   - AliyunOSSReadOnlyAccess——只读访问对象存储服务（OSS）的权限

## 三、配置 PicGo

### 1. 下载并安装 PicGo

访问 [PicGo 的 GitHub 仓库](https://github.com/Molunerfinn/PicGo/releases)，根据您的操作系统下载对应的安装包并安装。

### 2. 配置阿里云 OSS 图床

1. 打开 PicGo，点击左侧的“图床设置”，选择“阿里云 OSS”。
2. 填写以下信息：
   - **AccessKey ID**：填写您获取的 `AccessKey ID`。
   - **AccessKey Secret**：填写您获取的 `AccessKey Secret`。
   - **存储空间名（Bucket）**：填写您创建的 Bucket 名称。
   - **存储区域**：填写您选择的地域节点，例如 `oss-cn-hangzhou`。
   - **存储路径**：自定义路径，例如 `img/`，用于在 Bucket 中分类存储图片。
3. 点击“确定”保存配置，并设置为默认图床。

### 3. PicGo 基本设置

1. 在 PicGo 的“设置”中，启用以下选项：
   
   - **时间戳重命名**：避免上传重复文件名的图片。
   - **上传后自动复制 URL**：方便在粘贴图片后直接获取链接。
   
1. 如果PicGo上传失败可查看日志文件 picgo.log 进行排查
   
   

## 四、配置 Typora（可选）

如果您使用 Typora 进行 Markdown 写作，可以将 PicGo 与 Typora 集成，实现图片的自动上传。

1. 打开 Typora，进入“文件” > “偏好设置” > “图像”。
2. 在“插入图片时”选择“上传图片”，也可以本地保留原始图片文件（例如放在 `assets/` 文件夹中），本地+远程双存储（最稳妥）。
3. 在“上传服务”中选择“PicGo (app)”。
4. 在“PicGo 路径”中，填写 PicGo 的安装路径，例如 `C:\Program Files\PicGo\PicGo.exe`。
5. 点击“验证图片上传选项”，确保配置正确。

## 五、使用方法

配置完成后，在 Typora 中插入图片时，PicGo 会自动将图片上传到阿里云 OSS，并返回图片的外链地址。
您可以直接在文档中使用该链接，确保图片在不同设备和平台上都能正常显示。

## 六、注意事项

- 阿里云 OSS 的存储和流量可能会产生费用，建议根据实际使用情况选择合适的计费方式。
- 确保您的 Bucket 设置为“公共读”，否则图片链接可能无法访问。
- 定期备份您的图片数据，防止意外丢失。

## 七、参考资料

- [阿里云 OSS 官方文档](https://help.aliyun.com/product/31815.html)
- [PicGo GitHub 仓库](https://github.com/Molunerfinn/PicGo)
- [手把手教你搭建阿里云图床 (PicGo+Typora+阿里云OSS)](https://developer.aliyun.com/article/1465963)
