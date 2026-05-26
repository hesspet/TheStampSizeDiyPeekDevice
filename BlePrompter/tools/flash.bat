@echo off
setlocal

set "TOOLS_DIRECTORY=%~dp0"
call "%TOOLS_DIRECTORY%env.bat"

set "PROJECT_DIRECTORY=%TOOLS_DIRECTORY%.."

if "%BLEPROMPTER_UPLOAD_PORT%"=="" (
    echo Fehler: BLEPROMPTER_UPLOAD_PORT ist in env.bat nicht gesetzt.
    set "EXIT_STATUS=1"
    goto finishWithStatus
)

echo Flashe BlePrompter auf %BLEPROMPTER_UPLOAD_PORT%...
pushd "%PROJECT_DIRECTORY%" || (
    set "EXIT_STATUS=1"
    goto finishWithStatus
)
"%PLATFORMIO_EXECUTABLE%" run --target upload --upload-port "%BLEPROMPTER_UPLOAD_PORT%"
set "EXIT_STATUS=%ERRORLEVEL%"
popd

if not "%EXIT_STATUS%"=="0" (
    echo Flashen fehlgeschlagen.
    goto finishWithStatus
)

echo Flashen abgeschlossen.
set "EXIT_STATUS=0"
goto finishWithStatus

:finishWithStatus
set "STARTED_WITH_COMMAND_CLOSE="
set "STARTED_FROM_EXPLORER="

echo %CMDCMDLINE% | findstr /I /C:" /c " >nul
if "%ERRORLEVEL%"=="0" set "STARTED_WITH_COMMAND_CLOSE=1"

for /f "usebackq delims=" %%P in (`powershell -NoProfile -ExecutionPolicy Bypass -Command "$powerShellProcess = Get-CimInstance Win32_Process -Filter ('ProcessId=' + $PID); $commandProcess = Get-CimInstance Win32_Process -Filter ('ProcessId=' + $powerShellProcess.ParentProcessId); $launcherProcess = Get-CimInstance Win32_Process -Filter ('ProcessId=' + $commandProcess.ParentProcessId); if ($launcherProcess.Name -ieq 'explorer.exe') { '1' }"`) do set "STARTED_FROM_EXPLORER=%%P"

if "%STARTED_WITH_COMMAND_CLOSE%"=="1" if "%STARTED_FROM_EXPLORER%"=="1" (
    echo.
    pause
)
pause
exit /b %EXIT_STATUS%
