@echo off
chcp 65001 >nul
cd /d "%~dp0"
python "实时图传上位机_webview.py"
pause
