# 03 · 设计模式 — 命令模式与 Undo/Redo 框架

> **面试价值**：⭐⭐⭐⭐⭐  **优先级**：P0  
> **所属专题**：设计模式  
> **相关文件**：
> - `modules/BaseService/BsUndoManager/BsUndoAutoTemplateCommand.h/.cpp`
> - `modules/BaseService/BsUndoManager/BsUndoAutoTemplateCommand.h`
> - `modules/BaseService/BsUndoManager/BsUndoTemplateStack.cpp`
> - `Include/Common/VblUndoRedoWrapper.h`
> - `Interface/BaseService/IBsUndoManager.h`

---

## 1. 概念介绍

**命令模式（Command Pattern）** 将操作封装为对象，实现：
- **解耦**：调用者不需要知道操作的具体实现
- **可撤销**：每个命令对象持有 undo/redo 逻辑
- **可组合**：多个命令可以组合成宏命令（Macro Command）

Undo/Redo 框架是命令模式最经典的应用场景。其核心结构：

```
执行操作 → 创建 Command 对象 → push 到 Undo 栈
撤销操作 → pop Undo 栈 → 调用 command.undo() → push 到 Redo 栈
重做操作 → pop Redo 栈 → 调用 command.redo() → push 到 Undo 栈
```

---

## 2. 项目中的实现

### 2.1 核心接口 `IBsUndoTemplateItem`

```cpp
// Interface/BaseService/IBsUndoManager.h（接口定义）
class IBsUndoTemplateItem {
public:
    virtual void undo() = 0;          // 撤销操作
    virtual void redo() = 0;          // 重做操作（第一次执行也走此路）
    virtual const char* getText() = 0; // 操作描述（显示在 "撤销XXX" 菜单中）
    virtual void release() = 0;       // 释放命令对象（引用计数归零）
    virtual ~IBsUndoTemplateItem() = default;
};
```

---

### 2.2 `BsUndoAutoTemplateCommand` — std::function 风格命令

**文件**：`modules/BaseService/BsUndoManager/BsUndoAutoTemplateCommand.h`

这是项目中最常用的命令实现方式，用 `std::function` 存储 undo/redo 逻辑，避免为每个操作都写一个子类：

```cpp
// BsUndoAutoTemplateCommand.h
class BsUndoAutoTemplateCommand : public IBsUndoTemplateItem {
public:
    using UndoRedoFunc = std::function<void()>;
    
    BsUndoAutoTemplateCommand(
        VBLConstPChar text,
        UndoRedoFunc  redoFunc,   // 执行 / 重做逻辑
        UndoRedoFunc  undoFunc    // 撤销逻辑
    );
    
    void undo() override { m_undoFunc(); }
    void redo() override { m_redoFunc(); }
    const char* getText() override { return m_text.c_str(); }
    
private:
    std::string  m_text;
    UndoRedoFunc m_redoFunc;
    UndoRedoFunc m_undoFunc;
};
```

**使用示例**（业务代码中添加 Clip 的 Undo 命令）：
```cpp
// 文件: modules/BusinessLayer/TimelineEditor/（示意）
void TimelineEditor::AddClip(SafePtr<IDmClip> clip, int trackIndex, int64_t position) {
    // 先执行操作
    m_timeline->InsertClip(clip, trackIndex, position);
    
    // 创建 Undo 命令（Lambda 捕获足够的上下文）
    auto cmd = new BsUndoAutoTemplateCommand(
        "添加片段",
        /* redo = */ [this, clip, trackIndex, position]() {
            m_timeline->InsertClip(clip, trackIndex, position);
        },
        /* undo = */ [this, clip, trackIndex]() {
            m_timeline->RemoveClip(clip, trackIndex);
        }
    );
    
    // 推入 Undo 栈
    m_undoManager->Push(cmd);
}
```

**关键设计亮点**：
- Lambda + `std::function` 使 undo/redo 逻辑与业务代码紧密内联，不需要为每个操作写一个类
- Lambda 捕获的变量（`clip`、`trackIndex`、`position`）就是命令的"状态"，自然地保存了操作的上下文
- `SafePtr<IDmClip>` 在 Lambda 中捕获，引用计数自动延长 Clip 的生命周期

---

### 2.3 `BsUndoCombCommand` — 宏命令（组合命令）

**文件**：`modules/BaseService/BsUndoManager/BsUndoAutoTemplateCommand.cpp`

将多个命令组合为一个"原子操作"，一次 Undo 撤销所有子命令：

```cpp
// BsUndoAutoTemplateCommand.h
class BsUndoCombCommand : public IBsUndoTemplateItem {
public:
    explicit BsUndoCombCommand(VBLConstPChar text);
    ~BsUndoCombCommand() override;
    
    // 添加子命令
    void addChild(IBsUndoTemplateItem* item) {
        m_childList.push_back(item);
    }
    
    void undo() override;  // 逆序执行所有子命令的 undo
    void redo() override;  // 正序执行所有子命令的 redo
    
private:
    std::list<IBsUndoTemplateItem*> m_childList;
    IBsUndoTemplateItem* m_curRunCommand = nullptr;  // 当前正在执行的子命令（调试用）
};
```

**关键实现** — 撤销是逆序的（对应实际代码 `BsUndoAutoTemplateCommand.cpp`）：
```cpp
// 文件: modules/BaseService/BsUndoManager/BsUndoAutoTemplateCommand.cpp
void BsUndoCombCommand::undo() {
    // Lambda：按索引访问 list
    auto getItem = [](std::list<IBsUndoTemplateItem*> list, size_t index) {
        auto iter = list.begin();
        std::advance(iter, index);
        return *iter;
    };
    
    // ← 逆序撤销：最后执行的操作最先被撤销（LIFO）
    for (auto i = 0; i < m_childList.size(); i++) {
        auto item = getItem(m_childList, m_childList.size() - i - 1);
        m_curRunCommand = item;
        item->undo();
    }
    m_curRunCommand = nullptr;
}

void BsUndoCombCommand::redo() {
    // → 正序重做
    for (auto iter = m_childList.begin(); iter != m_childList.end(); ++iter) {
        m_curRunCommand = *iter;
        (*iter)->redo();
    }
    m_curRunCommand = nullptr;
}
```

**使用场景**：批量操作（如"粘贴多个 Clip"、"AI 一键成片同时添加多轨元素"）需要作为一个整体被撤销。

---

### 2.4 Undo 栈管理 — `BsUndoTemplateStack`

```cpp
// modules/BaseService/BsUndoManager/BsUndoTemplateStack.h（示意）
class BsUndoTemplateStack {
public:
    void Push(IBsUndoTemplateItem* cmd);   // 执行后 push 到 undo 栈
    
    bool Undo();    // 从 undo 栈 pop，执行 undo()，push 到 redo 栈
    bool Redo();    // 从 redo 栈 pop，执行 redo()，push 到 undo 栈
    
    void Clear();   // 清空两栈（如新建工程时）
    
    // 限制栈深度，防止内存无限增长
    void SetMaxDepth(int depth);
    int  GetUndoCount() const;
    int  GetRedoCount() const;
    
private:
    std::deque<IBsUndoTemplateItem*> m_undoStack;
    std::deque<IBsUndoTemplateItem*> m_redoStack;
    int m_maxDepth = 100;
};
```

**Undo 执行流**：
```
用户按 Ctrl+Z
    ↓
BsUndoManager::Undo()
    ↓
BsUndoTemplateStack::Undo()
    ↓
cmd = m_undoStack.back(); m_undoStack.pop_back()
    ↓
cmd->undo()   ← 执行撤销逻辑（Lambda/子命令）
    ↓
m_redoStack.push_back(cmd)   ← 推入 Redo 栈
    ↓
通知 UI 更新（通过 EventBus postEvent）
```

---

## 3. 关键代码片段

### 片段1：完整的"添加/删除 Clip"Undo 命令注册
```cpp
// 文件: modules/BusinessLayer/TimelineEditor/（示意）
// 注意 SafePtr 在 lambda 中的捕获保证了 clip 的生命周期
void RegisterAddClipCommand(BsUndoManager* um, SafePtr<IDmClip> clip, int64_t pos) {
    auto cmd = new BsUndoAutoTemplateCommand(
        "添加片段",
        [clip, pos, this]() { timeline_->InsertClip(clip, pos); },  // redo
        [clip, this]()      { timeline_->RemoveClip(clip); }        // undo
    );
    um->Push(cmd);
}
```

### 片段2：BsUndoCombCommand 组合多操作
```cpp
// 批量粘贴操作
auto combCmd = new BsUndoCombCommand("粘贴");
for (auto& clip : clipsToPaste) {
    auto singleCmd = new BsUndoAutoTemplateCommand(
        "添加片段",
        [clip, pos]() { /* redo */ },
        [clip]()      { /* undo */ }
    );
    combCmd->addChild(singleCmd);  // 聚合到宏命令
}
undoManager->Push(combCmd);  // 整体推栈，一次 Ctrl+Z 撤销全部
```

---

## 4. 面试要点

1. **std::function + Lambda = 轻量命令模式**：VBL 没有为每个操作写一个 Command 子类，而是用 `BsUndoAutoTemplateCommand` 配合 Lambda，把 undo/redo 逻辑直接内联在业务代码旁边，减少类爆炸，代码更直观。

2. **Lambda 捕获 = 命令状态**：命令模式需要保存操作时的上下文（操作前的状态 or 足以重建状态的参数）。Lambda 的值捕获（`[clip, pos, trackIndex]`）自然地承担了这个职责，`SafePtr<IDmClip>` 捕获还附带了生命周期延长。

3. **组合命令（BsUndoCombCommand）解决批量操作的原子性**：用户眼中"粘贴10个片段"是一个操作，技术上是10个独立命令。`BsUndoCombCommand` 将其包装为一个整体，undo 时逆序执行子命令（LIFO），保证操作的逻辑正确性。

4. **Undo 栈的内存管理**：命令对象由 `BsUndoTemplateStack` 的 `m_undoStack` 持有，弹出时 `release()`。`SafePtr` 在 Lambda 中的捕获确保了命令存活期间 Clip 对象不会被销毁。

5. **与 EventBus 协作**：Undo/Redo 操作完成后，通过 `MsEventBus::postMainThreadEvent()` 通知 UI 层刷新状态（如时间线视图、撤销按钮灰化状态），实现解耦。

---

## 5. 可能被追问的问题

**Q1：命令模式中，undo 需要保存什么状态？有哪些策略？**  
> **策略1（差量/逆操作）**：保存足以重建逆操作的参数（如 AddClip 的 undo 只需知道 ClipId 即可删除）。VBL 主要用此策略，配合 Lambda 捕获。  
> **策略2（快照）**：保存操作前的完整状态副本，undo 时恢复。适合结构复杂、逆操作难以实现的场景（如复杂滤镜参数）。缺点：内存消耗大。  
> VBL 混合使用：简单操作用逆操作 Lambda，复杂操作（如 AI 批量处理）可能保存状态快照。

**Q2：如何防止 Undo 栈无限增长？**  
> `BsUndoTemplateStack` 通过 `SetMaxDepth(100)` 限制最大深度，超出后淘汰最老的命令（从 deque 头部弹出并 `release()`）。  

**Q3：多线程下 Undo/Redo 如何保证线程安全？**  
> VBL 的 Undo/Redo 操作设计在主线程执行（UI 操作触发）。后台任务完成后，通过 `postMainThreadEvent` 将状态变更 dispatch 到主线程，再由主线程创建并推入 Undo 命令，避免并发写入 Undo 栈。

**Q4：`BsUndoCombCommand::undo()` 中为什么要逆序执行子命令？**  
> 保证操作的逻辑一致性。假设组合命令先"添加A轨道"再"在A轨道上添加Clip"，undo 时必须先"删除A轨道上的Clip"再"删除A轨道"，否则操作B的 undo 会因为依赖对象不存在而失败。逆序（LIFO）和操作执行的自然依赖顺序正好相反，恰好正确。

**Q5：如果 redo 失败了（比如引用的对象已被删除），如何处理？**  
> VBL 的 Lambda 捕获 `SafePtr<IDmClip>` 而非裸指针，所以只要 Undo 栈存活，Clip 对象就不会被销毁。但如果外部逻辑强制清除了数据（如新建工程），VBL 会调用 `Clear()` 清空 Undo/Redo 栈，防止失效引用的执行。

