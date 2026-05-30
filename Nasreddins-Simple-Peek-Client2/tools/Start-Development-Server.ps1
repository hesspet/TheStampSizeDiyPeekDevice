. "$PSScriptRoot\Gemeinsame-Umgebung.ps1"

Setze-ProjektUmgebung
Teste-Befehl 'npx'

Write-Host 'Starte den Expo Development Server. Das ist der Metro-Server für diese App.'
Write-Host 'Dieses Fenster offen lassen, solange die Development App läuft.'
Write-Host ''

Push-Location $ProjektWurzel
try {
  npx expo start --dev-client --lan --port $DevelopmentServerPort
} finally {
  Pop-Location
}
