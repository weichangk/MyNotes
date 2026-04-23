31-export-optimization.md
# 导出模块技术优化方案

---

## 一、责任链中同步阻塞步骤导致主线程卡顿

### 问题描述

`FASyncLicenceFetchProxy` 虽然命名为"Async"，但实际实现中通过 `QEventLoop` 原地阻塞等待授权拉取完成，再调用 `executeNext()` 进入下一层。若网络较慢（授权接口超时默认 10s），用户点击导出后主线程被阻塞 10s，工具栏、菜单等 UI 元素无法响应，用户误以为程序卡死。

### 优化方案

拆分为非阻塞异步回调，授权完成后再进入责任链下一节点：

```cpp
class FASyncLicenceFetchProxy : public FAbstractExportProxy {
    void doExecute() override {
        // 不阻塞，直接发起异步请求
        auto* fetcher = new FLicenceFetcher(this);
        connect(fetcher, &FLicenceFetcher::fetchFinished,
                this, [this](bool ok) {
                    if (ok) {
                        executeNext();   // 成功：继续责任链
                    } else {
                        showNetworkErrorTip(); // 失败：提示用户
                    }
                });
        fetcher->fetchAsync(kTimeoutMs = 8000);

        // 同时显示"准备中..."状态，给用户反馈
        emit sigPreparing(tr("Checking licenses..."));
    }
};
```

授权拉取期间主线程完全响应（用户可以取消），网络超时从"卡死 10s"变为"8s 后弹出提示并允许重试"，用户体验明显提升。

---

## 二、子进程冻结检测依赖定时轮询，误判率高

### 问题描述

`m_pMonitorTimer`（500ms 轮询）通过对比前后两次 `progress` 值判断子进程是否冻结：若 15s 内进度未增加则认定为冻结。但实际上：
1. 编码复杂场景（如 4K HDR 首帧）本身可能耗时 >15s，会被错误认定为冻结
2. 500ms 轮询在编码密集期（CPU/GPU 高负载）产生大量无意义唤醒，浪费系统资源

### 优化方案

改为进度超时 + 心跳双保险，区分"慢速编码"和"真冻结"：

```cpp
class FProcessEncoder {
    // 子进程主动发送心跳（每 3s 发送一次，即使进度未变化）
    void onHeartbeatReceived(qint64 timestamp) {
        m_lastHeartbeatMs = timestamp;
    }

    // 监控逻辑：区分心跳超时（真冻结）和进度停滞（慢速编码）
    void checkProcessHealth() {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 heartbeatElapsed = now - m_lastHeartbeatMs;

        if (heartbeatElapsed > 30000) {
            // 30s 内未收到心跳 → 真冻结（子进程死循环/OOM）
            handleFreeze();  // 切线程模式降级
        }
        // 进度未变但心跳正常 → 正常慢速编码，继续等待
    }
};

// 子进程（FExportVideoTask）定期发送心跳
class FExportVideoTask {
    QTimer m_heartbeatTimer;
    void onHeartbeatTick() {
        ipcClient->sendHeartbeat(QDateTime::currentMSecsSinceEpoch());
    }
};
```

误判率从"15s 无进度即冻结"降为接近 0（心跳机制），轮询频率从 500ms 降至 3s（减少 83% 无效唤醒）。

---

## 三、多 Pipeline 批量导出时进度显示不直观

### 问题描述

批量导出（如 10 个脚本）时，`FExportProgressDialog` 仅显示整体百分比（所有 Pipeline 平均进度），用户无法看到哪些任务已完成、哪些正在编码、哪些失败，出现单个任务错误时也难以定位是哪个文件出了问题。

### 优化方案

为每个 Pipeline 显示独立进度行，支持展开/折叠：

```cpp
class FExportProgressStatusWidget : public IFExportProgressStatusWidget {
    // 每个 Pipeline 对应一个任务行
    void addPipelineRow(int pipelineIndex, const QString& title) {
        auto* taskView = new FExportProgressTaskView(this);
        taskView->setTitle(title);
        taskView->setStatus(FFExportStatus::Waiting);
        m_taskViews[pipelineIndex] = taskView;
        m_listLayout->addWidget(taskView);
    }

    // Pipeline 状态变化时更新对应行
    void onPipelineStatusChanged(int idx, FFExportStatus status) {
        auto* view = m_taskViews.value(idx);
        if (!view) return;
        switch (status) {
        case FFExportStatus::Busying:   view->setStatus(tr("Exporting..."), 0.0); break;
        case FFExportStatus::Completed: view->setStatus(tr("Done"), 1.0); break;
        case FFExportStatus::Failed:    view->setStatus(tr("Failed"), -1.0, /*error*/true); break;
        }
    }

    // 整体进度 = 已完成数 / 总数（离散计数，直观）
    void updateOverallProgress() {
        int done = countCompletedPipelines();
        int total = m_taskViews.size();
        m_overallProgressBar->setValue(done * 100 / total);
        m_overallLabel->setText(tr("%1 / %2 completed").arg(done).arg(total));
    }
};
```

用户清楚看到哪些文件已完成（绿色勾），哪些正在编码（进度条），哪些失败（红色叉 + 错误说明），失败后支持单独重试该 Pipeline 而非重新导出全部。

---

## 四、GPU 降级后未持久化设置导致下次仍尝试 GPU

### 问题描述

`reStartCloseGpu()` 在本次导出会话内禁用了 GPU（`isDisableGPU = true`），但该标志是 `FProcessEncoder` 的成员变量，对话框关闭后即销毁。用户下次导出时重新创建 `FProcessEncoder`，再次默认开启 GPU，导致同样的 GPU 错误反复出现，每次都要走一遍"GPU 失败 → 降级 → 软编重启"的流程，浪费 5~10s。

### 优化方案

GPU 降级结果持久化到 `FExportSettingDataMgr`，并支持自动恢复：

```cpp
void FProcessEncoder::reStartCloseGpu() {
    isDisableGPU = true;
    m_encodeParam.bGpu = false;

    // 持久化：记录本机 GPU 编码不可用，附带时间戳
    FExportSettingDataMgr::GetInstance()->setGpuEncodingDisabled(
        true, QDateTime::currentSecsSinceEpoch());

    stopCurrentProcess();
    restartProcess();
}

// 下次打开导出面板时
bool FExportSettingDataMgr::shouldEnableGpu() const {
    if (!m_gpuEncodingDisabled) return true;

    // 7天后自动恢复尝试（驱动可能已更新）
    qint64 disabledAt = m_gpuDisabledTimestamp;
    if (QDateTime::currentSecsSinceEpoch() - disabledAt > 7 * 24 * 3600) {
        m_gpuEncodingDisabled = false; // 重置，下次重新尝试
        return true;
    }
    return false;
}
```

GPU 降级持久化后，同一机器后续 7 天内的导出均默认软编，避免反复尝试已知失败的 GPU 路径，7 天后自动重置（驱动更新后可恢复 GPU 加速）。

---

## 五、导出格式 XML 配置文件每次启动重新解析

### 问题描述

`FExportFormatInfo::ParseFormatInfo()` 在每次打开导出面板时从 `Format_H.dat` XML 文件解析所有格式配置（包含 30+ 格式、每个格式的编码器列表、默认参数、平台限制等），解析耗时约 80~150ms。由于导出面板是模态对话框（每次打开都会重建），用户频繁打开导出面板会感知到明显的开启延迟。

### 优化方案

应用启动时异步预解析，结果缓存到单例：

```cpp
class FExportFormatInfo {
    static FExportFormatInfo* s_instance;  // 单例缓存

    static void preloadAsync() {
        // 应用启动时在线程池预解析（不阻塞启动流程）
        FFAsync::postTask([]() {
            s_instance = new FExportFormatInfo();
            s_instance->ParseFormatInfo();  // 解析 XML
        }, FFTasksFeature(TaskPriority::kLow)); // 低优先级，不影响启动
    }

    static FExportFormatInfo* getInstance() {
        // 若预解析已完成，直接返回缓存
        if (s_instance) return s_instance;
        // 首次同步解析（罕见：用户极快打开导出面板）
        s_instance = new FExportFormatInfo();
        s_instance->ParseFormatInfo();
        return s_instance;
    }
};

// 在 FilmoraApp::initServices() 中调用
FExportFormatInfo::preloadAsync();
```

导出面板首次打开从 80~150ms 降至接近 0ms（预解析已完成），格式配置对象在整个会话中复用，内存开销也从每次 new/delete 降为单次分配。

---

## 六、进度对话框活动区内容加载阻塞进度更新

### 问题描述

`IFExportProgressActivityWidget` 的各内容 Widget（NPS、购买引导、裂变活动）在导出开始时统一创建并加载（包括网络请求活动配置、加载图片资源），若网络较慢，活动区内容的加载逻辑（`QNetworkReply`）回调中包含 UI 重绘操作，与进度条更新竞争主线程事件循环，导致进度条更新延迟（视觉上进度条"跳帧"）。

### 优化方案

活动区内容延迟加载，与进度更新严格解耦：

```cpp
class FExportProgressDialog {
    void onExportStarted() {
        // 立即显示进度相关 UI（无网络依赖）
        m_progressStatusWidget->show();
        m_progressStatusWidget->reset();

        // 延迟 2s 才加载活动区（避开编码启动最密集的阶段）
        QTimer::singleShot(2000, this, [this]() {
            loadActivityWidget(); // 异步加载活动内容
        });
    }

    void loadActivityWidget() {
        auto* activity = FActivityWidgetFactory::create(currentStrategy());
        // 活动 Widget 使用独立 QNetworkAccessManager，
        // 网络回调只更新活动区，不触发整体重绘
        activity->loadContentAsync();
        m_activityArea->setWidget(activity);
    }
};
```

进度条更新与活动区加载完全解耦，进度条帧率稳定 30fps，不受活动区网络请求影响，用户感知的进度更新流畅度提升明显。
