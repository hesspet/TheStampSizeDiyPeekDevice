@echo off
setlocal enabledelayedexpansion

set "TOOLS_DIRECTORY=%~dp0"
set "PROJECT_DIRECTORY=%TOOLS_DIRECTORY%.."
set "PLATFORMIO_EXECUTABLE=%APPDATA%\Python\Python313\Scripts\pio.exe"
set "ACTIVE_ENV_FILE=%TOOLS_DIRECTORY%.active_env"

:: Aktive Umgebung laden
set "ACTIVE_ENV="
if exist "%ACTIVE_ENV_FILE%" (
    set /p ACTIVE_ENV=<"%ACTIVE_ENV_FILE%"
)

:: PIO-Kommando ermitteln
set "PIO_CMD="
if exist "%PLATFORMIO_EXECUTABLE%" (
    set "PIO_CMD=%PLATFORMIO_EXECUTABLE%"
) else (
    set "PIO_CMD=pio"
)

echo Kompiliere BlePrompter...
pushd "%PROJECT_DIRECTORY%" || (
    set "EXIT_STATUS=1"
    goto finishWithStatus
)

if defined ACTIVE_ENV (
    echo Umgebung: %ACTIVE_ENV%
    %PIO_CMD% run -e %ACTIVE_ENV%
) else (
    echo Keine Umgebung ausgewaehlt. Bitte zuerst env.bat ausfuehren.
    echo.
    echo Alternativ: Default-Umgebung wird verwendet.
    %PIO_CMD% run
)

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
