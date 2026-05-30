param(
  [string] $Geraet
)

. "$PSScriptRoot\Gemeinsame-Umgebung.ps1"

Setze-ProjektUmgebung
Teste-Befehl 'adb'

if (-not (Test-Path -LiteralPath $ApkPfad)) {
  throw "APK wurde nicht gefunden. Bitte zuerst 'Build Development Version.cmd' ausführen. Erwarteter Pfad: $ApkPfad"
}

$ZielGeraet = Waehle-AdbGeraet -VorgegebenesGeraet $Geraet

Write-Host "Installiere APK auf ADB-Gerät: $ZielGeraet"
adb -s $ZielGeraet install -r $ApkPfad
Teste-LetztesProgramm 'APK-Installation'

Write-Host ''
Write-Host "Setze Portweiterleitung für Metro/Expo auf Port $DevelopmentServerPort ..."
adb -s $ZielGeraet reverse "tcp:$DevelopmentServerPort" "tcp:$DevelopmentServerPort"
Teste-LetztesProgramm 'ADB-Portweiterleitung'

Write-Host ''
Write-Host 'Öffne den Expo Development Client auf dem Gerät ...'
adb -s $ZielGeraet shell am start -W -a android.intent.action.VIEW -d $DevelopmentClientAdresse $AppPaketName
Teste-LetztesProgramm 'Start des Expo Development Clients'

Write-Host ''
Write-Host 'Fertig. Falls die App keine Verbindung findet, bitte zuerst den Development Server starten.'
