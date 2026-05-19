01-WPF专家级能力-续.md
# WPF专家级能力准备（续）

> 续前文：MVVM框架实战、自定义控件开发、Visual Layer、多线程与异步编程

---

## 8️⃣ MVVM框架实战

### MVVM架构模式

```
┌─────────────────────────────────────────────┐
│              View (XAML)                    │
│  - 用户界面                                  │
│  - 纯展示逻辑                                │
└─────────────┬───────────────────────────────┘
              │ DataBinding
              │ Command Binding
┌─────────────▼───────────────────────────────┐
│           ViewModel                         │
│  - 视图逻辑                                  │
│  - 数据绑定属性                              │
│  - 命令实现                                  │
│  - UI状态管理                                │
└─────────────┬───────────────────────────────┘
              │ 调用
┌─────────────▼───────────────────────────────┐
│              Model                          │
│  - 业务逻辑                                  │
│  - 数据访问                                  │
│  - 领域对象                                  │
└─────────────────────────────────────────────┘
```

### Caliburn.Micro 实战

**溜云库使用的框架**

```csharp
// 1. Bootstrapper 配置
public class AppBootstrapper : BootstrapperBase
{
    private SimpleContainer _container;

    public AppBootstrapper()
    {
        Initialize();
    }

    protected override void Configure()
    {
        _container = new SimpleContainer();
        
        // 注册框架服务
        _container.Singleton<IWindowManager, WindowManager>();
        _container.Singleton<IEventAggregator, EventAggregator>();
        
        // 注册ViewModel
        _container.PerRequest<ShellViewModel>();
        _container.PerRequest<DetailViewModel>();
    }

    protected override object GetInstance(Type service, string key)
    {
        return _container.GetInstance(service, key);
    }

    protected override IEnumerable<object> GetAllInstances(Type service)
    {
        return _container.GetAllInstances(service);
    }

    protected override void OnStartup(object sender, StartupEventArgs e)
    {
        DisplayRootViewFor<ShellViewModel>();
    }
}

// 2. ViewModel实现
public class ShellViewModel : Conductor<object>
{
    private readonly IWindowManager _windowManager;
    private readonly IEventAggregator _events;

    public ShellViewModel(IWindowManager windowManager, IEventAggregator events)
    {
        _windowManager = windowManager;
        _events = events;
    }

    // 约定：属性名 对应 XAML中的控件名
    private string _title = "My Application";
    public string Title
    {
        get => _title;
        set
        {
            _title = value;
            NotifyOfPropertyChange(() => Title);
        }
    }

    // 约定：方法名 对应 按钮名称
    public void OpenDetail()
    {
        // 打开详情窗口
        _windowManager.ShowDialog(IoC.Get<DetailViewModel>());
    }

    // CanXXX 控制按钮状态
    public bool CanOpenDetail => SelectedItem != null;

    // 协程支持
    public IEnumerable<IResult> LoadData()
    {
        yield return new ShowBusyIndicator();
        yield return new LoadDataResult();
        yield return new HideBusyIndicator();
    }
}

// 3. 视图约定
// ShellViewModel.cs → ShellView.xaml (自动关联)
```

### CommunityToolkit.Mvvm 实战

**现代推荐框架**

```csharp
// 1. ViewModel 基类
public partial class MainViewModel : ObservableObject
{
    // 自动生成属性和通知
    [ObservableProperty]
    private string _title = "My App";

    [ObservableProperty]
    private ObservableCollection<Item> _items = new();

    [ObservableProperty]
    private Item _selectedItem;

    // 自动生成命令
    [RelayCommand]
    private void AddItem()
    {
        Items.Add(new Item { Name = $"Item {Items.Count + 1}" });
    }

    [RelayCommand(CanExecute = nameof(CanDeleteItem))]
    private void DeleteItem()
    {
        if (SelectedItem != null)
        {
            Items.Remove(SelectedItem);
        }
    }

    private bool CanDeleteItem() => SelectedItem != null;

    // 异步命令
    [RelayCommand]
    private async Task LoadDataAsync(CancellationToken cancellationToken)
    {
        try
        {
            var data = await _dataService.GetDataAsync(cancellationToken);
            Items = new ObservableCollection<Item>(data);
        }
        catch (Exception ex)
        {
            // 错误处理
        }
    }

    // 属性变更时自动调用
    partial void OnSelectedItemChanged(Item value)
    {
        // SelectedItem 变更时的逻辑
        DeleteItemCommand.NotifyCanExecuteChanged();
    }
}
```

### Prism 实战

**企业级框架**

```csharp
// 1. App配置
public partial class App : PrismApplication
{
    protected override Window CreateShell()
    {
        return Container.Resolve<MainWindow>();
    }

    protected override void RegisterTypes(IContainerRegistry containerRegistry)
    {
        // 注册导航
        containerRegistry.RegisterForNavigation<ViewA, ViewAViewModel>();
        containerRegistry.RegisterForNavigation<ViewB, ViewBViewModel>();
        
        // 注册服务
        containerRegistry.RegisterSingleton<IDataService, DataService>();
        
        // 注册对话框
        containerRegistry.RegisterDialog<ConfirmDialog, ConfirmDialogViewModel>();
    }

    protected override void ConfigureModuleCatalog(IModuleCatalog moduleCatalog)
    {
        // 模块化
        moduleCatalog.AddModule<ModuleA>();
        moduleCatalog.AddModule<ModuleB>();
    }
}

// 2. ViewModel实现
public class MainViewModel : BindableBase
{
    private readonly IRegionManager _regionManager;
    private readonly IDialogService _dialogService;
    private readonly IEventAggregator _eventAggregator;

    public MainViewModel(IRegionManager regionManager, 
                        IDialogService dialogService,
                        IEventAggregator eventAggregator)
    {
        _regionManager = regionManager;
        _dialogService = dialogService;
        _eventAggregator = eventAggregator;
        
        NavigateCommand = new DelegateCommand<string>(Navigate);
        ShowDialogCommand = new DelegateCommand(ShowDialog);
        
        // 订阅事件
        _eventAggregator.GetEvent<DataUpdatedEvent>().Subscribe(OnDataUpdated);
    }

    public DelegateCommand<string> NavigateCommand { get; }
    public DelegateCommand ShowDialogCommand { get; }

    private void Navigate(string viewName)
    {
        _regionManager.RequestNavigate("ContentRegion", viewName);
    }

    private void ShowDialog()
    {
        _dialogService.ShowDialog("ConfirmDialog", result =>
        {
            if (result.Result == ButtonResult.OK)
            {
                // 用户点击确认
            }
        });
    }

    private void OnDataUpdated(DataModel data)
    {
        // 处理数据更新事件
    }
}

// 3. 区域定义
<Window>
    <DockPanel>
        <Menu DockPanel.Dock="Top" />
        <ContentControl prism:RegionManager.RegionName="ContentRegion" />
    </DockPanel>
</Window>
```

### 面试题：MVVM框架对比

**问题**: Caliburn.Micro、CommunityToolkit.Mvvm、Prism如何选择？

| 特性 | Caliburn.Micro | CommunityToolkit.Mvvm | Prism |
|------|---------------|---------------------|-------|
| **学习曲线** | 中等 | 简单 | 陡峭 |
| **约定式开发** | ✅ 强约定 | ❌ | ❌ |
| **代码生成** | ❌ | ✅ Source Generator | ❌ |
| **模块化** | ❌ | ❌ | ✅ 完整支持 |
| **导航** | ✅ Conductor | ❌ 需自己实现 | ✅ RegionManager |
| **事件聚合** | ✅ | ✅ Messenger | ✅ EventAggregator |
| **依赖注入** | 简单容器 | ❌ 框架无关 | ✅ 完整DI |
| **性能** | 中等 | ⭐⭐⭐⭐⭐ | 中等 |
| **社区活跃度** | 较低 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ |

**选择建议**:
```
1. CommunityToolkit.Mvvm (推荐新项目)
   ✓ 微软官方支持
   ✓ 现代化，Source Generator
   ✓ 性能最佳
   ✓ 轻量级，易于集成

2. Prism (企业级大型项目)
   ✓ 模块化架构
   ✓ 完整的导航系统
   ✓ 适合团队协作
   ✗ 学习成本高

3. Caliburn.Micro (遗留项目)
   ✓ 约定优于配置
   ✓ 快速开发
   ✗ 更新缓慢
   ✗ 不支持.NET 8新特性

溜云库现状与建议：
- 当前：Caliburn.Micro 4.0.212
- 建议：逐步迁移到 CommunityToolkit.Mvvm
  - 保留现有ViewModel结构
  - 新模块使用新框架
  - 渐进式重构
```

---

## 9️⃣ 自定义控件开发

### 自定义控件的方式

```
1. UserControl（用户控件）
   - 组合现有控件
   - 快速开发
   - 不可重写样式

2. Custom Control（自定义控件）
   - 继承现有控件
   - 完全自定义
   - 可样式化

3. Attached Behavior（附加行为）
   - 扩展现有控件
   - 无需继承
```

### UserControl 示例

```xml
<!-- ImageCard.xaml -->
<UserControl x:Class="MyApp.Controls.ImageCard"
             xmlns="...">
    <Border BorderBrush="Gray" BorderThickness="1" CornerRadius="5">
        <Grid>
            <Grid.RowDefinitions>
                <RowDefinition Height="*" />
                <RowDefinition Height="Auto" />
            </Grid.RowDefinitions>
            
            <Image Source="{Binding Image, RelativeSource={RelativeSource AncestorType=UserControl}}" 
                   Stretch="UniformToFill" />
            
            <StackPanel Grid.Row="1" Margin="10">
                <TextBlock Text="{Binding Title, RelativeSource={RelativeSource AncestorType=UserControl}}" 
                           FontWeight="Bold" />
                <TextBlock Text="{Binding Description, RelativeSource={RelativeSource AncestorType=UserControl}}" 
                           TextWrapping="Wrap" />
            </StackPanel>
        </Grid>
    </Border>
</UserControl>
```

```csharp
// ImageCard.xaml.cs
public partial class ImageCard : UserControl
{
    public static readonly DependencyProperty ImageProperty =
        DependencyProperty.Register("Image", typeof(ImageSource), typeof(ImageCard));

    public static readonly DependencyProperty TitleProperty =
        DependencyProperty.Register("Title", typeof(string), typeof(ImageCard));

    public static readonly DependencyProperty DescriptionProperty =
        DependencyProperty.Register("Description", typeof(string), typeof(ImageCard));

    public ImageSource Image
    {
        get => (ImageSource)GetValue(ImageProperty);
        set => SetValue(ImageProperty, value);
    }

    public string Title
    {
        get => (string)GetValue(TitleProperty);
        set => SetValue(TitleProperty, value);
    }

    public string Description
    {
        get => (string)GetValue(DescriptionProperty);
        set => SetValue(DescriptionProperty, value);
    }

    public ImageCard()
    {
        InitializeComponent();
    }
}
```

### Custom Control 示例

```csharp
// NumericUpDown.cs
[TemplatePart(Name = "PART_TextBox", Type = typeof(TextBox))]
[TemplatePart(Name = "PART_UpButton", Type = typeof(Button))]
[TemplatePart(Name = "PART_DownButton", Type = typeof(Button))]
public class NumericUpDown : Control
{
    private TextBox _textBox;
    private Button _upButton;
    private Button _downButton;

    static NumericUpDown()
    {
        DefaultStyleKeyProperty.OverrideMetadata(typeof(NumericUpDown),
            new FrameworkPropertyMetadata(typeof(NumericUpDown)));
    }

    // 依赖属性
    public static readonly DependencyProperty ValueProperty =
        DependencyProperty.Register("Value", typeof(double), typeof(NumericUpDown),
            new FrameworkPropertyMetadata(0.0, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                OnValueChanged, CoerceValue));

    public static readonly DependencyProperty MinimumProperty =
        DependencyProperty.Register("Minimum", typeof(double), typeof(NumericUpDown),
            new PropertyMetadata(0.0, OnMinMaxChanged));

    public static readonly DependencyProperty MaximumProperty =
        DependencyProperty.Register("Maximum", typeof(double), typeof(NumericUpDown),
            new PropertyMetadata(100.0, OnMinMaxChanged));

    public static readonly DependencyProperty IncrementProperty =
        DependencyProperty.Register("Increment", typeof(double), typeof(NumericUpDown),
            new PropertyMetadata(1.0));

    public double Value
    {
        get => (double)GetValue(ValueProperty);
        set => SetValue(ValueProperty, value);
    }

    public double Minimum
    {
        get => (double)GetValue(MinimumProperty);
        set => SetValue(MinimumProperty, value);
    }

    public double Maximum
    {
        get => (double)GetValue(MaximumProperty);
        set => SetValue(MaximumProperty, value);
    }

    public double Increment
    {
        get => (double)GetValue(IncrementProperty);
        set => SetValue(IncrementProperty, value);
    }

    // 模板应用
    public override void OnApplyTemplate()
    {
        base.OnApplyTemplate();

        // 移除旧事件
        if (_upButton != null)
            _upButton.Click -= UpButton_Click;
        if (_downButton != null)
            _downButton.Click -= DownButton_Click;

        // 获取模板部件
        _textBox = GetTemplateChild("PART_TextBox") as TextBox;
        _upButton = GetTemplateChild("PART_UpButton") as Button;
        _downButton = GetTemplateChild("PART_DownButton") as Button;

        // 绑定事件
        if (_upButton != null)
            _upButton.Click += UpButton_Click;
        if (_downButton != null)
            _downButton.Click += DownButton_Click;

        UpdateTextBox();
    }

    private void UpButton_Click(object sender, RoutedEventArgs e)
    {
        Value += Increment;
    }

    private void DownButton_Click(object sender, RoutedEventArgs e)
    {
        Value -= Increment;
    }

    private static void OnValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var control = (NumericUpDown)d;
        control.UpdateTextBox();
        control.RaiseValueChangedEvent((double)e.OldValue, (double)e.NewValue);
    }

    private static object CoerceValue(DependencyObject d, object baseValue)
    {
        var control = (NumericUpDown)d;
        double value = (double)baseValue;
        
        if (value < control.Minimum)
            return control.Minimum;
        if (value > control.Maximum)
            return control.Maximum;
        
        return value;
    }

    private static void OnMinMaxChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var control = (NumericUpDown)d;
        control.CoerceValue(ValueProperty);
    }

    private void UpdateTextBox()
    {
        if (_textBox != null)
            _textBox.Text = Value.ToString("F2");
    }

    // 路由事件
    public static readonly RoutedEvent ValueChangedEvent =
        EventManager.RegisterRoutedEvent("ValueChanged", RoutingStrategy.Bubble,
            typeof(RoutedPropertyChangedEventHandler<double>), typeof(NumericUpDown));

    public event RoutedPropertyChangedEventHandler<double> ValueChanged
    {
        add { AddHandler(ValueChangedEvent, value); }
        remove { RemoveHandler(ValueChangedEvent, value); }
    }

    private void RaiseValueChangedEvent(double oldValue, double newValue)
    {
        var args = new RoutedPropertyChangedEventArgs<double>(oldValue, newValue, ValueChangedEvent);
        RaiseEvent(args);
    }
}
```

```xml
<!-- Themes/Generic.xaml -->
<Style TargetType="{x:Type local:NumericUpDown}">
    <Setter Property="Template">
        <Setter.Value>
            <ControlTemplate TargetType="{x:Type local:NumericUpDown}">
                <Border Background="{TemplateBinding Background}"
                        BorderBrush="{TemplateBinding BorderBrush}"
                        BorderThickness="{TemplateBinding BorderThickness}">
                    <Grid>
                        <Grid.ColumnDefinitions>
                            <ColumnDefinition Width="*" />
                            <ColumnDefinition Width="Auto" />
                        </Grid.ColumnDefinitions>
                        
                        <TextBox x:Name="PART_TextBox" 
                                 Grid.Column="0"
                                 BorderThickness="0" />
                        
                        <StackPanel Grid.Column="1">
                            <Button x:Name="PART_UpButton" Content="▲" Width="20" Height="15" />
                            <Button x:Name="PART_DownButton" Content="▼" Width="20" Height="15" />
                        </StackPanel>
                    </Grid>
                </Border>
            </ControlTemplate>
        </Setter.Value>
    </Setter>
</Style>
```

### Attached Behavior 示例

```csharp
// WatermarkBehavior.cs
public static class WatermarkBehavior
{
    public static readonly DependencyProperty WatermarkProperty =
        DependencyProperty.RegisterAttached(
            "Watermark",
            typeof(string),
            typeof(WatermarkBehavior),
            new PropertyMetadata(null, OnWatermarkChanged));

    public static string GetWatermark(DependencyObject obj)
    {
        return (string)obj.GetValue(WatermarkProperty);
    }

    public static void SetWatermark(DependencyObject obj, string value)
    {
        obj.SetValue(WatermarkProperty, value);
    }

    private static void OnWatermarkChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is TextBox textBox)
        {
            textBox.GotFocus -= TextBox_GotFocus;
            textBox.LostFocus -= TextBox_LostFocus;

            if (e.NewValue != null)
            {
                textBox.GotFocus += TextBox_GotFocus;
                textBox.LostFocus += TextBox_LostFocus;
                UpdateWatermark(textBox);
            }
        }
    }

    private static void TextBox_GotFocus(object sender, RoutedEventArgs e)
    {
        var textBox = (TextBox)sender;
        if (textBox.Text == GetWatermark(textBox))
        {
            textBox.Text = string.Empty;
            textBox.Foreground = Brushes.Black;
        }
    }

    private static void TextBox_LostFocus(object sender, RoutedEventArgs e)
    {
        UpdateWatermark((TextBox)sender);
    }

    private static void UpdateWatermark(TextBox textBox)
    {
        if (string.IsNullOrEmpty(textBox.Text))
        {
            textBox.Text = GetWatermark(textBox);
            textBox.Foreground = Brushes.Gray;
        }
    }
}

// 使用
<TextBox local:WatermarkBehavior.Watermark="请输入搜索内容..." />
```

### 面试题：自定义控件最佳实践

**问题**: 开发自定义控件时需要注意什么？

**答案**:
```
1. 选择合适的方式
   - 简单组合 → UserControl
   - 完全自定义 → Custom Control
   - 扩展现有 → Attached Behavior

2. 依赖属性设计
   ✓ 使用FrameworkPropertyMetadata设置绑定选项
   ✓ 提供默认值
   ✓ 实现属性变更回调
   ✓ 使用CoerceValueCallback限制值范围

3. 模板设计
   ✓ 使用TemplatePart标记关键部件
   ✓ 在OnApplyTemplate中获取部件
   ✓ 检查部件是否为null
   ✓ 解除旧事件绑定

4. 性能考虑
   ✓ 避免频繁的PropertyChanged通知
   ✓ 使用虚拟化（VirtualizingPanel）
   ✓ 懒加载复杂内容
   ✓ 缓存测量和布局结果

5. 可样式化
   ✓ 暴露关键属性为依赖属性
   ✓ 使用TemplateBinding
   ✓ 提供默认样式

6. 可访问性
   ✓ 实现AutomationPeer
   ✓ 支持键盘导航
   ✓ 提供合适的焦点视觉效果

溜云库自定义控件分析：
- MImageWaterfallView: 瀑布流控件
  → 需要虚拟化优化
  → 图片懒加载
  → 滚动性能优化

- MLZWindowLib: 自定义窗口
  → 无边框窗口拖拽
  → 阴影效果
  → 最大化/最小化/关闭逻辑
```

---

## 🔟 Visual Layer高性能绘制

### WPF渲染层次

```
┌─────────────────────────────────┐
│   Application / Logical Layer   │ 高级API (Controls, Shapes)
├─────────────────────────────────┤
│       Visual Layer              │ 中级API (DrawingVisual, ContainerVisual)
├─────────────────────────────────┤
│       Media Layer               │ 低级API (RenderTargetBitmap, WriteableBitmap)
├─────────────────────────────────┤
│       DirectX / GPU             │ 硬件加速
└─────────────────────────────────┘
```

### DrawingVisual 基础

```csharp
public class MyVisualHost : FrameworkElement
{
    private VisualCollection _children;

    public MyVisualHost()
    {
        _children = new VisualCollection(this);
        
        // 创建视觉对象
        CreateDrawingVisualRectangle();
    }

    private void CreateDrawingVisualRectangle()
    {
        DrawingVisual drawingVisual = new DrawingVisual();
        
        using (DrawingContext dc = drawingVisual.RenderOpen())
        {
            // 绘制矩形
            dc.DrawRectangle(Brushes.LightBlue, new Pen(Brushes.Black, 2), 
                new Rect(10, 10, 100, 100));
            
            // 绘制文本
            FormattedText text = new FormattedText(
                "Hello Visual Layer",
                CultureInfo.CurrentCulture,
                FlowDirection.LeftToRight,
                new Typeface("Arial"),
                16,
                Brushes.Black,
                VisualTreeHelper.GetDpi(this).PixelsPerDip);
            
            dc.DrawText(text, new Point(20, 50));
        }
        
        _children.Add(drawingVisual);
    }

    // 必须实现的属性和方法
    protected override int VisualChildrenCount => _children.Count;

    protected override Visual GetVisualChild(int index)
    {
        if (index < 0 || index >= _children.Count)
            throw new ArgumentOutOfRangeException();
        
        return _children[index];
    }
}
```

### 高性能图表示例

```csharp
// 使用Visual Layer绘制大量点
public class PerformanceChart : FrameworkElement
{
    private VisualCollection _visuals;
    private DrawingVisual _chartVisual;

    public PerformanceChart()
    {
        _visuals = new VisualCollection(this);
        _chartVisual = new DrawingVisual();
        _visuals.Add(_chartVisual);
    }

    public void DrawChart(List<Point> dataPoints)
    {
        using (DrawingContext dc = _chartVisual.RenderOpen())
        {
            // 绘制背景
            dc.DrawRectangle(Brushes.White, null, new Rect(0, 0, ActualWidth, ActualHeight));
            
            // 绘制网格
            Pen gridPen = new Pen(Brushes.LightGray, 0.5);
            for (int i = 0; i < 10; i++)
            {
                double y = i * ActualHeight / 10;
                dc.DrawLine(gridPen, new Point(0, y), new Point(ActualWidth, y));
            }
            
            // 绘制数据线
            if (dataPoints.Count > 1)
            {
                PathGeometry geometry = new PathGeometry();
                PathFigure figure = new PathFigure { StartPoint = dataPoints[0] };
                
                for (int i = 1; i < dataPoints.Count; i++)
                {
                    figure.Segments.Add(new LineSegment(dataPoints[i], true));
                }
                
                geometry.Figures.Add(figure);
                dc.DrawGeometry(null, new Pen(Brushes.Blue, 2), geometry);
            }
            
            // 绘制数据点
            foreach (var point in dataPoints)
            {
                dc.DrawEllipse(Brushes.Red, null, point, 3, 3);
            }
        }
    }

    protected override int VisualChildrenCount => _visuals.Count;
    protected override Visual GetVisualChild(int index) => _visuals[index];
}
```

### 命中测试（Hit Testing）

```csharp
public class InteractiveVisualHost : FrameworkElement
{
    private VisualCollection _children;
    private Dictionary<DrawingVisual, object> _visualToData;

    public InteractiveVisualHost()
    {
        _children = new VisualCollection(this);
        _visualToData = new Dictionary<DrawingVisual, object>();
        
        MouseDown += OnMouseDown;
    }

    private void OnMouseDown(object sender, MouseButtonEventArgs e)
    {
        Point pt = e.GetPosition(this);
        
        // 命中测试
        VisualTreeHelper.HitTest(
            this,
            null,  // filter callback
            HitTestResultCallback,
            new PointHitTestParameters(pt));
    }

    private HitTestResultBehavior HitTestResultCallback(HitTestResult result)
    {
        if (result.VisualHit is DrawingVisual visual)
        {
            if (_visualToData.TryGetValue(visual, out object data))
            {
                // 处理点击事件
                OnVisualClicked(data);
                return HitTestResultBehavior.Stop;
            }
        }
        
        return HitTestResultBehavior.Continue;
    }

    protected virtual void OnVisualClicked(object data)
    {
        // 触发事件或执行逻辑
    }

    protected override int VisualChildrenCount => _children.Count;
    protected override Visual GetVisualChild(int index) => _children[index];
}
```

### 面试题：Visual Layer应用场景

**问题**: 何时使用Visual Layer而不是普通控件？

**答案**:
```
使用Visual Layer的场景：

1. 大量图形绘制
   ✓ 图表（10000+数据点）
   ✓ 游戏画面
   ✓ 粒子效果
   ✓ 热力图

2. 性能关键场景
   ✓ 实时数据可视化
   ✓ 高帧率动画
   ✓ 大量UI元素（虚拟化不适用）

3. 自定义绘制
   ✓ 复杂的几何图形
   ✓ 自定义渲染逻辑
   ✓ 特殊视觉效果

性能对比：
- Shape控件（Rectangle等）: 每个都是完整UIElement
  → 1000个Rectangle = 大量内存和CPU
  
- DrawingVisual: 轻量级视觉对象
  → 1000个Visual = 少量内存，高性能

溜云库应用建议：
- 瀑布流图片预览: 考虑Visual Layer优化
  → 大量缩略图显示
  → 滚动性能提升
  
- 图形编辑器: 使用Visual Layer
  → 自由绘制
  → 图形选择和操作
```

---

## 1️⃣1️⃣ 多线程与异步编程

### Dispatcher 机制

```csharp
// UI线程检查
if (Application.Current.Dispatcher.CheckAccess())
{
    // 当前在UI线程
    UpdateUI();
}
else
{
    // 在后台线程，需要调度到UI线程
    Application.Current.Dispatcher.Invoke(() => UpdateUI());
}

// 异步调度
Application.Current.Dispatcher.BeginInvoke(new Action(() =>
{
    UpdateUI();
}));

// 优先级调度
Dispatcher.BeginInvoke(DispatcherPriority.Background, new Action(() =>
{
    // 低优先级任务
}));
```

### 异步编程最佳实践

```csharp
public class DataViewModel : ObservableObject
{
    private bool _isLoading;
    public bool IsLoading
    {
        get => _isLoading;
        set => SetProperty(ref _isLoading, value);
    }

    // async/await 模式
    [RelayCommand]
    private async Task LoadDataAsync()
    {
        IsLoading = true;
        
        try
        {
            // 后台线程执行
            var data = await Task.Run(() =>
            {
                // CPU密集型操作
                return ProcessLargeData();
            });
            
            // 自动回到UI线程
            Items = new ObservableCollection<Item>(data);
        }
        catch (Exception ex)
        {
            // 错误处理
            await ShowErrorAsync(ex.Message);
        }
        finally
        {
            IsLoading = false;
        }
    }

    // 取消支持
    private CancellationTokenSource _cts;
    
    [RelayCommand]
    private async Task LoadWithCancellationAsync()
    {
        _cts?.Cancel();
        _cts = new CancellationTokenSource();
        
        try
        {
            var data = await LoadDataFromApiAsync(_cts.Token);
            Items = new ObservableCollection<Item>(data);
        }
        catch (OperationCanceledException)
        {
            // 操作被取消
        }
    }

    // 进度报告
    [RelayCommand]
    private async Task LoadWithProgressAsync()
    {
        var progress = new Progress<int>(percent =>
        {
            // 自动在UI线程执行
            ProgressValue = percent;
        });
        
        await Task.Run(() =>
        {
            for (int i = 0; i <= 100; i++)
            {
                Thread.Sleep(50);
                ((IProgress<int>)progress).Report(i);
            }
        });
    }
}
```

### 集合更新与线程安全

```csharp
// 问题：ObservableCollection不是线程安全的
public class UnsafeViewModel : ObservableObject
{
    public ObservableCollection<Item> Items { get; } = new();

    public void AddItemFromBackgroundThread(Item item)
    {
        // ❌ 错误：跨线程访问集合
        Items.Add(item);  // 会抛出异常
    }
}

// 解决方案1：Dispatcher调度
public class SafeViewModel1 : ObservableObject
{
    public ObservableCollection<Item> Items { get; } = new();

    public void AddItemFromBackgroundThread(Item item)
    {
        Application.Current.Dispatcher.Invoke(() =>
        {
            Items.Add(item);
        });
    }
}

// 解决方案2：启用集合同步
public class SafeViewModel2 : ObservableObject
{
    private readonly object _lock = new object();
    public ObservableCollection<Item> Items { get; }

    public SafeViewModel2()
    {
        Items = new ObservableCollection<Item>();
        BindingOperations.EnableCollectionSynchronization(Items, _lock);
    }

    public void AddItemFromBackgroundThread(Item item)
    {
        lock (_lock)
        {
            Items.Add(item);
        }
    }
}

// 解决方案3：批量更新
public class SafeViewModel3 : ObservableObject
{
    public ObservableCollection<Item> Items { get; } = new();

    public async Task LoadItemsAsync()
    {
        // 后台加载
        var items = await Task.Run(() => LoadItemsFromDatabase());
        
        // UI线程批量更新
        Items.Clear();
        foreach (var item in items)
        {
            Items.Add(item);
        }
    }
}
```

### BackgroundWorker（遗留但仍常见）

```csharp
public class LegacyViewModel : ObservableObject
{
    private BackgroundWorker _worker;

    public LegacyViewModel()
    {
        _worker = new BackgroundWorker();
        _worker.WorkerReportsProgress = true;
        _worker.WorkerSupportsCancellation = true;
        
        _worker.DoWork += Worker_DoWork;
        _worker.ProgressChanged += Worker_ProgressChanged;
        _worker.RunWorkerCompleted += Worker_RunWorkerCompleted;
    }

    private void Worker_DoWork(object sender, DoWorkEventArgs e)
    {
        var worker = (BackgroundWorker)sender;
        
        for (int i = 0; i <= 100; i++)
        {
            if (worker.CancellationPending)
            {
                e.Cancel = true;
                return;
            }
            
            Thread.Sleep(50);
            worker.ReportProgress(i);
        }
        
        e.Result = "Completed";
    }

    private void Worker_ProgressChanged(object sender, ProgressChangedEventArgs e)
    {
        ProgressValue = e.ProgressPercentage;
    }

    private void Worker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
    {
        if (e.Cancelled)
        {
            Status = "Cancelled";
        }
        else if (e.Error != null)
        {
            Status = $"Error: {e.Error.Message}";
        }
        else
        {
            Status = (string)e.Result;
        }
    }

    public void Start()
    {
        if (!_worker.IsBusy)
        {
            _worker.RunWorkerAsync();
        }
    }

    public void Cancel()
    {
        if (_worker.IsBusy)
        {
            _worker.CancelAsync();
        }
    }
}
```

### 面试题：WPF多线程最佳实践

**问题**: WPF中如何正确处理多线程？

**答案**:
```
核心原则：
1. UI元素只能在UI线程访问
2. 长时间操作必须异步执行
3. 及时取消不需要的操作

最佳实践：

1. 使用 async/await (推荐)
   ✓ 现代、简洁
   ✓ 自动处理线程切换
   ✓ 异常处理简单
   ✓ 支持取消和进度

2. 避免阻塞UI线程
   ❌ Thread.Sleep()
   ❌ .Result / .Wait()
   ✅ await Task.Delay()
   ✅ async/await

3. 集合更新策略
   - 小量更新: Dispatcher.Invoke
   - 大量更新: 后台准备 + UI批量更新
   - 实时更新: EnableCollectionSynchronization

4. 性能优化
   - 虚拟化长列表
   - 懒加载图片
   - 防抖动（Debounce）搜索
   - 节流（Throttle）滚动事件

5. 调试技巧
   - 使用 Dispatcher.VerifyAccess()检查线程
   - 性能分析工具（PerfView, Visual Studio Profiler）
   - 监控UI线程占用率

溜云库常见场景：
- 图片加载: 异步加载 + 缩略图缓存
- 文件扫描: BackgroundWorker + 进度报告
- 云端同步: async/await + CancellationToken
- 批量下载: Task.WhenAll + 进度聚合
```

---

## 📝 WPF面试重点总结

### 必须掌握的核心概念

1. **依赖属性系统**
   - 注册、元数据、回调
   - 值优先级、继承、强制
   - vs CLR属性

2. **数据绑定**
   - 绑定模式、路径、转换器
   - INotifyPropertyChanged
   - 性能优化

3. **MVVM模式**
   - View、ViewModel、Model分离
   - ICommand实现
   - 框架选择（Caliburn.Micro/CommunityToolkit/Prism）

4. **样式与模板**
   - Style、ControlTemplate、DataTemplate
   - 触发器系统
   - 资源字典

5. **自定义控件**
   - UserControl vs CustomControl
   - TemplatePart、OnApplyTemplate
   - Attached Property/Behavior

6. **性能优化**
   - Visual Layer绘制
   - 虚拟化
   - 异步加载
   - 内存管理

7. **多线程**
   - Dispatcher机制
   - async/await
   - 线程安全集合

### 面试准备检查清单

- [ ] 能手写依赖属性注册代码
- [ ] 能实现INotifyPropertyChanged
- [ ] 能实现RelayCommand
- [ ] 了解至少2种MVVM框架
- [ ] 能创建自定义控件
- [ ] 能解释WPF渲染流程
- [ ] 了解Dispatcher原理
- [ ] 能优化性能问题
- [ ] 有实际WPF项目经验

---

**下一步**: 继续学习 [02-Qt专家级能力.md](02-Qt专家级能力.md)
