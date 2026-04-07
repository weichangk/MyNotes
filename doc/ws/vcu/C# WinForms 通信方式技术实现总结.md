# C# WinForms 通信方式技术实现总结

> 面向 WinForms 实战的 7 种常见通信方式整理（原理 + 示例 + 注意事项）

---

# 1. 进程间通信（Windows 消息）

## 原理
通过 Win32 API（SendMessage / PostMessage）向目标窗口发送消息。

## 示例
```csharp
[DllImport("user32.dll")]
static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

[DllImport("user32.dll")]
static extern IntPtr SendMessage(IntPtr hWnd, int Msg, IntPtr wParam, IntPtr lParam);

const int WM_USER = 0x0400;
const int WM_MYMSG = WM_USER + 1;

var hwnd = FindWindow(null, "TargetForm");
SendMessage(hwnd, WM_MYMSG, IntPtr.Zero, IntPtr.Zero);
```

```csharp
protected override void WndProc(ref Message m)
{
    if (m.Msg == WM_MYMSG)
    {
        MessageBox.Show("收到消息");
    }
    base.WndProc(ref m);
}
```

---

# 2. 外部进程通信（Process）

```csharp
var p = new Process();
p.StartInfo.FileName = "test.exe";
p.StartInfo.Arguments = "arg1 arg2";
p.StartInfo.UseShellExecute = false;
p.StartInfo.RedirectStandardOutput = true;

p.OutputDataReceived += (s, e) =>
{
    Console.WriteLine(e.Data);
};

p.Start();
p.BeginOutputReadLine();
p.WaitForExit();
```

---

# 3. 进程内异步通信（事件 / 委托）

```csharp
public class Worker
{
    public event Action<string> OnData;

    public void DoWork()
    {
        Task.Run(() =>
        {
            Thread.Sleep(1000);
            OnData?.Invoke("完成");
        });
    }
}
```

```csharp
worker.OnData += msg =>
{
    Invoke(() => label1.Text = msg);
};
```

---

# 4. 线程间通信（UI 线程切换）

```csharp
Task.Run(() =>
{
    this.Invoke(() =>
    {
        label1.Text = "更新 UI";
    });
});
```

```csharp
var ctx = SynchronizationContext.Current;

Task.Run(() =>
{
    ctx.Post(_ =>
    {
        label1.Text = "更新 UI";
    }, null);
});
```

---

# 5. 定时轮询通信（Timer）

```csharp
Timer timer = new Timer();
timer.Interval = 1000;

timer.Tick += (s, e) =>
{
    if (File.Exists("flag.txt"))
    {
        label1.Text = "检测到文件";
    }
};

timer.Start();
```

---

# 6. 网络通信（HTTP）

```csharp
var client = new HttpClient();

var response = await client.GetAsync("https://api.example.com/data");
var content = await response.Content.ReadAsStringAsync();

label1.Text = content;
```

---

# 7. Mutex 互斥

```csharp
bool created;
var mutex = new Mutex(true, "MyApp_Mutex", out created);

if (!created)
{
    MessageBox.Show("程序已运行");
    Application.Exit();
}
```

---

# 总结

- UI 更新：Invoke / SynchronizationContext  
- 模块解耦：事件 / 委托  
- 跨进程：Process  
- 网络：HTTP  
- 单实例：Mutex  
