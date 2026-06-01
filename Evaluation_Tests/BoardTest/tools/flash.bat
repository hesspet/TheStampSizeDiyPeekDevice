@echo off
setlocal

set "TOOLS_DIRECTORY=%~dp0"
call "%TOOLS_DIRECTORY%env.bat"

set "PROJECT_DIRECTORY=%TOOLS_DIRECTORY%.."

if "%BOARDTEST_UPLOAD_PORT%"=="" (
    echo Fehler: BOARDTEST_UPLOAD_PORT ist in env.bat nicht gesetzt.
    set "EXIT_STATUS=1"
    goto finishWithStatus
)

echo Flashe BoardTest auf %BOARDTEST_UPLOAD_PORT%...
pushd "%PROJECT_DIRECTORY%" || (
    set "EXIT_STATUS=1"
    goto finishWithStatus
)
"%PLATFORMIO_EXECUTABLE%" run --target upload --upload-port "%BOARDTEST_UPLOAD_PORT%"
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
echo %CMDCMDLINE% | findstr /I /C:" /c " >nul
if "%ERRORLEVEL%"=="0" (
    echo.
    pause
)
exit /b %EXIT_STATUS%