# Tools

Diese Dateien sind für Windows gedacht, wenn Codex gerade nicht läuft.

## Reihenfolge

1. `Build Development Version.cmd`
   - Baut die Android Development APK.
   - Ergebnis: `android\app\build\outputs\apk\debug\app-debug.apk`

2. `Starte Development Server.cmd`
   - Startet Expo/Metro auf Port `8081`.
   - Das Fenster offen lassen.

3. `Install Update via ADB.cmd`
   - Installiert die APK per ADB auf Emulator oder Smartphone.
   - Setzt `adb reverse tcp:8081 tcp:8081`.
   - Öffnet den Expo Development Client.

Wenn mehrere Geräte angeschlossen sind, fragt das Installationsskript nach dem Zielgerät.
