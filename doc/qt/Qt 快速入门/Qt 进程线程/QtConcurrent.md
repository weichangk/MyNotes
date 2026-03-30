# QtConcurrent

## 概述

`QtConcurrent` 是 Qt 提供的**高级多线程 API**，它将线程管理的复杂性完全隐藏在底层，让开发者只需关注"做什么"而不需要关心"在哪个线程做"。

与手动使用 `QThread` 相比：

| 特性 | QThread | QtConcurrent |
|---|---|---|
| 线程管理 | 手动创建/启动/停止 | 自动使用全局线程池 |
| 代码量 | 需要 Worker 类 + 信号槽 | 一行函数调用 |
| 结果获取 | 通过信号传递 | `QFuture<T>` 直接获取 |
| 进度监控 | 手动实现 | `QFutureWatcher` 内置支持 |
| 取消任务 | 手动实现中断标志 | `QFuture::cancel()` |
| 适用场景 | 长期运行的后台服务 | 独立的并行计算任务 |

**使用前提**：在 `.pro` 文件或 CMakeLists.txt 中添加：

```cmake
# CMake
find_package(Qt5 COMPONENTS Concurrent REQUIRED)
target_link_libraries(myapp PRIVATE Qt5::Concurrent)
```

```qmake
# qmake
QT += concurrent
```

---

## 1. QtConcurrent::run() —— 在线程池中执行函数

### 1.1 基本用法

`QtConcurrent::run()` 将一个函数投递到全局线程池执行，返回 `QFuture<T>` 用于获取结果。

```cpp
#include <QtConcurrent>
#include <QFuture>
#include <QDebug>

// 普通函数
QString downloadFile(const QUrl &url) {
    // 模拟耗时下载（在线程池的某个线程中执行）
    QThread::sleep(3);
    return "文件内容：" + url.toString();
}

// 调用
QFuture<QString> future = QtConcurrent::run(downloadFile, QUrl("https://example.com/data.json"));

// 阻塞等待结果
QString result = future.result();
qDebug() << result;
```

### 1.2 使用 Lambda

```cpp
QFuture<int> future = QtConcurrent::run([]() {
    int sum = 0;
    for (int i = 0; i < 1000000; ++i)
        sum += i;
    return sum;
});

qDebug() << "计算结果:" << future.result();
```

### 1.3 带参数的函数

```cpp
// 自由函数
int multiply(int a, int b) { return a * b; }
QFuture<int> f1 = QtConcurrent::run(multiply, 6, 7);

// 成员函数
class Calculator {
public:
    double compute(double x) const { return x * x + 2 * x + 1; }
};

Calculator calc;
QFuture<double> f2 = QtConcurrent::run(&calc, &Calculator::compute, 3.14);
// 注意：对象指针在前，成员函数指针在后，参数跟在最后

// Lambda 捕获
QString prefix = "结果:";
QFuture<QString> f3 = QtConcurrent::run([prefix](int val) {
    return prefix + QString::number(val);
}, 42);
```

### 1.4 无返回值

```cpp
QFuture<void> future = QtConcurrent::run([]() {
    qDebug() << "后台任务执行中，线程:" << QThread::currentThread();
    QThread::sleep(2);
    qDebug() << "后台任务完成";
});

future.waitForFinished(); // 等待完成（无返回值）
```

### 1.5 指定线程池

默认使用 `QThreadPool::globalInstance()`，也可以指定自定义线程池：

```cpp
QThreadPool customPool;
customPool.setMaxThreadCount(2); // 限制并发数

QFuture<int> future = QtConcurrent::run(&customPool, []() {
    return heavyComputation();
});
```

---

## 2. QFuture —— 异步结果容器

### 2.1 核心 API

```cpp
QFuture<int> future = QtConcurrent::run(compute);

// 状态查询
future.isStarted();      // 是否已开始
future.isRunning();      // 是否正在运行
future.isFinished();     // 是否已完成
future.isCanceled();     // 是否被取消
future.isPaused();       // 是否被暂停

// 获取结果
int val = future.result();         // 阻塞等待并返回结果
int val = future.resultAt(0);      // 获取指定索引的结果（用于 map/filter）
QList<int> all = future.results(); // 获取所有结果

// 控制
future.cancel();            // 请求取消
future.pause();             // 请求暂停（仅 map/filter/reduce 支持）
future.resume();            // 恢复
future.waitForFinished();   // 阻塞等待完成

// 进度
future.progressValue();     // 当前进度值
future.progressMinimum();   // 进度最小值
future.progressMaximum();   // 进度最大值
future.progressText();      // 进度文本
```

### 2.2 QFuture 的线程安全性

`QFuture` 是**值语义**的，可以在线程间安全地拷贝和传递。多个 `QFuture` 副本共享同一个底层结果状态。

```cpp
QFuture<int> f1 = QtConcurrent::run(compute);
QFuture<int> f2 = f1; // f2 和 f1 指向同一个任务

// 在任意线程中
int r1 = f1.result(); // OK
int r2 = f2.result(); // OK，同一个结果
```

---

## 3. QFutureWatcher —— 非阻塞结果监控

### 3.1 为什么需要 Watcher

`future.result()` 和 `future.waitForFinished()` 都是**阻塞调用**，会冻结 UI。`QFutureWatcher` 通过信号槽实现非阻塞的结果获取。

### 3.2 基本用法

```cpp
#include <QFutureWatcher>
#include <QtConcurrent>

// 在成员变量中声明（生命周期需覆盖任务执行期间）
QFutureWatcher<QString> *m_watcher;

void MainWindow::startTask()
{
    m_watcher = new QFutureWatcher<QString>(this);

    // 连接信号
    connect(m_watcher, &QFutureWatcher<QString>::finished, this, [this]() {
        QString result = m_watcher->result();
        m_label->setText(result);
        m_progressBar->setVisible(false);
    });

    // 启动任务
    QFuture<QString> future = QtConcurrent::run([]() {
        QThread::sleep(3);
        return QString("处理完成！");
    });

    m_watcher->setFuture(future);
    m_progressBar->setVisible(true);
}
```

### 3.3 QFutureWatcher 信号一览

| 信号 | 触发时机 |
|---|---|
| `started()` | 任务开始执行 |
| `finished()` | 任务执行完毕 |
| `canceled()` | 任务被取消 |
| `paused()` | 任务被暂停 |
| `resumed()` | 任务被恢复 |
| `resultReadyAt(int index)` | 第 index 个结果就绪 |
| `resultsReadyAt(int begin, int end)` | 一批结果就绪 |
| `progressValueChanged(int value)` | 进度值变化 |
| `progressRangeChanged(int min, int max)` | 进度范围变化 |
| `progressTextChanged(const QString &text)` | 进度文本变化 |

### 3.4 进度条集成

```cpp
void MainWindow::processImages(const QStringList &files)
{
    auto *watcher = new QFutureWatcher<QImage>(this);

    connect(watcher, &QFutureWatcher<QImage>::progressRangeChanged,
            m_progressBar, &QProgressBar::setRange);
    connect(watcher, &QFutureWatcher<QImage>::progressValueChanged,
            m_progressBar, &QProgressBar::setValue);
    connect(watcher, &QFutureWatcher<QImage>::resultReadyAt,
            this, [this, watcher](int index) {
        QImage img = watcher->resultAt(index);
        addToGallery(img);
    });
    connect(watcher, &QFutureWatcher<QImage>::finished,
            this, [watcher]() {
        qDebug() << "全部处理完成";
        watcher->deleteLater();
    });

    QFuture<QImage> future = QtConcurrent::mapped(files, loadAndProcess);
    watcher->setFuture(future);
}
```

### 3.5 取消任务

```cpp
// 启动
m_watcher->setFuture(QtConcurrent::mapped(hugeList, processItem));

// 用户点击取消按钮
connect(cancelBtn, &QPushButton::clicked, this, [this]() {
    m_watcher->future().cancel();
});

// 监听取消完成
connect(m_watcher, &QFutureWatcher<void>::canceled, this, [this]() {
    m_statusLabel->setText("任务已取消");
});
```

> **注意**：`cancel()` 仅对 `mapped()`、`filtered()`、`mappedReduced()` 等批量操作有效。`QtConcurrent::run()` 启动的单个任务**不支持取消**——因为它只是在线程池中调用一个函数，没有检查点。

---

## 4. QtConcurrent::map() —— 并行转换（原地修改）

### 4.1 概念

`map()` 对容器中的每个元素**并行地**应用同一个函数，**原地修改**容器元素。

```
输入: [a₁, a₂, a₃, a₄, a₅]
         │    │    │    │    │
         ▼    ▼    ▼    ▼    ▼     （并行执行）
       f(a₁) f(a₂) f(a₃) f(a₄) f(a₅)
         │    │    │    │    │
         ▼    ▼    ▼    ▼    ▼
输出: [a₁', a₂', a₃', a₄', a₅']  （原地修改）
```

### 4.2 基本用法

```cpp
#include <QtConcurrent>

// 函数：原地修改元素
void normalize(QImage &image) {
    image = image.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QList<QImage> images = loadImages();

// 并行原地修改所有图片
QFuture<void> future = QtConcurrent::map(images, normalize);
future.waitForFinished();

// images 中的每张图片都被缩放了
```

### 4.3 使用 Lambda

```cpp
QList<QString> names = {"alice", "bob", "charlie"};

QtConcurrent::map(names, [](QString &name) {
    name = name.toUpper(); // 原地转大写
}).waitForFinished();

// names: ["ALICE", "BOB", "CHARLIE"]
```

> **注意**：`map()` 的函数参数接收**引用**，返回 `void`，因为是原地修改。

---

## 5. QtConcurrent::mapped() —— 并行转换（生成新容器）

### 5.1 概念

`mapped()` 与 `map()` 类似，但**不修改**原容器，而是返回一个包含转换结果的新序列。

```
输入: [a₁, a₂, a₃]
       │    │    │
       ▼    ▼    ▼     （并行执行）
     f(a₁) f(a₂) f(a₃)
       │    │    │
       ▼    ▼    ▼
输出: [b₁,  b₂,  b₃]  （新容器）
```

### 5.2 基本用法

```cpp
// 函数：接收值（或 const 引用），返回转换结果
QImage createThumbnail(const QString &filePath) {
    QImage img(filePath);
    return img.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QStringList files = {"/photos/a.jpg", "/photos/b.jpg", "/photos/c.jpg"};

// 并行生成缩略图
QFuture<QImage> future = QtConcurrent::mapped(files, createThumbnail);

// 阻塞获取所有结果
QList<QImage> thumbnails = future.results();
```

### 5.3 使用 Lambda

```cpp
QList<int> numbers = {1, 2, 3, 4, 5};

QFuture<int> future = QtConcurrent::mapped(numbers, [](int n) {
    return n * n; // 计算平方
});

QList<int> squares = future.results();
// squares: [1, 4, 9, 16, 25]
```

### 5.4 类型转换

输入和输出类型可以不同：

```cpp
QList<QUrl> urls = { ... };

QFuture<QPair<QUrl, int>> future = QtConcurrent::mapped(urls,
    [](const QUrl &url) -> QPair<QUrl, int> {
        QByteArray data = downloadSync(url);
        return {url, data.size()};
    });

for (const auto &[url, size] : future.results()) {
    qDebug() << url << "→" << size << "bytes";
}
```

---

## 6. QtConcurrent::filtered() —— 并行过滤

### 6.1 概念

`filtered()` 对容器中的每个元素**并行地**应用过滤函数，返回满足条件的元素组成的新序列。

```
输入:  [1, 2, 3, 4, 5, 6, 7, 8]
        │  │  │  │  │  │  │  │
        ▼  ▼  ▼  ▼  ▼  ▼  ▼  ▼    （并行判断）
        ✗  ✓  ✗  ✓  ✗  ✓  ✗  ✓    isEven?
              │     │     │     │
              ▼     ▼     ▼     ▼
输出:        [2,    4,    6,    8]
```

### 6.2 基本用法

```cpp
// 过滤函数：返回 bool
bool isLargeFile(const QString &filePath) {
    QFileInfo info(filePath);
    return info.size() > 1024 * 1024; // > 1MB
}

QStringList allFiles = scanDirectory("/data");

QFuture<QString> future = QtConcurrent::filtered(allFiles, isLargeFile);
QStringList largeFiles = future.results();
```

### 6.3 使用 Lambda

```cpp
QList<int> data = {3, 7, 11, 2, 8, 15, 6, 1, 9};

QFuture<int> future = QtConcurrent::filtered(data, [](int n) {
    return n > 5; // 保留大于 5 的
});

QList<int> result = future.results();
// result: [7, 11, 8, 15, 6, 9]（顺序保持不变）
```

### 6.4 原地过滤：filter()

```cpp
QList<int> data = {3, 7, 11, 2, 8, 15, 6, 1, 9};

// 原地移除不满足条件的元素
QFuture<void> future = QtConcurrent::filter(data, [](int n) {
    return n > 5;
});
future.waitForFinished();
// data: [7, 11, 8, 15, 6, 9]
```

---

## 7. QtConcurrent::reduce / mappedReduced / filteredReduced —— 并行归约

### 7.1 概念

归约（Reduce）将多个元素**聚合**为单个结果。`mappedReduced()` 和 `filteredReduced()` 将**映射/过滤**与**归约**组合成一个操作。

```
mappedReduced:
  [a₁, a₂, a₃, a₄]
       │    │    │    │
       ▼    ▼    ▼    ▼     Map（并行）
     f(a₁) f(a₂) f(a₃) f(a₄)
       │    │    │    │
       └────┴────┴────┘
              │
              ▼              Reduce（顺序聚合）
           result
```

### 7.2 mappedReduced() —— 先映射后归约

```cpp
// 统计目录中所有文件的总大小
QStringList files = getAllFiles("/data");

// Map 函数：获取每个文件的大小（并行执行）
qint64 getFileSize(const QString &path) {
    return QFileInfo(path).size();
}

// Reduce 函数：累加（顺序执行）
void sumSizes(qint64 &total, const qint64 &size) {
    total += size;
}

QFuture<qint64> future = QtConcurrent::mappedReduced(
    files,       // 输入容器
    getFileSize, // Map 函数
    sumSizes     // Reduce 函数
);

qint64 totalSize = future.result();
qDebug() << "总大小:" << totalSize / 1024 / 1024 << "MB";
```

### 7.3 Lambda 写法

```cpp
QList<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// 并行计算所有元素平方的总和：Σ(x²)
QFuture<int> future = QtConcurrent::mappedReduced(
    data,
    [](int x) -> int { return x * x; },       // Map：计算平方
    [](int &sum, const int &square) { sum += square; } // Reduce：累加
);

qDebug() << "平方和:" << future.result(); // 385
```

### 7.4 filteredReduced() —— 先过滤后归约

```cpp
QList<int> scores = {85, 42, 91, 67, 78, 53, 98, 36, 72, 88};

// 计算及格分数（>=60）的平均值
struct Average {
    int sum = 0;
    int count = 0;
};

QFuture<Average> future = QtConcurrent::filteredReduced(
    scores,
    [](int score) { return score >= 60; },           // Filter
    [](Average &avg, const int &score) {              // Reduce
        avg.sum += score;
        avg.count++;
    }
);

Average avg = future.result();
qDebug() << "及格平均分:" << (avg.count > 0 ? avg.sum / avg.count : 0);
```

### 7.5 Reduce 选项

```cpp
// 默认：Reduce 函数按无序方式调用（更快）
QtConcurrent::mappedReduced(data, mapFn, reduceFn);

// 保持顺序：Reduce 按输入顺序调用（较慢）
QtConcurrent::mappedReduced(data, mapFn, reduceFn,
                            QtConcurrent::OrderedReduce);

// 无序（默认行为，显式指定）
QtConcurrent::mappedReduced(data, mapFn, reduceFn,
                            QtConcurrent::UnorderedReduce);

// 顺序归约：只有一个 Reduce 线程（当 Reduce 函数不安全时使用）
QtConcurrent::mappedReduced(data, mapFn, reduceFn,
                            QtConcurrent::SequentialReduce);
```

> **重要**：Reduce 函数默认是在互斥保护下调用的（一次只有一个线程执行 Reduce），所以 Reduce 函数**不需要额外加锁**。

---

## 8. 实战综合示例

### 8.1 并行文件搜索

```cpp
#include <QtConcurrent>
#include <QDirIterator>

class FileSearcher : public QObject
{
    Q_OBJECT
public:
    void search(const QString &rootPath, const QString &keyword)
    {
        // 收集所有文本文件
        QStringList files;
        QDirIterator it(rootPath, {"*.txt", "*.cpp", "*.h", "*.md"},
                        QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext())
            files << it.next();

        qDebug() << "共" << files.size() << "个文件待搜索";

        // 并行搜索
        auto *watcher = new QFutureWatcher<QString>(this);

        connect(watcher, &QFutureWatcher<QString>::resultReadyAt,
                this, [this, watcher](int index) {
            emit fileFound(watcher->resultAt(index));
        });

        connect(watcher, &QFutureWatcher<QString>::progressValueChanged,
                this, &FileSearcher::progressChanged);

        connect(watcher, &QFutureWatcher<QString>::finished,
                this, [this, watcher]() {
            emit searchCompleted(watcher->future().resultCount());
            watcher->deleteLater();
        });

        // filtered：只保留包含关键词的文件
        QFuture<QString> future = QtConcurrent::filtered(files,
            [keyword](const QString &filePath) {
                QFile file(filePath);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                    return false;
                return file.readAll().contains(keyword.toUtf8());
            });

        watcher->setFuture(future);
    }

signals:
    void fileFound(const QString &path);
    void progressChanged(int value);
    void searchCompleted(int matchCount);
};
```

### 8.2 并行图像批处理

```cpp
struct ImageTask {
    QString inputPath;
    QString outputPath;
    QSize targetSize;
};

struct BatchResult {
    int success = 0;
    int failed = 0;
    QStringList errors;
};

void processBatch(const QList<ImageTask> &tasks)
{
    auto *watcher = new QFutureWatcher<bool>(this);

    connect(watcher, &QFutureWatcher<bool>::progressValueChanged,
            m_progressBar, &QProgressBar::setValue);
    connect(watcher, &QFutureWatcher<bool>::progressRangeChanged,
            m_progressBar, &QProgressBar::setRange);

    connect(watcher, &QFutureWatcher<bool>::finished, this, [watcher]() {
        int success = 0, failed = 0;
        for (int i = 0; i < watcher->future().resultCount(); ++i) {
            if (watcher->resultAt(i)) success++;
            else failed++;
        }
        qDebug() << "完成:" << success << "成功," << failed << "失败";
        watcher->deleteLater();
    });

    QFuture<bool> future = QtConcurrent::mapped(tasks,
        [](const ImageTask &task) -> bool {
            QImage img(task.inputPath);
            if (img.isNull()) return false;
            QImage scaled = img.scaled(task.targetSize,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
            return scaled.save(task.outputPath, "JPEG", 85);
        });

    watcher->setFuture(future);
}
```

### 8.3 MapReduce 统计词频

```cpp
#include <QtConcurrent>
#include <QMap>

using WordCount = QMap<QString, int>;

// Map：统计单个文件的词频
WordCount countWordsInFile(const QString &filePath) {
    WordCount counts;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return counts;

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine().toLower();
        const QStringList words = line.split(QRegularExpression("\\W+"),
                                             Qt::SkipEmptyParts);
        for (const QString &word : words)
            counts[word]++;
    }
    return counts;
}

// Reduce：合并两个词频表
void mergeWordCounts(WordCount &result, const WordCount &partial) {
    for (auto it = partial.cbegin(); it != partial.cend(); ++it) {
        result[it.key()] += it.value();
    }
}

// 调用
QStringList sourceFiles = {"main.cpp", "widget.cpp", "utils.cpp"};

QFuture<WordCount> future = QtConcurrent::mappedReduced(
    sourceFiles, countWordsInFile, mergeWordCounts);

WordCount total = future.result();

// 输出 Top 10 高频词
QList<QPair<QString, int>> sorted;
for (auto it = total.cbegin(); it != total.cend(); ++it)
    sorted.append({it.key(), it.value()});
std::sort(sorted.begin(), sorted.end(),
          [](const auto &a, const auto &b) { return a.second > b.second; });

for (int i = 0; i < qMin(10, sorted.size()); ++i)
    qDebug() << sorted[i].first << ":" << sorted[i].second;
```

---

## 9. QThreadPool —— 线程池配置

### 9.1 全局线程池

`QtConcurrent` 默认使用 `QThreadPool::globalInstance()`：

```cpp
QThreadPool *pool = QThreadPool::globalInstance();

// 查看线程池状态
qDebug() << "最大线程数:" << pool->maxThreadCount();   // 默认 = CPU 核心数
qDebug() << "活跃线程数:" << pool->activeThreadCount();
qDebug() << "过期时间:" << pool->expiryTimeout() << "ms"; // 默认 30000ms

// 调整配置
pool->setMaxThreadCount(8);     // 设置最大并发数
pool->setExpiryTimeout(60000);  // 空闲线程 60 秒后回收
```

### 9.2 自定义线程池

避免与其他模块争抢全局线程池资源：

```cpp
class ImageProcessor : public QObject
{
public:
    ImageProcessor() {
        m_pool.setMaxThreadCount(4); // 专用 4 线程
    }

    QFuture<QImage> process(const QStringList &files) {
        return QtConcurrent::mapped(&m_pool, files, loadAndResize);
    }

private:
    QThreadPool m_pool;
};
```

### 9.3 QRunnable —— 线程池的直接接口

虽然不属于 `QtConcurrent`，但 `QRunnable` 是 `QThreadPool` 的底层任务单元：

```cpp
class MyTask : public QRunnable
{
public:
    void run() override {
        qDebug() << "任务执行于" << QThread::currentThread();
    }
};

// 提交到线程池
QThreadPool::globalInstance()->start(new MyTask);

// 或使用 Lambda（Qt 5.15+）
QThreadPool::globalInstance()->start([]() {
    qDebug() << "Lambda 任务于" << QThread::currentThread();
});
```

---

## 10. 注意事项与最佳实践

### 10.1 QtConcurrent::run() 不支持取消

```cpp
QFuture<void> future = QtConcurrent::run([]() {
    while (true) { /* 死循环 */ }
});

future.cancel(); // ❌ 无效！函数仍在执行
```

需要取消单个 `run()` 任务时，手动使用原子标志：

```cpp
QAtomicInt cancelled(0);

QFuture<void> future = QtConcurrent::run([&cancelled]() {
    while (!cancelled.loadAcquire()) {
        doOneStep();
    }
});

// 需要取消时
cancelled.storeRelease(1);
future.waitForFinished();
```

### 10.2 Lambda 捕获的生命周期

```cpp
// ❌ 悬空引用：局部变量可能在 lambda 执行时已销毁
void someFunction() {
    QString localData = "hello";
    QtConcurrent::run([&localData]() {
        QThread::sleep(1);
        qDebug() << localData; // 可能崩溃！localData 已析构
    });
} // localData 在此销毁，但 lambda 可能仍在执行

// ✅ 按值捕获
void someFunction() {
    QString localData = "hello";
    QtConcurrent::run([localData]() { // 值拷贝
        QThread::sleep(1);
        qDebug() << localData; // 安全
    });
}
```

### 10.3 不要在并行函数中操作 GUI

```cpp
// ❌ mapped 的函数在线程池线程中执行，不能操作 GUI
QFuture<void> future = QtConcurrent::map(images, [this](QImage &img) {
    img = img.mirrored();
    m_label->setPixmap(QPixmap::fromImage(img)); // 崩溃！
});

// ✅ 通过 Watcher 信号在主线程更新 UI
connect(watcher, &QFutureWatcher<void>::resultReadyAt, this, [this](int i) {
    m_label->setPixmap(QPixmap::fromImage(images[i])); // 主线程，安全
});
```

### 10.4 map/filter 中函数必须是线程安全的

```cpp
// ❌ 共享非线程安全的状态
int counter = 0;
QtConcurrent::map(items, [&counter](Item &item) {
    counter++; // 竞态条件！
    item.process();
});

// ✅ 使用原子变量
QAtomicInt counter(0);
QtConcurrent::map(items, [&counter](Item &item) {
    counter.fetchAndAddOrdered(1);
    item.process();
});

// ✅ 或使用 mappedReduced 进行统计
```

### 10.5 避免线程池耗尽

```cpp
// ❌ 危险：如果线程池只有 N 个线程，提交 N 个任务且每个任务内部又等待
// 另一个 QtConcurrent::run() 的结果 → 死锁
QtConcurrent::run([]() {
    // 占用一个线程池线程
    auto inner = QtConcurrent::run([]() { return 42; });
    inner.waitForFinished(); // 如果所有线程都在等 → 死锁
});

// ✅ 使用独立线程池隔离
QThreadPool outerPool, innerPool;
outerPool.setMaxThreadCount(4);
innerPool.setMaxThreadCount(4);

QtConcurrent::run(&outerPool, [&innerPool]() {
    auto inner = QtConcurrent::run(&innerPool, []() { return 42; });
    return inner.result(); // 不会与 outerPool 竞争
});
```

---

## 11. API 选择决策

```
需要并行处理？
  │
  ├─ 单个异步任务（不需要取消/暂停）？
  │     └─ QtConcurrent::run()
  │
  ├─ 对容器每个元素应用函数？
  │     ├─ 修改原容器？ → map()
  │     └─ 生成新容器？ → mapped()
  │
  ├─ 从容器中筛选元素？
  │     ├─ 修改原容器？ → filter()
  │     └─ 生成新容器？ → filtered()
  │
  ├─ 映射 + 聚合？
  │     └─ mappedReduced()
  │
  ├─ 过滤 + 聚合？
  │     └─ filteredReduced()
  │
  └─ 需要非阻塞地监控结果和进度？
        └─ QFutureWatcher + 以上任意 API
```

---

## 12. 总结

| 要点 | 说明 |
|---|---|
| **定位** | 高级并行 API，隐藏线程管理细节 |
| **核心组件** | `QtConcurrent` 函数 + `QFuture` + `QFutureWatcher` |
| **run()** | 在线程池执行单个函数，最简单的异步方式 |
| **map/mapped** | 并行对容器元素应用转换（原地 / 新容器） |
| **filter/filtered** | 并行过滤容器元素 |
| **mappedReduced** | Map + Reduce 一站式并行聚合 |
| **QFutureWatcher** | 非阻塞监控，通过信号槽获取结果/进度 |
| **线程池** | 默认使用全局线程池，可自定义隔离 |
| **取消** | `map`/`filter`/`reduce` 支持取消，`run()` 不支持 |
| **关键限制** | 不能在并行函数中操作 GUI；Lambda 捕获注意生命周期 |
