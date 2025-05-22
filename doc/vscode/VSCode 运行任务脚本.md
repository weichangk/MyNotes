
使用快捷键 Ctrl+Shift+P，输入 Run Task，然后选择配置的 task 名，执行 shell、bat、python等脚本
```bash
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Run Build Script",
      "type": "shell",
      "command": "${workspaceRoot}/script/build.sh",
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "problemMatcher": []
    }
  ]
}
```

可选配置项说明
| 字段名              | 说明                                  |
| ---------------- | ----------------------------------- |
| `label`          | 任务的名称，会在“Run Task”列表中显示             |
| `type`           | 一般用 `shell`，表示使用 shell 来运行          |
| `command`        | 实际执行的命令或脚本路径                        |
| `args`           | 传递给脚本的参数，格式为数组（如：\["arg1", "arg2"]） |
| `group`          | 将任务归类为 `build`、`test` 等             |
| `problemMatcher` | 用于识别编译错误信息，暂时可以为空                   |
