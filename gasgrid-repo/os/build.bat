@echo off
cd /d %~dp0
if not exist bin mkdir bin
for %%f in (src\*.c) do (
  gcc -O2 -Wall -Wextra -Wno-misleading-indentation -std=c99 -o bin\%%~nf.exe %%f || exit /b 1
)
echo OS engine built in os\bin
