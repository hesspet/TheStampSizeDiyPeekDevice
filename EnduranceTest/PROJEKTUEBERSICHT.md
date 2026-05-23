---
Datum: 23.05.2026
Version: 1
Autor: Peter Heß, Germany (+Codex)
---
# EnduranceTest

`EnduranceTest` ist eine ESP32-C3-Dauertest-Firmware für das 0,42-Zoll-OLED-Entwicklungsboard aus `../docs/ESP32-C3 -OLED-Entwicklungsboard.description.md`.

## Zweck

Die Firmware simuliert einen normalen Nutzungszyklus, um ein Gefühl für die Laufzeit der Board-Akku-Kombination zu bekommen.

- Programmversion: `1.0.0`.
- OLED-Initialisierung über I2C auf GPIO5 und GPIO6.
- USB-Serial mit 115200 Baud über USB-CDC.
- Große App-Partition `huge_app.csv`, damit BLE, WLAN und U8g2 gemeinsam in den Flash passen.
- Automatischer Testzyklus von ungefähr `60 s`.
- BLE-Scan.
- Verschiedene OLED-Darstellungen über die gemeinsame `StampDisplay`-Library.
- WLAN-Scan.
- Simulierte normale Nutzung mit Ruhephasen und wechselnden Displayanzeigen.
- Protokollsammlung im RAM ohne Persistenz.
- Ausgabe aller Protokolle auf USB-Serial als JSON Lines.
- Nach jedem Zyklus WLAN-Verbindung zu `Agathas-Netz-16`.
- UDP-Broadcast der letzten Protokollinformationen auf Port `4210`.
- Nach erfolgreichem Broadcast wird WLAN abgeschaltet und der RAM-Protokollpuffer geleert.

Wenn das Ziel-WLAN nicht erreicht wird, zeigt die Firmware einen Fehler auf dem OLED an und stoppt dauerhaft.

## Hardwarebelegung

| Funktion | GPIO | Hinweis |
| --- | --- | --- |
| OLED SDA | GPIO5 | I2C-Datenleitung |
| OLED SCL | GPIO6 | I2C-Taktleitung |

Der Button GPIO9 wird in diesem Unterprojekt nicht benötigt. Der Dauertest startet automatisch.

## Wichtige Dateien

- `EnduranceTest/platformio.ini`
- `EnduranceTest/include/config.h`
- `EnduranceTest/src/main.cpp`
- `EnduranceTest/BROADCAST_DATENFORMAT.md`

## Konfiguration

Die Konfiguration liegt in `include/config.h`.

Wichtige Werte:

- `wifiNetworkName`: Ziel-WLAN.
- `wifiPassword`: WLAN-Passwort.
- `wifiConnectionTimeoutMillis`: maximale Wartezeit beim Verbinden.
- `enduranceBroadcastPort`: UDP-Port für die Broadcast-Pakete.
- `enduranceCycleDurationMillis`: geplante Zyklusdauer.
- `storedLogRecordCount`: Größe des RAM-Protokollpuffers.

Der Debuglevel ist über `configuredDebugLevel` schaltbar:

- `DebugLevel::none`
- `DebugLevel::info`
- `DebugLevel::debug`
- `DebugLevel::trace`

Auch bei `DebugLevel::none` wird beim Start ein kurzer Programmheader ausgegeben.

## Broadcast

Die Firmware sendet UDP-Broadcasts an `255.255.255.255:4210`. Jede UDP-Nachricht enthält genau ein JSON-Objekt. Details stehen in [BROADCAST_DATENFORMAT.md](BROADCAST_DATENFORMAT.md).

## PlatformIO

Build:

```powershell
cd C:\dev\TheStampSizeDiyPeekDevice\EnduranceTest
pio run
```

Upload:

```powershell
cd C:\dev\TheStampSizeDiyPeekDevice\EnduranceTest
pio run --target upload
```

Monitor:

```powershell
cd C:\dev\TheStampSizeDiyPeekDevice\EnduranceTest
pio device monitor --port COM6 --baud 115200
```

Falls das Board nicht auf `COM6` liegt, müssen `upload_port`, `monitor_port` und der Monitor-Befehl angepasst werden.
