@echo off
chcp 65001 >nul
cd /d "%~dp0"
for %%F in (*_webview.py) do (
    python "%%F"
    goto :done
)
echo [ERROR] not found *_webview.py
:done
pause
