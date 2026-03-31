# JSON 解析与生成

Qt 在 `QtCore` 模块中内置了完整的 JSON 支持，无需额外依赖。核心类：`QJsonDocument`（文档容器）、`QJsonObject`（对象/字典）、`QJsonArray`（数组）、`QJsonValue`（值）、`QJsonParseError`（错误信息）。

---

## 1. 核心类体系

```
QJsonDocument          JSON 文档容器（入口）
├── QJsonObject        { "key": value, ... }  有序键值对
├── QJsonArray         [ value, value, ... ]   有序值列表
└── QJsonValue         单个值（6 种类型之一）
    ├── Null
    ├── Bool
    ├── Double
    ├── String
    ├── Array   → QJsonArray
    └── Object  → QJsonObject

QJsonParseError        解析错误信息
```

### 类型映射

| JSON 类型 | Qt 类型 | QJsonValue::Type 枚举 |
|---|---|---|
| `null` | 空 | `QJsonValue::Null` |
| `true` / `false` | `bool` | `QJsonValue::Bool` |
| `number`（整数/浮点） | `double` | `QJsonValue::Double` |
| `"string"` | `QString` | `QJsonValue::String` |
| `[...]` | `QJsonArray` | `QJsonValue::Array` |
| `{...}` | `QJsonObject` | `QJsonValue::Object` |
| （不存在的键） | — | `QJsonValue::Undefined` |

> **注意**：JSON 数字在 Qt 中统一用 `double` 存储。整数精度上限为 2^53（约 9×10¹⁵），超过此范围需用字符串传输。

---

## 2. 解析 JSON（字符串/文件 → Qt 对象）

### 2.1 从字符串解析

```cpp
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

QString jsonStr = R"({
    "name": "Qt Framework",
    "version": 6.7,
    "stable": true,
    "modules": ["Core", "Gui", "Widgets", "Network"],
    "license": {
        "type": "LGPL",
        "version": "3.0"
    },
    "deprecated": null
})";

QJsonParseError error;
QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

if (error.error != QJsonParseError::NoError) {
    qWarning() << "JSON parse error:" << error.errorString()
               << "at offset" << error.offset;
    return;
}

// 获取根对象
QJsonObject root = doc.object();
```

### 2.2 从文件解析

```cpp
QJsonDocument loadJson(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filePath;
        return QJsonDocument();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Parse error:" << error.errorString()
                   << "at offset" << error.offset;
        return QJsonDocument();
    }

    return doc;
}

// 使用
QJsonDocument doc = loadJson("config.json");
if (!doc.isNull()) {
    QJsonObject root = doc.object();
    // ...
}
```

### 2.3 从 QByteArray 解析

```cpp
// 常见于网络响应
QNetworkReply *reply = ...;
QByteArray responseData = reply->readAll();

QJsonDocument doc = QJsonDocument::fromJson(responseData);
if (doc.isObject()) {
    QJsonObject obj = doc.object();
} else if (doc.isArray()) {
    QJsonArray arr = doc.array();
}
```

---

## 3. 读取 JSON 值

### 3.1 QJsonObject：读取键值对

```cpp
QJsonObject root = doc.object();

// 基本类型读取
QString name = root["name"].toString();           // "Qt Framework"
double version = root["version"].toDouble();      // 6.7
bool stable = root["stable"].toBool();            // true

// 带默认值（键不存在时返回默认值）
QString desc = root["description"].toString("N/A");  // "N/A"（键不存在）
int count = root["count"].toInt(0);                  // 0（键不存在）

// null 值判断
bool isNull = root["deprecated"].isNull();   // true
bool isUndef = root["nonexist"].isUndefined(); // true（键不存在）

// 嵌套对象
QJsonObject license = root["license"].toObject();
QString licenseType = license["type"].toString();     // "LGPL"
QString licenseVer  = license["version"].toString();  // "3.0"

// 数组
QJsonArray modules = root["modules"].toArray();
```

### 3.2 QJsonArray：读取数组

```cpp
QJsonArray modules = root["modules"].toArray();

// 按下标访问
QString first = modules[0].toString();    // "Core"
QString last  = modules.last().toString(); // "Network"
int size = modules.size();                 // 4

// 遍历（范围 for）
for (const QJsonValue &val : modules) {
    qDebug() << val.toString();
}

// 下标遍历
for (int i = 0; i < modules.size(); i++) {
    qDebug() << i << modules[i].toString();
}

// 是否包含某个值
bool hasWidgets = false;
for (const auto &v : modules) {
    if (v.toString() == "Widgets") {
        hasWidgets = true;
        break;
    }
}
```

### 3.3 QJsonValue：类型检查与安全取值

```cpp
QJsonValue val = root["version"];

// 类型检查
val.isDouble();    // true
val.isString();    // false
val.isObject();    // false
val.isArray();     // false
val.isBool();      // false
val.isNull();      // false
val.isUndefined(); // false

// 类型枚举
QJsonValue::Type type = val.type();  // QJsonValue::Double

// 安全取值（类型不匹配时返回默认值，不会崩溃）
val.toString();    // ""（不是 string，返回默认空串）
val.toString("fallback");  // "fallback"
val.toDouble();    // 6.7（正确类型）
val.toInt();       // 6（double → int 截断）
val.toBool();      // false（不是 bool，返回默认 false）
```

### 3.4 深层嵌套访问

```cpp
// 示例 JSON
// { "data": { "users": [{ "name": "Alice", "age": 30 }] } }

QJsonObject root = doc.object();

// 逐层取值
QString name = root["data"].toObject()
                   ["users"].toArray()
                   [0].toObject()
                   ["name"].toString();
// "Alice"

// 任一层级不存在都不会崩溃，返回默认值（Undefined → 继续取返回 Undefined）
QString missing = root["data"].toObject()
                      ["nonexist"].toObject()
                      ["key"].toString("default");
// "default"
```

### 3.5 遍历 QJsonObject 的所有键值

```cpp
QJsonObject obj = doc.object();

// 方式 1：keys() + 范围 for
for (const QString &key : obj.keys()) {
    QJsonValue val = obj[key];
    qDebug() << key << ":" << val;
}

// 方式 2：迭代器
for (auto it = obj.begin(); it != obj.end(); ++it) {
    qDebug() << it.key() << "=" << it.value();
}

// 方式 3：Qt 6 结构化绑定（C++17）
for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
    const QString &key = it.key();
    const QJsonValue &val = it.value();
    // ...
}
```

---

## 4. 生成 JSON（Qt 对象 → 字符串/文件）

### 4.1 构建 QJsonObject

```cpp
// 方式 1：逐个插入
QJsonObject obj;
obj["name"]    = "Qt Framework";
obj["version"] = 6.7;
obj["stable"]  = true;

// 方式 2：insert()
obj.insert("description", "Cross-platform toolkit");

// 方式 3：初始化列表（Qt 5.12+）
QJsonObject obj2{
    {"name",    "Qt Framework"},
    {"version", 6.7},
    {"stable",  true}
};

// null 值
obj["deprecated"] = QJsonValue::Null;
// 或 QJsonValue(QJsonValue::Null)
```

### 4.2 构建 QJsonArray

```cpp
// 方式 1：逐个追加
QJsonArray arr;
arr.append("Core");
arr.append("Gui");
arr.append("Widgets");

// 方式 2：初始化列表
QJsonArray arr2{"Core", "Gui", "Widgets", "Network"};

// 方式 3：从 QStringList 转换
QStringList modules = {"Core", "Gui", "Widgets"};
QJsonArray arr3 = QJsonArray::fromStringList(modules);

// 混合类型
QJsonArray mixed;
mixed.append(42);
mixed.append("hello");
mixed.append(true);
mixed.append(QJsonValue::Null);
mixed.append(QJsonObject{{"key", "value"}});
```

### 4.3 嵌套构建

```cpp
QJsonObject buildBookJson() {
    // 内层对象
    QJsonObject author;
    author["name"] = "Stanley Lippman";
    author["country"] = "USA";

    // 内层数组
    QJsonArray tags{"programming", "C++", "reference"};

    // 外层对象
    QJsonObject book;
    book["id"]     = 1;
    book["title"]  = "C++ Primer";
    book["price"]  = 59.99;
    book["author"] = author;    // 嵌套对象
    book["tags"]   = tags;      // 嵌套数组
    book["inStock"] = true;

    // 根对象包裹
    QJsonObject root;
    root["book"] = book;
    root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    return root;
}
```

### 4.4 序列化为字符串

```cpp
QJsonObject root = buildBookJson();
QJsonDocument doc(root);

// 美化格式（带缩进换行，可读性好）
QByteArray prettyJson = doc.toJson(QJsonDocument::Indented);
qDebug().noquote() << prettyJson;
// {
//     "book": {
//         "id": 1,
//         "title": "C++ Primer",
//         ...
//     },
//     "timestamp": "2024-03-15T14:30:00"
// }

// 紧凑格式（无空格换行，体积小，适合网络传输）
QByteArray compactJson = doc.toJson(QJsonDocument::Compact);
// {"book":{"id":1,"title":"C++ Primer",...},"timestamp":"2024-03-15T14:30:00"}

// 转为 QString
QString jsonString = QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
```

### 4.5 保存到文件

```cpp
bool saveJson(const QJsonDocument &doc, const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << filePath;
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

// 使用
QJsonObject config;
config["language"] = "zh_CN";
config["theme"]    = "dark";
config["fontSize"] = 14;
saveJson(QJsonDocument(config), "config.json");
```

### 4.6 从数组创建文档

```cpp
// 根元素为数组的 JSON
QJsonArray users;
users.append(QJsonObject{{"name", "Alice"}, {"age", 30}});
users.append(QJsonObject{{"name", "Bob"},   {"age", 25}});

QJsonDocument doc(users);  // 根是数组
QByteArray json = doc.toJson();
// [
//     { "name": "Alice", "age": 30 },
//     { "name": "Bob",   "age": 25 }
// ]
```

---

## 5. 修改 JSON

### 5.1 修改对象

```cpp
QJsonDocument doc = QJsonDocument::fromJson(jsonData);
QJsonObject root = doc.object();

// 修改值
root["version"] = 7.0;
root["stable"] = false;

// 添加新键
root["author"] = "The Qt Company";

// 删除键
root.remove("deprecated");

// 检查键是否存在
if (root.contains("modules")) {
    // ...
}

// ❗ 关键：修改后需要回写到 document
doc.setObject(root);
```

### 5.2 修改数组

```cpp
QJsonArray arr = root["modules"].toArray();

// 追加
arr.append("Quick");

// 插入到指定位置
arr.insert(0, "Bootstrap");

// 修改指定下标
arr[2] = "WidgetsPlus";

// 删除
arr.removeAt(0);        // 删除第一个
arr.removeLast();       // 删除最后一个
arr.removeFirst();      // 删除第一个

// 回写到父对象
root["modules"] = arr;
doc.setObject(root);
```

### 5.3 深层修改的注意事项

`QJsonObject` 和 `QJsonArray` 是**值语义**（隐式共享），从父对象取出后是一份拷贝。修改后必须回写：

```cpp
// ❌ 错误：修改了拷贝，原对象不变
QJsonObject license = root["license"].toObject();
license["version"] = "4.0";
// root 中的 license 没有任何变化！

// ✅ 正确：修改后回写
QJsonObject license = root["license"].toObject();
license["version"] = "4.0";
root["license"] = license;   // 回写到父对象
doc.setObject(root);          // 回写到文档
```

```cpp
// 嵌套层级越深，回写链越长
QJsonObject root = doc.object();
QJsonArray users = root["data"].toObject()["users"].toArray();
QJsonObject user = users[0].toObject();
user["name"] = "Charlie";
users[0] = user;                              // 回写到数组

QJsonObject data = root["data"].toObject();
data["users"] = users;                        // 回写到 data
root["data"] = data;                          // 回写到 root
doc.setObject(root);                          // 回写到 doc
```

---

## 6. QJsonValue 与 QVariant 互转

`QJsonValue` 和 `QVariant` 可以互相转换，便于与 Qt 的其他 API 配合。

```cpp
// QJsonValue → QVariant
QJsonValue val(42.5);
QVariant var = val.toVariant();  // QVariant(double, 42.5)

QJsonObject obj{{"name", "Qt"}, {"version", 6}};
QVariant mapVar = QJsonValue(obj).toVariant();  // QVariant(QVariantMap, ...)
QVariantMap map = mapVar.toMap();
// map["name"] = QVariant("Qt")
// map["version"] = QVariant(6.0)

// QVariant → QJsonValue
QJsonValue fromVar = QJsonValue::fromVariant(QVariant(3.14));  // Double: 3.14
QJsonValue fromStr = QJsonValue::fromVariant(QVariant("hello")); // String: "hello"

// QVariantMap → QJsonObject
QVariantMap vmap{{"width", 800}, {"height", 600}};
QJsonObject fromMap = QJsonObject::fromVariantMap(vmap);

// QVariantList → QJsonArray
QVariantList vlist{1, "two", 3.0, true};
QJsonArray fromList = QJsonArray::fromVariantList(vlist);

// 反向
QVariantMap backToMap = obj.toVariantMap();
QVariantList backToList = QJsonArray{"a", "b"}.toVariantList();
```

---

## 7. 二进制 JSON（Qt 5 特有）

Qt 5 提供 `QJsonDocument::toBinaryData()` / `fromBinaryData()` 以二进制格式存储 JSON，读写速度更快。

```cpp
// Qt 5 二进制 JSON
QByteArray binary = doc.toBinaryData();   // 序列化为二进制
QJsonDocument doc2 = QJsonDocument::fromBinaryData(binary);  // 反序列化

// 与文本 JSON 对比
QByteArray textJson   = doc.toJson(QJsonDocument::Compact);
QByteArray binaryJson = doc.toBinaryData();
qDebug() << "Text size:" << textJson.size();     // 较大
qDebug() << "Binary size:" << binaryJson.size(); // 较小，且解析更快
```

> **Qt 6 中已移除**此 API。Qt 6 推荐使用 `CBOR`（`QCborValue`）作为二进制替代方案。

---

## 8. CBOR：Qt 6 二进制替代方案

`QCborValue` 是 Qt 6 推荐的二进制数据交换格式（RFC 7049），与 JSON 类型几乎一一对应，额外支持 `QByteArray`、`QDateTime` 等。

```cpp
#include <QCborValue>
#include <QCborMap>
#include <QCborArray>

// JSON ↔ CBOR 互转
QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8());
QCborValue cbor = QCborValue::fromJsonValue(jsonDoc.object());

// CBOR 序列化（二进制，紧凑高效）
QByteArray cborData = cbor.toCbor();

// 反序列化
QCborValue decoded = QCborValue::fromCbor(cborData);

// CBOR → JSON
QJsonValue jsonVal = decoded.toJsonValue();
QJsonDocument backToJson(jsonVal.toObject());

// CBOR 独有类型
QCborMap map;
map.insert("bytes", QCborValue(QByteArray("\x00\x01\x02", 3)));  // 原始字节
map.insert("date",  QCborValue(QDateTime::currentDateTime()));    // 日期时间（带 tag）
```

---

## 9. 与 C++ 结构体互转

### 9.1 手动序列化/反序列化

```cpp
struct User {
    int id;
    QString name;
    QString email;
    bool active;
    QStringList roles;

    // 转为 JSON
    QJsonObject toJson() const {
        return {
            {"id",     id},
            {"name",   name},
            {"email",  email},
            {"active", active},
            {"roles",  QJsonArray::fromStringList(roles)}
        };
    }

    // 从 JSON 构造
    static User fromJson(const QJsonObject &obj) {
        User u;
        u.id     = obj["id"].toInt();
        u.name   = obj["name"].toString();
        u.email  = obj["email"].toString();
        u.active = obj["active"].toBool(true);  // 默认 true
        u.roles  = {};
        for (const auto &v : obj["roles"].toArray()) {
            u.roles.append(v.toString());
        }
        return u;
    }
};

// 序列化
User user{1, "Alice", "alice@example.com", true, {"admin", "editor"}};
QJsonDocument doc(user.toJson());
QByteArray json = doc.toJson();

// 反序列化
QJsonDocument doc2 = QJsonDocument::fromJson(json);
User user2 = User::fromJson(doc2.object());
```

### 9.2 列表序列化

```cpp
// QList<User> → QJsonArray
QJsonArray usersToJson(const QList<User> &users) {
    QJsonArray arr;
    for (const auto &u : users) {
        arr.append(u.toJson());
    }
    return arr;
}

// QJsonArray → QList<User>
QList<User> usersFromJson(const QJsonArray &arr) {
    QList<User> users;
    users.reserve(arr.size());
    for (const auto &v : arr) {
        users.append(User::fromJson(v.toObject()));
    }
    return users;
}
```

### 9.3 嵌套结构体

```cpp
struct Address {
    QString city;
    QString street;
    int zipCode;

    QJsonObject toJson() const {
        return {{"city", city}, {"street", street}, {"zipCode", zipCode}};
    }
    static Address fromJson(const QJsonObject &obj) {
        return {obj["city"].toString(), obj["street"].toString(), obj["zipCode"].toInt()};
    }
};

struct Employee {
    int id;
    QString name;
    Address address;            // 嵌套对象
    QList<QString> skills;      // 数组

    QJsonObject toJson() const {
        return {
            {"id",      id},
            {"name",    name},
            {"address", address.toJson()},   // 嵌套序列化
            {"skills",  QJsonArray::fromStringList(skills)}
        };
    }

    static Employee fromJson(const QJsonObject &obj) {
        Employee e;
        e.id      = obj["id"].toInt();
        e.name    = obj["name"].toString();
        e.address = Address::fromJson(obj["address"].toObject());
        for (const auto &v : obj["skills"].toArray()) {
            e.skills.append(v.toString());
        }
        return e;
    }
};
```

### 9.4 可选字段与版本兼容

```cpp
struct Config {
    // 必填字段
    QString appName;
    int version;

    // 可选字段（带默认值）
    QString theme = "light";
    int fontSize = 14;
    bool autoSave = true;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["appName"]  = appName;
        obj["version"]  = version;
        obj["theme"]    = theme;
        obj["fontSize"] = fontSize;
        obj["autoSave"] = autoSave;
        return obj;
    }

    static Config fromJson(const QJsonObject &obj) {
        Config c;
        // 必填字段（无默认值，缺失则用空/0）
        c.appName = obj["appName"].toString();
        c.version = obj["version"].toInt();

        // 可选字段（键不存在时用 toXxx 的默认参数）
        c.theme    = obj["theme"].toString("light");
        c.fontSize = obj["fontSize"].toInt(14);
        c.autoSave = obj["autoSave"].toBool(true);
        return c;
    }
};

// 即使 JSON 中缺少 theme/fontSize/autoSave，也不会出错
// 旧版本保存的 JSON 可以被新版本代码安全读取
```

---

## 10. 实战案例

### 10.1 REST API 响应解析

```cpp
// GitHub API 返回的仓库信息
void parseGitHubRepo(const QByteArray &responseData) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "API response parse error:" << error.errorString();
        return;
    }

    QJsonObject repo = doc.object();

    QString name       = repo["full_name"].toString();
    QString desc       = repo["description"].toString();
    int stars          = repo["stargazers_count"].toInt();
    int forks          = repo["forks_count"].toInt();
    QString language   = repo["language"].toString("unknown");
    bool isPrivate     = repo["private"].toBool();
    QString createdAt  = repo["created_at"].toString();

    // 嵌套对象
    QJsonObject owner = repo["owner"].toObject();
    QString ownerName  = owner["login"].toString();
    QString avatarUrl  = owner["avatar_url"].toString();

    // 可能为 null 的字段
    QString homepage = repo["homepage"].isNull()
                         ? "(none)"
                         : repo["homepage"].toString();

    qDebug() << name << stars << "stars" << language;
}
```

### 10.2 分页列表解析

```cpp
// 常见的分页 API 响应格式
// { "total": 100, "page": 1, "pageSize": 20, "data": [{...}, ...] }

struct PagedResult {
    int total;
    int page;
    int pageSize;
    QList<User> data;

    static PagedResult fromJson(const QByteArray &json) {
        QJsonDocument doc = QJsonDocument::fromJson(json);
        QJsonObject root = doc.object();

        PagedResult result;
        result.total    = root["total"].toInt();
        result.page     = root["page"].toInt();
        result.pageSize = root["pageSize"].toInt();

        QJsonArray dataArr = root["data"].toArray();
        result.data.reserve(dataArr.size());
        for (const auto &v : dataArr) {
            result.data.append(User::fromJson(v.toObject()));
        }
        return result;
    }

    bool hasNextPage() const {
        return page * pageSize < total;
    }
};
```

### 10.3 应用配置读写

```cpp
class AppConfig {
public:
    bool load(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return false;

        QJsonParseError err;
        m_doc = QJsonDocument::fromJson(file.readAll(), &err);
        if (err.error != QJsonParseError::NoError) {
            qWarning() << "Config parse error:" << err.errorString();
            return false;
        }
        m_path = path;
        return true;
    }

    bool save() {
        if (m_path.isEmpty()) return false;
        QFile file(m_path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
        file.write(m_doc.toJson(QJsonDocument::Indented));
        return true;
    }

    // 读取（支持点号路径 "window.width"）
    QJsonValue get(const QString &dotPath) const {
        QStringList parts = dotPath.split('.');
        QJsonValue current = m_doc.object();
        for (const auto &part : parts) {
            if (current.isObject()) {
                current = current.toObject()[part];
            } else {
                return QJsonValue::Undefined;
            }
        }
        return current;
    }

    // 写入（仅支持顶层键）
    void set(const QString &key, const QJsonValue &value) {
        QJsonObject root = m_doc.object();
        root[key] = value;
        m_doc.setObject(root);
    }

private:
    QJsonDocument m_doc;
    QString m_path;
};

// 使用
AppConfig config;
config.load("settings.json");
int width = config.get("window.width").toInt(1280);
config.set("theme", "dark");
config.save();
```

### 10.4 JSON Merge（合并两个 JSON 对象）

```cpp
// 浅合并：src 的键值覆盖到 dest
QJsonObject mergeObjects(const QJsonObject &dest, const QJsonObject &src) {
    QJsonObject result = dest;
    for (auto it = src.constBegin(); it != src.constEnd(); ++it) {
        result[it.key()] = it.value();
    }
    return result;
}

// 深合并：递归合并嵌套对象
QJsonObject deepMerge(const QJsonObject &dest, const QJsonObject &src) {
    QJsonObject result = dest;
    for (auto it = src.constBegin(); it != src.constEnd(); ++it) {
        if (result.contains(it.key())
            && result[it.key()].isObject()
            && it.value().isObject()) {
            // 两边都是对象时递归合并
            result[it.key()] = deepMerge(
                result[it.key()].toObject(),
                it.value().toObject()
            );
        } else {
            result[it.key()] = it.value();
        }
    }
    return result;
}

// 示例
QJsonObject defaults{{"theme", "light"}, {"font", QJsonObject{{"size", 14}, {"family", "Arial"}}}};
QJsonObject userPref{{"theme", "dark"},  {"font", QJsonObject{{"size", 16}}}};
QJsonObject merged = deepMerge(defaults, userPref);
// { "theme": "dark", "font": { "size": 16, "family": "Arial" } }
```

---

## 11. 与其他格式互操作

### 11.1 JSON ↔ XML 转换思路

```cpp
// JSON 对象 → XML
void jsonToXml(QXmlStreamWriter &xml, const QString &tag, const QJsonValue &value) {
    if (value.isObject()) {
        xml.writeStartElement(tag);
        QJsonObject obj = value.toObject();
        for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
            jsonToXml(xml, it.key(), it.value());
        }
        xml.writeEndElement();
    } else if (value.isArray()) {
        for (const auto &item : value.toArray()) {
            jsonToXml(xml, tag, item);
        }
    } else if (value.isNull()) {
        xml.writeEmptyElement(tag);
    } else {
        xml.writeTextElement(tag, value.toVariant().toString());
    }
}

// 使用
QString output;
QXmlStreamWriter xml(&output);
xml.setAutoFormatting(true);
xml.writeStartDocument();
jsonToXml(xml, "root", QJsonValue(root));
xml.writeEndDocument();
```

### 11.2 JSON ↔ QSettings

```cpp
// QSettings → JSON（导出配置）
QJsonObject settingsToJson(QSettings &settings) {
    QJsonObject obj;
    for (const auto &key : settings.allKeys()) {
        QVariant val = settings.value(key);
        obj[key] = QJsonValue::fromVariant(val);
    }
    return obj;
}

// JSON → QSettings（导入配置）
void jsonToSettings(const QJsonObject &obj, QSettings &settings) {
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        settings.setValue(it.key(), it.value().toVariant());
    }
}
```

---

## 12. 性能与限制

### 12.1 性能特征

| 操作 | Qt JSON 性能 | 说明 |
|---|---|---|
| 解析 | 中等 | 比 RapidJSON 慢 2-3 倍，但对多数场景足够 |
| 序列化 | 快 | `toJson(Compact)` 很高效 |
| 随机访问 | O(n) 查找 | QJsonObject 内部是排序数组，非哈希表 |
| 内存 | 较高 | 每个值有类型标记和 QVariant 开销 |

### 12.2 大 JSON 处理

```cpp
// 对于几十 MB 的 JSON 文件：

// 1. 确保一次性读取（避免多次 IO）
QByteArray data = file.readAll();
QJsonDocument doc = QJsonDocument::fromJson(data);

// 2. 解析后尽快提取需要的数据，不要长期持有整个 QJsonDocument

// 3. 超大 JSON（100MB+）考虑第三方流式库（如 RapidJSON SAX 接口）
//    Qt JSON 没有流式解析 API，必须一次性加载全部内容
```

### 12.3 整数精度限制

```cpp
// JSON 数字在 Qt 中用 double 存储
// double 可精确表示的最大整数：2^53 = 9007199254740992

QJsonObject obj;
obj["small"]   = 12345;                    // OK
obj["big"]     = 9007199254740992LL;       // OK（2^53 恰好精确）
obj["tooBig"]  = 9007199254740993LL;       // ⚠ 精度丢失！

// 解决方案：大整数用字符串
obj["bigId"] = QString::number(9007199254740993LL);

// 取回时
qlonglong id = obj["bigId"].toString().toLongLong();
```

---

## 13. QJsonParseError 详解

| 错误码 | 含义 |
|---|---|
| `NoError` | 解析成功 |
| `UnterminatedObject` | `{` 未闭合 |
| `UnterminatedArray` | `[` 未闭合 |
| `UnterminatedString` | `"` 未闭合 |
| `MissingNameSeparator` | `:` 缺失 |
| `MissingValueSeparator` | `,` 缺失 |
| `IllegalValue` | 非法值 |
| `IllegalNumber` | 非法数字 |
| `IllegalEscapeSequence` | 非法转义序列 |
| `IllegalUTF8String` | 非法 UTF-8 |
| `DeepNesting` | 嵌套过深（默认限制 128 层） |
| `DocumentTooLarge` | 文档过大 |
| `GarbageAtEnd` | 有效 JSON 之后有多余内容 |

```cpp
QJsonParseError err;
QJsonDocument doc = QJsonDocument::fromJson(data, &err);
if (err.error != QJsonParseError::NoError) {
    // 定位错误位置
    int offset = err.offset;
    QString context = QString::fromUtf8(data.mid(
        qMax(0, offset - 20), 40));  // 错误附近 40 字符
    qWarning() << "Error:" << err.errorString()
               << "\nAt offset:" << offset
               << "\nContext: ..." << context << "...";
}
```

---

## 14. 常见问题

**问题 1：修改嵌套值不生效**

```cpp
// ❌ 值语义陷阱
QJsonObject root = doc.object();
root["child"].toObject()["key"] = "value";  // 修改的是临时拷贝！

// ✅ 逐层取出、修改、回写
QJsonObject root = doc.object();
QJsonObject child = root["child"].toObject();
child["key"] = "value";
root["child"] = child;
doc.setObject(root);
```

**问题 2：数组/对象判断错误**

```cpp
// JSON 根可以是对象或数组
QJsonDocument doc = QJsonDocument::fromJson(data);

// ❌ 直接 .object() 可能为空
QJsonObject root = doc.object();  // 如果根是数组，返回空对象

// ✅ 先判断类型
if (doc.isObject()) {
    QJsonObject root = doc.object();
} else if (doc.isArray()) {
    QJsonArray arr = doc.array();
}
```

**问题 3：UTF-8 编码问题**

```cpp
// ❌ 直接传 QString 给 fromJson（参数不匹配）
QJsonDocument doc = QJsonDocument::fromJson(stringValue);  // 编译错误

// ✅ 必须传 QByteArray
QJsonDocument doc = QJsonDocument::fromJson(stringValue.toUtf8());
```

**问题 4：空文档判断**

```cpp
QJsonDocument doc = QJsonDocument::fromJson("null");
doc.isNull();    // true（JSON null）
doc.isEmpty();   // true

QJsonDocument doc2;
doc2.isNull();   // true（默认构造）

// fromJson 失败也返回 null document
QJsonDocument doc3 = QJsonDocument::fromJson("invalid json");
doc3.isNull();   // true

// 区分方式：检查 QJsonParseError
QJsonParseError err;
QJsonDocument doc4 = QJsonDocument::fromJson("invalid", &err);
if (err.error != QJsonParseError::NoError) {
    // 解析失败
} else if (doc4.isNull()) {
    // JSON 内容就是 null
}
```

**问题 5：键的顺序**

```cpp
// QJsonObject 内部按键名字母序排序（Qt 5），不保留插入顺序
QJsonObject obj;
obj["z"] = 1;
obj["a"] = 2;
obj["m"] = 3;

// 输出：{"a":2,"m":3,"z":1}（字母序）
// 如果需要保持顺序，可用 QJsonArray 存储有序条目
// 或用第三方库（如 nlohmann/json 的 ordered_json）
```

---

## 15. 最佳实践

| 实践 | 说明 |
|---|---|
| 始终检查 `QJsonParseError` | 外部 JSON 不可信，解析可能失败 |
| `toXxx()` 带默认值 | `toInt(0)`、`toString("default")` 避免键缺失时的意外 |
| 值语义回写 | 修改嵌套结构后逐层回写到父对象和文档 |
| 大整数用字符串 | 超过 2^53 的整数用 `QString` 传输 |
| Compact 用于传输 | 网络传输用 `QJsonDocument::Compact` 减小体积 |
| Indented 用于存储 | 配置文件用 `QJsonDocument::Indented` 便于阅读和 diff |
| 结构体封装 toJson/fromJson | 集中序列化逻辑，避免散落各处 |
| 可选字段给默认值 | `fromJson` 中对可选字段用带默认值的 `toXxx()` 保证向后兼容 |
| 文件写入先到临时文件 | 写临时文件再 rename，防止写入中断导致 JSON 文件损坏 |
| Qt 6 考虑 CBOR | 二进制序列化场景用 `QCborValue` 替代已废弃的二进制 JSON |
