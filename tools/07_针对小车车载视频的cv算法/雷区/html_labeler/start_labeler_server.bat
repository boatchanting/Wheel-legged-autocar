@echo off
setlocal
cd /d "%~dp0"

if exist "..\..\..\..\..\.venv\Scripts\python.exe" (
  set "PYTHON_EXE=..\..\..\..\..\.venv\Scripts\python.exe"
) else (
  set "PYTHON_EXE=python"
)

echo Starting html_labeler on http://127.0.0.1:8765/index.html
"%PYTHON_EXE%" serve_labeler.py
