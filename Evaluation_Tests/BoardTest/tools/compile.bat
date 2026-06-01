@echo off
setlocal

set "TOOLS_DIRECTORY=%~dp0"
call "%TOOLS_DIRECTORY%env.bat"

set "PROJECT_DIRECTORY=%TOOLS_DIRECTORY%.."

echo Kompiliere BoardTest...
pushd "%PROJECT_DIRECTORY%" || (
    set "EXIT_STATUS=1"
    goto finishWithStatus
)
"%PLATFORMIO_EXECUTABLE%" run
set "EXIT_STATUS=%ERRORLEVEL%"
popd

if not "%EXIT_STATUS%"=="0" (
    echo Kompilieren fehlgeschlagen.
    goto finishWithStatus
)

echo Kompilieren abgeschlossen.
set "EXIT_STATUS=0"
goto finishWithStatus

:finishWithStatus
echo %CMDCMDLINE% | findstr /I /C:" /c " >nul
if "%ERRORLEVEL%"=="0" (
    echo.
    pause
)
exit /b %EXIT_STATUS%