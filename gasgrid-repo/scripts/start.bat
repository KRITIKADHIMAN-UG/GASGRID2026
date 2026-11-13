@echo off
setlocal
cd /d %~dp0\..
where java >nul 2>&1 || (echo Java JDK 8+ not found & exit /b 1)
where mvn  >nul 2>&1 || (echo Maven not found & exit /b 1)
where gcc  >nul 2>&1 || (echo GCC not found - install MinGW-w64 & exit /b 1)
if not exist .env copy .env.example .env >nul
for /f "usebackq eol=# tokens=1,* delims==" %%a in (".env") do set "%%a=%%b"
if "%GASGRID_PORT%"=="" set GASGRID_PORT=8080
echo [1/3] Building C OS engine
pushd os & call build.bat & popd
echo [2/3] Building backend
if not exist backend\target\gasgrid.jar ( pushd backend & call mvn -q -DskipTests package & popd )
if not exist backend\target\gasgrid.jar ( echo Backend build failed & exit /b 1 )
echo [3/3] Starting GasGrid on http://localhost:%GASGRID_PORT%  (first start creates the schema and sample data)
start "" http://localhost:%GASGRID_PORT%
pushd backend
java -jar target\gasgrid.jar
popd
