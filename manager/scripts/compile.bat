@echo off
setlocal
setlocal enabledelayedexpansion
chcp 65001

if not exist "%VISUAL_CPP_VARIABLES_PATH%" goto fail
call "%VISUAL_CPP_VARIABLES_PATH%" x86

where msbuild >nul 2>nul
if not %ERRORLEVEL%==0 goto fail

msbuild "%SOLUTION_TO_BUILD_NAME%" /p:Configuration=%BUILD_TYPE% /p:Platform=%PLATFORM_TYPE% /t:Clean,Build
if not %ERRORLEVEL%==0 goto fail

exit /b 0
 
:fail
echo Failed to compile %SOLUTION_TO_BUILD_NAME%
exit /b 1
