#!/usr/bin/env node
const { spawn } = require('child_process');
const path = require('path');
const os = require('os');

const scriptPath = path.join(__dirname, 'mycc.ps1');
const args = process.argv.slice(2);

const isWindows = os.platform() === 'win32';

if (isWindows) {
    // Windows: 使用 PowerShell
    const ps = spawn('powershell.exe', [
        '-ExecutionPolicy', 'Bypass',
        '-File', scriptPath,
        ...args
    ], {
        stdio: 'inherit',
        shell: true
    });
    
    ps.on('exit', (code) => {
        process.exit(code || 0);
    });
} else {
    // Linux/Mac: 使用 pwsh
    const ps = spawn('pwsh', [
        '-File', scriptPath,
        ...args
    ], {
        stdio: 'inherit'
    });
    
    ps.on('exit', (code) => {
        process.exit(code || 0);
    });
}