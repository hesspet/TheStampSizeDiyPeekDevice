@echo off
chcp 65001 >nul
setlocal

set "PORT=8443"
if not "%~1"=="" set "PORT=%~1"

cd /d "%~dp0"

echo BlePrompter JS Client HTTPS-Demo-Server
echo.
where node >nul 2>nul
if errorlevel 1 goto nodeMissing

where powershell.exe >nul 2>nul
if errorlevel 1 goto powershellMissing

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\create-demo-certificate.ps1" -OutputDirectory "%~dp0certificates"
if errorlevel 1 goto certificateError

echo.
echo Wichtig für Smartphones:
echo   Installiere certificates\ble-prompter-demo-root-ca.cer auf dem Smartphone
echo   als vertrauenswürdiges Zertifikat. Sonst kann Chrome Web Bluetooth blockieren.
echo.

node "%~dp0scripts\https-demo-server.js" %PORT%
if errorlevel 1 goto serverError
exit /b 0

:certificateError
echo.
echo Zertifikatserzeugung fehlgeschlagen.
pause
exit /b 1

:serverError
echo.
echo Serverstart fehlgeschlagen.
echo Prüfe, ob der Port %PORT% bereits belegt ist.
echo Aktuelle Belegung:
netstat -ano | findstr ":%PORT%"
pause
exit /b 1

:nodeMissing
echo Node.js wurde nicht gefunden.
echo Installiere Node.js oder starte einen anderen HTTPS-Webserver in diesem Ordner.
pause
exit /b 1

:powershellMissing
echo PowerShell wurde nicht gefunden.
echo Das lokale Zertifikat kann nicht erzeugt werden.
pause
exit /b 1
