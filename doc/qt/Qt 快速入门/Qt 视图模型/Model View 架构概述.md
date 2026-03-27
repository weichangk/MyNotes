# Model/View 架构概述

## 1. 从 MVC 到 Model/View

### 1.1 经典 MVC 回顾

经典 MVC（Model-View-Controller）将应用分为三层：

- **Model（模型）**：管理数据和业务逻辑
- **View（视图）**：负责数据的可视化呈现
- **Controller（控制器）**：处理用户输入，协调 Model 和 View

```
┌────────────┐     通知变化     ┌────────────┐
│   Model    │ ──────────────→ │    View    │
│  (数据层)   │ ←────────────── │  (显示层)   │
└────────────┘    请求数据      └────────────┘
      ↑                              ↑
      │          用户操作              │
      └────────── Controller ─────────┘
```

### 1.2 Qt 的 Model/View 架构

Qt 将 Controller 的角色合并到 View 中，并引入 **Delegate（委托）** 来处理数据的渲染和编辑，形成了 **Model/View/Delegate** 三位一体的架构：

```
┌────────────┐  数据访问接口  ┌────────────┐  渲染/编辑  ┌────────────┐
│   Model    │ ←──────────→ │    View    │ ←────────→ │  Delegate  │
│  数据抽象层  │   QModelIndex │  视图呈现层  │            │  委托渲染层  │
└────────────┘              └────────────┘            └────────────┘
      ↑                          ↑
      │        信号/槽机制          │
      └──────────────────────────┘
         dataChanged / layoutChanged
```

**核心设计思想：数据与表现分离。** 同一份数据（Model）可以同时被多个 View 展示，且修改会自动同步到所有视图。

---

## 2. 核心组件详解

### 2.1 Model（模型）

Model 是数据的抽象接口，不关心数据如何存储（可以是内存数组、数据库、文件、网络等），只需通过统一的 API 向 View 提供数据。

**模型继承体系：**

```
QAbstractItemModel          ← 所有模型的基类（最通用，支持树/表/列表）
├── QAbstractListModel      ← 一维列表模型基类
├── QAbstractTableModel     ← 二维表格模型基类
├── QStandardItemModel      ← 通用标准模型（开箱即用）
├── QStringListModel        ← 字符串列表模型
├── QFileSystemModel        ← 文件系统模型
├── QSqlTableModel          ← SQL 数据库表模型
├── QSqlQueryModel          ← SQL 查询结果模型
├── QSqlRelationalTableModel← SQL 关联表模型
└── QSortFilterProxyModel   ← 排序/过滤代理模型
```

**Model 的核心职责：**

| 职责 | 说明 |
|------|------|
| 提供数据 | 通过 `data()` 返回指定索引和角色的数据 |
| 报告结构 | 通过 `rowCount()`、`columnCount()` 描述数据维度 |
| 提供索引 | 通过 `index()` 和 `parent()` 构建导航结构 |
| 通知变化 | 数据变化时发射 `dataChanged`、`layoutChanged` 等信号 |
| 支持编辑 | 可选实现 `setData()`、`flags()` 以支持数据修改 |
| 拖放支持 | 可选实现 `mimeData()`、`dropMimeData()` 支持拖放操作 |

### 2.2 View（视图）

View 负责从 Model 中读取数据并渲染展示，同时处理用户交互（滚动、选择、点击等）。

**Qt 预置的视图控件：**

| 视图类 | 展示方式 | 典型用途 |
|--------|----------|----------|
| `QListView` | 一维列表 | 文件列表、消息列表 |
| `QTableView` | 二维表格 | 数据库表格、电子表格 |
| `QTreeView` | 树形层级 | 文件浏览器、XML/JSON 结构 |
| `QColumnView` | 多列级联 | macOS Finder 风格的目录浏览 |
| `QHeaderView` | 表头 | 配合 QTableView / QTreeView 使用 |

> **注意：** `QListWidget`、`QTableWidget`、`QTreeWidget` 是为方便使用而封装的"便捷类"，它们**内部自带了 Model**，适合简单场景但灵活性较低。

### 2.3 Delegate（委托）

Delegate 控制数据项在 View 中的**渲染方式**和**编辑交互**。

- **`QStyledItemDelegate`**（推荐）：使用当前应用样式渲染，支持 QSS 样式表
- **`QItemDelegate`**：直接绘制，不使用样式引擎（已不推荐）

Delegate 的三大职责：

1. **paint()** — 自定义绘制数据项的外观
2. **createEditor()** — 创建编辑控件（如 QSpinBox、QComboBox）
3. **setEditorData() / setModelData()** — 在 Model 与 Editor 之间传递数据

### 2.4 QModelIndex（模型索引）

`QModelIndex` 是 Model/View 架构中最核心的数据定位器，它唯一标识 Model 中的一个数据项：

```cpp
QModelIndex index = model->index(row, column, parentIndex);
```

**QModelIndex 的组成：**

| 成员 | 说明 |
|------|------|
| `row()` | 行号（从 0 开始） |
| `column()` | 列号（从 0 开始） |
| `parent()` | 父索引（用于树形结构） |
| `model()` | 所属的 Model 指针 |
| `internalPointer()` | 内部数据指针（自定义 Model 使用） |

**关键规则：**

- `QModelIndex` 是**临时对象**，不应长期存储（Model 结构变化后会失效）
- 需持久保存索引时，使用 `QPersistentModelIndex`
- 无效索引用 `QModelIndex()` 表示（通常作为顶层 parent）

```cpp
// 遍历 Model 中所有数据
for (int row = 0; row < model->rowCount(); ++row) {
    for (int col = 0; col < model->columnCount(); ++col) {
        QModelIndex index = model->index(row, col);
        QString text = model->data(index, Qt::DisplayRole).toString();
        qDebug() << text;
    }
}
```

### 2.5 数据角色（Item Role）

Qt 使用**角色（Role）**机制实现"一个数据项，多种表现"。同一个 `QModelIndex`，传入不同的角色会返回不同的数据：

```cpp
QVariant data = model->data(index, role);
```

**常用角色一览：**

| 角色枚举 | 值 | 说明 | 典型数据类型 |
|----------|---|------|-------------|
| `Qt::DisplayRole` | 0 | 显示文本 | `QString` |
| `Qt::EditRole` | 2 | 编辑时使用的数据 | `QString`、`int` 等 |
| `Qt::DecorationRole` | 1 | 图标/颜色装饰 | `QIcon`、`QPixmap`、`QColor` |
| `Qt::ToolTipRole` | 3 | 鼠标悬停提示 | `QString` |
| `Qt::StatusTipRole` | 4 | 状态栏提示 | `QString` |
| `Qt::WhatsThisRole` | 5 | "这是什么"帮助文本 | `QString` |
| `Qt::FontRole` | 6 | 字体 | `QFont` |
| `Qt::TextAlignmentRole` | 7 | 文本对齐方式 | `Qt::Alignment` |
| `Qt::BackgroundRole` | 8 | 背景色/画刷 | `QBrush` |
| `Qt::ForegroundRole` | 9 | 前景色/画刷 | `QBrush` |
| `Qt::CheckStateRole` | 10 | 复选框状态 | `Qt::CheckState` |
| `Qt::SizeHintRole` | 13 | 项的建议尺寸 | `QSize` |
| `Qt::UserRole` | 0x0100 | 自定义数据起始值 | 任意 `QVariant` |

```cpp
// 在自定义 Model 中为同一项返回不同角色的数据
QVariant MyModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();

    const auto &item = m_items[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return item.name;                    // 显示名称
    case Qt::DecorationRole:
        return item.icon;                    // 显示图标
    case Qt::ToolTipRole:
        return tr("大小: %1 KB").arg(item.size / 1024);  // 悬停提示
    case Qt::ForegroundRole:
        return item.isModified ? QBrush(Qt::red) : QBrush(Qt::black);
    case Qt::UserRole:
        return item.filePath;                // 自定义数据
    default:
        return QVariant();
    }
}
```

---

## 3. 三种数据组织形式

Qt Model/View 支持三种数据结构，对应不同的 Model 基类：

### 3.1 列表模型（List Model）

一维线性结构，只有行的概念，无列和层级。

```
Row 0: "Apple"
Row 1: "Banana"
Row 2: "Cherry"
```

```cpp
// 使用 QStringListModel
QStringList fruits = {"Apple", "Banana", "Cherry"};
QStringListModel *model = new QStringListModel(fruits, this);

QListView *view = new QListView(this);
view->setModel(model);
```

### 3.2 表格模型（Table Model）

二维表格结构，有行和列，但没有层级关系。

```
        Column 0    Column 1    Column 2
Row 0:  "张三"      "25"        "工程师"
Row 1:  "李四"      "30"        "设计师"
Row 2:  "王五"      "28"        "产品经理"
```

```cpp
// 使用 QStandardItemModel 构建表格
QStandardItemModel *model = new QStandardItemModel(3, 3, this);
model->setHorizontalHeaderLabels({"姓名", "年龄", "职位"});

model->setItem(0, 0, new QStandardItem("张三"));
model->setItem(0, 1, new QStandardItem("25"));
model->setItem(0, 2, new QStandardItem("工程师"));

QTableView *view = new QTableView(this);
view->setModel(model);
```

### 3.3 树形模型（Tree Model）

层级结构，每个节点可以有子节点，形成父子关系。

```
[根]
├── 水果
│   ├── 苹果
│   ├── 香蕉
│   └── 樱桃
└── 蔬菜
    ├── 胡萝卜
    └── 土豆
```

```cpp
// 使用 QStandardItemModel 构建树
QStandardItemModel *model = new QStandardItemModel(this);
model->setHorizontalHeaderLabels({"名称", "数量"});

QStandardItem *rootFruit = new QStandardItem("水果");
rootFruit->appendRow({new QStandardItem("苹果"), new QStandardItem("50")});
rootFruit->appendRow({new QStandardItem("香蕉"), new QStandardItem("30")});
rootFruit->appendRow({new QStandardItem("樱桃"), new QStandardItem("20")});

QStandardItem *rootVegetable = new QStandardItem("蔬菜");
rootVegetable->appendRow({new QStandardItem("胡萝卜"), new QStandardItem("40")});
rootVegetable->appendRow({new QStandardItem("土豆"), new QStandardItem("60")});

model->appendRow(rootFruit);
model->appendRow(rootVegetable);

QTreeView *view = new QTreeView(this);
view->setModel(model);
view->expandAll();
```

---

## 4. 选择机制（Selection Model）

`QItemSelectionModel` 管理 View 中的选择状态，**独立于 Model 和 View**，可被多个 View 共享：

```cpp
// 两个视图共享同一个选择模型 → 选中同步
QTableView *view1 = new QTableView;
QTableView *view2 = new QTableView;
view1->setModel(model);
view2->setModel(model);
view2->setSelectionModel(view1->selectionModel());  // 共享选择
```

**选择模式配置：**

```cpp
// 选择行为（选单元格/行/列）
view->setSelectionBehavior(QAbstractItemView::SelectRows);

// 选择模式（单选/多选/扩展选择等）
view->setSelectionMode(QAbstractItemView::ExtendedSelection);
```

| 选择模式 | 说明 |
|----------|------|
| `SingleSelection` | 只能选一个 |
| `ContiguousSelection` | 连续多选（Shift 键） |
| `ExtendedSelection` | 扩展多选（Ctrl/Shift 键） |
| `MultiSelection` | 点击切换选中/取消 |
| `NoSelection` | 禁止选择 |

**监听选择变化：**

```cpp
connect(view->selectionModel(), &QItemSelectionModel::selectionChanged,
    this, [](const QItemSelection &selected, const QItemSelection &deselected) {
        for (const QModelIndex &index : selected.indexes()) {
            qDebug() << "选中:" << index.data().toString();
        }
    });
```

---

## 5. 信号与通知机制

Model/View 架构通过**信号/槽**实现组件间解耦通信：

### 5.1 Model → View 的通知信号

| 信号 | 触发时机 |
|------|----------|
| `dataChanged(topLeft, bottomRight, roles)` | 数据值发生变化 |
| `headerDataChanged(orientation, first, last)` | 表头数据变化 |
| `layoutChanged()` | 数据布局（排序/筛选）发生变化 |
| `layoutAboutToBeChanged()` | 布局即将变化（View 可保存状态） |
| `modelReset()` | 模型完全重置 |
| `modelAboutToBeReset()` | 模型即将重置 |
| `rowsInserted(parent, first, last)` | 新行插入后 |
| `rowsAboutToBeInserted(parent, first, last)` | 新行即将插入 |
| `rowsRemoved(parent, first, last)` | 行删除后 |
| `columnsInserted(parent, first, last)` | 新列插入后 |
| `columnsRemoved(parent, first, last)` | 列删除后 |

### 5.2 正确的数据修改模式

```cpp
// ❌ 错误：直接修改数据但不通知
m_data[index.row()] = newValue;

// ✅ 正确：在 setData() 中修改并发射信号
bool MyModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    m_data[index.row()] = value.toString();
    emit dataChanged(index, index, {role});  // 通知所有视图
    return true;
}

// ✅ 正确：批量插入行
void MyModel::addItems(const QStringList &items) {
    int first = m_data.size();
    int last = first + items.size() - 1;
    beginInsertRows(QModelIndex(), first, last);  // 开始插入
    m_data.append(items);
    endInsertRows();                               // 结束插入 → 自动通知视图
}
```

---

## 6. 完整实战示例：联系人管理器

下面用一个完整的联系人管理器示例，展示 Model/View 的各组件如何协作：

### 6.1 数据结构

```cpp
// contact.h
struct Contact {
    QString name;
    QString phone;
    QString email;
    bool    isFavorite = false;
};
```

### 6.2 自定义 Model

```cpp
// contactmodel.h
#include <QAbstractTableModel>
#include "contact.h"

class ContactModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { Name = 0, Phone, Email, Favorite, ColumnCount };

    explicit ContactModel(QObject *parent = nullptr)
        : QAbstractTableModel(parent) {}

    // --- 只读接口 ---
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return parent.isValid() ? 0 : m_contacts.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        return parent.isValid() ? 0 : ColumnCount;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_contacts.size())
            return QVariant();

        const Contact &c = m_contacts[index.row()];

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (index.column()) {
            case Name:  return c.name;
            case Phone: return c.phone;
            case Email: return c.email;
            default: break;
            }
        }
        // 收藏列使用复选框
        if (index.column() == Favorite && role == Qt::CheckStateRole) {
            return c.isFavorite ? Qt::Checked : Qt::Unchecked;
        }
        // 收藏的联系人用粗体显示
        if (role == Qt::FontRole && c.isFavorite) {
            QFont font;
            font.setBold(true);
            return font;
        }
        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
            return QVariant();
        switch (section) {
        case Name:     return tr("姓名");
        case Phone:    return tr("电话");
        case Email:    return tr("邮箱");
        case Favorite: return tr("收藏");
        default:       return QVariant();
        }
    }

    // --- 可编辑接口 ---
    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags f = QAbstractTableModel::flags(index);
        if (index.column() == Favorite)
            f |= Qt::ItemIsUserCheckable;  // 收藏列可勾选
        else
            f |= Qt::ItemIsEditable;       // 其他列可编辑
        return f;
    }

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override {
        if (!index.isValid() || index.row() >= m_contacts.size())
            return false;

        Contact &c = m_contacts[index.row()];

        if (role == Qt::EditRole) {
            switch (index.column()) {
            case Name:  c.name  = value.toString(); break;
            case Phone: c.phone = value.toString(); break;
            case Email: c.email = value.toString(); break;
            default: return false;
            }
        } else if (role == Qt::CheckStateRole && index.column() == Favorite) {
            c.isFavorite = (value.toInt() == Qt::Checked);
        } else {
            return false;
        }

        emit dataChanged(index, index, {role});
        return true;
    }

    // --- 增删接口 ---
    void addContact(const Contact &contact) {
        int row = m_contacts.size();
        beginInsertRows(QModelIndex(), row, row);
        m_contacts.append(contact);
        endInsertRows();
    }

    void removeContact(int row) {
        if (row < 0 || row >= m_contacts.size()) return;
        beginRemoveRows(QModelIndex(), row, row);
        m_contacts.removeAt(row);
        endRemoveRows();
    }

private:
    QList<Contact> m_contacts;
};
```

### 6.3 使用示例

```cpp
// main.cpp
#include <QApplication>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include "contactmodel.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 创建 Model
    ContactModel *model = new ContactModel;
    model->addContact({"张三", "138-0001-0001", "zhangsan@example.com", true});
    model->addContact({"李四", "139-0002-0002", "lisi@example.com", false});
    model->addContact({"王五", "137-0003-0003", "wangwu@example.com", false});

    // 创建排序/过滤代理
    QSortFilterProxyModel *proxy = new QSortFilterProxyModel;
    proxy->setSourceModel(model);
    proxy->setFilterKeyColumn(-1);          // 搜索所有列
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    // 创建 View
    QTableView *view = new QTableView;
    view->setModel(proxy);
    view->setSortingEnabled(true);          // 启用列头点击排序
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->horizontalHeader()->setStretchLastSection(true);

    // 搜索框
    QLineEdit *searchBox = new QLineEdit;
    searchBox->setPlaceholderText("搜索联系人...");
    QObject::connect(searchBox, &QLineEdit::textChanged,
                     proxy, &QSortFilterProxyModel::setFilterFixedString);

    // 删除按钮
    QPushButton *deleteBtn = new QPushButton("删除选中");
    QObject::connect(deleteBtn, &QPushButton::clicked, [&]() {
        QModelIndex proxyIndex = view->currentIndex();
        if (proxyIndex.isValid()) {
            QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);
            model->removeContact(sourceIndex.row());
        }
    });

    // 布局
    QWidget window;
    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->addWidget(searchBox);
    layout->addWidget(view);
    layout->addWidget(deleteBtn);
    window.setWindowTitle("联系人管理器");
    window.resize(600, 400);
    window.show();

    return app.exec();
}
```

---

## 7. 代理模型（Proxy Model）

代理模型是 Model/View 架构的强大扩展——它**不持有数据**，而是包装在源 Model 之上，对数据进行变换后呈现给 View：

```
View ←→ ProxyModel ←→ SourceModel ←→ 数据
```

### 7.1 QSortFilterProxyModel

最常用的代理模型，提供排序和过滤功能：

```cpp
QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
proxy->setSourceModel(sourceModel);

// 按正则过滤
proxy->setFilterRegularExpression(QRegularExpression("^张"));
proxy->setFilterKeyColumn(0);   // 只对第 0 列过滤

// 自定义排序
proxy->setSortRole(Qt::UserRole);  // 按 UserRole 数据排序
proxy->sort(1, Qt::DescendingOrder);
```

### 7.2 代理链

代理模型可以串联形成处理链：

```cpp
// 先过滤再排序（语义清晰，各司其职）
QSortFilterProxyModel *filterProxy = new QSortFilterProxyModel;
filterProxy->setSourceModel(dataModel);
filterProxy->setFilterFixedString("重要");

QSortFilterProxyModel *sortProxy = new QSortFilterProxyModel;
sortProxy->setSourceModel(filterProxy);    // 代理的代理
sortProxy->sort(0);

view->setModel(sortProxy);
```

### 7.3 索引映射

使用代理时需注意 **View 的索引是代理索引**，与源 Model 索引不同：

```cpp
// 代理索引 → 源索引
QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);

// 源索引 → 代理索引
QModelIndex proxyIndex = proxy->mapFromSource(sourceIndex);
```

---

## 8. 多视图共享模型

Model/View 分离的直接好处——**一个 Model 可以驱动多个 View**：

```cpp
QStandardItemModel *model = new QStandardItemModel;
// ... 填充数据 ...

QListView  *listView  = new QListView;
QTableView *tableView = new QTableView;
QTreeView  *treeView  = new QTreeView;

// 三个视图展示同一份数据
listView->setModel(model);
tableView->setModel(model);
treeView->setModel(model);

// 任何一个视图的编辑都会自动同步到其他视图
```

---

## 9. 便捷控件 vs Model/View 控件对比

| 便捷控件 | Model/View 控件 | 适用场景 |
|----------|-----------------|----------|
| `QListWidget` | `QListView` + Model | 便捷类适合少量静态数据 |
| `QTableWidget` | `QTableView` + Model | Model/View 适合大量/动态数据 |
| `QTreeWidget` | `QTreeView` + Model | Model/View 适合需自定义数据源 |

**便捷控件的局限：**

- 内部使用 `QStandardItemModel`，每个项都是一个 `QWidget` 子对象，数万项时性能差
- 无法接入自定义数据源（如数据库、网络API）
- 无法使用代理模型（排序/过滤需手动实现）
- 无法多视图共享

**经验法则：** 数据量 < 1000 且无特殊需求 → 便捷控件；其他情况 → Model/View。

---

## 10. 性能与最佳实践

### 10.1 延迟加载（Lazy Loading）

对于大数据集（如文件系统），不要一次加载全部数据：

```cpp
// QAbstractItemModel 支持懒加载
bool MyModel::canFetchMore(const QModelIndex &parent) const {
    return m_hasMoreData;  // 是否还有更多数据
}

void MyModel::fetchMore(const QModelIndex &parent) {
    int remaining = m_totalCount - m_loadedCount;
    int toFetch = qMin(100, remaining);  // 每次加载 100 条

    beginInsertRows(parent, m_loadedCount, m_loadedCount + toFetch - 1);
    // ... 从数据源加载数据 ...
    m_loadedCount += toFetch;
    endInsertRows();
}
```

### 10.2 批量更新优化

```cpp
// ❌ 逐个通知（每次都触发视图重绘）
for (int i = 0; i < 1000; ++i) {
    m_data[i] = newValues[i];
    emit dataChanged(index(i, 0), index(i, 0));
}

// ✅ 批量通知（只触发一次重绘）
for (int i = 0; i < 1000; ++i) {
    m_data[i] = newValues[i];
}
emit dataChanged(index(0, 0), index(999, columnCount() - 1));
```

### 10.3 模型重置

大规模数据替换时，使用 `beginResetModel()` / `endResetModel()`：

```cpp
void MyModel::replaceAllData(const QList<Contact> &newData) {
    beginResetModel();   // 通知视图：模型即将完全重置
    m_contacts = newData;
    endResetModel();     // 通知视图：重置完成，请重新获取所有数据
}
```

> **注意：** 重置会清除所有选择状态和展开状态，应谨慎使用。

### 10.4 常见陷阱

| 陷阱 | 说明 |
|------|------|
| 修改数据不发信号 | View 不会更新。务必调用 `dataChanged` / `beginInsertRows` 系列 |
| 长期持有 QModelIndex | 模型结构变化后索引失效。使用 `QPersistentModelIndex` 替代 |
| 在 beginXxx / endXxx 之间访问视图 | 数据处于不一致状态，可能导致崩溃 |
| data() 中做耗时操作 | `data()` 被频繁调用，应保证 O(1) 复杂度 |
| 忘记处理无效索引 | `data()`、`setData()` 中必须先检查 `index.isValid()` |

---

## 11. 架构总结

```
┌─────────────────────────────────────────────────────────┐
│                      用户界面层                          │
│  ┌──────────┐  ┌───────────┐  ┌──────────┐              │
│  │ QListView│  │QTableView │  │QTreeView │  ...         │
│  └────┬─────┘  └─────┬─────┘  └────┬─────┘              │
│       │              │              │                    │
│       └──────────────┼──────────────┘                    │
│                      │                                   │
│              QItemSelectionModel (选择状态)               │
│              QStyledItemDelegate (渲染/编辑)              │
│                      │                                   │
├──────────────────────┼───────────────────────────────────┤
│                      │         接口层                     │
│           QAbstractItemModel                             │
│            index() / data() / setData()                  │
│            rowCount() / columnCount()                    │
│            parent() / flags()                            │
│                      │                                   │
├──────────────────────┼───────────────────────────────────┤
│                      │         数据层                     │
│  ┌──────────┐  ┌─────┴─────┐  ┌──────────┐              │
│  │ 内存数据  │  │  数据库   │  │ 网络API  │  ...         │
│  │ QList/Map│  │ QSqlQuery │  │ REST/WS  │              │
│  └──────────┘  └───────────┘  └──────────┘              │
└─────────────────────────────────────────────────────────┘
```

**核心原则回顾：**

1. **数据与表现分离** — Model 管数据，View 管展示，Delegate 管渲染
2. **通过 QModelIndex 定位** — 行、列、父索引三元组唯一确定一个数据项
3. **通过 Role 多态呈现** — 同一数据项可返回文本、图标、字体、颜色等多种表现
4. **信号驱动更新** — Model 变化通过信号自动同步到所有 View
5. **代理模型层层包装** — 不修改原始数据即可实现排序、过滤、映射等变换
6. **多视图共享模型** — 一份数据，多种展示，自动同步
