# BuildDownloadBins.ps1
#
# Zweck
# -----
# Dieses Skript baut alle veröffentlichbaren BlePrompter-Firmwarevarianten
# und erzeugt daraus vollständige Download-Binärdateien für ESP Web Tools.
#
# Warum PowerShell statt Batch?
# -----------------------------
# Die frühere Batchvariante wurde schnell schwer wartbar: verschachtelte
# Unterroutinen, Encoding-Probleme bei Umlauten und lange esptool-Aufrufe sind
# in cmd.exe fehleranfällig. PowerShell erlaubt typisierte Konfigurationsobjekte,
# saubere Fehlerbehandlung, zuverlässige UTF-8-Ausgabe und besser lesbare
# Kommentare. Die Datei BuildDownloadBins.bat bleibt nur als bequemer Starter
# für Doppelklick, Explorer und bestehende Gewohnheiten erhalten.
#
# Ergebnisformat
# --------------
# Für jede PlatformIO-Umgebung entsteht im Verzeichnis download_bins ein eigenes
# Unterverzeichnis:
#
#   <umgebung>/
#     BlePrompter-<umgebung>-v<version>-<zeitstempel>-download.bin
#     BlePrompter-<umgebung>-v<version>-<zeitstempel>-download.md
#     manifest.json
#
# Die .bin-Datei ist ein zusammengeführtes Flash-Image. Sie enthält Bootloader,
# Partitionstabelle, boot_app0.bin und die eigentliche Anwendungsfirmware an den
# korrekten ESP-Flash-Adressen. Dadurch kann sie im Zauberhaft-Projekt direkt als
# einzelner ESP-Web-Tools-Manifest-Part mit Offset 0 bereitgestellt werden.
#
# Die manifest.json verweist relativ auf die .bin-Datei im selben Ordner.
#
# Die .md-Datei enthält bewusst nur technische Buildinformationen und Platzhalter
# für spätere Releasenotes und Installationshinweise.

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# Pfade und globale Einstellungen
# ---------------------------------------------------------------------------

# Das Skript liegt in <projekt>\tools. Das Projektverzeichnis ist deshalb ein
# Ordner oberhalb des Skriptverzeichnisses. Resolve-Path normalisiert den Pfad,
# damit später keine Ausgaben wie "tools\.." in den Metadaten landen.
$scriptDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectDirectory = Resolve-Path -LiteralPath (Join-Path $scriptDirectory "..")
$outputDirectory = Join-Path $projectDirectory "download_bins"
$configurationFile = Join-Path $projectDirectory "include\config.h"

# PlatformIO wird auf diesem Rechner häufig als Benutzer-Python-Skript
# installiert. Falls diese Datei nicht existiert, fällt das Skript auf "pio"
# zurück. Das funktioniert, wenn PlatformIO im PATH liegt.
$platformIoExecutable = Join-Path $env:APPDATA "Python\Python313\Scripts\pio.exe"
$platformIoCommand = if (Test-Path -LiteralPath $platformIoExecutable) {
    $platformIoExecutable
} else {
    "pio"
}

# esptool.py und boot_app0.bin kommen aus den PlatformIO-Paketen. Sie sind keine
# Projektdateien, werden aber zum Erzeugen eines vollständigen Flash-Images
# benötigt. PlatformIO nutzt dieselben Dateien beim normalen Upload.
$pythonCommand = "python"
$espToolScript = Join-Path $env:USERPROFILE ".platformio\packages\tool-esptoolpy\esptool.py"
$bootApplicationFile = Join-Path $env:USERPROFILE ".platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin"

# ---------------------------------------------------------------------------
# Firmwarevarianten
# ---------------------------------------------------------------------------

# Jede Umgebung beschreibt exakt eine downloadbare Firmwaredatei.
#
# Die Offsets stammen aus den PlatformIO-Uploadbefehlen im Verbose-Modus:
#
#   pio run -e esp32c3 --target upload --upload-port COM0 --verbose
#   pio run -e cyd     --target upload --upload-port COM0 --verbose
#
# Wichtig:
# - ESP32-C3 verwendet den Bootloader-Offset 0x0000.
# - Der klassische ESP32 verwendet den Bootloader-Offset 0x1000.
# - Partitionstabelle, boot_app0.bin und Anwendungsfirmware liegen bei beiden
#   Varianten an 0x8000, 0xe000 und 0x10000.
# - Das zusammengeführte Ergebnis wird später als Gesamtimage bei Offset 0
#   geflasht.
$firmwareBuildConfigurations = @(
    [pscustomobject]@{
        EnvironmentName = "esp32c3"
        DisplayName = "ESP32-C3 OLED"
        ChipFamily = "ESP32-C3"
        EspToolChip = "esp32c3"
        BoardName = "esp32-c3-devkitm-1"
        BootloaderOffset = "0x0000"
        FlashFrequency = "80m"
        FlashMode = "dio"
        FlashSize = "4MB"
    },
    [pscustomobject]@{
        EnvironmentName = "cyd"
        DisplayName = "CYB-CYD"
        ChipFamily = "ESP32"
        EspToolChip = "esp32"
        BoardName = "esp32dev"
        BootloaderOffset = "0x1000"
        FlashFrequency = "40m"
        FlashMode = "dio"
        FlashSize = "4MB"
    }
)

# ---------------------------------------------------------------------------
# Hilfsfunktionen
# ---------------------------------------------------------------------------

function Read-ConfigurationString {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ConfigurationText,

        [Parameter(Mandatory = $true)]
        [string]$ConfigurationName,

        [Parameter(Mandatory = $true)]
        [string]$DefaultValue
    )

    # Die Programmversion und der Programmname stehen als constexpr-Strings in
    # include/config.h. Diese kleine Regex liest genau solche Zuweisungen:
    #
    #   constexpr const char *programVersion = "1.7.0";
    #
    # Falls der Eintrag später umformatiert wird, bricht der Build nicht hart ab,
    # sondern verwendet den übergebenen Standardwert.
    $configurationPattern = "$ConfigurationName\s*=\s*`"([^`"]+)`""
    if ($ConfigurationText -match $configurationPattern) {
        return $Matches[1]
    }

    return $DefaultValue
}

function Invoke-NativeCommand {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Executable,

        [Parameter(Mandatory = $true)]
        [string[]]$Arguments,

        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    # Native Programme wie PlatformIO und esptool melden Fehler über den
    # Prozess-Exitcode. PowerShell wirft dafür nicht automatisch eine Exception.
    # Diese Funktion zentralisiert deshalb Aufruf, Anzeige und Exitcode-Prüfung.
    Write-Host ""
    Write-Host $Description
    Write-Host ("  {0} {1}" -f $Executable, ($Arguments -join " "))

    & $Executable @Arguments
    $nativeExitCode = $LASTEXITCODE

    if ($nativeExitCode -ne 0) {
        throw "Befehl fehlgeschlagen mit Exitcode ${nativeExitCode}: $Description"
    }
}

function Assert-RequiredFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,

        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "$Description nicht gefunden: $Path"
    }
}

function New-BuildInformationMarkdown {
    param(
        [Parameter(Mandatory = $true)]
        [pscustomobject]$BuildConfiguration,

        [Parameter(Mandatory = $true)]
        [string]$ProgramName,

        [Parameter(Mandatory = $true)]
        [string]$ProgramVersion,

        [Parameter(Mandatory = $true)]
        [string]$BuildDate,

        [Parameter(Mandatory = $true)]
        [string]$BuildTime,

        [Parameter(Mandatory = $true)]
        [string]$OutputBaseName,

        [Parameter(Mandatory = $true)]
        [string]$OutputMarkdownFile,

        [Parameter(Mandatory = $true)]
        [string]$OutputHash,

        [Parameter(Mandatory = $true)]
        [string]$BuildDirectory
    )

    # Die Markdown-Datei wird als UTF-8 geschrieben. Dadurch bleiben Umlaute in
    # Releasenotes und späteren Anwendertexten korrekt erhalten. Der Inhalt ist
    # noch kein fertiger Anwendertext, sondern eine technische Begleitdatei mit
    # Platzhaltern für spätere Abschnitte.
    $markdownLines = @(
        "# $ProgramName $($BuildConfiguration.DisplayName) Download-Firmware",
        "",
        "## Buildinformationen",
        "",
        "- **Programm:** $ProgramName",
        "- **Version:** $ProgramVersion",
        "- **Build-Datum:** $BuildDate",
        "- **Build-Zeit:** $BuildTime",
        "- **PlatformIO-Umgebung:** $($BuildConfiguration.EnvironmentName)",
        "- **Gerät:** $($BuildConfiguration.DisplayName)",
        "- **Board:** $($BuildConfiguration.BoardName)",
        "- **Chipfamilie:** $($BuildConfiguration.ChipFamily)",
        "- **esptool-Chip:** $($BuildConfiguration.EspToolChip)",
        "- **Flash-Modus:** $($BuildConfiguration.FlashMode)",
        "- **Flash-Frequenz:** $($BuildConfiguration.FlashFrequency)",
        "- **Flash-Größe:** $($BuildConfiguration.FlashSize)",
        "- **Firmware-Datei:** $OutputBaseName.bin",
        "- **Manifest-Datei:** manifest.json",
        "- **SHA256:** $OutputHash",
        "",
        "## Flash-Format",
        "",
        "Diese Datei ist ein zusammengeführtes ESP-Flash-Image. Sie ist für ESP Web Tools als einzelner Manifest-Part mit Offset ``0`` gedacht.",
        "",
        "Enthaltene Bestandteile:",
        "",
        "- ``$($BuildConfiguration.BootloaderOffset)`` - ``bootloader.bin``",
        "- ``0x8000`` - ``partitions.bin``",
        "- ``0xe000`` - ``boot_app0.bin``",
        "- ``0x10000`` - ``firmware.bin``",
        "",
        "## Herkunft",
        "",
        "- **Quellprojekt:** ``$projectDirectory``",
        "- **Build-Verzeichnis:** ``$BuildDirectory``",
        "- **PlatformIO-Befehl:** ``pio run -e $($BuildConfiguration.EnvironmentName)``",
        "- **Bündelung:** ``esptool.py --chip $($BuildConfiguration.EspToolChip) merge_bin``",
        "",
        "## Releasenotes",
        "",
        "Noch nicht ausgefüllt.",
        "",
        "## Installationshinweise für Anwender",
        "",
        "Noch nicht ausgefüllt."
    )

    Set-Content -LiteralPath $OutputMarkdownFile -Value $markdownLines -Encoding UTF8
}

function New-EspWebToolsManifest {
    param(
        [Parameter(Mandatory = $true)]
        [pscustomobject]$BuildConfiguration,

        [Parameter(Mandatory = $true)]
        [string]$ProgramName,

        [Parameter(Mandatory = $true)]
        [string]$ProgramVersion,

        [Parameter(Mandatory = $true)]
        [string]$FirmwareFileName,

        [Parameter(Mandatory = $true)]
        [string]$ManifestFile
    )

    # ESP Web Tools lädt zuerst diese Datei. Liegt die manifest.json im selben
    # Verzeichnis wie die Firmware, genügt ein relativer Dateiname als path.
    # Damit ist das Paket unabhängig davon, ob es lokal, unter GitHub Pages mit
    # baseurl oder in einem anderen statischen Hostingpfad liegt.
    $manifest = [ordered]@{
        name = "$ProgramName $($BuildConfiguration.DisplayName) $ProgramVersion"
        builds = @(
            [ordered]@{
                chipFamily = $BuildConfiguration.ChipFamily
                parts = @(
                    [ordered]@{
                        path = $FirmwareFileName
                        offset = 0
                    }
                )
            }
        )
    }

    $manifestJson = $manifest | ConvertTo-Json -Depth 8
    Set-Content -LiteralPath $ManifestFile -Value $manifestJson -Encoding UTF8
}

function Build-AndBundleFirmware {
    param(
        [Parameter(Mandatory = $true)]
        [pscustomobject]$BuildConfiguration,

        [Parameter(Mandatory = $true)]
        [string]$ProgramName,

        [Parameter(Mandatory = $true)]
        [string]$ProgramVersion,

        [Parameter(Mandatory = $true)]
        [string]$BuildDate,

        [Parameter(Mandatory = $true)]
        [string]$BuildTime,

        [Parameter(Mandatory = $true)]
        [string]$FileTimestamp
    )

    # PlatformIO legt die Buildartefakte immer unter .pio/build/<environment> ab.
    # Genau dort werden die drei projektbezogenen Binärdateien erwartet.
    $buildDirectory = Join-Path $projectDirectory ".pio\build\$($BuildConfiguration.EnvironmentName)"
    $bootloaderFile = Join-Path $buildDirectory "bootloader.bin"
    $partitionsFile = Join-Path $buildDirectory "partitions.bin"
    $applicationFirmwareFile = Join-Path $buildDirectory "firmware.bin"

    # Die Namen sind bewusst sprechend und enthalten Umgebung, Version und
    # Zeitstempel. So können mehrere Releases nebeneinander liegen, ohne sich zu
    # überschreiben. Für die Bereitstellung im Zauberhaft-Projekt kann eine Datei
    # später gezielt in firmware.bin umbenannt oder im Manifest direkt referenziert
    # werden.
    $environmentOutputDirectory = Join-Path $outputDirectory $BuildConfiguration.EnvironmentName
    $outputBaseName = "$ProgramName-$($BuildConfiguration.EnvironmentName)-v$ProgramVersion-$FileTimestamp-download"
    $outputFirmwareFileName = "$outputBaseName.bin"
    $outputMarkdownFileName = "$outputBaseName.md"
    $outputFirmwareFile = Join-Path $environmentOutputDirectory $outputFirmwareFileName
    $outputMarkdownFile = Join-Path $environmentOutputDirectory $outputMarkdownFileName
    $outputManifestFile = Join-Path $environmentOutputDirectory "manifest.json"

    Write-Host ""
    Write-Host "------------------------------------------------------------"
    Write-Host "Baue Umgebung: $($BuildConfiguration.EnvironmentName) - $($BuildConfiguration.DisplayName)"
    Write-Host "------------------------------------------------------------"

    Invoke-NativeCommand `
        -Executable $platformIoCommand `
        -Arguments @("run", "-e", $BuildConfiguration.EnvironmentName, "--target", "clean") `
        -Description "Bereinige PlatformIO-Umgebung $($BuildConfiguration.EnvironmentName)"

    Invoke-NativeCommand `
        -Executable $platformIoCommand `
        -Arguments @("run", "-e", $BuildConfiguration.EnvironmentName) `
        -Description "Baue PlatformIO-Umgebung $($BuildConfiguration.EnvironmentName)"

    Assert-RequiredFile -Path $bootloaderFile -Description "Bootloader"
    Assert-RequiredFile -Path $partitionsFile -Description "Partitionstabelle"
    Assert-RequiredFile -Path $applicationFirmwareFile -Description "Anwendungsfirmware"

    if (Test-Path -LiteralPath $environmentOutputDirectory) {
        Get-ChildItem -LiteralPath $environmentOutputDirectory -Force | Remove-Item -Recurse -Force
    } else {
        New-Item -ItemType Directory -Path $environmentOutputDirectory | Out-Null
    }

    # esptool merge_bin erzeugt aus mehreren Einzeldateien ein Gesamtimage. Die
    # übergebenen Adressen sind die Adressen, an die PlatformIO beim normalen
    # Upload schreiben würde. Das erzeugte Gesamtimage ist anschließend bereit,
    # als einzelner Block ab Offset 0 geschrieben zu werden.
    Invoke-NativeCommand `
        -Executable $pythonCommand `
        -Arguments @(
            $espToolScript,
            "--chip", $BuildConfiguration.EspToolChip,
            "merge_bin",
            "--output", $outputFirmwareFile,
            "--flash_mode", $BuildConfiguration.FlashMode,
            "--flash_freq", $BuildConfiguration.FlashFrequency,
            "--flash_size", $BuildConfiguration.FlashSize,
            $BuildConfiguration.BootloaderOffset, $bootloaderFile,
            "0x8000", $partitionsFile,
            "0xe000", $bootApplicationFile,
            "0x10000", $applicationFirmwareFile
        ) `
        -Description "Bündle vollständiges Flash-Image für $($BuildConfiguration.DisplayName)"

    $outputHash = (Get-FileHash -LiteralPath $outputFirmwareFile -Algorithm SHA256).Hash

    New-BuildInformationMarkdown `
        -BuildConfiguration $BuildConfiguration `
        -ProgramName $ProgramName `
        -ProgramVersion $ProgramVersion `
        -BuildDate $BuildDate `
        -BuildTime $BuildTime `
        -OutputBaseName $outputBaseName `
        -OutputMarkdownFile $outputMarkdownFile `
        -OutputHash $outputHash `
        -BuildDirectory $buildDirectory

    New-EspWebToolsManifest `
        -BuildConfiguration $BuildConfiguration `
        -ProgramName $ProgramName `
        -ProgramVersion $ProgramVersion `
        -FirmwareFileName $outputFirmwareFileName `
        -ManifestFile $outputManifestFile

    Write-Host "Erstellt:"
    Write-Host "  $outputFirmwareFile"
    Write-Host "  $outputMarkdownFile"
    Write-Host "  $outputManifestFile"
}

# ---------------------------------------------------------------------------
# Hauptablauf
# ---------------------------------------------------------------------------

Assert-RequiredFile -Path $configurationFile -Description "Konfigurationsdatei"
Assert-RequiredFile -Path $espToolScript -Description "esptool.py"
Assert-RequiredFile -Path $bootApplicationFile -Description "boot_app0.bin"

if (-not (Test-Path -LiteralPath $outputDirectory)) {
    New-Item -ItemType Directory -Path $outputDirectory | Out-Null
}

$configurationText = Get-Content -Raw -LiteralPath $configurationFile
$programName = Read-ConfigurationString -ConfigurationText $configurationText -ConfigurationName "programName" -DefaultValue "BlePrompter"
$programVersion = Read-ConfigurationString -ConfigurationText $configurationText -ConfigurationName "programVersion" -DefaultValue "unknown"

# Alte flache Artefakte aus früheren Generatorversionen entfernen. Die neue
# Zielstruktur ist download_bins/<umgebung>/ mit .bin, .md und manifest.json.
Get-ChildItem -LiteralPath $outputDirectory -File -Force |
    Where-Object { $_.Name -like "$programName-*-download.bin" -or $_.Name -like "$programName-*-download.md" } |
    Remove-Item -Force

$currentDate = Get-Date
$buildDate = $currentDate.ToString("dd.MM.yyyy")
$buildTime = $currentDate.ToString("HH:mm:ss")
$fileTimestamp = $currentDate.ToString("yyyyMMdd-HHmmss")

Write-Host "============================================================"
Write-Host "$programName - Download-Firmwarepakete bauen"
Write-Host "============================================================"
Write-Host "Version:      $programVersion"
Write-Host "Build-Datum:  $buildDate"
Write-Host "Ausgabe:      $outputDirectory"
Write-Host "============================================================"

Push-Location -LiteralPath $projectDirectory
try {
    foreach ($firmwareBuildConfiguration in $firmwareBuildConfigurations) {
        Build-AndBundleFirmware `
            -BuildConfiguration $firmwareBuildConfiguration `
            -ProgramName $programName `
            -ProgramVersion $programVersion `
            -BuildDate $buildDate `
            -BuildTime $buildTime `
            -FileTimestamp $fileTimestamp
    }
}
finally {
    Pop-Location
}

Write-Host ""
Write-Host "============================================================"
Write-Host "Alle Download-Firmwarepakete wurden erstellt."
Write-Host "Zielverzeichnis: $outputDirectory"
Write-Host "============================================================"
