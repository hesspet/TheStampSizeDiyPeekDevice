. "$PSScriptRoot\Gemeinsame-Umgebung.ps1"

Setze-ProjektUmgebung
Teste-Befehl 'npm'

if (-not (Test-Path -LiteralPath $AndroidProjektPfad)) {
  throw "Android-Projektordner wurde nicht gefunden: $AndroidProjektPfad"
}

if (-not (Test-Path -LiteralPath (Join-Path $ProjektWurzel 'node_modules'))) {
  Write-Host 'node_modules fehlt. Installiere Abhängigkeiten mit npm install ...'
  Push-Location $ProjektWurzel
  try {
    npm install
    Teste-LetztesProgramm 'npm install'
  } finally {
    Pop-Location
  }
}

Write-Host 'Baue die Android Development Version ...'
Push-Location $AndroidProjektPfad
try {
  .\gradlew.bat app:assembleDebug -x lint -x test --configure-on-demand --build-cache '-PreactNativeArchitectures=arm64-v8a,x86_64' --console=plain --warning-mode=summary
  Teste-LetztesProgramm 'Gradle-Build'
} finally {
  Pop-Location
}

if (-not (Test-Path -LiteralPath $ApkPfad)) {
  throw "Build fertig, aber APK wurde nicht gefunden: $ApkPfad"
}

Write-Host ''
Write-Host "Fertig. APK liegt hier:"
Write-Host $ApkPfad
