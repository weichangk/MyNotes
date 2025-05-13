## 安装 VS2022 社区版
- VS2022 在线安装器下载地址：https://visualstudio.microsoft.com/zh-hans/downloads/
- 运行 VS2022 在线安装器
- 工作负荷选使用 C++ 的桌面开发、通用 Windows 平台开发、Visual Studio 扩展开发
- 单组件不用管
- 语言包选中文
- 安装位置建议默认，取消安装后保留下载缓存

## 安装扩展插件 Qt VS Tools
- VS2022 qt-vsaddin-msvc2022-3.0.2.vsix 插件：https://mirrors.cloud.tencent.com/qt/official_releases/vsaddin/3.0.2/ 安装好 VS2022 后直接安装插件
- Qt Versions配置参考：https://blog.csdn.net/qq_33867131/article/details/129747751

## 使用VS2022双击编辑UI时闪退问题
这个主要是因为 Qt Designer 默认是不允许在分离窗口运行的，解决办法：
1. 找到菜单栏-- 扩展 – Qt Vs Tool – Options，在对话框中找到Qt – General – Qt Designer – Run in detached window，把 False 改为 True，如下图所示，就能双击打开ui文件了
2. ui文件也可以更改打开方式为 Qt Creator