---
Datum: 19.05.2026
Version: 2
Autor: Peter Heß, Germany (+Codex)
---
# BoardTest

`BoardTest` ist eine ESP32-C3-Test-Firmware für das 0,42-Zoll-OLED-Entwicklungsboard aus `../docs/ESP32-C3 -OLED-Entwicklungsboard.description.md`.

## Zweck

Die Firmware prüft die zentralen Board-Funktionen:

- OLED-Initialisierung über I2C auf GPIO5 und GPIO6.
- Startanzeige mit Programmname und Builddatum im Format `DD.MM.YYYY`.
- Button-Auswertung auf GPIO9 mit internem Pull-up.
- Anzeige großer Pfeile aus dem U8g2-Font `u8g2_font_open_iconic_arrow_4x_t`.
- Anzeige eines vollständigen Pokerdecks mit 52 Karten plus 2 Joker.
- Selbst gezeichnete große Spielkartensymbole für Herz, Karo, Kreuz und Pik.
- Kartenanzeige in der Reihenfolge Farbe zuerst, dann Wert, zum Beispiel `Herz X` für Herz 10.
- Bei jedem stabil erkannten Tastendruck wechselt die Anzeige zum nächsten Testbild.
- Serieller Programmheader über USB-Serial mit Programmname, Builddatum, Buildzeit und Debuglevel.
- USB-Serial wartet beim Start kurz auf eine Monitor-Verbindung, damit der Startheader nach einem Reset sichtbar wird.

## Kartenanzeige

Die Kartenlogik liegt isoliert in `include/PlayingCardDisplay.h` und `src/PlayingCardDisplay.cpp`.

Aufrufbar sind 54 Kartenindizes:

- `0` bis `51`: normales Pokerdeck mit den Farben Herz, Karo, Kreuz und Pik.
- `52`: Joker 1, angezeigt als `J1`.
- `53`: Joker 2, angezeigt als `J2`.

Die Werte werden angezeigt als `1`, `2`, `3`, `4`, `5`, `6`, `7`, `8`, `9`, `X`, `J`, `Q`, `K`. `1` steht für Ass, `X` steht für 10.

## Bedienung

- Nach dem Flashen zeigt das OLED kurz `BoardTest`, `Build:` und das Builddatum.
- Danach zeigt das OLED einen großen Pfeil.
- Jeder Druck auf den Button GPIO9 schaltet eine Position weiter.
- Die Reihenfolge ist oben, unten, links, rechts, danach das vollständige Pokerdeck inklusive `J1` und `J2`, danach wieder oben.

## Hardwarebelegung

| Funktion | GPIO | Hinweis |
| --- | --- | --- |
| OLED SDA | GPIO5 | I2C-Datenleitung |
| OLED SCL | GPIO6 | I2C-Taktleitung |
| Button | GPIO9 | `INPUT_PULLUP`, gedrückt ist `LOW` |

## Debug und Konfiguration

Die Konfiguration liegt in `include/config.h`.

Der Debuglevel ist über `configuredDebugLevel` schaltbar:

- `DebugLevel::none`
- `DebugLevel::info`
- `DebugLevel::debug`
- `DebugLevel::trace`

Auch bei `DebugLevel::none` wird beim Start ein kurzer Programmheader ausgegeben. Debugausgaben gehen über USB-Serial, damit das Verhalten der funktionierenden Referenz-Firmware entspricht.

## PlatformIO

Build:

```powershell
cd BoardTest
pio run
```

Upload:

```powershell
cd BoardTest
pio run --target upload
```

Monitor:

```powershell
cd BoardTest
pio device monitor --port COM6 --baud 115200
```

Falls das Board nicht auf `COM6` liegt, müssen `upload_port`, `monitor_port` und der Monitor-Befehl angepasst werden.

Für das aktuell angeschlossene Board ist `COM6` konfiguriert. `ARDUINO_USB_MODE=1` mit `ARDUINO_USB_CDC_ON_BOOT=1` nutzt die ESP32-C3-Hardware-USB-CDC/JTAG-Schnittstelle. Diese PlatformIO-Konfiguration ist bewusst an `C:\dev\AnwesenheitsAnzeige\firmware\AnwesenheitsAnzeige.Esp32` angeglichen.
