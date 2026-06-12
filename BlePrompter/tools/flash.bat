@echo off
setlocal enabledelayedexpansion

set "TOOLS_DIRECTORY=%~dp0"
set "PROJECT_DIRECTORY=%TOOLS_DIRECTORY%.."
set "PLATFORMIO_EXECUTABLE=%APPDATA%\Python\Python313\Scripts\pio.exe"
set "ACTIVE_ENV_FILE=%TOOLS_DIRECTORY%.active_env"
set "COM_PORTS_FILE=%TOOLS_DIRECTORY%.com_ports"

:: Aktive Umgebung laden
set "ACTIVE_ENV="
if exist "%ACTIVE_ENV_FILE%" (
    set /p ACTIVE_ENV=<"%ACTIVE_ENV_FILE%"
)

:: COM-Port für aktive Umgebung laden
set "FLASH_PORT="
if defined ACTIVE_ENV (
    if exist "%COM_PORTS_FILE%" (
        for /f "usebackq tokens=1,* delims==" %%a in ("%COM_PORTS_FILE%") do (
            if "%%a"=="%ACTIVE_ENV%" set "FLASH_PORT=%%b"
        )
    )
)

:: PIO-Kommando ermitteln
set "PIO_CMD="
if exist "%PLATFORMIO_EXECUTABLE%" (
    set "PIO_CMD=%PLATFORMIO_EXECUTABLE%"
) else (
    set "PIO_CMD=pio"
)

echo Flashe BlePrompter...
pushd "%PROJECT_DIRECTORY%" || (
    set "EXIT_STATUS=1"
    goto finishWithStatus
)

if not defined ACTIVE_ENV (
    echo Keine Umgebung ausgewaehlt. Bitte zuerst env.bat ausfuehren.
    echo.
    echo Alternativ: Default-Umgebung wird verwendet.
)

if defined FLASH_PORT (
    set "PORT_ARG=--upload-port %FLASH_PORT%"
) else (
    echo.
    echo Kein COM-Port konfiguriert. Bitte in env.bat mit [p] setzen,
    echo oder direkt COM-Port eingeben:
    set /p "FLASH_PORT=COM-Port (z.B. COM6): "
    if "!FLASH_PORT!"=="" (
        echo Flash abgebrochen.
        popd
        exit /b 1
    )
    set "PORT_ARG=--upload-port !FLASH_PORT!"
)

echo Umgebung: %ACTIVE_ENV%
echo COM-Port:  %FLASH_PORT%
echo ============================================================

if defined ACTIVE_ENV (
    %PIO_CMD% run -e %ACTIVE_ENV% --target upload %PORT_ARG%
) else (
    %PIO_CMD% run --target upload %PORT_ARG%
)

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
