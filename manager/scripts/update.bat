@echo off
setlocal
setlocal enabledelayedexpansion

goto wait_close

:success
start /d manager manager/Manager.exe
exit /b 0

:fail
echo Failed to update Gamebase Manager
pause
exit /b 1

:wait_close
echo Waiting for app to close...
for /L %%x in (1,1,10) do (
   echo Try #%%x
   move manager manager_ > nul
   IF ERRORLEVEL 1 (
      ping 127.0.0.1 -n 2 > nul || ping ::1 -n 2 > nul
   ) else (
      goto main
   )
)

goto fail

:main
move manager_ manager > nul
echo Starting update...
