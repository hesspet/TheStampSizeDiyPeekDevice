@echo off
setlocal enabledelayedexpansion

:: ========================================================================
:: Pfade
:: ========================================================================
set "PLATFORMIO_EXECUTABLE=%APPDATA%\Python\Python313\Scripts\pio.exe"
set "PROJECT_DIRECTORY=%~dp0.."
set "ACTIVE_ENV_FILE=%~dp0.active_env"
set "COM_PORTS_FILE=%~dp0.com_ports"

pushd "%PROJECT_DIRECTORY%"

:: ========================================================================
:: PIO-Kommando ermitteln
:: ========================================================================
set "PIO_CMD="
if exist "%PLATFORMIO_EXECUTABLE%" (
    set "PIO_CMD=%PLATFORMIO_EXECUTABLE%"
) else (
    where pio >nul 2>&1 && set "PIO_CMD=pio" || set "PIO_CMD=pio"
)

:: ========================================================================
:: Verfügbare Umgebungen aus platformio.ini lesen (Name + upload_port)
:: ========================================================================
set "ENV_COUNT=0"
set "CURRENT_ENV="
for /f "usebackq tokens=*" %%l in (platformio.ini) do (
    set "line=%%l"
    :: Leerzeichen entfernen
    set "line=!line: =!"

    :: [env:NAME] erkennen
    if "!line:~0,5!"=="[env:" (
        for /f "tokens=1,* delims=:" %%a in ("!line!") do (
            set "env_name=%%b"
            set "env_name=!env_name:~0,-1!"
            set /a ENV_COUNT+=1
            set "ENV_NAME_!ENV_COUNT!=!env_name!"
            set "CURRENT_ENV=!env_name!"
            set "ENV_PORT_!env_name!="
        )
    )

    :: upload_port = VALUE erkennen
    if defined CURRENT_ENV (
        if "!line:~0,12!"=="upload_port=" (
            set "port=!line:~12!"
            if defined port set "ENV_PORT_!CURRENT_ENV!=!port!"
        )
    )
)

if %ENV_COUNT%==0 (
    echo Keine [env:*]-Sektionen in platformio.ini gefunden.
    popd
    exit /b 1
)

:: ========================================================================
:: Benutzerdefinierte COM-Ports aus .com_ports laden
:: ========================================================================
if exist "%COM_PORTS_FILE%" (
    for /f "usebackq tokens=1,* delims==" %%a in ("%COM_PORTS_FILE%") do (
        set "ENV_PORT_%%a=%%b"
    )
)

:: ========================================================================
:: Aktive Umgebung laden
:: ========================================================================
set "ACTIVE_ENV="
if exist "%ACTIVE_ENV_FILE%" (
    set /p ACTIVE_ENV=<"%ACTIVE_ENV_FILE%"
)

set "ENV_VALID=0"
if defined ACTIVE_ENV (
    for /l %%i in (1,1,%ENV_COUNT%) do (
        if "!ACTIVE_ENV!"=="!ENV_NAME_%%i!" set "ENV_VALID=1"
    )
)
if "!ENV_VALID!"=="0" set "ACTIVE_ENV=!ENV_NAME_1!"

:: ========================================================================
:: Argument-Verarbeitung
:: ========================================================================
set "ARG_ENV="
set "ARG_ACTION="

if not "%~1"=="" (
    set "ARG_ENV=%~1"
    set "ARG_ACTION=%~2"
)

if defined ARG_ENV (
    set "ENV_FOUND=0"
    for /l %%i in (1,1,%ENV_COUNT%) do (
        if /i "!ARG_ENV!"=="!ENV_NAME_%%i!" (
            set "ACTIVE_ENV=!ENV_NAME_%%i!"
            set "ENV_FOUND=1"
        )
    )
    if "!ENV_FOUND!"=="0" (
        echo Unbekannte Umgebung: !ARG_ENV!
        echo Verfuegbar:
        for /l %%i in (1,1,%ENV_COUNT%) do echo   !ENV_NAME_%%i!
        popd
        exit /b 1
    )
    echo !ACTIVE_ENV!>"%ACTIVE_ENV_FILE%"

    if /i "!ARG_ACTION!"=="build"   goto :do_build
    if /i "!ARG_ACTION!"=="flash"   goto :do_flash
    if /i "!ARG_ACTION!"=="monitor" goto :do_monitor
    if /i "!ARG_ACTION!"=="clean"   goto :do_clean
    if /i "!ARG_ACTION!"=="upload"  goto :do_flash

    echo Umgebung auf "!ACTIVE_ENV!" gesetzt.
    popd
    exit /b 0
)

:: ========================================================================
:: Hilfsfunktion: COM-Port-Datei neu schreiben
:: ========================================================================
goto :skip_save_com_ports
:save_com_ports
    >"%COM_PORTS_FILE%" (
        for /l %%i in (1,1,%ENV_COUNT%) do (
            set "env=!ENV_NAME_%%i!"
            for %%e in (!env!) do (
                echo %%e=!ENV_PORT_%%e!
            )
        )
    )
    exit /b
:skip_save_com_ports

:: ========================================================================
:: Interaktives Menü
:: ========================================================================
:menu
echo.
echo ============================================================
echo  I2C Scanner - Board-Auswahl
echo ============================================================
echo  Aktive Umgebung: !ACTIVE_ENV! ^(COM: !ENV_PORT_%ACTIVE_ENV%!^)
echo.
echo  Verfuegbare Umgebungen:
for /l %%i in (1,1,%ENV_COUNT%) do (
    set "env=!ENV_NAME_%%i!"
    for %%e in (!env!) do (
        echo    [%%i] !ENV_NAME_%%i!  ^(COM: !ENV_PORT_%%e!^)
    )
)
echo.
echo  Aktionen:
echo    [b] Build (Kompilieren)
echo    [f] Flash (Build + Upload)
echo    [p] COM-Port fuer !ACTIVE_ENV! setzen
echo    [m] Monitor starten
echo    [c] Clean (Build-Artefakte loeschen)
echo    [q] Beenden
echo ============================================================
set "CHOICE="
set /p "CHOICE=Auswahl: "

:: Numerische Auswahl
for /l %%i in (1,1,%ENV_COUNT%) do (
    if "!CHOICE!"=="%%i" (
        set "ACTIVE_ENV=!ENV_NAME_%%i!"
        echo !ACTIVE_ENV!>"%ACTIVE_ENV_FILE%"
        echo Umgebung auf "!ACTIVE_ENV!" gesetzt.
        goto :menu
    )
)

:: Aktions-Auswahl
if /i "!CHOICE!"=="b" goto :do_build
if /i "!CHOICE!"=="f" goto :do_flash
if /i "!CHOICE!"=="p" goto :do_set_port
if /i "!CHOICE!"=="m" goto :do_monitor
if /i "!CHOICE!"=="c" goto :do_clean
if /i "!CHOICE!"=="q" goto :end
if /i "!CHOICE!"==""  goto :end

echo Unbekannte Auswahl: !CHOICE!
goto :menu

:: ========================================================================
:: COM-Port setzen
:: ========================================================================
:do_set_port
echo.
echo Aktueller COM-Port fuer !ACTIVE_ENV!: !ENV_PORT_%ACTIVE_ENV%!
set "NEW_PORT="
set /p "NEW_PORT=Neuer COM-Port (z.B. COM6, Enter=behalten): "
if not "!NEW_PORT!"=="" (
    set "ENV_PORT_!ACTIVE_ENV!=!NEW_PORT!"
    call :save_com_ports
    echo COM-Port fuer !ACTIVE_ENV! auf !NEW_PORT! gesetzt.
) else (
    echo COM-Port unveraendert.
)
goto :menu

:: ========================================================================
:: Aktionen
:: ========================================================================
:do_build
echo.
echo Build fuer Umgebung: !ACTIVE_ENV!
echo ============================================================
!PIO_CMD! run -e !ACTIVE_ENV!
if errorlevel 1 (
    echo.
    echo FEHLER: Build fehlgeschlagen.
    pause
) else (
    echo.
    echo Build erfolgreich.
)
goto :menu

:do_flash
set "FLASH_PORT=!ENV_PORT_%ACTIVE_ENV%!"
if "!FLASH_PORT!"=="" (
    echo.
    echo Kein COM-Port fuer !ACTIVE_ENV! konfiguriert.
    set /p "FLASH_PORT=COM-Port eingeben (z.B. COM6): "
    if "!FLASH_PORT!"=="" (
        echo Flash abgebrochen.
        goto :menu
    )
    set "ENV_PORT_!ACTIVE_ENV!=!FLASH_PORT!"
    call :save_com_ports
)

echo.
echo Flash fuer Umgebung: !ACTIVE_ENV! auf !FLASH_PORT!
echo ============================================================
!PIO_CMD! run -e !ACTIVE_ENV! --target upload --upload-port !FLASH_PORT!
if errorlevel 1 (
    echo.
    echo FEHLER: Flash fehlgeschlagen.
    pause
) else (
    echo.
    echo Flash erfolgreich.
)
goto :menu

:do_monitor
set "MON_PORT=!ENV_PORT_%ACTIVE_ENV%!"
if "!MON_PORT!"=="" (
    echo.
    echo Kein COM-Port fuer !ACTIVE_ENV! konfiguriert.
    set /p "MON_PORT=COM-Port eingeben (z.B. COM6): "
    if "!MON_PORT!"=="" (
        echo Monitor abgebrochen.
        goto :menu
    )
)

echo.
echo Monitor fuer Umgebung: !ACTIVE_ENV! auf !MON_PORT!
echo ============================================================
echo Starte Serial Monitor... (Abbruch mit Strg+C)
!PIO_CMD! device monitor -e !ACTIVE_ENV! --port !MON_PORT!
goto :menu

:do_clean
echo.
echo Clean fuer Umgebung: !ACTIVE_ENV!
echo ============================================================
!PIO_CMD! run -e !ACTIVE_ENV! --target clean
echo Clean abgeschlossen.
goto :menu

:end
popd
exit /b 0
