在Ubuntu上安装 [Clash Verge Rev](https://www.clashverge.dev/install.html) 最简单的方法是下载并安装官方提供的 `.deb` 安装包。建议使用 Ubuntu 24.04 及以上版本以避免复杂的依赖问题。 [[1](https://www.clashverge.dev/faq/linux.html)]

安装步骤

1. **下载安装包**：访问 Clash Verge Rev Docs 或其 [GitHub 发布页](https://github.com/clashbk/clash/wiki/clash-verge)，下载适合 Ubuntu 的 `deb` 安装包（例如 `clash-verge_x.x.x_amd64.deb`）。

2. **安装软件**：打开终端，进入文件下载目录，运行以下命令进行安装（请将文件名替换为您实际下载的文件名）：

   bash

   ```
   sudo dpkg -i clash-verge_x.x.x_amd64.deb
   ```

3. **解决依赖（若报错）**：如果在安装过程中提示依赖错误，请运行以下命令修复并完成安装：

   bash

   ```
   sudo apt-get install -f
   ```



下载地址：https://github.com/clash-verge-rev/clash-verge-rev/releases