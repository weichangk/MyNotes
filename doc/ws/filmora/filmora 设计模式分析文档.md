# Filmora 设计模式分析文档

> 版本：Filmora 14.7.x（Windows / macOS）
> 日期：2026-04-08

本文档基于对源码的深度分析，梳理项目中实际使用的设计模式，每种模式均附有具体文件路径与关键代码片段作为证据。

---

## 目录

1. [MVP 模式](#1-mvp-模式)
2. [观察者模式](#2-观察者模式)
3. [工厂模式](#3-工厂模式)
4. [单例模式](#4-单例模式)
5. [命令模式 + Undo/Redo](#5-命令模式--undoredo)
6. [Builder 模式](#6-builder-模式)
7. [PIMPL 模式](#7-pimpl-模式)
8. [策略模式](#8-策略模式)
9. [代理模式](#9-代理模式)
10. [装饰器模式 + 组合模式](#10-装饰器模式--组合模式)
11. [状态机模式](#11-状态机模式)
12. [汇总一览](#12-汇总一览)

---

## 1. MVP 模式

MVP（Model-View-Presenter）是项目最核心的 UI 架构模式，几乎所有 UI 面板都遵循此分层。

### 三层结构

```
View      → *View 类，继承 FFUIView（Qt Widget 封装）
Presenter → *Presenter 类，继承 FFPresenter（框架级 Presenter 基类）
Model     → *Model 类，继承 QAbstractItemModel（Qt 数据模型）
```

### 关键证据

**Presenter 层：**
```cpp
// Include/FTimelineView/FTimelinePresenter.h
class FTimelinePresenter : public FFPresenter {
    Q_DECLARE_PRIVATE(FTimelinePresenter)
    QScopedPointer<FTimelinePresenterPrivate> d_ptr;
    ...
};
```

**View 层：**
```cpp
// Include/FMediaLibraryView/FMediaTreeView.h
class FMediaTreeView : public QTreeView, virtual public FFUIView {
    void addObserver(DisplayChangedObserver* observer);
    ...
};
```

**Model 层：**
```cpp
// Include/FFMediaLibrary/FFMediaDataModel.h
class FFMediaItemModel : public QAbstractItemModel,
                         public IFFMediaItemEventObserver {
    QScopedPointer<FMediaItemData> m_data;
    ...
};
```

### 适用模块

时间轴（`FTimelineView`）、媒体库（`FMediaLibraryView`）、属性面板（`FPropertyPanelView`）、导出面板（`FExportView`）、模板编辑（`TemplateMode`）等所有 UI 面板均遵循此模式。

---

## 2. 观察者模式

项目以三种形态并存实现观察者模式，按使用场景分层。

### 变体一：`ffcore::ObserverContainer`（弱引用泛型容器）

用于组件间的类型安全订阅，内置弱引用保护，避免野指针问题。

```cpp
// 3rdparty/FilmoraFrameworkPlatform/Include/FFCore/ObserverList.h
template<class ObserverType, class ObserverStoragePolicy = SafeObserverStorage>
class ObserverContainer {
    void AddObserver(ObserverPtrType observer);
    void RemoveObserver(ObserverPtrType observer);
    // NotifyObservers 遍历时自动跳过失效观察者
};

// Include/FFMediaLibrary/FFMediaDataModel.h 实际使用
class FFMediaItemModelObserver : public ffcore::ObserverBase {
    virtual void OnModelReset(FFMediaItemModel*) {}
};
ffcore::ObserverContainer<FFMediaItemModelObserver> observers_;

// Src/Filmora/FilmoraApp.h 中跨模块订阅示例
ffcore::ObserverContainer<IFStartedAppObserver> m_startedAppObserverList;
```

相关文件：
- `3rdparty/FilmoraFrameworkPlatform/Include/FFCore/ObserverList.h`
- `3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFObserverBase.h`
- `3rdparty/FilmoraFrameworkPlatform/Include/FFCore/FFObserverStorage.h`

### 变体二：`IFFEventBus` 事件总线（发布-订阅）

用于跨模块、跨场景的应用级事件广播。

```cpp
// 3rdparty/FilmoraFrameworkPlatform/Include/FFVBLModel/IFFEventBus.h
class IFFEventBus {
    virtual void attach(IFFEventBusObserver* observer) = 0;
    virtual void detach(IFFEventBusObserver* observer) = 0;
};

// 3rdparty/FilmoraFrameworkPlatform/Src/FFVBLModel/FFVBLEventBus.cpp
void FFVBLEventBus::dispatchEvent(VBLConstPChar evName,
                                   const VBL::EventValue& evValue) {
    m_pParserMgr->parseEvent(...);
}
```

`FilmoraAppEventHandler` 实现了 `IFFEventBusObserver`，响应底层事件（AI 任务状态变化、特效失败等），再通过 `pActiveScene->sendMessage(&msg)` 转发给 UI 场景层。

### 变体三：Qt 信号/槽

用于 UI 控件层的直接通知，是最轻量的观察者实现。

```cpp
connect(m_pBtnCancel, &QPushButton::clicked, this, &FTrimView::slotCancel);
connect(m_pTrimControl, &FTrimControl::sigSelectAreaChange,
        this, &FTrimView::slotSelectAreaChanged);
```

### 三种变体对比

| 变体 | 适用层次 | 特点 |
|------|----------|------|
| `ObserverContainer` | 组件/跨模块层 | 弱引用保护、类型安全、支持批量通知 |
| `IFFEventBus` | 应用级/场景层 | 跨模块广播、支持事件名称路由 |
| Qt 信号/槽 | UI 控件层 | 轻量、语法简洁、Qt 原生支持 |

---

## 3. 工厂模式

项目使用了多种形态的工厂，从泛型模板工厂到具体对象工厂均有覆盖。

### 泛型模板工厂（`FFVBLObjectFactory`）

通过 `typedef` 为不同 Clip 类型生成专属工厂，并提供 `getOrCreate` 缓存创建语义：

```cpp
// 3rdparty/FilmoraFrameworkPlatform/Src/FFVBLModel/FFClipFactory.h
typedef FFVBLObjectFactory<FFVideoClip, VBL::IDmClip, VBL::IDmVideoClip*>
    FFVideoClipFactory;
#define FF_VIDEO_CLIP_FACTORY FFVideoClipFactory::instance()

class FFClipFactory {
public:
    // 先查缓存，缓存未命中则创建新对象（对象池语义）
    static FFClip* getOrCreateClip(VBL::IDmClip* dmClip,
                                    FFTimeline* timeline = nullptr);
};
```

### 具体对象工厂

```cpp
// 3rdparty/FilmoraFrameworkPlatform/Src/FFVBLModel/FFExportFactory.h
class FFExportJobFactory {
public:
    static FFVBLObjectRefPtr<IFFExportJob> createEncodeExportJob(...);
    static FFVBLObjectRefPtr<IFFExportJob> createAutoReframeExportJob(...);
};
```

### 其他工厂类

- `FFEffectFactory` — 特效对象工厂
- `FFAnimationFactory` — 动画对象工厂
- `FFThumbnailCache` — 缩略图缓存工厂

---

## 4. 单例模式

项目封装了 `FFLazySingleton<T>` 懒加载单例基类，避免在各处重复实现单例逻辑。

### `FFLazySingleton` 基类（懒加载）

```cpp
// 3rdparty/FilmoraFrameworkPlatform/Src/FFVBLModel/FFVBLEventBus.h
class FFVBLEventBus : public FFLazySingleton<FFVBLEventBus> {
    ...
};
#define FF_DECLARE_EVENT_BUS FFVBLEventBus::instance()

// Include/FCore/FAICopilotService.h
class FAICopilotService : public FFLazySingleton<FAICopilotService> {
    ...
};
```

### 传统静态 `getInstance()` 风格

```cpp
// Include/FActiveLineEventTrace/FActiveLineSession.h
class FActiveLineSession {
public:
    static FActiveLineSession* getInstance();
    ...
};
```

使用单例的场景包括：事件总线、AI Copilot 服务、各类全局管理器与缓存助手类。

---

## 5. 命令模式 + Undo/Redo

项目采用双轨并行实现：Qt 原生命令栈负责具体操作，自定义 Presenter 负责 UI 层封装。

### Qt 原生命令栈（`QUndoCommand` / `QUndoStack`）

```cpp
// Include/FCommonView/AutoReframe/FAutoReframeHelper.h
class CropKeyFrameCommand : public QUndoCommand {
    void redo() override;
    void undo() override;
};
QUndoStack m_oUndoStack;
```

### 自定义 Undo/Redo Presenter

```cpp
// Include/FCommonView/Presenter/FUndoRedoPresenter.h
class FUndoRedoPresenter : public FFActionPresenter {
public:
    void setUndoRedoService(IFFUndoRedoService* undoRedoSrv);
protected:
    bool exec() override;
    QScopedPointer<FUndoRedoPresenterPrivate> d;
};
```

Undo/Redo 相关事件同时在 `FFVBLEventBus` 中注册（`registerUndoRedoEventType`），使得撤销/重做操作可在事件总线上广播，通知各模块同步状态。

---

## 6. Builder 模式

用于分步构建复杂对象，支持链式调用，提升可读性与可维护性。

### 时间轴 Builder

```cpp
// 3rdparty/FilmoraFrameworkPlatform/Include/FFVBLModel/FFTimelineBuilder.h
class FFTimelineBuilder {
public:
    FFTimelineBuilder(bool useUndoRedo, bool initConfig = false);
    FFTimelineBuilder& closeEvent();   // 链式配置
    FFVBLObjectRefPtr<IFFTimeline> build();  // 最终构建
private:
    QScopedPointer<FFTimelineBuilderPrivate> d;
};
```

### Undo/Redo Presenter Builder

```cpp
// Include/FCommonView/Presenter/FUndoRedoPresenter.h
class FUndoRedoBuilder {
public:
    FUndoRedoBuilder& useUndo(bool undo);
    FUndoRedoBuilder& buildAction();
    FUndoRedoPresenter* build();
    FUndoRedoPresenter* presenter() const;
};
```

---

## 7. PIMPL 模式

PIMPL（Pointer to IMPLementation）在项目中**全局广泛使用**，几乎所有 Presenter 类都采用此模式。

### 实现方式（Qt 风格）

```cpp
// Include/FTimelineView/FTimelinePresenter.h
class FTimelinePresenter : public FFPresenter {
    Q_DECLARE_PRIVATE(FTimelinePresenter)
    QScopedPointer<FTimelinePresenterPrivate> d_ptr;
};

// Include/TemplateMode/FVideoTemplate/FVideoTemplatePresenter.h
class FVideoTemplatePresenter : public FFPresenter {
    QScopedPointer<FVideoTemplatePresenterPrivate> d;
};
```

### 作用

- **隐藏实现细节**：头文件只暴露接口，实现细节藏在 `*Private` 类中
- **减少头文件依赖**：降低编译时间，避免实现变动导致大量文件重编译
- **保持 ABI 稳定**：对外接口不变时，内部实现可自由修改

---

## 8. 策略模式

### 编译期策略（模板参数替换）

通过模板参数在编译期选择不同策略，零运行时开销：

```cpp
// 3rdparty/FilmoraFrameworkPlatform/Include/FFCore/ObserverList.h
template<class ObserverType,
         class ObserverStoragePolicy = SafeObserverStorage>
class ObserverContainer { ... };

// 使用线程不安全存储（性能优先场景）
ObserverContainer<MyObserver, UnsafeObserverStorage> container;
```

### 运行时策略（接口注入）

通过注入不同实现来动态切换过滤行为：

```cpp
// Include/FFMediaLibrary/FFMediaDataModel.h
void setViewTypeFilterProxy(
    std::unique_ptr<IFFAbstractMediaFilterProxy> filter);
// 根据媒体库的不同视图模式（列表/缩略图/类别）注入对应的过滤策略
```

---

## 9. 代理模式

在操作执行前插入额外逻辑（权限检查、资源验证、路径决策），不改变被代理接口。

```cpp
// Src/TemplateMode/FVideoTemplate/StoryEdit/FEnterTimelineLicenseProxy.h
class FEnterTimelineLicenseProxy : public QObject {
public:
    void init(IFFTimeline*, QWidget*);
    bool doCheck();                    // 进入时间线前的授权检查
    bool isNeedSaveProject() const;    // 决定是否需要保存项目
    QString savePath() const;          // 确定保存路径
};
```

该类封装了"进入时间线"操作的前置检查逻辑，属于典型的访问代理（Access Proxy）。

---

## 10. 装饰器模式 + 组合模式

### 装饰器（UI 层功能透明扩展）

通过装饰器为控件添加阴影、蒙层等视觉效果，不修改原控件实现：

```cpp
// 3rdparty/FilmoraFrameworkPlatform/Include/FFWidgets/FEventFilter.h
// 注释原文："借助 Qt 的过滤器，定义的一些装饰器，
//           可以满足界面一定装饰性功能"
class FShadowDecorator : public QWidget {
    // 为目标窗口添加阴影效果
};
```

### 组合（复合媒体对象树）

复合媒体对象（模板包、主题包）由若干子媒体对象组成，统一接口处理：

```cpp
// Include/FFMediaLibrary/FFMediaDataModel.h
class FThemeMediaData {
    IFFCompositeMediaItem* item(const int index) const;
    // 模板/主题包作为复合节点，包含多个子媒体条目
};
```

特效链（`FFEffect` 叠加）也呈现组合结构：clip 上可挂载多个 `FFEffect`，每个 effect 是独立的可组合处理单元，多特效叠加时以组合/链式结构统一处理渲染管线。

---

## 11. 状态机模式

用于管理复杂 UI 交互的多状态切换，项目使用了层次状态机（HSM）：

```cpp
// 3rdparty/FilmoraFrameworkPlatform/Include/FFWidgets/FEventFilter.h
HSM::StateMachine  // 层次状态机（Hierarchical State Machine）
FFDragMoveFilter   // 基于状态机的拖拽交互过滤器
```

层次状态机支持状态继承与嵌套，适合处理拖拽、多模式切换等需要层次化状态管理的复杂 UI 交互场景。

---

## 12. 汇总一览

| 设计模式 | 代表文件 | 使用层次 | 主要用途 |
|----------|----------|----------|----------|
| **MVP** | `FTimelinePresenter.h` / `FMediaTreeView.h` / `FFMediaDataModel.h` | UI 层全局 | View、Presenter、Model 三层解耦 |
| **观察者** | `ObserverList.h` / `IFFEventBus.h` / Qt connect | UI → 应用层 | 跨模块事件通知与状态订阅 |
| **工厂** | `FFClipFactory.h` / `FFExportFactory.h` | 模型层 | 统一对象创建，支持缓存复用 |
| **单例** | `FFVBLEventBus.h` / `FAICopilotService.h` | 服务层 | 全局服务的唯一访问点 |
| **命令** | `FUndoRedoPresenter.h` / `FAutoReframeHelper.h` | UI & 模型层 | 用户操作的撤销与重做 |
| **Builder** | `FFTimelineBuilder.h` / `FUndoRedoBuilder` | 模型层 | 复杂对象的分步链式构建 |
| **PIMPL** | 所有 `*Presenter.h`（`d_ptr`） | UI 层全局 | 隐藏实现、减少依赖、ABI 稳定 |
| **策略** | `ObserverList.h` / `FFMediaDataModel.h` | 框架 & 模型层 | 编译期/运行时可替换行为策略 |
| **代理** | `FEnterTimelineLicenseProxy.h` | 业务逻辑层 | 访问控制与前置检查 |
| **装饰器** | `FEventFilter.h`（`FShadowDecorator`） | UI 层 | 透明扩展控件视觉/行为 |
| **组合** | `FFMediaDataModel.h`（`IFFCompositeMediaItem`） | 模型层 | 复合媒体对象树形结构 |
| **状态机** | `FEventFilter.h`（`HSM::StateMachine`） | UI 交互层 | 复杂拖拽与多模式交互状态管理 |
