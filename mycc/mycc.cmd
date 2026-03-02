@echo off
REM mycc.cmd - 一键启动 Claude Code with GitHub Copilot
REM Windows CMD 批处理版本

powershell.exe -ExecutionPolicy Bypass -File "%~dp0mycc.ps1" %*
