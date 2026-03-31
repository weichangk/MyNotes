# XML 解析与生成

Qt 提供两套 XML 处理方案：**流式 API**（`QXmlStreamReader` / `QXmlStreamWriter`，推荐）和 **DOM API**（`QDomDocument`，树形操作）。流式 API 在 `QtCore` 中，零额外依赖；DOM API 需要 `QtXml` 模块。

---

## 1. 两套 API 对比

| 特性 | QXmlStream（流式） | QDom（DOM 树） |
|---|---|---|
| 所属模块 | `QtCore`（无需额外链接） | `QtXml`（需 `QT += xml`） |
| 内存占用 | 极低，逐节点处理 | 高，整棵树加载到内存 |
| 适用场景 | 大文件、流式读取、一次遍历 | 小文件、需要随机访问/反复修改 |
| 编程模型 | 拉取式（pull parser） | 树形遍历 |
| 读写 | 读（Reader）+ 写（Writer）各自独立 | 读写用同一棵树操作 |
| 性能 | 快 | 慢（需构建完整树） |
| XPath | 不支持 | 不直接支持（Qt 有 `QXmlQuery`，已废弃） |

**推荐**：优先使用 `QXmlStreamReader` / `QXmlStreamWriter`，只有在需要反复修改 XML 结构时才用 DOM。

---

## 2. 示例 XML 文件

后续示例均基于此文件：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<library name="City Library" version="2.0">
    <!-- Book catalog -->
    <books>
        <book id="1" lang="en">
            <title>C++ Primer</title>
            <author>Stanley Lippman</author>
            <price currency="USD">59.99</price>
            <tags>
                <tag>programming</tag>
                <tag>C++</tag>
            </tags>
        </book>
        <book id="2" lang="zh">
            <title>Qt 实战</title>
            <author>张三</author>
            <price currency="CNY">89.00</price>
            <tags>
                <tag>Qt</tag>
                <tag>GUI</tag>
            </tags>
        </book>
    </books>
    <metadata>
        <updated>2024-03-15</updated>
        <count>2</count>
    </metadata>
</library>
```

---

## 3. QXmlStreamReader：流式读取

### 3.1 基本原理

`QXmlStreamReader` 是**拉取式解析器**（pull parser）：每次调用 `readNext()` 前进一步，检查当前 token 类型，提取数据。

```
readNext() → StartDocument
readNext() → StartElement "library"
readNext() → StartElement "books"
readNext() → StartElement "book"
readNext() → StartElement "title"
readNext() → Characters "C++ Primer"
readNext() → EndElement "title"
...循环直到 EndDocument
```

### 3.2 Token 类型

| TokenType | 含义 | 对应节点 |
|---|---|---|
| `StartDocument` | XML 声明 | `<?xml ...?>` |
| `EndDocument` | 文档结束 | EOF |
| `StartElement` | 开始标签 | `<book id="1">` |
| `EndElement` | 结束标签 | `</book>` |
| `Characters` | 文本内容 | `C++ Primer` |
| `Comment` | 注释 | `<!-- ... -->` |
| `DTD` | 文档类型声明 | `<!DOCTYPE ...>` |
| `ProcessingInstruction` | 处理指令 | `<?target data?>` |
| `EntityReference` | 实体引用 | `&amp;` |
| `Invalid` | 错误 | 解析失败 |

### 3.3 从文件读取

```cpp
#include <QXmlStreamReader>
#include <QFile>

struct Book {
    int id;
    QString lang;
    QString title;
    QString author;
    double price;
    QString currency;
    QStringList tags;
};

QList<Book> parseBooks(const QString &filePath) {
    QList<Book> books;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filePath;
        return books;
    }

    QXmlStreamReader xml(&file);
    Book currentBook;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString();

            if (name == "book") {
                currentBook = Book{};
                // 读取属性
                QXmlStreamAttributes attrs = xml.attributes();
                currentBook.id   = attrs.value("id").toInt();
                currentBook.lang = attrs.value("lang").toString();
            }
            else if (name == "title") {
                currentBook.title = xml.readElementText();
                // readElementText() 读取到对应的 EndElement，返回文本
            }
            else if (name == "author") {
                currentBook.author = xml.readElementText();
            }
            else if (name == "price") {
                currentBook.currency = xml.attributes().value("currency").toString();
                currentBook.price = xml.readElementText().toDouble();
            }
            else if (name == "tag") {
                currentBook.tags.append(xml.readElementText());
            }
        }
        else if (token == QXmlStreamReader::EndElement) {
            if (xml.name().toString() == "book") {
                books.append(currentBook);
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "XML error:" << xml.errorString()
                   << "at line" << xml.lineNumber()
                   << "column" << xml.columnNumber();
    }

    return books;
}

// 使用
auto books = parseBooks("library.xml");
for (const auto &book : books) {
    qDebug() << book.id << book.title << book.author
             << book.price << book.currency << book.tags;
}
```

### 3.4 从字符串读取

```cpp
QString xmlStr = R"(
    <config>
        <width>800</width>
        <height>600</height>
        <fullscreen>true</fullscreen>
    </config>
)";

QXmlStreamReader xml(xmlStr);
QMap<QString, QString> config;

while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement() && xml.name() != "config") {
        QString key = xml.name().toString();
        QString value = xml.readElementText();
        config[key] = value;
    }
}
// config: {"width":"800", "height":"600", "fullscreen":"true"}
```

### 3.5 readElementText() 详解

`readElementText()` 有一个策略参数：

```cpp
// 默认：遇到子元素报错
QString text = xml.readElementText();
// 等价于
QString text = xml.readElementText(QXmlStreamReader::ErrorOnUnexpectedElement);

// 跳过子元素，只取直接文本
QString text = xml.readElementText(QXmlStreamReader::SkipChildElements);

// 包含子元素内的文本（递归拼接）
QString text = xml.readElementText(QXmlStreamReader::IncludeChildElements);
```

示例：

```xml
<desc>This is <b>bold</b> text</desc>
```

```cpp
// ErrorOnUnexpectedElement → 报错（遇到 <b> 子元素）
// SkipChildElements        → "This is  text"（跳过 <b>bold</b>）
// IncludeChildElements     → "This is bold text"（包含子元素文本）
```

### 3.6 命名空间处理

```xml
<root xmlns:dc="http://purl.org/dc/elements/"
      xmlns:atom="http://www.w3.org/2005/Atom">
    <dc:title>My Feed</dc:title>
    <atom:link href="http://example.com"/>
</root>
```

```cpp
QXmlStreamReader xml(data);
xml.setNamespaceProcessing(true);  // 默认开启

while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
        qDebug() << "Local name:" << xml.name();           // "title"
        qDebug() << "Namespace:"  << xml.namespaceUri();   // "http://purl.org/dc/elements/"
        qDebug() << "Prefix:"     << xml.prefix();         // "dc"
        qDebug() << "Qualified:"  << xml.qualifiedName();  // "dc:title"
    }
}
```

---

## 4. QXmlStreamWriter：流式写入

### 4.1 写入到文件

```cpp
#include <QXmlStreamWriter>
#include <QFile>

void writeLibraryXml(const QString &filePath, const QList<Book> &books) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing";
        return;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);          // 自动缩进（美化输出）
    xml.setAutoFormattingIndent(4);       // 缩进 4 空格（默认 1 个 Tab）

    // XML 声明
    xml.writeStartDocument("1.0");       // <?xml version="1.0" encoding="UTF-8"?>

    // 根元素 + 属性
    xml.writeStartElement("library");
    xml.writeAttribute("name", "City Library");
    xml.writeAttribute("version", "2.0");

    // 注释
    xml.writeComment(" Book catalog ");

    // books 容器
    xml.writeStartElement("books");

    for (const auto &book : books) {
        xml.writeStartElement("book");
        xml.writeAttribute("id", QString::number(book.id));
        xml.writeAttribute("lang", book.lang);

        // 简单文本元素
        xml.writeTextElement("title", book.title);
        xml.writeTextElement("author", book.author);

        // 带属性的文本元素
        xml.writeStartElement("price");
        xml.writeAttribute("currency", book.currency);
        xml.writeCharacters(QString::number(book.price, 'f', 2));
        xml.writeEndElement();  // </price>

        // tags
        xml.writeStartElement("tags");
        for (const auto &tag : book.tags) {
            xml.writeTextElement("tag", tag);
        }
        xml.writeEndElement();  // </tags>

        xml.writeEndElement();  // </book>
    }

    xml.writeEndElement();  // </books>

    // metadata
    xml.writeStartElement("metadata");
    xml.writeTextElement("updated", QDate::currentDate().toString(Qt::ISODate));
    xml.writeTextElement("count", QString::number(books.size()));
    xml.writeEndElement();  // </metadata>

    xml.writeEndElement();  // </library>
    xml.writeEndDocument(); // 结束

    file.close();
}
```

### 4.2 写入到字符串

```cpp
QString xmlToString() {
    QString output;
    QXmlStreamWriter xml(&output);
    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("config");
    xml.writeTextElement("width", "800");
    xml.writeTextElement("height", "600");
    xml.writeEmptyElement("feature");     // <feature/>（自闭合标签）
    xml.writeAttribute("name", "opengl");
    xml.writeAttribute("enabled", "true");
    xml.writeEndElement();  // </config>
    xml.writeEndDocument();

    return output;
}
// 输出：
// <?xml version="1.0"?>
// <config>
//     <width>800</width>
//     <height>600</height>
//     <feature name="opengl" enabled="true"/>
// </config>
```

### 4.3 Writer 常用方法速查

| 方法 | 功能 | 示例输出 |
|---|---|---|
| `writeStartDocument("1.0")` | XML 声明 | `<?xml version="1.0" encoding="UTF-8"?>` |
| `writeEndDocument()` | 关闭所有未关闭标签 | 自动写入所有 `</...>` |
| `writeStartElement("tag")` | 开始标签 | `<tag>` |
| `writeEndElement()` | 结束标签 | `</tag>` |
| `writeAttribute("k", "v")` | 属性（紧跟 StartElement 后） | `k="v"` |
| `writeTextElement("tag", "text")` | 完整文本元素 | `<tag>text</tag>` |
| `writeCharacters("text")` | 文本内容 | `text`（自动转义 `<>&`） |
| `writeEmptyElement("tag")` | 空元素 | `<tag/>` |
| `writeComment("text")` | 注释 | `<!-- text -->` |
| `writeCDATA("text")` | CDATA 段 | `<![CDATA[text]]>` |
| `writeProcessingInstruction("t","d")` | 处理指令 | `<?t d?>` |
| `writeNamespace("uri", "prefix")` | 命名空间声明 | `xmlns:prefix="uri"` |

### 4.4 CDATA 段

当文本包含大量 `<`、`>`、`&` 时，用 CDATA 避免逐字符转义：

```cpp
xml.writeStartElement("script");
xml.writeCDATA(R"(
    function check(a, b) {
        return a < b && b > 0;
    }
)");
xml.writeEndElement();
```

输出：

```xml
<script><![CDATA[
    function check(a, b) {
        return a < b && b > 0;
    }
]]></script>
```

---

## 5. QDomDocument：DOM 树操作

### 5.1 环境配置

```cmake
# CMake
find_package(Qt6 REQUIRED COMPONENTS Xml)
target_link_libraries(myapp PRIVATE Qt6::Xml)
```

```pro
# qmake
QT += xml
```

### 5.2 DOM 核心类体系

```
QDomNode（所有节点基类）
├── QDomDocument        文档根
├── QDomElement         元素节点 <tag>
├── QDomText            文本节点
├── QDomComment         注释节点
├── QDomCDATASection    CDATA 节点
├── QDomAttr            属性节点
├── QDomProcessingInstruction  处理指令
└── QDomDocumentType    DOCTYPE
```

所有 DOM 节点都是**隐式共享**的值类型，拷贝代价低。

### 5.3 从文件加载

```cpp
#include <QDomDocument>
#include <QFile>

QDomDocument loadXml(const QString &filePath) {
    QDomDocument doc;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file";
        return doc;
    }

    QString errorMsg;
    int errorLine, errorCol;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorCol)) {
        qWarning() << "XML parse error:" << errorMsg
                   << "at line" << errorLine << "col" << errorCol;
        file.close();
        return doc;
    }

    file.close();
    return doc;
}
```

### 5.4 遍历 DOM 树

```cpp
void parseWithDom(const QDomDocument &doc) {
    // 根元素
    QDomElement root = doc.documentElement();
    qDebug() << "Root:" << root.tagName();                // "library"
    qDebug() << "Name:" << root.attribute("name");        // "City Library"
    qDebug() << "Version:" << root.attribute("version");  // "2.0"

    // 查找 <books> 元素
    QDomElement booksElem = root.firstChildElement("books");

    // 遍历所有 <book> 子元素
    QDomElement bookElem = booksElem.firstChildElement("book");
    while (!bookElem.isNull()) {
        qDebug() << "--- Book ---";
        qDebug() << "ID:" << bookElem.attribute("id");
        qDebug() << "Lang:" << bookElem.attribute("lang");

        // 子元素文本
        qDebug() << "Title:" << bookElem.firstChildElement("title").text();
        qDebug() << "Author:" << bookElem.firstChildElement("author").text();

        // 带属性的子元素
        QDomElement priceElem = bookElem.firstChildElement("price");
        qDebug() << "Price:" << priceElem.text()
                 << priceElem.attribute("currency");

        // 遍历 tags
        QDomElement tagsElem = bookElem.firstChildElement("tags");
        QDomElement tagElem = tagsElem.firstChildElement("tag");
        QStringList tags;
        while (!tagElem.isNull()) {
            tags.append(tagElem.text());
            tagElem = tagElem.nextSiblingElement("tag");
        }
        qDebug() << "Tags:" << tags;

        // 下一个兄弟 <book>
        bookElem = bookElem.nextSiblingElement("book");
    }
}
```

### 5.5 DOM 遍历方法速查

```cpp
QDomElement elem = ...;

// 导航
elem.parentNode();                    // 父节点
elem.firstChild();                    // 第一个子节点（任意类型）
elem.lastChild();                     // 最后一个子节点
elem.firstChildElement("name");       // 第一个名为 "name" 的子元素
elem.nextSibling();                   // 下一个兄弟（任意类型）
elem.nextSiblingElement("name");      // 下一个名为 "name" 的兄弟元素
elem.previousSibling();               // 上一个兄弟
elem.childNodes();                    // 所有子节点列表（QDomNodeList）

// 属性
elem.attribute("key");                // 获取属性值
elem.attribute("key", "default");     // 带默认值
elem.hasAttribute("key");             // 是否存在属性
elem.setAttribute("key", "value");    // 设置属性
elem.removeAttribute("key");          // 删除属性
elem.attributes();                    // 所有属性（QDomNamedNodeMap）

// 内容
elem.text();                          // 所有子文本（递归拼接）
elem.tagName();                       // 标签名
elem.isNull();                        // 是否为空节点

// 类型判断
node.isElement();
node.isText();
node.isComment();
node.isCDATASection();
node.toElement();                     // 转换为 QDomElement
node.toText();                        // 转换为 QDomText
```

### 5.6 通用遍历函数

```cpp
// 递归遍历整棵树
void traverseNode(const QDomNode &node, int depth = 0) {
    QString indent(depth * 2, ' ');

    if (node.isElement()) {
        QDomElement elem = node.toElement();
        qDebug().noquote() << indent << "<" + elem.tagName() + ">";

        // 遍历属性
        QDomNamedNodeMap attrs = elem.attributes();
        for (int i = 0; i < attrs.count(); i++) {
            QDomAttr attr = attrs.item(i).toAttr();
            qDebug().noquote() << indent << "  @" << attr.name() << "=" << attr.value();
        }
    }
    else if (node.isText()) {
        QString text = node.toText().data().trimmed();
        if (!text.isEmpty()) {
            qDebug().noquote() << indent << "TEXT:" << text;
        }
    }
    else if (node.isComment()) {
        qDebug().noquote() << indent << "COMMENT:" << node.toComment().data();
    }

    // 递归子节点
    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        traverseNode(child, depth + 1);
        child = child.nextSibling();
    }
}

// 使用
traverseNode(doc.documentElement());
```

---

## 6. QDomDocument：创建与修改 XML

### 6.1 从零创建

```cpp
QDomDocument createLibraryXml(const QList<Book> &books) {
    QDomDocument doc;

    // XML 声明
    QDomProcessingInstruction pi = doc.createProcessingInstruction(
        "xml", R"(version="1.0" encoding="UTF-8")");
    doc.appendChild(pi);

    // 根元素
    QDomElement root = doc.createElement("library");
    root.setAttribute("name", "City Library");
    root.setAttribute("version", "2.0");
    doc.appendChild(root);

    // 注释
    QDomComment comment = doc.createComment(" Book catalog ");
    root.appendChild(comment);

    // books 容器
    QDomElement booksElem = doc.createElement("books");
    root.appendChild(booksElem);

    for (const auto &book : books) {
        QDomElement bookElem = doc.createElement("book");
        bookElem.setAttribute("id", book.id);
        bookElem.setAttribute("lang", book.lang);

        // 辅助函数：创建文本元素
        auto addTextElement = [&](const QString &tag, const QString &text) {
            QDomElement elem = doc.createElement(tag);
            elem.appendChild(doc.createTextNode(text));
            return elem;
        };

        bookElem.appendChild(addTextElement("title", book.title));
        bookElem.appendChild(addTextElement("author", book.author));

        // 带属性的元素
        QDomElement priceElem = doc.createElement("price");
        priceElem.setAttribute("currency", book.currency);
        priceElem.appendChild(doc.createTextNode(
            QString::number(book.price, 'f', 2)));
        bookElem.appendChild(priceElem);

        // tags
        QDomElement tagsElem = doc.createElement("tags");
        for (const auto &tag : book.tags) {
            tagsElem.appendChild(addTextElement("tag", tag));
        }
        bookElem.appendChild(tagsElem);

        booksElem.appendChild(bookElem);
    }

    // metadata
    QDomElement metaElem = doc.createElement("metadata");
    auto addTextElement = [&](const QString &tag, const QString &text) {
        QDomElement elem = doc.createElement(tag);
        elem.appendChild(doc.createTextNode(text));
        return elem;
    };
    metaElem.appendChild(addTextElement("updated",
                         QDate::currentDate().toString(Qt::ISODate)));
    metaElem.appendChild(addTextElement("count",
                         QString::number(books.size())));
    root.appendChild(metaElem);

    return doc;
}

// 保存到文件
void saveXml(const QDomDocument &doc, const QString &filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        doc.save(stream, 4);  // 参数 2 = 缩进空格数
    }
}
```

### 6.2 修改现有 XML

```cpp
void modifyXml(QDomDocument &doc) {
    QDomElement root = doc.documentElement();

    // 1. 修改根元素属性
    root.setAttribute("version", "3.0");

    // 2. 查找并修改特定元素
    QDomElement booksElem = root.firstChildElement("books");
    QDomElement bookElem = booksElem.firstChildElement("book");
    while (!bookElem.isNull()) {
        if (bookElem.attribute("id") == "1") {
            // 修改子元素文本
            QDomElement priceElem = bookElem.firstChildElement("price");
            // 先删除旧文本节点
            QDomNode oldText = priceElem.firstChild();
            priceElem.removeChild(oldText);
            // 添加新文本
            priceElem.appendChild(doc.createTextNode("49.99"));
            priceElem.setAttribute("currency", "EUR");

            // 添加新子元素
            QDomElement isbnElem = doc.createElement("isbn");
            isbnElem.appendChild(doc.createTextNode("978-0321714114"));
            bookElem.appendChild(isbnElem);
        }
        bookElem = bookElem.nextSiblingElement("book");
    }

    // 3. 添加新的 book 节点
    QDomElement newBook = doc.createElement("book");
    newBook.setAttribute("id", "3");
    newBook.setAttribute("lang", "en");
    QDomElement titleElem = doc.createElement("title");
    titleElem.appendChild(doc.createTextNode("Design Patterns"));
    newBook.appendChild(titleElem);
    booksElem.appendChild(newBook);

    // 4. 删除节点
    QDomElement metaElem = root.firstChildElement("metadata");
    if (!metaElem.isNull()) {
        root.removeChild(metaElem);
    }
}
```

### 6.3 按条件查找元素

```cpp
// 按标签名查找所有匹配元素（整棵树中搜索）
QDomNodeList allBooks = doc.elementsByTagName("book");
for (int i = 0; i < allBooks.count(); i++) {
    QDomElement book = allBooks.at(i).toElement();
    qDebug() << book.attribute("id") << book.firstChildElement("title").text();
}

// 按属性值查找
QDomElement findBookById(const QDomDocument &doc, int id) {
    QDomNodeList books = doc.elementsByTagName("book");
    for (int i = 0; i < books.count(); i++) {
        QDomElement book = books.at(i).toElement();
        if (book.attribute("id").toInt() == id) {
            return book;
        }
    }
    return QDomElement();  // 空元素表示未找到
}
```

---

## 7. 流式 vs DOM：实际场景选择

### 7.1 大文件流式处理（只读一遍）

```cpp
// 1GB 的日志 XML，逐条提取错误记录
void processLargeLog(const QString &filePath) {
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);
    QXmlStreamReader xml(&file);

    int errorCount = 0;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "entry") {
            if (xml.attributes().value("level") == "ERROR") {
                QString msg = xml.readElementText();
                qDebug() << "Error:" << msg;
                errorCount++;
            }
        }
    }
    qDebug() << "Total errors:" << errorCount;
    // 内存始终保持在 KB 级别，不受文件大小影响
}
```

### 7.2 配置文件（需反复读写修改）

```cpp
// 应用配置文件，需要读取 → 修改 → 回写
class ConfigManager {
public:
    bool load(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return false;
        return m_doc.setContent(&file);
    }

    QString value(const QString &key, const QString &defaultVal = {}) {
        QDomElement root = m_doc.documentElement();
        QDomElement elem = root.firstChildElement(key);
        return elem.isNull() ? defaultVal : elem.text();
    }

    void setValue(const QString &key, const QString &val) {
        QDomElement root = m_doc.documentElement();
        QDomElement elem = root.firstChildElement(key);
        if (elem.isNull()) {
            // 新增
            elem = m_doc.createElement(key);
            root.appendChild(elem);
        } else {
            // 清空旧文本
            while (elem.hasChildNodes())
                elem.removeChild(elem.firstChild());
        }
        elem.appendChild(m_doc.createTextNode(val));
    }

    bool save(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
        QTextStream stream(&file);
        m_doc.save(stream, 4);
        return true;
    }

private:
    QDomDocument m_doc;
};

// 使用
ConfigManager config;
config.load("settings.xml");
QString lang = config.value("language", "en");
config.setValue("language", "zh_CN");
config.setValue("theme", "dark");
config.save("settings.xml");
```

---

## 8. 命名空间处理

### 8.1 流式读取带命名空间的 XML

```xml
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:dc="http://purl.org/dc/elements/1.1/">
    <title>My Blog</title>
    <entry>
        <title>First Post</title>
        <dc:creator>Author</dc:creator>
    </entry>
</feed>
```

```cpp
QXmlStreamReader xml(data);

while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
        // 匹配特定命名空间下的元素
        if (xml.namespaceUri() == "http://www.w3.org/2005/Atom") {
            if (xml.name() == "title") {
                qDebug() << "Atom title:" << xml.readElementText();
            }
        }
        if (xml.namespaceUri() == "http://purl.org/dc/elements/1.1/") {
            if (xml.name() == "creator") {
                qDebug() << "DC creator:" << xml.readElementText();
            }
        }
    }
}
```

### 8.2 流式写入命名空间

```cpp
QXmlStreamWriter xml(&output);
xml.setAutoFormatting(true);

xml.writeStartDocument();
xml.writeDefaultNamespace("http://www.w3.org/2005/Atom");
xml.writeNamespace("http://purl.org/dc/elements/1.1/", "dc");

xml.writeStartElement("http://www.w3.org/2005/Atom", "feed");
xml.writeTextElement("http://www.w3.org/2005/Atom", "title", "My Blog");

xml.writeStartElement("http://www.w3.org/2005/Atom", "entry");
xml.writeTextElement("http://www.w3.org/2005/Atom", "title", "First Post");
xml.writeTextElement("http://purl.org/dc/elements/1.1/", "creator", "Author");
xml.writeEndElement();  // </entry>

xml.writeEndElement();  // </feed>
xml.writeEndDocument();
```

### 8.3 DOM 命名空间操作

```cpp
// 创建带命名空间的元素
QDomElement elem = doc.createElementNS(
    "http://www.w3.org/2005/Atom",  // namespace URI
    "atom:entry"                     // qualified name（prefix:localName）
);

// 读取时按命名空间查找
QDomNodeList entries = doc.elementsByTagNameNS(
    "http://www.w3.org/2005/Atom",
    "entry"
);
```

---

## 9. 错误处理

### 9.1 QXmlStreamReader 错误处理

```cpp
QXmlStreamReader xml(data);

while (!xml.atEnd()) {
    xml.readNext();
    // ... 处理逻辑
}

// 检查错误
if (xml.hasError()) {
    switch (xml.error()) {
        case QXmlStreamReader::NotWellFormedError:
            qWarning() << "XML not well-formed:" << xml.errorString();
            break;
        case QXmlStreamReader::PrematureEndOfDocumentError:
            qWarning() << "Unexpected end of document";
            break;
        case QXmlStreamReader::UnexpectedElementError:
            qWarning() << "Unexpected element";
            break;
        case QXmlStreamReader::CustomError:
            qWarning() << "Custom error:" << xml.errorString();
            break;
        default:
            break;
    }
    qWarning() << "At line" << xml.lineNumber()
               << "column" << xml.columnNumber();
}
```

### 9.2 自定义错误

```cpp
// 业务逻辑发现异常时，主动中止解析
if (xml.isStartElement() && xml.name() == "version") {
    int ver = xml.readElementText().toInt();
    if (ver > 3) {
        xml.raiseError(QString("Unsupported version: %1").arg(ver));
        // 循环会在下一次 atEnd() 检查时退出
    }
}
```

### 9.3 QDomDocument 错误处理

```cpp
QString errorMsg;
int errorLine, errorCol;

if (!doc.setContent(xmlData, true,       // true = 启用命名空间处理
                    &errorMsg, &errorLine, &errorCol)) {
    qWarning() << "Parse error:" << errorMsg;
    qWarning() << "Line:" << errorLine << "Column:" << errorCol;
    // 常见错误：
    // "unexpected end of file"         → XML 不完整
    // "tag mismatch"                   → 标签未正确关闭
    // "error occurred while parsing element" → 格式错误
}
```

---

## 10. 性能优化

### 10.1 大文件用流式 API

```cpp
// ❌ DOM 加载 100MB 文件 → 内存暴涨到 500MB+
QDomDocument doc;
doc.setContent(&hugeFile);

// ✅ 流式处理，内存恒定在几 KB
QXmlStreamReader xml(&hugeFile);
while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement() && xml.name() == "target") {
        processItem(xml);  // 逐条处理
    }
}
```

### 10.2 QStringView 避免分配（Qt 6）

```cpp
// Qt 6 中 QXmlStreamReader 的 name() 等返回 QStringView（零拷贝）
QXmlStreamReader xml(data);
while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
        QStringView name = xml.name();      // 无内存分配
        if (name == u"book") {              // 用 u"" 字面量避免 QString 构造
            QStringView id = xml.attributes().value(u"id");
            // ...
        }
    }
}
```

### 10.3 预分配与减少拷贝

```cpp
// 解析大量元素时预分配容器
QList<Book> books;
books.reserve(1000);  // 预估数量

QXmlStreamReader xml(&file);
while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement() && xml.name() == u"book") {
        books.append(parseOneBook(xml));
    }
}
```

---

## 11. 与 QSettings 对比

`QSettings` 也能生成 XML（`QSettings::NativeFormat` 在 macOS 下即 plist XML），但功能有限。对比：

| 特性 | QSettings | QXmlStream / QDom |
|---|---|---|
| 数据结构 | 扁平 key-value | 任意层级嵌套 |
| 格式控制 | 无（自动） | 完全自定义 |
| 跨平台格式 | 因平台而异（注册表/ini/plist） | 统一 XML |
| 适用场景 | 简单配置 | 复杂数据交换、自定义格式 |

---

## 12. 实战：RSS/Atom Feed 解析器

```cpp
struct FeedItem {
    QString title;
    QString link;
    QString description;
    QDateTime pubDate;
};

QList<FeedItem> parseRssFeed(const QByteArray &xmlData) {
    QList<FeedItem> items;
    QXmlStreamReader xml(xmlData);

    bool inItem = false;
    FeedItem current;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            QString name = xml.name().toString();

            if (name == "item" || name == "entry") {
                inItem = true;
                current = FeedItem{};
            }
            else if (inItem) {
                if (name == "title") {
                    current.title = xml.readElementText();
                }
                else if (name == "link") {
                    // RSS: <link>url</link>
                    // Atom: <link href="url"/>
                    if (xml.attributes().hasAttribute("href")) {
                        current.link = xml.attributes().value("href").toString();
                    } else {
                        current.link = xml.readElementText();
                    }
                }
                else if (name == "description" || name == "summary") {
                    current.description = xml.readElementText();
                }
                else if (name == "pubDate") {
                    // RSS date: "Mon, 15 Mar 2024 12:00:00 GMT"
                    current.pubDate = QDateTime::fromString(
                        xml.readElementText(), Qt::RFC2822Date);
                }
                else if (name == "updated") {
                    // Atom date: "2024-03-15T12:00:00Z"
                    current.pubDate = QDateTime::fromString(
                        xml.readElementText(), Qt::ISODate);
                }
            }
        }
        else if (xml.isEndElement()) {
            QString name = xml.name().toString();
            if ((name == "item" || name == "entry") && inItem) {
                items.append(current);
                inItem = false;
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "Feed parse error:" << xml.errorString();
    }

    return items;
}
```

---

## 13. 常见问题

**问题 1：中文乱码**

```cpp
// ❌ 用 QFile + QTextStream 读取后传给 setContent，编码可能混乱
QTextStream in(&file);
QString content = in.readAll();
doc.setContent(content);  // 可能乱码

// ✅ 直接传 QFile（XML 解析器自动处理 encoding 声明）
doc.setContent(&file);
// 或
QXmlStreamReader xml(&file);  // 自动识别 UTF-8/UTF-16 等
```

**问题 2：特殊字符未转义**

```cpp
// ❌ 手动拼接 XML 字符串
QString xml = "<name>" + userName + "</name>";
// 如果 userName 包含 < 或 & 会破坏 XML 结构

// ✅ 用 Writer/DOM，自动处理转义
xml.writeTextElement("name", userName);  // 自动转义 < → &lt; 等
```

**问题 3：DOM 节点操作后保存为空**

```cpp
// ❌ 创建元素但忘记 appendChild
QDomElement elem = doc.createElement("new");  // 只是创建，未挂到树上

// ✅ 必须添加到父节点
root.appendChild(elem);
```

**问题 4：readElementText() 吞掉后续 token**

```cpp
// readElementText() 会读到该元素的 EndElement
// 调用后 reader 已经指向 EndElement 的下一个位置
xml.readNext();  // → StartElement "title"
QString title = xml.readElementText();  // 读取文本并跳到 EndElement
// 此时再 readNext() 会跳到 title 之后的下一个 token
```

---

## 14. 最佳实践

| 实践 | 说明 |
|---|---|
| 优先用流式 API | 除非需要随机修改 DOM，否则 `QXmlStreamReader/Writer` 更高效 |
| 不要手动拼 XML | 永远用 Writer/DOM 生成 XML，避免转义问题和格式错误 |
| 设置 `autoFormatting` | 生成的 XML 便于人类阅读和 diff |
| 检查错误 | 任何外部 XML 输入都要检查 `hasError()` |
| 编码交给解析器 | 不要手动转编码，让 XML 解析器根据声明处理 |
| XML 声明写 encoding | `<?xml version="1.0" encoding="UTF-8"?>` 明示编码 |
| 大文件流式处理 | 超过几 MB 的 XML 务必用 `QXmlStreamReader` |
| DOM 节点的生命周期 | DOM 节点由 `QDomDocument` 管理，不要在 document 销毁后使用节点 |
| 文件写入原子性 | 写入临时文件再 rename，避免写到一半崩溃导致 XML 损坏 |
