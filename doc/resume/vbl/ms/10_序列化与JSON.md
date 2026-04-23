# 10 · 序列化与 JSON 库选型

> **面试价值**：⭐⭐⭐  **优先级**：P2
> **相关文件**：
> - modules/Adapter/wes/ — DmWesMediaInfoAdapter（rapidjson）
> - modules/BusinessLayer/ProjectSerialize/ — WebProjectParser（rapidjson）
> - modules/BaseService/BsProxy/BsCloudStorage.cpp（jsoncpp）

---

## 1. 项目中并存两套 JSON 库

VBL 中同时使用了两种 JSON 库，分别用于不同场景：

| 库 | 使用场景 | 原因 |
|----|----------|------|
| **rapidjson** | 工程文件解析、媒体信息处理、WASM | 高性能、零依赖、无内存分配开销 |
| **jsoncpp** | 云存储 API、网络接口 | API 更友好、对象模型方便修改 |

---

## 2. rapidjson — 高性能场景

**文件**：modules/BusinessLayer/ProjectSerialize/WebProjectParser.cpp
**文件**：modules/Adapter/wes/DmWesMediaInfoAdapter.cpp

rapidjson 的核心特点：
- SAX（流式解析）和 DOM 两种解析模式
- 零内存拷贝：使用 in-place 解析（修改输入字符串，不分配新内存）
- 无异常（错误通过返回值/状态码）
- 头文件库，无需编译链接

    // rapidjson 使用示例（DmWesMediaInfoAdapter.cpp 风格）
    #include "rapidjson/document.h"
    #include "rapidjson/writer.h"
    #include "rapidjson/stringbuffer.h"
    using namespace rapidjson;
    
    // 解析 JSON
    bool ParseMediaInfo(const char* jsonStr, DmMediaInfo& outInfo) {
        Document doc;
        // ParseInsitu 模式：in-place 解析，修改输入字符串，零内存分配
        // 注意：ParseInsitu 会修改 jsonStr，调用方需要保证 char* 可写
        if (doc.ParseInsitu(const_cast<char*>(jsonStr)).HasParseError()) {
            return false;
        }
        
        // 访问字段（必须检查类型，否则 assert/未定义行为）
        if (doc.HasMember("duration") && doc["duration"].IsInt64()) {
            outInfo.duration = doc["duration"].GetInt64();
        }
        if (doc.HasMember("width") && doc["width"].IsInt()) {
            outInfo.width = doc["width"].GetInt();
        }
        
        // 遍历数组
        if (doc.HasMember("streams") && doc["streams"].IsArray()) {
            for (const auto& stream : doc["streams"].GetArray()) {
                DmStreamInfo streamInfo;
                streamInfo.type = stream["type"].GetString();
                outInfo.streams.push_back(streamInfo);
            }
        }
        return true;
    }
    
    // 序列化 JSON
    std::string SerializeProject(const VbProjectData& project) {
        Document doc;
        doc.SetObject();
        auto& alloc = doc.GetAllocator();  // 必须用同一个 allocator
        
        doc.AddMember("version", Value(project.version.c_str(), alloc), alloc);
        doc.AddMember("duration", project.duration, alloc);
        
        // 添加数组
        Value clips(kArrayType);
        for (const auto& clip : project.clips) {
            Value clipObj(kObjectType);
            clipObj.AddMember("id", Value(clip.id.c_str(), alloc), alloc);
            clipObj.AddMember("startTime", clip.startTime, alloc);
            clips.PushBack(clipObj, alloc);
        }
        doc.AddMember("clips", clips, alloc);
        
        // 序列化为字符串
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);
        return buffer.GetString();
    }

**rapidjson 的坑**：
- 字符串必须指定 allocator：`Value("str", alloc)` 而非 `Value("str")`
- `GetString()` 返回指针，生命周期绑定到 Document，不能在 Document 析构后使用
- 访问不存在的字段会 assert（debug）或未定义行为（release），必须先检查 HasMember

---

## 3. jsoncpp — 友好 API 场景

**文件**：modules/BaseService/BsProxy/BsCloudStorage.cpp

jsoncpp 的特点：
- 面向对象的 API，Json::Value 可以方便地动态修改
- 内建字符串的深拷贝，内存安全但有额外开销
- 适合需要动态构建 JSON 结构的场景（如网络请求参数）

    // jsoncpp 使用示例（BsCloudStorage.cpp 风格）
    #include "json/json.h"
    
    // 构建网络请求 JSON
    std::string BuildUploadRequest(const std::string& fileId,
                                   const std::string& token) {
        Json::Value root;
        root["fileId"] = fileId;
        root["token"] = token;
        root["timestamp"] = static_cast<Json::Int64>(GetCurrentTimestamp());
        
        Json::Value metadata;
        metadata["platform"] = "windows";
        metadata["version"] = "7.17.0";
        root["metadata"] = metadata;  // 嵌套对象
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";  // 紧凑格式（不带换行缩进）
        return Json::writeString(builder, root);
    }
    
    // 解析响应
    bool ParseUploadResponse(const std::string& responseJson, UploadResult& result) {
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errs;
        std::istringstream ss(responseJson);
        
        if (!Json::parseFromStream(builder, ss, &root, &errs)) {
            VBL_LOG_ERROR("JSON parse error: %s", errs.c_str());
            return false;
        }
        
        // jsoncpp 访问不存在字段返回默认值，不会崩溃
        result.fileUrl = root.get("url", "").asString();
        result.expireTime = root.get("expireTime", 0).asInt64();
        result.success = root.get("success", false).asBool();
        return true;
    }

---

## 4. 选型决策依据

    性能敏感、工程文件解析（大文件）     -->  rapidjson（SAX/DOM，零拷贝）
    网络接口、云 API（动态构建请求体）  -->  jsoncpp（API 友好，方便修改）
    WASM 环境                           -->  rapidjson（无线程局部存储问题）
    单元测试配置文件                    -->  jsoncpp（可读性更重要）

---

## 5. 面试要点

1. **rapidjson 的性能优势来自零内存分配**：ParseInsitu 模式直接在输入 buffer 上解析，
   不需要分配新内存来存储字符串。工程文件可能几十 MB，zero-copy 解析显著提升速度。

2. **rapidjson 需要更多防御性编码**：每次访问字段必须 HasMember() + Is<Type>() 双检查，
   否则可能 assert/崩溃。jsoncpp 的 get() 有默认值，更宽容。

3. **jsoncpp 适合动态构建 JSON**：网络请求参数在运行时根据条件动态添加字段，
   jsoncpp 的 operator[] 直接赋值更自然；rapidjson 需要显式传 allocator，麻烦。

4. **两库并存的维护成本**：需要两套解析/序列化代码，开发者需要同时了解两个 API。
   未来可能统一（VBL 有迁移到 nlohmann/json 的讨论，它兼顾性能和友好 API）。

5. **WASM 下只用 rapidjson**：jsoncpp 有线程局部存储（TLS）依赖，在 WASM 单线程模式
   下可能有兼容性问题；rapidjson 是纯头文件，无外部依赖，WASM 兼容性更好。

---

## 6. 可能被追问的问题

**Q1：rapidjson 和 nlohmann/json 相比有什么优劣？**
rapidjson：极致性能，SAX/DOM 双模式，零内存分配，API 相对繁琐。
nlohmann/json：现代 C++ 风格 API，语法简洁（支持 json["key"] = value），
性能略低但对大多数场景够用。VBL 用 rapidjson 是历史选择（7年前 nlohmann 还不成熟）。

**Q2：大 JSON 文件（100MB 工程文件）如何高效解析？**
用 rapidjson 的 SAX 模式（流式解析），不需要将整个文档加载到内存中构建 DOM 树。
实现 Handler 接口，流式处理每个 token（StartObject/Key/Value/EndObject），
内存占用为 O(1) 而非 O(n)。

**Q3：JSON 解析中如何处理版本兼容性（新版本增加字段，旧版本少字段）？**
rapidjson：HasMember() 检查字段存在性，不存在则跳过或用默认值。
jsoncpp：get("key", defaultValue) 字段不存在返回 defaultValue。
VBL 工程文件的版本兼容策略：新增字段给默认值（向前兼容）；删除字段时读取时忽略（向后兼容）。
