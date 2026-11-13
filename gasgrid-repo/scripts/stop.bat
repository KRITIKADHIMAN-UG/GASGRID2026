@echo off
setlocal
set PORT=8080
if exist "%~dp0\..\.env" for /f "usebackq eol=# tokens=1,* delims==" %%a in ("%~dp0\..\.env") do if "%%a"=="GASGRID_PORT" set PORT=%%b
for /f "tokens=5" %%p in ('netstat -ano ^| findstr :%PORT% ^| findstr LISTENING') do taskkill /F /PID %%p
echo GasGrid stopped.
