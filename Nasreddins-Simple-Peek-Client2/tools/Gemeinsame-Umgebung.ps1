$ErrorActionPreference = 'Stop'

$SkriptVerzeichnis = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjektWurzel = Resolve-Path (Join-Path $SkriptVerzeichnis '..')
$AndroidProjektPfad = Join-Path $ProjektWurzel 'android'
$ApkPfad = Join-Path $AndroidProjektPfad 'app\build\outputs\apk\debug\app-debug.apk'
$AndroidSdkPfad = Join-Path $env:LOCALAPPDATA 'Android\Sdk'
$JavaPfad = 'C:\Program Files\Android\Android Studio\jbr'
$DevelopmentServerPort = 8081
$AppPaketName = 'de.nasreddin.simplepeekclient2'
$DevelopmentClientAdresse = 'exp+nasreddins-simple-peek-client2://expo-development-client/?url=http%3A%2F%2F127.0.0.1%3A8081'

function Setze-ProjektUmgebung {
  if (-not (Test-Path -LiteralPath $AndroidSdkPfad)) {
    throw "Android SDK wurde nicht gefunden: $AndroidSdkPfad"
  }

  if (-not (Test-Path -LiteralPath $JavaPfad)) {
    throw "Java aus Android Studio wurde nicht gefunden: $JavaPfad"
  }

  $env:ANDROID_HOME = $AndroidSdkPfad
  $env:ANDROID_SDK_ROOT = $AndroidSdkPfad
  $env:JAVA_HOME = $JavaPfad

  $WerkzeugPfade = @(
    (Join-Path $AndroidSdkPfad 'platform-tools'),
    (Join-Path $AndroidSdkPfad 'emulator'),
    (Join-Path $AndroidSdkPfad 'cmdline-tools\latest\bin'),
    (Join-Path $JavaPfad 'bin')
  )

  $env:Path = ($WerkzeugPfade + $env:Path) -join ';'
}

function Teste-Befehl {
  param(
    [Parameter(Mandatory = $true)]
    [string] $BefehlName
  )

  if (-not (Get-Command $BefehlName -ErrorAction SilentlyContinue)) {
    throw "Befehl nicht gefunden: $BefehlName"
  }
}

function Teste-LetztesProgramm {
  param(
    [Parameter(Mandatory = $true)]
    [string] $Beschreibung
  )

  if ($LASTEXITCODE -ne 0) {
    throw "$Beschreibung ist fehlgeschlagen. Exitcode: $LASTEXITCODE"
  }
}

function Waehle-AdbGeraet {
  param(
    [string] $VorgegebenesGeraet
  )

  $GeraeteZeilen = adb devices | Where-Object { $_ -match "`tdevice$" }
  Teste-LetztesProgramm 'ADB-Geräteliste'
  $Geraete = @($GeraeteZeilen | ForEach-Object { ($_ -split "`t")[0] })

  if ($VorgegebenesGeraet) {
    if ($Geraete -notcontains $VorgegebenesGeraet) {
      throw "ADB-Gerät nicht gefunden: $VorgegebenesGeraet"
    }

    return $VorgegebenesGeraet
  }

  if ($Geraete.Count -eq 0) {
    throw 'Kein ADB-Gerät gefunden. Bitte Emulator starten oder Smartphone per USB verbinden.'
  }

  if ($Geraete.Count -eq 1) {
    return $Geraete[0]
  }

  Write-Host 'Mehrere ADB-Geräte gefunden:'
  for ($GeraetIndex = 0; $GeraetIndex -lt $Geraete.Count; $GeraetIndex++) {
    Write-Host "[$($GeraetIndex + 1)] $($Geraete[$GeraetIndex])"
  }

  $Auswahl = Read-Host 'Nummer des Zielgeräts eingeben'
  $AuswahlIndex = [int]$Auswahl - 1

  if ($AuswahlIndex -lt 0 -or $AuswahlIndex -ge $Geraete.Count) {
    throw 'Ungültige Geräteauswahl.'
  }

  return $Geraete[$AuswahlIndex]
}
