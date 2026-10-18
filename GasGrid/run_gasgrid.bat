@echo off
setlocal
cd /d "%~dp0"
title GasGrid LPG Management

echo ======================================
echo       GasGrid LPG Management
echo ======================================
where py >nul 2>nul
if %errorlevel%==0 (
  set "PYTHON=py"
) else (
  where python >nul 2>nul
  if %errorlevel%==0 (
    set "PYTHON=python"
  ) else (
    echo ERROR: Python was not found. Install Python 3 and enable Add Python to PATH.
    pause
    exit /b 1
  )
)
%PYTHON% --version
if not exist ".venv\Scripts\python.exe" (
  echo Creating the project virtual environment...
  %PYTHON% -m venv .venv
  if errorlevel 1 (
    echo ERROR: Could not create the virtual environment.
    pause
    exit /b 1
  )
)
echo Installing required packages. Internet access is needed on first run...
".venv\Scripts\python.exe" -m pip install -r requirements.txt
if errorlevel 1 (
  echo ERROR: Dependency installation failed. Check internet access, then run this file again.
  pause
  exit /b 1
)
echo Starting the GasGrid server. Keep this window open while using the website.
start "" /b powershell -NoProfile -Command "Start-Sleep -Seconds 2; Start-Process 'http://127.0.0.1:5000'"
".venv\Scripts\python.exe" app.py
if errorlevel 1 echo The server stopped with an error. Read the message above.
pause
