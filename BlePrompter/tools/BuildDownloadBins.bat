@echo off
setlocal

set "SCRIPT_DIRECTORY=%~dp0"
set "POWERSHELL_SCRIPT=%SCRIPT_DIRECTORY%BuildDownloadBins.ps1"

if not exist "%POWERSHELL_SCRIPT%" (
    echo PowerShell-Buildskript nicht gefunden: %POWERSHELL_SCRIPT%
    exit /b 1
)

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%POWERSHELL_SCRIPT%" %*
exit /b %ERRORLEVEL%

