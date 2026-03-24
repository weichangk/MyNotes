# Qt 快速入门

## 目录结构

- [**一、Qt 对象模型**](./Qt%20对象模型/README.md)
  - [QObject 与对象树](./Qt%20对象模型/QObject%20与对象树.md)
  - [信号与槽机制](./Qt%20对象模型/信号与槽机制.md)
  - [元对象系统](./Qt%20对象模型/元对象系统.md)
  - [属性系统](./Qt%20对象模型/属性系统.md)
  - [对象的内存管理](./Qt%20对象模型/对象的内存管理.md)
- [**二、Qt 窗口部件**](./Qt%20窗口部件/README.md)
  - [QWidget 基础](./Qt%20窗口部件/QWidget%20基础.md)
  - [常用控件](./Qt%20窗口部件/常用控件.md)（QPushButton、QLabel、QLineEdit 等）
  - [对话框](./Qt%20窗口部件/对话框.md)（QDialog、QMessageBox、QFileDialog）
  - [主窗口框架](./Qt%20窗口部件/主窗口框架.md)（QMainWindow、菜单栏、工具栏、状态栏）
  - [自定义控件](./Qt%20窗口部件/自定义控件.md)
- [**三、Qt 布局管理**](./Qt%20布局管理/README.md)
  - [基本布局](./Qt%20布局管理/基本布局.md)（QHBoxLayout、QVBoxLayout）
  - [网格与表单布局](./Qt%20布局管理/网格与表单布局.md)（QGridLayout、QFormLayout）
  - [分裂器与堆叠布局](./Qt%20布局管理/分裂器与堆叠布局.md)（QSplitter、QStackedLayout）
  - [布局嵌套与间距](./Qt%20布局管理/布局嵌套与间距.md)
- [**四、Qt 事件系统**](./Qt%20事件系统/README.md)
  - [事件处理机制](./Qt%20事件系统/事件处理机制.md)
  - [事件过滤器](./Qt%20事件系统/事件过滤器.md)
  - [自定义事件](./Qt%20事件系统/自定义事件.md)
  - [键盘与鼠标事件](./Qt%20事件系统/键盘与鼠标事件.md)
  - [定时器](./Qt%20事件系统/定时器.md)（QTimer、timerEvent）
- [**五、Qt 样式**](./Qt%20样式/README.md)
  - [QSS 基础语法](./Qt%20样式/QSS%20基础语法.md)
  - [QSS 选择器](./Qt%20样式/QSS%20选择器.md)
  - [QPalette 调色板](./Qt%20样式/QPalette%20调色板.md)
  - [主题切换](./Qt%20样式/主题切换.md)
- [**六、Qt 绘图**](./Qt%20绘图/README.md)
  - [QPainter 基础](./Qt%20绘图/QPainter%20基础.md)
  - [画笔与画刷](./Qt%20绘图/画笔与画刷.md)（QPen、QBrush）
  - [坐标系统与变换](./Qt%20绘图/坐标系统与变换.md)
  - [图片绘制](./Qt%20绘图/图片绘制.md)（QPixmap、QImage）
  - [QGraphicsView 框架](./Qt%20绘图/QGraphicsView%20框架.md)
- [**七、Qt 视图模型**](./Qt%20视图模型/README.md)
  - [Model/View 架构概述](./Qt%20视图模型/Model%20View%20架构概述.md)
  - [标准模型](./Qt%20视图模型/标准模型.md)（QStandardItemModel）
  - [自定义模型](./Qt%20视图模型/自定义模型.md)（QAbstractItemModel）
  - [视图控件](./Qt%20视图模型/视图控件.md)（QListView、QTreeView、QTableView）
  - [代理与委托](./Qt%20视图模型/代理与委托.md)（QStyledItemDelegate）
  - [排序与过滤](./Qt%20视图模型/排序与过滤.md)（QSortFilterProxyModel）
- [**八、Qt 文件与 IO**](./Qt%20文件与%20IO/README.md)
  - [QFile 与 QDir](./Qt%20文件与%20IO/QFile%20与%20QDir.md)
  - [文本流与数据流](./Qt%20文件与%20IO/文本流与数据流.md)（QTextStream、QDataStream）
  - [文件监控](./Qt%20文件与%20IO/文件监控.md)（QFileSystemWatcher）
- [**九、Qt 数据库**](./Qt%20数据库/README.md)
  - [数据库连接](./Qt%20数据库/数据库连接.md)（QSqlDatabase）
  - [SQL 查询](./Qt%20数据库/SQL%20查询.md)（QSqlQuery）
  - [数据模型](./Qt%20数据库/数据模型.md)（QSqlTableModel、QSqlQueryModel）
- [**十、Qt XML JSON**](./Qt%20XML%20JSON/README.md)
  - [XML 解析与生成](./Qt%20XML%20JSON/XML%20解析与生成.md)（QXmlStreamReader/Writer、QDomDocument）
  - [JSON 解析与生成](./Qt%20XML%20JSON/JSON%20解析与生成.md)（QJsonDocument、QJsonObject、QJsonArray）
- [**十一、Qt 网络编程**](./Qt%20网络编程/README.md)
  - [HTTP 请求](./Qt%20网络编程/HTTP%20请求.md)（QNetworkAccessManager）
  - [TCP 通信](./Qt%20网络编程/TCP%20通信.md)（QTcpSocket、QTcpServer）
  - [UDP 通信](./Qt%20网络编程/UDP%20通信.md)（QUdpSocket）
  - [WebSocket](./Qt%20网络编程/WebSocket.md)（QWebSocket）
- [**十二、Qt 进程线程**](./Qt%20进程线程/README.md)
  - [QThread 基础](./Qt%20进程线程/QThread%20基础.md)
  - [线程同步](./Qt%20进程线程/线程同步.md)（QMutex、QWaitCondition）
  - [QtConcurrent](./Qt%20进程线程/QtConcurrent.md)
  - [信号槽跨线程](./Qt%20进程线程/信号槽跨线程.md)
  - [QProcess](./Qt%20进程线程/QProcess.md)
- [**十三、Qt 多媒体**](./Qt%20多媒体/README.md)
  - [音频播放与录制](./Qt%20多媒体/音频播放与录制.md)
  - [视频播放](./Qt%20多媒体/视频播放.md)
  - [摄像头](./Qt%20多媒体/摄像头.md)
- [**十四、Qt 资源系统**](./Qt%20资源系统/README.md)
  - [qrc 资源文件](./Qt%20资源系统/qrc%20资源文件.md)
  - [国际化与翻译](./Qt%20资源系统/国际化与翻译.md)（tr()、QTranslator、.ts/.qm）
- [**十五、Qt 动画框架**](./Qt%20动画框架/README.md)
  - [属性动画](./Qt%20动画框架/属性动画.md)（QPropertyAnimation）
  - [动画组合](./Qt%20动画框架/动画组合.md)（QSequentialAnimationGroup、QParallelAnimationGroup）
- [**十六、Qt WebEngine**](./Qt%20WebEngine/README.md)
  - [QWebEngineView 基础](./Qt%20WebEngine/QWebEngineView%20基础.md)
  - [JS 与 C++ 交互](./Qt%20WebEngine/JS%20与%20C++%20交互.md)

---

## 📊 完成进度

### 一、Qt 对象模型

| 文档 | 状态 |
|------|------|
| QObject 与对象树 | ✅ 已完成 |
| 信号与槽机制 | ✅ 已完成 |
| 元对象系统 | ✅ 已完成 |
| 属性系统 | ✅ 已完成 |
| 对象的内存管理 | ✅ 已完成 |

### 二、Qt 窗口部件

| 文档 | 状态 |
|------|------|
| QWidget 基础 | ❌ 待完成 |
| 常用控件 | ❌ 待完成 |
| 对话框 | ❌ 待完成 |
| 主窗口框架 | ❌ 待完成 |
| 自定义控件 | ❌ 待完成 |

### 三、Qt 布局管理

| 文档 | 状态 |
|------|------|
| 基本布局 | ❌ 待完成 |
| 网格与表单布局 | ❌ 待完成 |
| 分裂器与堆叠布局 | ❌ 待完成 |
| 布局嵌套与间距 | ❌ 待完成 |

### 四、Qt 事件系统

| 文档 | 状态 |
|------|------|
| 事件处理机制 | ❌ 待完成 |
| 事件过滤器 | ❌ 待完成 |
| 自定义事件 | ❌ 待完成 |
| 键盘与鼠标事件 | ❌ 待完成 |
| 定时器 | ❌ 待完成 |

### 五、Qt 样式

| 文档 | 状态 |
|------|------|
| QSS 基础语法 | ❌ 待完成 |
| QSS 选择器 | ❌ 待完成 |
| QPalette 调色板 | ❌ 待完成 |
| 主题切换 | ❌ 待完成 |

### 六、Qt 绘图

| 文档 | 状态 |
|------|------|
| QPainter 基础 | ❌ 待完成 |
| 画笔与画刷 | ❌ 待完成 |
| 坐标系统与变换 | ❌ 待完成 |
| 图片绘制 | ❌ 待完成 |
| QGraphicsView 框架 | ❌ 待完成 |

### 七、Qt 视图模型

| 文档 | 状态 |
|------|------|
| Model/View 架构概述 | ❌ 待完成 |
| 标准模型 | ❌ 待完成 |
| 自定义模型 | ❌ 待完成 |
| 视图控件 | ❌ 待完成 |
| 代理与委托 | ❌ 待完成 |
| 排序与过滤 | ❌ 待完成 |

### 八、Qt 文件与 IO

| 文档 | 状态 |
|------|------|
| QFile 与 QDir | ❌ 待完成 |
| 文本流与数据流 | ❌ 待完成 |
| 文件监控 | ❌ 待完成 |

### 九、Qt 数据库

| 文档 | 状态 |
|------|------|
| 数据库连接 | ❌ 待完成 |
| SQL 查询 | ❌ 待完成 |
| 数据模型 | ❌ 待完成 |

### 十、Qt XML JSON

| 文档 | 状态 |
|------|------|
| XML 解析与生成 | ❌ 待完成 |
| JSON 解析与生成 | ❌ 待完成 |

### 十一、Qt 网络编程

| 文档 | 状态 |
|------|------|
| HTTP 请求 | ❌ 待完成 |
| TCP 通信 | ❌ 待完成 |
| UDP 通信 | ❌ 待完成 |
| WebSocket | ❌ 待完成 |

### 十二、Qt 进程线程

| 文档 | 状态 |
|------|------|
| QThread 基础 | ❌ 待完成 |
| 线程同步 | ❌ 待完成 |
| QtConcurrent | ❌ 待完成 |
| 信号槽跨线程 | ❌ 待完成 |
| QProcess | ❌ 待完成 |

### 十三、Qt 多媒体

| 文档 | 状态 |
|------|------|
| 音频播放与录制 | ❌ 待完成 |
| 视频播放 | ❌ 待完成 |
| 摄像头 | ❌ 待完成 |

### 十四、Qt 资源系统

| 文档 | 状态 |
|------|------|
| qrc 资源文件 | ❌ 待完成 |
| 国际化与翻译 | ❌ 待完成 |

### 十五、Qt 动画框架

| 文档 | 状态 |
|------|------|
| 属性动画 | ❌ 待完成 |
| 动画组合 | ❌ 待完成 |

### 十六、Qt WebEngine

| 文档 | 状态 |
|------|------|
| QWebEngineView 基础 | ❌ 待完成 |
| JS 与 C++ 交互 | ❌ 待完成 |

---

*最后更新时间：2026年3月19日*
