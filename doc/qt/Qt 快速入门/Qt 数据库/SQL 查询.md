# SQL 查询

## 1. 你需要先建立的认知

在 Qt 里做 SQL 查询，本质上是三层：

1. `QSqlDatabase`：连接管理层（连接哪个数据库、连接名、打开/关闭）
2. `QSqlQuery`：执行层（执行 SQL、绑定参数、读结果）
3. `QSqlRecord` / `QSqlError`：结果与错误信息层

如果你使用表格控件或 MVC，还会涉及：

1. `QSqlQueryModel`：只读查询模型（适合报表、列表）
2. `QSqlTableModel`：单表增删改查模型
3. `QSqlRelationalTableModel`：带外键关系的单表模型

常见数据库驱动：

1. SQLite：`QSQLITE`（最轻量，文件型，学习与中小工具首选）
2. MySQL：`QMYSQL`
3. PostgreSQL：`QPSQL`
4. ODBC：`QODBC`

## 2. 工程配置与最小可运行代码

### 2.1 CMake

```cmake
find_package(Qt5 REQUIRED COMPONENTS Core Sql)

target_link_libraries(your_target PRIVATE
    Qt5::Core
    Qt5::Sql
)
```

### 2.2 连接数据库（推荐封装）

```cpp
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

QSqlDatabase openSqlite(const QString& dbPath, const QString& connName)
{
    if (QSqlDatabase::contains(connName)) {
        QSqlDatabase db = QSqlDatabase::database(connName);
        if (!db.isOpen() && !db.open()) {
            qWarning() << "open existing connection failed:" << db.lastError().text();
        }
        return db;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qWarning() << "open sqlite failed:" << db.lastError().text();
    }
    return db;
}
```

要点：

1. 连接名非常关键，别全部依赖默认连接。
2. 多线程场景每个线程使用独立连接名。
3. `QSqlDatabase` 是句柄语义，底层连接生命周期要明确管理。

## 3. 查询方式总览

## 3.1 直接执行 SQL（适合一次性语句）

```cpp
QSqlQuery q(db);
if (!q.exec("SELECT id, name, age FROM employee")) {
    qWarning() << q.lastError().text();
}
while (q.next()) {
    int id = q.value("id").toInt();
    QString name = q.value("name").toString();
    int age = q.value("age").toInt();
}
```

## 3.2 预处理 + 参数绑定（强烈推荐）

```cpp
QSqlQuery q(db);
q.prepare("SELECT id, name, age FROM employee WHERE age >= :minAge AND dept = :dept");
q.bindValue(":minAge", 25);
q.bindValue(":dept", "R&D");

if (!q.exec()) {
    qWarning() << q.lastError().text();
}
```

优势：

1. 防 SQL 注入
2. 减少字符串拼接错误
3. 某些驱动可复用执行计划

## 3.3 批量插入（`execBatch`）

```cpp
QSqlQuery q(db);
q.prepare("INSERT INTO employee(name, age, dept, salary) VALUES(?, ?, ?, ?)");

QVariantList names{ "Alice", "Bob", "Carol" };
QVariantList ages{ 26, 31, 28 };
QVariantList depts{ "R&D", "QA", "R&D" };
QVariantList salaries{ 18000, 22000, 20000 };

q.addBindValue(names);
q.addBindValue(ages);
q.addBindValue(depts);
q.addBindValue(salaries);

if (!q.execBatch()) {
    qWarning() << q.lastError().text();
}
```

## 4. 结果读取的关键细节

## 4.1 下标 vs 字段名

1. 按下标：速度通常更好（`q.value(0)`）
2. 按字段名：可读性更高（`q.value("name")`）

建议：高频路径用下标，普通业务优先字段名。

## 4.2 NULL 处理

```cpp
QVariant v = q.value("bonus");
if (v.isNull()) {
    // 数据库字段是 NULL
}
```

不要把 `NULL` 和空字符串、0 混为一谈。

## 4.3 时间与时区

1. 统一存 UTC（`QDateTime::toUTC()`）
2. 展示层再转本地时区
3. 库内字段类型尽量明确（`DATETIME` / `TIMESTAMP`）

## 5. 事务：保证一致性

事务是实战中最容易被忽略但最重要的能力。

```cpp
if (!db.transaction()) {
    qWarning() << "begin tx failed:" << db.lastError().text();
    return;
}

QSqlQuery q(db);
q.prepare("UPDATE account SET balance = balance - ? WHERE uid = ?");
q.addBindValue(100);
q.addBindValue(1001);
if (!q.exec()) {
    db.rollback();
    qWarning() << "debit failed:" << q.lastError().text();
    return;
}

q.prepare("UPDATE account SET balance = balance + ? WHERE uid = ?");
q.addBindValue(100);
q.addBindValue(2001);
if (!q.exec()) {
    db.rollback();
    qWarning() << "credit failed:" << q.lastError().text();
    return;
}

if (!db.commit()) {
    db.rollback();
    qWarning() << "commit failed:" << db.lastError().text();
}
```

最佳实践：

1. 多条相关更新必须在一个事务中。
2. 失败立即 `rollback()`。
3. 大批量导入用事务包裹，速度通常会显著提升。

## 6. 常用 SQL 查询模板（Qt 场景）

## 6.1 分页查询（通用）

```cpp
int pageNo = 3;
int pageSize = 20;
int offset = (pageNo - 1) * pageSize;

QSqlQuery q(db);
q.prepare("SELECT id, name, dept FROM employee ORDER BY id DESC LIMIT :limit OFFSET :offset");
q.bindValue(":limit", pageSize);
q.bindValue(":offset", offset);
q.exec();
```

## 6.2 统计 + 分组

```cpp
QSqlQuery q(db);
q.exec("SELECT dept, COUNT(*) AS cnt, AVG(salary) AS avg_salary "
       "FROM employee GROUP BY dept HAVING COUNT(*) >= 2");
```

## 6.3 联表查询

```cpp
QSqlQuery q(db);
q.prepare("SELECT e.id, e.name, d.dept_name "
          "FROM employee e "
          "LEFT JOIN department d ON e.dept_id = d.id "
          "WHERE e.age >= :age");
q.bindValue(":age", 25);
q.exec();
```

## 7. 实战案例：员工管理查询模块（可直接迁移）

目标：实现“可筛选 + 可分页 + 可排序 + 汇总统计”的查询能力。

### 7.1 表结构（SQLite 示例）

```sql
CREATE TABLE IF NOT EXISTS department (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    dept_name TEXT NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS employee (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    age INTEGER NOT NULL,
    dept_id INTEGER,
    salary REAL DEFAULT 0,
    hire_time TEXT,
    FOREIGN KEY(dept_id) REFERENCES department(id)
);

CREATE INDEX IF NOT EXISTS idx_employee_age ON employee(age);
CREATE INDEX IF NOT EXISTS idx_employee_dept_id ON employee(dept_id);
```

### 7.2 查询条件对象

```cpp
struct EmployeeQueryCond
{
    QString keyword;      // 按姓名模糊匹配
    int minAge = -1;
    int maxAge = -1;
    int deptId = -1;
    double minSalary = -1;
    int pageNo = 1;
    int pageSize = 20;
    QString orderBy = "e.id";
    QString orderDir = "DESC"; // ASC / DESC
};
```

### 7.3 动态 SQL 构建（安全版）

```cpp
#include <QStringList>
#include <QVariantMap>

struct QueryBuildResult
{
    QString sql;
    QVariantMap params;
};

static QueryBuildResult buildEmployeeListSql(const EmployeeQueryCond& c)
{
    QString base =
        "SELECT e.id, e.name, e.age, e.salary, e.hire_time, d.dept_name "
        "FROM employee e "
        "LEFT JOIN department d ON e.dept_id = d.id ";

    QStringList where;
    QVariantMap params;

    if (!c.keyword.trimmed().isEmpty()) {
        where << "e.name LIKE :keyword";
        params[":keyword"] = "%" + c.keyword.trimmed() + "%";
    }
    if (c.minAge >= 0) {
        where << "e.age >= :minAge";
        params[":minAge"] = c.minAge;
    }
    if (c.maxAge >= 0) {
        where << "e.age <= :maxAge";
        params[":maxAge"] = c.maxAge;
    }
    if (c.deptId > 0) {
        where << "e.dept_id = :deptId";
        params[":deptId"] = c.deptId;
    }
    if (c.minSalary >= 0) {
        where << "e.salary >= :minSalary";
        params[":minSalary"] = c.minSalary;
    }

    QString sql = base;
    if (!where.isEmpty()) {
        sql += " WHERE " + where.join(" AND ");
    }

    // 排序字段白名单，避免拼接注入。
    static const QSet<QString> orderWhiteList{ "e.id", "e.age", "e.salary", "e.hire_time" };
    QString orderBy = orderWhiteList.contains(c.orderBy) ? c.orderBy : "e.id";
    QString orderDir = (c.orderDir.compare("ASC", Qt::CaseInsensitive) == 0) ? "ASC" : "DESC";

    sql += QString(" ORDER BY %1 %2 LIMIT :limit OFFSET :offset").arg(orderBy, orderDir);

    int safePageNo = qMax(1, c.pageNo);
    int safePageSize = qBound(1, c.pageSize, 200);
    params[":limit"] = safePageSize;
    params[":offset"] = (safePageNo - 1) * safePageSize;

    return { sql, params };
}
```

### 7.4 执行查询并返回结果

```cpp
QVector<QVariantMap> queryEmployees(QSqlDatabase db, const EmployeeQueryCond& cond)
{
    QVector<QVariantMap> out;
    QueryBuildResult b = buildEmployeeListSql(cond);

    QSqlQuery q(db);
    q.prepare(b.sql);
    for (auto it = b.params.cbegin(); it != b.params.cend(); ++it) {
        q.bindValue(it.key(), it.value());
    }

    if (!q.exec()) {
        qWarning() << "queryEmployees failed:" << q.lastError().text() << "sql=" << b.sql;
        return out;
    }

    while (q.next()) {
        QVariantMap row;
        row["id"] = q.value("id");
        row["name"] = q.value("name");
        row["age"] = q.value("age");
        row["salary"] = q.value("salary");
        row["hire_time"] = q.value("hire_time");
        row["dept_name"] = q.value("dept_name");
        out.push_back(row);
    }
    return out;
}
```

### 7.5 汇总统计接口

```cpp
QVariantMap queryEmployeeStats(QSqlDatabase db)
{
    QVariantMap m;
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) AS total, AVG(salary) AS avg_salary, MAX(salary) AS max_salary FROM employee");
    if (!q.exec() || !q.next()) {
        qWarning() << "queryEmployeeStats failed:" << q.lastError().text();
        return m;
    }
    m["total"] = q.value("total");
    m["avg_salary"] = q.value("avg_salary");
    m["max_salary"] = q.value("max_salary");
    return m;
}
```

这个案例可直接落地到 Qt Widget / QML 后端接口。

## 8. 与界面模型联动

## 8.1 `QSqlQueryModel`（只读高效）

```cpp
QSqlQueryModel* model = new QSqlQueryModel(this);
model->setQuery("SELECT id, name, age FROM employee", db);
ui->tableView->setModel(model);
```

适合报表、浏览页；不适合直接编辑。

## 8.2 `QSqlTableModel`（单表可编辑）

```cpp
QSqlTableModel* model = new QSqlTableModel(this, db);
model->setTable("employee");
model->setEditStrategy(QSqlTableModel::OnManualSubmit);
model->select();

// 编辑后统一提交
if (!model->submitAll()) {
    qWarning() << model->lastError().text();
    model->revertAll();
}
```

## 9. 性能优化（实战非常关键）

1. 必建索引：过滤列、排序列、关联列。
2. 避免 `SELECT *`：只取需要字段。
3. 分页必须稳定排序：`ORDER BY` + 索引。
4. 批量写入用事务 + `execBatch`。
5. 大结果集懒加载：分页或游标策略。
6. 用 `EXPLAIN` 分析 SQL 执行计划（不同数据库语法有差异）。

### 9.1 简单性能对比思路

1. 场景 A：逐条 `INSERT` + 无事务
2. 场景 B：事务包裹 + `execBatch`

通常 B 会快很多，量级越大差异越明显。

## 10. 常见坑与排查清单

1. 驱动不存在：`QSqlDatabase::drivers()` 查看可用驱动。
2. 连接未打开就查询：每次执行前确认 `db.isOpen()`。
3. 线程误用连接：跨线程复用同一连接会触发随机问题。
4. 事务没提交：写入看似成功但实际回滚。
5. 字段类型不匹配：`QVariant` 转换后精度或格式异常。
6. SQL 里拼接字符串：注入风险 + 转义问题。

调试建议：

1. 打印 `lastError().text()` 和 `nativeErrorCode()`。
2. 打印最终 SQL 模板与绑定参数（不要在生产环境泄露隐私）。
3. 在数据库客户端复现 SQL，排除 Qt 层问题。

## 11. 可复用的工程化建议

1. 建立 `DatabaseManager` 统一管理连接生命周期。
2. 建立 `Repository/DAO` 层集中写 SQL，UI 不直接拼语句。
3. 为每个查询定义明确输入结构体与输出 DTO。
4. 为关键查询写单元测试（空数据、边界值、异常参数）。
5. 给核心 SQL 建性能基线（1k/10k/100k 数据量）。

## 12. 学习路径建议

1. 第 1 阶段：SQLite + `QSqlQuery` 完成 CRUD。
2. 第 2 阶段：事务、分页、联表、统计。
3. 第 3 阶段：模型联动（`QSqlQueryModel` / `QSqlTableModel`）。
4. 第 4 阶段：性能优化与线程化访问。
5. 第 5 阶段：抽象 DAO 层与自动化测试。

---

这份文档可作为 Qt 数据库查询的开发基线。后续你可以继续加两个方向：

1. 面向 MySQL/PostgreSQL 的方言差异专题（占位符、分页、返回主键）
2. 面向业务的复杂报表专题（多维统计、时间窗口、物化视图）
