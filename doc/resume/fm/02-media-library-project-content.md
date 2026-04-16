02-media-library-project-content.md

# 项目内容

负责资源面板（Media Library）模块的开发与维护，该模块承载本地媒体文件、云端素材等全类型资源的导入、浏览、搜索与管理，是用户进入编辑器后的核心入口面板。

- 主导资源面板 **MVP 架构重构**，将原本耦合严重的代码拆分为 View / Presenter / Model 三层，View 构造注入 Presenter、Presenter 通过 Observer 接口回调通知 View，彻底解耦视图与业务逻辑，新功能接入成本显著降低。

- 设计并实现 **双层缩略图缓存系统**（路径缓存 + 像素缓存分离，`mediaId+size` 联合 Key），将图像解码通过 `FFAsync` 异步框架移至线程池，以 `QtPromise` 链式回调安全回写主线程，全程用弱指针保护对象生命周期；Hi-DPI 下以 2× 尺寸读取后缩放，解决缩略图模糊问题；1000+ 素材场景下加载耗时下降约 40%。

- 开发**鼠标悬停视频预览**功能：帧解码迁移至独立 Worker 线程，复用底层 `FFTimelineBuilder` 逐帧解码并预缓冲，主线程以 40ms QTimer 驱动 Delegate 局部刷新，实现主线程零解码开销的流畅预览体验。

- 引入**惰性初始化 + 分页加载**优化启动性能：分类 Presenter 改为首次访问时按需创建，Model 层实现 `canFetchMore / fetchMore` 按需渲染，大素材库首屏速度明显提升。

- 基于**策略模式**构建可扩展多维过滤框架（`IFFMediaItemFilter` 接口），支持关键字、媒体类型、云端/本地等多条件组合过滤，新增过滤维度无需改动核心 Model；引入 100ms 合并更新定时器，将高频 VBL 事件节流为批量局部刷新，消除批量导入时的界面抖动。
