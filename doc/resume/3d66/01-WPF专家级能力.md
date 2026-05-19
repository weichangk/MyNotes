01-WPF专家级能力.md
# WPF专家级能力准备

> 核心要求：精通C#和.NET Framework/.NET Core/.NET 5+，深刻理解XAML、数据绑定、依赖属性、路由事件、命令系统、样式与模板

---

## 📚 目录

- [C# 和 .NET 技术栈](#c-和-net-技术栈)
- [XAML 核心概念](#xaml-核心概念)
- [数据绑定机制](#数据绑定机制)
- [依赖属性详解](#依赖属性详解)
- [路由事件系统](#路由事件系统)
- [命令系统](#命令系统)
- [样式与模板](#样式与模板)
- [MVVM框架实战](#mvvm框架实战)
- [自定义控件开发](#自定义控件开发)
- [Visual Layer高性能绘制](#visual-layer高性能绘制)
- [多线程与异步编程](#多线程与异步编程)

---

## 1️⃣ C# 和 .NET 技术栈

### .NET 版本演进与特性

| 版本 | 发布时间 | 关键特性 | 适用场景 |
|------|---------|---------|---------|
| **.NET Framework 4.8** | 2019 | Windows专用，WPF/WinForms完整支持 | 企业遗留系统 |
| **.NET Core 3.1** | 2019 | 跨平台，LTS长期支持，WPF支持 | 现代桌面应用 |
| **.NET 5** | 2020 | 统一.NET平台 | 过渡版本 |
| **.NET 6** | 2021 | LTS，性能提升，C# 10 | 推荐新项目 |
| **.NET 7** | 2022 | 性能优化，C# 11 | 短期支持 |
| **.NET 8** | 2023 | LTS，最新特性，C# 12 | 当前最佳选择 |

**溜云库使用**: .NET Core 3.1

### C# 现代特性（面试高频）

#### C# 8.0 (.NET Core 3.x)
```csharp
// 可空引用类型
public string? Name { get; set; }

// 异步流
public async IAsyncEnumerable<int> GenerateSequence()
{
    for (int i = 0; i < 20; i++)
    {
        await Task.Delay(100);
        yield return i;
    }
}

// Switch 表达式
public static string GetTaxRate(string country) => country switch
{
    "CN" => "13%",
    "US" => "7%",
    _ => "Unknown"
};

// Using 声明
public void ProcessFile(string path)
{
    using var reader = new StreamReader(path);
    var content = reader.ReadToEnd();
}
```

#### C# 9.0 (.NET 5)
```csharp
// 记录类型
public record Person(string FirstName, string LastName);

// Init-only 属性
public class Model
{
    public string Name { get; init; }
}

// 顶级语句
using System;
Console.WriteLine("Hello!");
```

#### C# 10.0 (.NET 6)
```csharp
// 全局 using
global using System;
global using System.Collections.Generic;

// 文件范围命名空间
namespace MyApp;

public class MyClass { }

// Record struct
public record struct Point(double X, double Y);
```

#### C# 11.0 (.NET 7/8)
```csharp
// 原始字符串字面量
string json = """
    {
        "name": "John",
        "age": 30
    }
    """;

// 列表模式
int[] numbers = { 1, 2, 3 };
if (numbers is [1, 2, 3])
{
    Console.WriteLine("Matched!");
}
```

### 面试题：.NET Framework vs .NET Core

**问题**: 如何决定是使用 .NET Framework 还是 .NET Core/5+？

**答案**:
```
选择 .NET Framework 的场景：
✓ 遗留代码维护，依赖特定Windows API
✓ 使用了第三方组件不支持.NET Core
✓ COM互操作、Windows服务等Windows特定功能
✓ 企业要求（某些企业还在使用）

选择 .NET Core/5+ 的场景：
✓ 新项目优先选择
✓ 需要跨平台支持
✓ 性能要求高（.NET Core性能更好）
✓ 容器化部署
✓ 享受最新C#特性

溜云库迁移建议：
- 当前：.NET Core 3.1（已停止支持）
- 建议：升级到 .NET 8 LTS
  - 性能提升 20-30%
  - 安全补丁支持到2026年
  - C# 12 新特性
```

---

## 2️⃣ XAML 核心概念

### XAML 是什么？

**XAML (Extensible Application Markup Language)** 是一种声明式标记语言，用于定义WPF用户界面。

### 核心语法

```xml
<!-- 1. 对象元素语法 -->
<Button Content="Click Me" Width="100" Height="30" />

<!-- 2. 属性元素语法 -->
<Button>
    <Button.Content>
        <StackPanel>
            <Image Source="icon.png" />
            <TextBlock Text="Click Me" />
        </StackPanel>
    </Button.Content>
</Button>

<!-- 3. 标记扩展 -->
<TextBlock Text="{Binding Name}" />
<Rectangle Fill="{StaticResource PrimaryBrush}" />
<Button Command="{Binding SaveCommand}" />

<!-- 4. 附加属性 -->
<Button Grid.Row="0" Grid.Column="1" />
<TextBlock Canvas.Left="50" Canvas.Top="100" />

<!-- 5. 内容属性 -->
<Button>Click Me</Button>  <!-- Content 是默认内容属性 -->
<StackPanel>
    <Button />
    <TextBox />
</StackPanel>  <!-- Children 是默认内容属性 -->
```

### XAML 命名空间

```xml
<Window x:Class="MyApp.MainWindow"
        xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
        xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
        xmlns:local="clr-namespace:MyApp"
        xmlns:vm="clr-namespace:MyApp.ViewModels"
        xmlns:sys="clr-namespace:System;assembly=mscorlib">
    
    <!-- 使用本地类型 -->
    <local:MyCustomControl />
    
    <!-- 使用ViewModel -->
    <Window.DataContext>
        <vm:MainViewModel />
    </Window.DataContext>
    
    <!-- 使用.NET基础类型 -->
    <sys:Double x:Key="DefaultWidth">100</sys:Double>
</Window>
```

### 面试题：XAML 编译原理

**问题**: XAML 如何转换为C#代码？

**答案**:
```
1. XAML编译过程：
   - XAML文件 → XamlParser → 对象树
   - 生成 .g.cs 文件（InitializeComponent方法）
   - 编译为IL代码

2. InitializeComponent示例：
   public void InitializeComponent() 
   {
       if (_contentLoaded)
           return;
       _contentLoaded = true;
       
       // 加载XAML资源
       System.Uri resourceLocater = new System.Uri(
           "/MyApp;component/mainwindow.xaml", 
           System.UriKind.Relative);
       System.Windows.Application.LoadComponent(this, resourceLocater);
   }

3. 性能考虑：
   - 编译型XAML性能更好
   - 运行时加载XAML（XamlReader.Load）较慢
   - 大型项目建议拆分XAML提升编译速度
```

---

## 3️⃣ 数据绑定机制

### 绑定模式

```csharp
public enum BindingMode
{
    TwoWay,      // 双向绑定（控件 ↔ 数据源）
    OneWay,      // 单向绑定（数据源 → 控件）
    OneTime,     // 一次性绑定（仅初始化时）
    OneWayToSource, // 反向单向（控件 → 数据源）
    Default      // 由控件决定默认模式
}
```

### 绑定语法

```xml
<!-- 1. 基础绑定 -->
<TextBlock Text="{Binding Name}" />

<!-- 2. 指定绑定模式 -->
<TextBox Text="{Binding Name, Mode=TwoWay}" />

<!-- 3. 绑定路径 -->
<TextBlock Text="{Binding Person.Address.City}" />

<!-- 4. 绑定到集合 -->
<ListBox ItemsSource="{Binding Customers}" 
         SelectedItem="{Binding SelectedCustomer}" />

<!-- 5. 绑定转换器 -->
<TextBlock Text="{Binding Age, Converter={StaticResource AgeToStringConverter}}" />

<!-- 6. 绑定验证 -->
<TextBox Text="{Binding Email, ValidatesOnDataErrors=True, NotifyOnValidationError=True}" />

<!-- 7. 相对源绑定 -->
<TextBlock Text="{Binding RelativeSource={RelativeSource AncestorType=Window}, Path=Title}" />

<!-- 8. ElementName 绑定 -->
<Slider x:Name="slider" Minimum="0" Maximum="100" />
<TextBlock Text="{Binding ElementName=slider, Path=Value}" />
```

### INotifyPropertyChanged 实现

```csharp
// 传统方式
public class Person : INotifyPropertyChanged
{
    private string _name;
    public string Name
    {
        get => _name;
        set
        {
            if (_name != value)
            {
                _name = value;
                OnPropertyChanged(nameof(Name));
            }
        }
    }

    public event PropertyChangedEventHandler PropertyChanged;

    protected virtual void OnPropertyChanged(string propertyName)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}

// 使用 CallerMemberName
public class PersonModern : INotifyPropertyChanged
{
    private string _name;
    public string Name
    {
        get => _name;
        set => SetProperty(ref _name, value);
    }

    public event PropertyChangedEventHandler PropertyChanged;

    protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value))
            return false;
        
        field = value;
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        return true;
    }
}

// CommunityToolkit.Mvvm 现代方式
public partial class PersonViewModel : ObservableObject
{
    [ObservableProperty]
    private string _name; // 自动生成 Name 属性和通知
}
```

### 值转换器 (IValueConverter)

```csharp
// 布尔转可见性
public class BoolToVisibilityConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if (value is bool boolValue)
            return boolValue ? Visibility.Visible : Visibility.Collapsed;
        return Visibility.Collapsed;
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if (value is Visibility visibility)
            return visibility == Visibility.Visible;
        return false;
    }
}

// 使用
<Window.Resources>
    <local:BoolToVisibilityConverter x:Key="BoolToVisConverter" />
</Window.Resources>

<Button Visibility="{Binding IsEnabled, Converter={StaticResource BoolToVisConverter}}" />
```

### 面试题：绑定性能优化

**问题**: 如何优化数据绑定性能？

**答案**:
```
1. 选择合适的绑定模式
   - 只读数据用 OneWay 而非 TwoWay
   - 静态数据用 OneTime

2. 避免复杂的绑定路径
   ❌ {Binding Person.Company.Address.City.Name}
   ✅ 在ViewModel中展平数据

3. 使用虚拟化
   <ListBox VirtualizingPanel.IsVirtualizing="True" 
            VirtualizingPanel.VirtualizationMode="Recycling" />

4. 批量更新时暂停绑定
   BindingOperations.DisableCollectionSynchronization(collection, lockObject);

5. 避免频繁的PropertyChanged通知
   - 使用防抖动（Debounce）
   - 批量更新后统一通知

6. 使用编译绑定（x:Bind in UWP）
   - WPF不支持，但可参考思路
   - 考虑迁移到MAUI/WinUI 3
```

---

## 4️⃣ 依赖属性详解

### 什么是依赖属性？

**依赖属性（Dependency Property）** 是WPF属性系统的核心，提供：
- 数据绑定支持
- 样式和模板支持
- 属性值继承
- 变更通知
- 默认值和强制值

### 依赖属性定义

```csharp
public class MyControl : Control
{
    // 1. 注册依赖属性
    public static readonly DependencyProperty TitleProperty =
        DependencyProperty.Register(
            name: "Title",                          // 属性名
            propertyType: typeof(string),           // 属性类型
            ownerType: typeof(MyControl),           // 所有者类型
            typeMetadata: new PropertyMetadata(    // 元数据
                defaultValue: string.Empty,         // 默认值
                propertyChangedCallback: OnTitleChanged  // 变更回调
            )
        );

    // 2. CLR 属性包装器
    public string Title
    {
        get => (string)GetValue(TitleProperty);
        set => SetValue(TitleProperty, value);
    }

    // 3. 属性变更回调
    private static void OnTitleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var control = (MyControl)d;
        string oldValue = (string)e.OldValue;
        string newValue = (string)e.NewValue;
        
        // 处理属性变更逻辑
        control.OnTitleChangedImpl(oldValue, newValue);
    }

    private void OnTitleChangedImpl(string oldValue, string newValue)
    {
        // 实例方法，可以访问实例成员
    }
}
```

### 依赖属性特性

#### 1. 属性值继承
```csharp
// 父控件设置字体大小，子控件自动继承
<StackPanel FontSize="20">
    <TextBlock Text="继承了FontSize=20" />
</StackPanel>

// 代码实现
public static readonly DependencyProperty MyInheritedProperty =
    DependencyProperty.Register(
        "MyInherited",
        typeof(string),
        typeof(MyControl),
        new FrameworkPropertyMetadata(
            "DefaultValue",
            FrameworkPropertyMetadataOptions.Inherits  // 启用继承
        )
    );
```

#### 2. 属性值强制（Coercion）
```csharp
public static readonly DependencyProperty AgeProperty =
    DependencyProperty.Register(
        "Age",
        typeof(int),
        typeof(Person),
        new PropertyMetadata(0, null, CoerceAge)  // 强制回调
    );

private static object CoerceAge(DependencyObject d, object baseValue)
{
    int age = (int)baseValue;
    
    // 限制年龄范围
    if (age < 0) return 0;
    if (age > 150) return 150;
    
    return age;
}
```

#### 3. 属性验证
```csharp
public static readonly DependencyProperty EmailProperty =
    DependencyProperty.Register(
        "Email",
        typeof(string),
        typeof(User),
        new PropertyMetadata(""),
        ValidateEmail  // 验证回调
    );

private static bool ValidateEmail(object value)
{
    string email = value as string;
    if (string.IsNullOrEmpty(email))
        return false;
    
    // 简单邮箱验证
    return email.Contains("@");
}
```

### 附加属性（Attached Property）

```csharp
// 定义附加属性
public class GridHelper
{
    public static readonly DependencyProperty RowProperty =
        DependencyProperty.RegisterAttached(
            "Row",
            typeof(int),
            typeof(GridHelper),
            new PropertyMetadata(0)
        );

    public static int GetRow(DependencyObject obj)
    {
        return (int)obj.GetValue(RowProperty);
    }

    public static void SetRow(DependencyObject obj, int value)
    {
        obj.SetValue(RowProperty, value);
    }
}

// 使用
<Button local:GridHelper.Row="1" />
```

### 面试题：依赖属性 vs CLR属性

**问题**: 为什么WPF使用依赖属性而不是普通CLR属性？

**答案**:
```
依赖属性的优势：

1. 内存效率
   - CLR属性：每个实例都占用内存
   - 依赖属性：只在赋值时才占用内存，共享默认值
   - 示例：1000个Button，只有10个设置了Background
     CLR：1000个内存槽位
     依赖属性：10个实际值 + 默认值引用

2. 功能丰富
   - 支持数据绑定
   - 支持样式和模板
   - 支持动画
   - 支持属性值继承
   - 支持属性变更通知
   - 支持值强制和验证

3. 性能考虑
   - 优点：内存占用少，共享默认值
   - 缺点：访问速度比CLR属性慢（字典查找）
   - 最佳实践：高频访问的属性考虑缓存

4. 何时使用普通CLR属性？
   - 不需要绑定、样式、动画
   - 性能敏感的内部实现
   - 简单的ViewModel属性
```

---

## 5️⃣ 路由事件系统

### 路由事件类型

```csharp
public enum RoutingStrategy
{
    Bubble,   // 冒泡：从子元素向父元素传播
    Tunnel,   // 隧道：从父元素向子元素传播（Preview事件）
    Direct    // 直接：只在触发元素上触发
}
```

### 事件传播示例

```xml
<Window PreviewMouseDown="Window_PreviewMouseDown"
        MouseDown="Window_MouseDown">
    <StackPanel PreviewMouseDown="StackPanel_PreviewMouseDown"
                MouseDown="StackPanel_MouseDown">
        <Button PreviewMouseDown="Button_PreviewMouseDown"
                MouseDown="Button_MouseDown"
                Content="Click Me" />
    </StackPanel>
</Window>
```

**点击Button的事件触发顺序**:
```
1. Window_PreviewMouseDown        (隧道 ↓)
2. StackPanel_PreviewMouseDown    (隧道 ↓)
3. Button_PreviewMouseDown        (隧道 ↓)
4. Button_MouseDown               (冒泡 ↑)
5. StackPanel_MouseDown           (冒泡 ↑)
6. Window_MouseDown               (冒泡 ↑)
```

### 注册路由事件

```csharp
public class MyButton : Button
{
    // 1. 注册路由事件
    public static readonly RoutedEvent ClickedEvent =
        EventManager.RegisterRoutedEvent(
            name: "Clicked",
            routingStrategy: RoutingStrategy.Bubble,
            handlerType: typeof(RoutedEventHandler),
            ownerType: typeof(MyButton)
        );

    // 2. CLR 事件包装器
    public event RoutedEventHandler Clicked
    {
        add { AddHandler(ClickedEvent, value); }
        remove { RemoveHandler(ClickedEvent, value); }
    }

    // 3. 触发事件
    protected virtual void OnClicked()
    {
        RoutedEventArgs args = new RoutedEventArgs(ClickedEvent, this);
        RaiseEvent(args);
    }
}
```

### 事件处理

```csharp
// 停止事件传播
private void Button_MouseDown(object sender, MouseButtonEventArgs e)
{
    // 处理事件
    DoSomething();
    
    // 标记为已处理，停止继续传播
    e.Handled = true;
}

// 即使Handled也响应
myButton.AddHandler(Button.ClickEvent, new RoutedEventHandler(Button_Click), 
    handledEventsToo: true);
```

### 面试题：路由事件的应用场景

**问题**: 何时使用隧道事件（Preview）vs 冒泡事件？

**答案**:
```
使用 Preview 事件（隧道）的场景：

1. 事件预处理
   - 在子控件处理前拦截
   - 统一验证或过滤
   
2. 阻止默认行为
   PreviewKeyDown="PreviewKeyDown_Handler"
   private void PreviewKeyDown_Handler(object sender, KeyEventArgs e)
   {
       if (e.Key == Key.Enter)
       {
           // 阻止Enter键的默认行为
           e.Handled = true;
       }
   }

3. 全局监听
   - 在根元素统一处理
   - 日志记录、性能监控

使用冒泡事件的场景：

1. 事件委托
   <ListBox MouseDown="ListBox_MouseDown">
       <ListBoxItem>Item 1</ListBoxItem>
       <ListBoxItem>Item 2</ListBoxItem>
   </ListBox>
   
   // 统一在ListBox处理，无需每个Item单独绑定

2. 常规事件处理
   - 大多数情况使用冒泡事件
   - 符合直觉的事件流
```

---

## 6️⃣ 命令系统

### ICommand 接口

```csharp
public interface ICommand
{
    event EventHandler CanExecuteChanged;
    bool CanExecute(object parameter);
    void Execute(object parameter);
}
```

### 实现 ICommand

```csharp
// 简单实现
public class RelayCommand : ICommand
{
    private readonly Action<object> _execute;
    private readonly Func<object, bool> _canExecute;

    public RelayCommand(Action<object> execute, Func<object, bool> canExecute = null)
    {
        _execute = execute ?? throw new ArgumentNullException(nameof(execute));
        _canExecute = canExecute;
    }

    public event EventHandler CanExecuteChanged
    {
        add { CommandManager.RequerySuggested += value; }
        remove { CommandManager.RequerySuggested -= value; }
    }

    public bool CanExecute(object parameter)
    {
        return _canExecute == null || _canExecute(parameter);
    }

    public void Execute(object parameter)
    {
        _execute(parameter);
    }
}

// 泛型版本
public class RelayCommand<T> : ICommand
{
    private readonly Action<T> _execute;
    private readonly Func<T, bool> _canExecute;

    public RelayCommand(Action<T> execute, Func<T, bool> canExecute = null)
    {
        _execute = execute ?? throw new ArgumentNullException(nameof(execute));
        _canExecute = canExecute;
    }

    public event EventHandler CanExecuteChanged
    {
        add { CommandManager.RequerySuggested += value; }
        remove { CommandManager.RequerySuggested -= value; }
    }

    public bool CanExecute(object parameter)
    {
        return _canExecute == null || _canExecute((T)parameter);
    }

    public void Execute(object parameter)
    {
        _execute((T)parameter);
    }
}
```

### ViewModel 中使用命令

```csharp
public class MainViewModel : INotifyPropertyChanged
{
    private string _searchText;
    
    public string SearchText
    {
        get => _searchText;
        set
        {
            _searchText = value;
            OnPropertyChanged();
            // 触发命令状态刷新
            CommandManager.InvalidateRequerySuggested();
        }
    }

    // 命令定义
    public ICommand SearchCommand { get; }
    public ICommand ClearCommand { get; }

    public MainViewModel()
    {
        SearchCommand = new RelayCommand(
            execute: _ => ExecuteSearch(),
            canExecute: _ => !string.IsNullOrWhiteSpace(SearchText)
        );

        ClearCommand = new RelayCommand(
            execute: _ => SearchText = string.Empty
        );
    }

    private void ExecuteSearch()
    {
        // 执行搜索逻辑
        Debug.WriteLine($"Searching for: {SearchText}");
    }

    public event PropertyChangedEventHandler PropertyChanged;
    
    protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}
```

### XAML 绑定命令

```xml
<Window>
    <Window.DataContext>
        <vm:MainViewModel />
    </Window.DataContext>
    
    <StackPanel>
        <!-- 绑定到命令 -->
        <TextBox Text="{Binding SearchText, UpdateSourceTrigger=PropertyChanged}" />
        
        <Button Content="Search" 
                Command="{Binding SearchCommand}" />
        
        <!-- 命令参数 -->
        <Button Content="Delete" 
                Command="{Binding DeleteCommand}" 
                CommandParameter="{Binding SelectedItem}" />
        
        <!-- 输入绑定 -->
        <TextBox>
            <TextBox.InputBindings>
                <KeyBinding Key="Enter" Command="{Binding SearchCommand}" />
            </TextBox.InputBindings>
        </TextBox>
    </StackPanel>
</Window>
```

### CommunityToolkit.Mvvm 命令

```csharp
public partial class MainViewModel : ObservableObject
{
    [ObservableProperty]
    private string _searchText;

    // 自动生成 SearchCommand
    [RelayCommand(CanExecute = nameof(CanSearch))]
    private void Search()
    {
        Debug.WriteLine($"Searching for: {SearchText}");
    }

    private bool CanSearch()
    {
        return !string.IsNullOrWhiteSpace(SearchText);
    }

    // 异步命令
    [RelayCommand]
    private async Task LoadDataAsync()
    {
        await Task.Delay(1000);
        // 加载数据
    }
}
```

### 面试题：命令 vs 事件

**问题**: 为什么MVVM推荐使用命令而不是事件？

**答案**:
```
命令的优势：

1. 解耦合
   - 事件：View直接调用ViewModel方法（代码后置）
   - 命令：通过绑定，纯XAML实现

2. 可测试性
   // 测试命令
   var viewModel = new MainViewModel();
   viewModel.SearchCommand.Execute(null);
   Assert.IsTrue(viewModel.SearchResults.Any());
   
   // 测试事件需要模拟UI交互

3. 可重用性
   同一个命令可绑定到多个控件
   <Button Command="{Binding SaveCommand}" />
   <MenuItem Command="{Binding SaveCommand}" />
   <KeyBinding Key="S" Modifiers="Control" Command="{Binding SaveCommand}" />

4. 状态管理
   CanExecute自动控制UI状态
   按钮自动启用/禁用，无需手动管理

5. 何时使用事件？
   - 纯UI逻辑（如动画、布局）
   - 性能敏感场景
   - 遗留代码维护
```

---

## 7️⃣ 样式与模板

### 样式（Style）

```xml
<!-- 1. 基础样式 -->
<Style x:Key="MyButtonStyle" TargetType="Button">
    <Setter Property="Background" Value="LightBlue" />
    <Setter Property="Foreground" Value="White" />
    <Setter Property="FontSize" Value="16" />
    <Setter Property="Padding" Value="10,5" />
</Style>

<Button Style="{StaticResource MyButtonStyle}" Content="Styled Button" />

<!-- 2. 隐式样式（自动应用） -->
<Style TargetType="Button">
    <Setter Property="Margin" Value="5" />
</Style>

<!-- 所有Button自动应用 -->

<!-- 3. 样式继承 -->
<Style x:Key="BaseButtonStyle" TargetType="Button">
    <Setter Property="FontSize" Value="14" />
</Style>

<Style x:Key="PrimaryButtonStyle" TargetType="Button" BasedOn="{StaticResource BaseButtonStyle}">
    <Setter Property="Background" Value="Blue" />
    <Setter Property="Foreground" Value="White" />
</Style>

<!-- 4. 触发器 -->
<Style TargetType="Button">
    <Setter Property="Background" Value="LightGray" />
    <Style.Triggers>
        <!-- 属性触发器 -->
        <Trigger Property="IsMouseOver" Value="True">
            <Setter Property="Background" Value="LightBlue" />
        </Trigger>
        
        <!-- 数据触发器 -->
        <DataTrigger Binding="{Binding IsActive}" Value="True">
            <Setter Property="Background" Value="Green" />
        </DataTrigger>
        
        <!-- 多条件触发器 -->
        <MultiTrigger>
            <MultiTrigger.Conditions>
                <Condition Property="IsMouseOver" Value="True" />
                <Condition Property="IsPressed" Value="True" />
            </MultiTrigger.Conditions>
            <Setter Property="Background" Value="DarkBlue" />
        </MultiTrigger>
        
        <!-- 事件触发器 -->
        <EventTrigger RoutedEvent="MouseEnter">
            <BeginStoryboard>
                <Storyboard>
                    <DoubleAnimation Storyboard.TargetProperty="Opacity"
                                   To="0.5" Duration="0:0:0.3" />
                </Storyboard>
            </BeginStoryboard>
        </EventTrigger>
    </Style.Triggers>
</Style>
```

### 控件模板（ControlTemplate）

```xml
<!-- 完全自定义Button外观 -->
<Style TargetType="Button">
    <Setter Property="Template">
        <Setter.Value>
            <ControlTemplate TargetType="Button">
                <Border x:Name="border"
                        Background="{TemplateBinding Background}"
                        BorderBrush="{TemplateBinding BorderBrush}"
                        BorderThickness="2"
                        CornerRadius="5">
                    <ContentPresenter HorizontalAlignment="Center"
                                    VerticalAlignment="Center"
                                    Margin="{TemplateBinding Padding}" />
                </Border>
                
                <!-- 模板触发器 -->
                <ControlTemplate.Triggers>
                    <Trigger Property="IsMouseOver" Value="True">
                        <Setter TargetName="border" Property="Background" Value="LightBlue" />
                    </Trigger>
                    <Trigger Property="IsPressed" Value="True">
                        <Setter TargetName="border" Property="Background" Value="DarkBlue" />
                    </Trigger>
                    <Trigger Property="IsEnabled" Value="False">
                        <Setter TargetName="border" Property="Opacity" Value="0.5" />
                    </Trigger>
                </ControlTemplate.Triggers>
            </ControlTemplate>
        </Setter.Value>
    </Setter>
</Style>
```

### 数据模板（DataTemplate）

```xml
<!-- 定义数据显示方式 -->
<DataTemplate x:Key="PersonTemplate">
    <StackPanel Orientation="Horizontal" Margin="5">
        <Image Source="{Binding Avatar}" Width="50" Height="50" />
        <StackPanel Margin="10,0">
            <TextBlock Text="{Binding Name}" FontWeight="Bold" />
            <TextBlock Text="{Binding Email}" Foreground="Gray" />
        </StackPanel>
    </StackPanel>
</DataTemplate>

<!-- 使用数据模板 -->
<ListBox ItemsSource="{Binding People}"
         ItemTemplate="{StaticResource PersonTemplate}" />

<!-- 数据模板选择器 -->
<local:PersonDataTemplateSelector x:Key="PersonTemplateSelector">
    <local:PersonDataTemplateSelector.AdminTemplate>
        <DataTemplate>
            <Border Background="Gold">
                <TextBlock Text="{Binding Name}" FontWeight="Bold" />
            </Border>
        </DataTemplate>
    </local:PersonDataTemplateSelector.AdminTemplate>
    <local:PersonDataTemplateSelector.UserTemplate>
        <DataTemplate>
            <TextBlock Text="{Binding Name}" />
        </DataTemplate>
    </local:PersonDataTemplateSelector.UserTemplate>
</local:PersonDataTemplateSelector>

<ListBox ItemsSource="{Binding People}"
         ItemTemplateSelector="{StaticResource PersonTemplateSelector}" />
```

```csharp
// DataTemplateSelector 实现
public class PersonDataTemplateSelector : DataTemplateSelector
{
    public DataTemplate AdminTemplate { get; set; }
    public DataTemplate UserTemplate { get; set; }

    public override DataTemplate SelectTemplate(object item, DependencyObject container)
    {
        if (item is Person person)
        {
            return person.IsAdmin ? AdminTemplate : UserTemplate;
        }
        return base.SelectTemplate(item, container);
    }
}
```

### 面试题：Style vs Template

**问题**: Style、ControlTemplate、DataTemplate的区别和使用场景？

**答案**:
```
1. Style（样式）
   - 作用：修改控件的属性值
   - 不改变控件结构
   - 示例：改变颜色、字体、边距
   - 使用场景：
     ✓ 统一界面风格
     ✓ 主题切换
     ✓ 简单外观调整

2. ControlTemplate（控件模板）
   - 作用：完全重定义控件的视觉树
   - 改变控件结构和外观
   - 保留控件行为
   - 使用场景：
     ✓ 完全自定义控件外观
     ✓ 创建独特的UI设计
     ✓ 开发自定义控件

3. DataTemplate（数据模板）
   - 作用：定义数据对象的显示方式
   - 用于ItemsControl的Item显示
   - 使用场景：
     ✓ ListBox/ListView的Item显示
     ✓ ComboBox下拉项显示
     ✓ ContentControl的Content显示

比喻：
- Style: 给人换衣服（外观属性）
- ControlTemplate: 整容手术（改变外观结构）
- DataTemplate: 证件照格式（数据显示格式）

溜云库应用：
- 瀑布流图片：DataTemplate定义图片卡片样式
- 自定义窗口：ControlTemplate重定义Window外观
- 主题系统：Style统一控件风格
```

---

*由于内容较长，WPF专家级能力准备文档将继续在下一部分...*

**已完成章节**: 1-7
**待续章节**: 8-11 (MVVM框架实战、自定义控件开发、Visual Layer、多线程与异步)

是否继续生成剩余内容？
