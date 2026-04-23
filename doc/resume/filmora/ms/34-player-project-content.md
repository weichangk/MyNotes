34-player-project-content.md
# 项目内容

- 负责**播放器模块**的设计与实现，采用三级 Presenter 继承体系（`FFMediaPlayerPresenter`→`FFMediaPlayerEditorPresenter`→`FFMainPlayerPresenter`）按职责分层——基类封装播放状态机、seek、进度回调；中间层负责可视化编辑（关键帧/Mask/Trim）和时间线绑定；顶层管理音频波形、示波器、多机位和工具栏命令分发；所有 Presenter 通过 PIMPL 模式隐藏 VBL 接口，Public 类头文件零 VBL 类型暴露，编译隔离彻底，VBL 引擎升级无需重编上层模块。

- 设计并实现**原生窗口（Native Window）嵌入渲染**：`FFVBLMediaPlayerWidget` 通过 `Qt::WA_NativeWindow` 创建真实 HWND，VBL 引擎调用 `setParentWindow(HWND, size)` 将 D3D11 SwapChain 直接绑定到该 HWND 进行 Present，Qt 合成器完全不介入视频渲染路径，消除 Qt 与 D3D 的合成冲突；`FFMediaPlayerWidget` 以 `QStackedWidget` 管理三个子视图（VBL 原生渲染 / 流媒体 Qt 渲染 / 静帧 QPainter），三种播放器类型无缝切换时渲染上下文保持，无重建开销。

- 实现**多渲染后端适配与 GPU 硬解码**：`IFFMediaPlayerSetting` 在 `setParentWindow()` 前通过 `setVideoRenderType()` 选择 D3D11（Windows 推荐）/ D3D9 / OpenGL（macOS）/ GDI（软件降级），`setUseGpuDecode(true)` 启用 DXVA2/NVDEC/QSV 硬解码，硬解码帧直接驻留显存避免 CPU→GPU 拷贝，4K 视频预览 CPU 占用降低显著；VBL 引擎通过 C linkage 工厂函数（`createMediaPlayer / releaseMediaPlayer`）导出，解耦 DLL 边界的 C++ 对象所有权。

- 实现 **Quick Preview 同步等待机制**：时间线添加特效或修改属性时，VBL 内部触发 Quick Preview，通过 `onPlayerChangedEvent(pctWaitQuickPreviewFinish, before=true)` 通知上层；`FFMediaPlayerPresenter` 启动 `QEventLoop::exec(QEventLoop::ExcludeUserInputEvents)` 阻塞用户输入但保持事件循环（定时器和绘制正常运转，窗口不冻结），VBL 完成后回调 `before=false` 触发 `QEventLoop::quit()`，UI 精确恢复响应，防止用户在渲染不稳定期间操作导致画面撕裂。

- 实现**播放画质动态切换与帧级缩略图异步提取**：Presenter 维护时间线预览与素材预览两套独立画质配置（`pqtFull / pqt1_2 / pqt1_4`），暂停时自动切换高画质（`setPauseHighQuality`），播放时降采样保帧率；`FFTimelineThumbnailPresenter` 通过 `IFFBsThumbManager::startFetchTimelineThumbnailTask` 异步提取帧缩略图，`IFFThumbCallback::onThumbnailReady` 逐帧回调填充时间线缩略图条；`FFUpdateImageThread`（`std::function<QImage()>` 回调驱动）在线程池异步获取当前帧图像，完成后通过信号更新静帧预览 Widget。

- 实现**全屏多显示器支持与多实例播放器管理**：`FFPlayerFullScreenPreviewHelper` 检测目标显示器，Windows 使用 `Qt::Window | FramelessWindowHint`，macOS 使用 `Qt::Tool`（避免与 Spaces/Mission Control 冲突），退出全屏时精确恢复 `m_originalWindowFlags` 和 `m_originalGeometry`，同步调用 `vblPlayer->setFullScreen(true/false)` 通知 VBL 调整 SwapChain 大小；系统同时运行多个独立播放器实例（主播放器、高级字幕/分屏/文字特效编辑各一个），`sycnMainPlayerVolume()` 保证所有实例音量一致，`FFMediaPlayerFactory` 工厂方法支持产品差异化替换。
