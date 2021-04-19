@echo off
setlocal

set DEVENV="c:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\IDE\devenv.exe"
if exist "%DEVENV%" ( echo Using %DEVENV% ) else (echo "Please install Visual Studio 2019" && exit /b 1 )

set SOLUTION="OTInterviewExercise1.sln"

if exist Output rd /Q /S Output
md Output

Echo Building %SOLUTION% Debug x86
call %DEVENV% %SOLUTION% /Out Output\build-debug-x86.log /Build "Debug|x86"
if %errorlevel% neq 0 exit /b %errorlevel%

Echo Building %SOLUTION% Release x86
call %DEVENV% %SOLUTION% /Out Output\build-release-x86.log /Build "Release|x86"
if %errorlevel% neq 0 exit /b %errorlevel%

Echo Building %SOLUTION% Debug x64
call %DEVENV% %SOLUTION% /Out Output\build-debug-x64.log /Build "Debug|x64"
if %errorlevel% neq 0 exit /b %errorlevel%

Echo Building %SOLUTION% Release x64
call %DEVENV% %SOLUTION% /Out Output\build-release-x64.log /Build "Release|x64"
if %errorlevel% neq 0 exit /b %errorlevel%

echo All configuration were built. The results are in Output directory.

