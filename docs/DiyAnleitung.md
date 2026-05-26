# DIY-Anleitung

Datum: 21.05.2026

Diese Anleitung erklärt den Bau und die Nutzung von `The Stamp Size Diy Peek Device` für Menschen, die nicht täglich Software kompilieren. Sie richtet sich besonders an Zauberer, die ein kleines, unauffälliges OLED-Gerät per Bluetooth Low Energy steuern möchten.

## Einleitung

Das Projekt besteht aus einem sehr kleinen ESP32-C3-Board mit OLED-Display und einer Firmware namens `BlePrompter`. Die Firmware empfängt Befehle über Bluetooth Low Energy und zeigt danach Text, Pfeile, Symbole oder Spielkarten auf dem Display an.

Die Idee ist einfach:

1. Du spielst die Firmware auf das ESP32-C3-OLED-Board.
2. Das Board startet als Bluetooth-Gerät mit dem Namen `BlePrompter`.
3. Du öffnest eine BLE-App.
4. Du sendest einen Befehl.
5. Das Display zeigt die gewünschte Information.

Für Vorführungen ist wichtig: Das Gerät arbeitet nicht wie ein normales Bluetooth-Lautsprecher-Pairing. Es verwendet Bluetooth Low Energy und den Nordic UART Service. Darum brauchst du eine App, die BLE-UART sprechen kann.

## Materialliste

Für den ersten funktionierenden Aufbau brauchst du:

- ESP32-C3-OLED-Entwicklungsboard mit SSD1306-OLED, 72 x 40 Pixel.
- USB-C-Kabel, das Daten übertragen kann. Manche Ladekabel funktionieren nicht.
- Windows-PC mit USB-Anschluss.
- PlatformIO zum Kompilieren und Aufspielen der Firmware.
- Android-Smartphone.
- Android-App wie nRF Connect, Serial Bluetooth Terminal oder MacroDroid mit BLE-Plugin.

Nützlich, aber nicht zwingend:

- Kleine Powerbank oder LiPo-Stromversorgung für mobile Tests.
- Gehäuse oder Schrumpfschlauch für eine sichere, unauffällige Unterbringung.
- Klebeband oder Magnete, falls das Gerät in ein Requisit eingebaut wird.

## Überblick über die Ordner

Die wichtigsten Ordner im Repository sind:

| Ordner | Zweck |
| --- | --- |
| `BlePrompter` | Firmware für das ESP32-C3-OLED-Board. Diese Firmware wird kompiliert und auf das Board geladen. |
| `BoardTest` | Test-Firmware für Display, Pfeile, ASCII-Zeichen und Spielkarten. |
| `docs` | Hardwaredatenblätter und diese DIY-Anleitung. |
| `lib/StampDisplay` | Gemeinsame Display-Bibliothek für Pfeile, Symbole und Spielkarten. |

Für den normalen Bau ist zuerst `BlePrompter` wichtig. `BoardTest` ist hilfreich, wenn du prüfen möchtest, ob Display und Board grundsätzlich funktionieren.

## Vorbereitung des Rechners

Installiere zuerst die Werkzeuge:

1. Installiere Python.
2. Installiere PlatformIO.
3. Öffne danach eine neue PowerShell, damit Windows die neuen Programme findet.

PlatformIO kann mit folgendem Befehl installiert oder aktualisiert werden:

```powershell
python -m pip install --user -U platformio
```

Prüfe danach, ob PlatformIO erreichbar ist:

```powershell
pio --version
```

Falls Windows `pio` nicht findet, kann im Projekt auch der direkte Pfad verwendet werden:

```powershell
& "$env:APPDATA\Python\Python313\Scripts\pio.exe" --version
```

## Board anschließen

Schließe das ESP32-C3-OLED-Board per USB an den Rechner an. Danach sollte Windows einen COM-Port anzeigen.

Zum Prüfen der seriellen Ports kannst du ausführen:

```powershell
python -m serial.tools.list_ports -v
```

Im Projekt ist aktuell `COM6` als Upload- und Monitor-Port für `BlePrompter` eingetragen. Falls dein Board einen anderen Port bekommt, ändere in `BlePrompter/platformio.ini` diese Zeilen:

```ini
upload_port = COM6
monitor_port = COM6
```

Setze dort deinen Port ein, zum Beispiel `COM7`.

## Anleitung zum Kompilieren der Firmware

Wechsle zuerst in den Firmware-Ordner:

```powershell
cd C:\dev\TheStampSizeDiyPeekDevice\BlePrompter
```

Starte dann den Build:

```powershell
pio run
```

Wenn `pio` nicht direkt gefunden wird, nutze:

```powershell
& "$env:APPDATA\Python\Python313\Scripts\pio.exe" run
```

Beim ersten Build lädt PlatformIO die benötigten Bibliotheken herunter. Das kann einige Minuten dauern. Wichtig sind hier vor allem:

- `olikraus/U8g2` für das OLED-Display.
- `h2zero/NimBLE-Arduino` für Bluetooth Low Energy.

Ein erfolgreicher Build endet sinngemäß mit `SUCCESS`. Wenn stattdessen eine Fehlermeldung erscheint, lies die letzten 20 bis 40 Zeilen. Dort steht meistens, ob eine Bibliothek fehlt, der Port blockiert ist oder ein Schreibfehler im Code vorliegt.

## Firmware auf das Board laden

Bleibe im Ordner `BlePrompter` und führe aus:

```powershell
pio run --target upload
```

Oder mit direktem PlatformIO-Pfad:

```powershell
& "$env:APPDATA\Python\Python313\Scripts\pio.exe" run --target upload
```

Nach dem Upload startet das Board neu. Auf dem OLED sollte kurz erscheinen:

- `BlePrompter`
- die Versionsnummer
- `Build:`
- das Builddatum im Format `DD.MM.YYYY`

Danach zeigt das Gerät den BLE-Bereitschaftsstatus.

## Seriellen Monitor nutzen

Der serielle Monitor ist die einfachste Methode, um Startmeldungen und Debugausgaben zu sehen:

```powershell
pio device monitor --port COM6 --baud 115200
```

Wenn dein Board nicht auf `COM6` liegt, ersetze `COM6` durch deinen Port.

Beenden kannst du den Monitor meistens mit `Strg+C`. Falls ein späterer Upload mit `PermissionError(13, Zugriff verweigert)` fehlschlägt, ist oft noch ein Monitor-Fenster geöffnet. Beende es und versuche den Upload erneut.

## Erster Funktionstest

Nach dem Flashen kannst du das Gerät mit einer BLE-App testen. In nRF Connect oder einer vergleichbaren App:

1. Suche nach einem BLE-Gerät mit dem Namen `BlePrompter`.
2. Verbinde dich mit dem Gerät.
3. Öffne den Nordic UART Service.
4. Schreibe Text in die RX-Characteristic.
5. Prüfe das OLED-Display.

Die wichtigsten UUIDs sind:

| Zweck | UUID |
| --- | --- |
| Service | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` |
| RX, Schreiben zum Gerät | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` |
| TX, Antworten vom Gerät | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` |

Einfache Testbefehle:

```text
TEXT Hallo
SYMBOL OK
ARROW NE
CARD Heart X
CHX
I1
I0
CL
```

Das Display ist sehr klein. Für gut lesbaren Text sind kurze Wörter besser als lange Sätze.

## Codex als Hilfsmittel nutzen

Codex kann beim Bau helfen, wenn du konkrete Aufgaben formulierst. Gute Aufgaben sind klein, prüfbar und nennen den Ordner, um den es geht.

Beispiele:

```text
Prüfe im Projekt BlePrompter, ob die Firmware kompiliert.
```

```text
Ändere den Upload-Port in BlePrompter/platformio.ini von COM6 auf COM7.
```

```text
Erkläre mir die Datei BlePrompter/BEFEHLE.md so, dass ich die Befehle in MacroDroid nachbauen kann.
```

```text
Starte den Build für BlePrompter und fasse die Fehlermeldung verständlich zusammen.
```

Für Laien ist die beste Arbeitsweise:

1. Immer nur eine konkrete Änderung auf einmal beauftragen.
2. Codex zuerst die vorhandenen Dateien lesen lassen.
3. Nach Codeänderungen einen Build ausführen lassen.
4. Fehlermeldungen nicht abschreiben, sondern Codex direkt den Build ausführen lassen.
5. Vor Vorführungen eine kurze Checkliste erstellen lassen.

Codex kann auch Doku verbessern. Sinnvolle Anweisungen sind zum Beispiel:

```text
Ergänze die DIY-Anleitung um einen Abschnitt für MacroDroid.
```

```text
Schreibe eine kurze Vorführ-Checkliste für einen Auftritt.
```

```text
Erstelle eine Tabelle mit allen Karten-Kurzbefehlen.
```

## Typische Probleme und Lösungen

| Problem | Mögliche Ursache | Lösung |
| --- | --- | --- |
| `pio` wird nicht gefunden | PlatformIO-Pfad ist nicht in der Windows-Umgebung | Direkten Pfad `& "$env:APPDATA\Python\Python313\Scripts\pio.exe"` verwenden. |
| Upload bricht mit Zugriff verweigert ab | Serieller Monitor oder anderes Programm blockiert den COM-Port | Monitor schließen und Upload erneut starten. |
| Board erscheint nicht als COM-Port | USB-Kabel kann nur laden oder Treiber fehlt | Datenfähiges USB-Kabel verwenden und Board neu einstecken. |
| BLE-App findet kein Gerät | Gerät ist ausgeschaltet, schläft oder die App filtert zu stark | Gerät neu starten und in der App nach `BlePrompter` suchen. |
| Text ist schlecht lesbar | Display ist sehr klein | Kurze Wörter, Symbole, Pfeile oder Kartenbefehle verwenden. |

## Vorführ-Checkliste

Vor einer Probe oder Vorführung:

1. Board vollständig laden oder zuverlässige Stromversorgung anschließen.
2. Firmware starten und Startanzeige prüfen.
3. BLE-App öffnen.
4. Verbindung mit `BlePrompter` testen.
5. Befehle `SYMBOL OK`, `CHX` und `CL` senden.
6. Display bei normalem Licht und Vorführlicht prüfen.
7. Smartphone-Bildschirmhelligkeit und Energiesparmodus prüfen.
8. Ersatz-USB-Kabel und Powerbank bereitlegen.

## Nächste Schritte

Wenn der Grundaufbau funktioniert, kannst du das Projekt an deine Routine anpassen:

- Eigene Symbolkürzel definieren.
- MacroDroid-Aktionen für bestimmte Situationen bauen.
- Ein kleines Gehäuse oder Requisit für das Board entwerfen.
- Eine zweite Firmware mit reduziertem Funktionsumfang für eine bestimmte Routine erstellen.

Ändere immer nur einen Teil auf einmal und prüfe danach den Build. So bleibt klar, welche Änderung funktioniert und welche Änderung ein Problem verursacht.
