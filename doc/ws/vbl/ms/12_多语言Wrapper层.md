# 12 - 多语言 Wrapper 层

> **面试价值**：⭐⭐⭐⭐  **优先级**：P1
> **相关文件**：
> - wrapper/vbl-c/ — C/ObjC++ wrapper
> - wrapper/vbl-swift/ — Swift wrapper（依赖 vbl-c）
> - wrapper/vbl-clrWrapper/ — C++/CLI .NET wrapper
> - wrapper/vbl_web_sdk/ — Web/WASM wrapper（emscripten）

## 1. Wrapper 层架构

VBL C++ 核心通过多个 wrapper 层暴露给不同上层语言：

    Swift(iOS/macOS) --> vbl-swift --> vbl-c --> VBL C++ 核心
    C#/.NET          --> vbl-clrWrapper      --> VBL C++ 核心
    JavaScript       --> vbl_web_sdk(WASM)   --> VBL C++ 核心
    ObjC/ObjC++      --> vbl-c              --> VBL C++ 核心

---

## 2. vbl-c — C 接口 / ObjC++ 桥接层

文件：wrapper/vbl-c/

C wrapper 层将 C++ 接口封装为 C 函数，解决跨语言 ABI 问题：

    // wrapper/vbl-c/VblCInterface.h（示意）
    #ifdef __cplusplus
    extern "C" {   // 禁止 C++ name mangling
    #endif
    
    typedef void* VblProjectHandle;  // 不透明句柄（隐藏 C++ 类指针类型）
    
    VblProjectHandle VblCreateProject(const char* configPath);
    void VblReleaseProject(VblProjectHandle handle);
    int  VblImportMedia(VblProjectHandle project, const char* filePath);
    
    typedef void (*VblEventCallback)(const char* eventName,
                                     const char* eventData, void* userData);
    void VblRegisterEventCallback(VblProjectHandle project,
                                  VblEventCallback callback, void* userData);
    #ifdef __cplusplus
    }
    #endif

    // wrapper/vbl-c/VblCInterface.mm（Objective-C++ 实现文件，.mm 扩展名）
    VblProjectHandle VblCreateProject(const char* configPath) {
        VBL::VbProjectEditor* editor = new VBL::VbProjectEditor();
        if (editor->Initialize(configPath) != VBL::Result::rOk) {
            delete editor;
            return nullptr;
        }
        return (void*)editor;  // C++ 对象指针作为不透明句柄
    }
    
    void VblReleaseProject(VblProjectHandle handle) {
        auto editor = static_cast<VBL::VbProjectEditor*>(handle);
        if (editor) editor->Release();
    }
    
    int VblImportMedia(VblProjectHandle project, const char* filePath) {
        auto editor = static_cast<VBL::VbProjectEditor*>(project);
        if (!editor) return -1;
        VBL::Result r = editor->ImportMedia(std::string(filePath));
        return static_cast<int>(r);  // Result 枚举转 int 返回给 C
    }

关键技术点：
- extern "C" 禁止 name mangling，使 Swift/C# 可以通过名字找到函数
- 不透明句柄（void*）隐藏 C++ 类细节，调用方不需要包含 C++ 头文件
- .mm 文件：Xcode 专有扩展名，可混用 C++、ObjC、ObjC++

---

## 3. vbl-swift — Swift Wrapper

文件：wrapper/vbl-swift/

Swift 通过 vbl-c 层（C 接口）访问 VBL：

    // wrapper/vbl-swift/VblProject.swift
    import Foundation
    
    public class VblProject {
        private var handle: VblProjectHandle?
        
        public init?(configPath: String) {
            handle = VblCreateProject(configPath)
            guard handle != nil else { return nil }
        }
        
        deinit {
            if let h = handle { VblReleaseProject(h); handle = nil }
        }
        
        public func importMedia(filePath: String) -> Bool {
            guard let h = handle else { return false }
            return VblImportMedia(h, filePath) == 0  // 0 = rOk
        }
        
        // C 函数指针转换为 Swift 闭包
        public func onEvent(_ handler: @escaping (String, String) -> Void) {
            let context = Unmanaged.passRetained(handler as AnyObject).toOpaque()
            VblRegisterEventCallback(handle, { name, data, userData in
                let h = Unmanaged<AnyObject>.fromOpaque(userData!).takeUnretainedValue()
                (h as! (String, String) -> Void)(
                    String(cString: name!), String(cString: data!))
            }, context)
        }
    }

---

## 4. vbl-clrWrapper — C++/CLI .NET Wrapper

文件：wrapper/vbl-clrWrapper/

C++/CLI 是微软的托管 C++ 扩展，在同一文件中混用 C++ 和 .NET 代码（/clr 编译）：

    // wrapper/vbl-clrWrapper/VblProject.h（C++/CLI）
    #pragma once
    using namespace System;
    
    // ref class：托管类（.NET GC 管理内存）
    public ref class VblProject {
    public:
        VblProject(String^ configPath) {
            // 字符串转换：.NET String（Unicode）-> C++ std::string
            std::string nativePath = msclr::interop::marshal_as<std::string>(configPath);
            m_nativeProject = new VBL::VbProjectEditor();
            m_nativeProject->Initialize(nativePath);
        }
        
        // ~ 析构函数：对应 IDisposable.Dispose()（using 语句触发）
        ~VblProject() { this->!VblProject(); }
        
        // ! 终结器：GC 收集时释放（非托管资源必须在此释放）
        !VblProject() {
            if (m_nativeProject) {
                m_nativeProject->Release();
                m_nativeProject = nullptr;
            }
        }
        
        bool ImportMedia(String^ filePath) {
            std::string nativePath = msclr::interop::marshal_as<std::string>(filePath);
            return m_nativeProject->ImportMedia(nativePath) == VBL::Result::rOk;
        }
        
    private:
        VBL::VbProjectEditor* m_nativeProject;  // 非托管指针
    };

C++/CLI 关键概念：
- ref class：托管类，由 .NET GC 管理
- ^ （hat）符号：托管引用（类似 shared_ptr 但由 GC 管理）
- marshal_as<>：托管/非托管类型转换模板
- /clr 编译选项：启用托管代码支持

---

## 5. vbl_web_sdk — Web/WASM Wrapper

文件：wrapper/vbl_web_sdk/

emscripten 的 embind 将 C++ 类直接绑定为 JavaScript 类：

    // wrapper/vbl_web_sdk/VblWebBinding.cpp
    #include <emscripten/bind.h>
    using namespace emscripten;
    
    EMSCRIPTEN_BINDINGS(vbl_module) {
        class_<VBL::VbProjectEditor>("VblProject")
            .constructor<>()
            .function("importMedia", &VBL::VbProjectEditor::ImportMedia)
            .function("undo",        &VBL::VbProjectEditor::Undo)
            .function("redo",        &VBL::VbProjectEditor::Redo);
        
        enum_<VBL::Result>("VblResult")
            .value("Ok",     VBL::Result::rOk)
            .value("Failed", VBL::Result::rFailed);
        
        value_object<VBL::DmMediaInfo>("DmMediaInfo")
            .field("duration", &VBL::DmMediaInfo::duration)
            .field("width",    &VBL::DmMediaInfo::width);
    }

JavaScript 调用：

    const VBL = await createVblModule();
    const project = new VBL.VblProject();
    const result = project.importMedia("/path/to/video.mp4");
    if (result === VBL.VblResult.Ok) { console.log("success"); }
    project.delete();  // 必须手动释放！WASM 对象不被 JS GC 管理

---

## 6. 面试要点

1. **extern "C" 是跨语言 ABI 的基础**：C++ name mangling 使函数符号名不可预测，
   extern "C" 禁止 mangling，使 Swift/C#/Python 可以用固定名字找到函数。
   所有 C wrapper 函数用 VblXxx 统一前缀防止名字冲突。

2. **不透明句柄隐藏 C++ 类型**：C 接口返回 void*，调用方不需要包含 C++ 头文件，
   完全解耦。类型安全由 wrapper 层维护（内部 static_cast 转换）。

3. **C++/CLI 的析构/终结器模式**：~ 析构函数对应 IDisposable.Dispose()（using 语句触发），
   ! 终结器对应 GC 收集时的清理。非托管资源必须在 ! 中释放，防止 GC 延迟导致 C++ 对象泄漏。

4. **WASM 对象必须手动 delete()**：embind 创建的 C++ 对象包装在 JS 对象中，
   但 C++ 内存在 WASM heap 中，不受 JS GC 管理。不调用 delete() 会造成 WASM 内存泄漏。

5. **vbl-c 作为中间层的价值**：Swift 5.9 之前不能直接调用 C++，
   vbl-c 的 C 接口同时服务 Swift 和 ObjC 调用方，不需要为每种语言单独实现 C++ 绑定逻辑。

---

## 7. 可能被追问的问题

**Q1：为什么不用 SWIG 等自动绑定工具？**
自动工具生成代码通常对 VBL 的 COM 风格生命周期（AddRef/Release）理解不好。
手写 wrapper 可以精确控制对象创建/销毁、错误码转换、字符串编码转换等细节。
WASM 场景用 embind 合理（emscripten 官方推荐），Web 没有其他成熟方案。

**Q2：C++/CLI 的 /clr 编译和普通 C++ 编译可以混合吗？**
可以，但有限制。同一个 .lib/.dll 可以包含 /clr 和普通 C++ 文件。
ref class 只能在 /clr 文件中定义。混合时需要注意：/clr 不支持某些 C++ 特性
（如某些 RTTI 操作）。

**Q3：WASM 对象泄漏如何排查？**
用 emscripten 的 ASAN/LeakSanitizer（-fsanitize=address）。
也可以监控 Module.HEAP32.byteLength 趋势来发现泄漏。

**Q4：Swift 5.9 引入 C++ Interop 后，vbl-c 还有必要吗？**
Swift 5.9 的 C++ Interop 对虚函数、模板、COM 风格接口支持还不完整。
VBL 的 COM 风格接口（虚函数 + AddRef/Release）需要特殊处理。
保持 vbl-c 中间层是更稳定的方案，且兼容旧版本 Swift（<5.9）和 ObjC 调用方。
