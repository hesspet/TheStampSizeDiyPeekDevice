---
Datum: 19.05.2026
Version: 6
Autor: Peter Heß, Germany (+Codex)
---
# BoardTest

`BoardTest` ist eine ESP32-C3-Test-Firmware für das 0,42-Zoll-OLED-Entwicklungsboard aus `../docs/ESP32-C3 -OLED-Entwicklungsboard.description.md`.

## Zweck

Die Firmware prüft die zentralen Board-Funktionen:

- Programmversion: `1.2.0`.
- OLED-Initialisierung über I2C auf GPIO5 und GPIO6.
- Startanzeige mit Programmname, Programmversion und Builddatum im Format `DD.MM.YYYY`.
- Button-Auswertung auf GPIO9 mit internem Pull-up.
- Selbst gezeichnete große Pfeile für die Kompassrichtungen `N`, `NO`, `O`, `SO`, `S`, `SW`, `W` und `NW`.
- Anzeige von ein oder zwei ASCII-Zeichen über eine eigene Hilfsklasse.
- Anzeige eines vollständigen Pokerdecks mit 52 Karten plus 2 Joker.
- Selbst gezeichnete große Spielkartensymbole für Herz, Karo, Kreuz und Pik.
- Kartenanzeige in der Reihenfolge Farbe zuerst, dann Wert, zum Beispiel `Herz X` für Herz 10.
- GPIO9 startet und stoppt einen automatischen Testmodus.
- Jede Testanzeige bleibt eine Sekunde sichtbar und wechselt dann automatisch zur nächsten Anzeige.
- Der Testmodus zeigt die komplette Sequenz zuerst normal und danach invertiert mit hellem Hintergrund.
- Serieller Programmheader über USB-Serial mit Programmname, Builddatum, Buildzeit und Debuglevel.
- USB-Serial wartet beim Start kurz auf eine Monitor-Verbindung, damit der Startheader nach einem Reset sichtbar wird.

## Gemeinsame Display-Library

Die wiederverwendbaren Display-Darstellungsklassen liegen nicht mehr direkt in `BoardTest`, sondern in der projektweiten lokalen PlatformIO-Library `../lib/StampDisplay`.

`BoardTest/platformio.ini` bindet die Library ein:

```ini
lib_extra_dirs =
    ../lib
```

Die Header werden über den Library-Präfix eingebunden:

```cpp
#include <StampDisplay/ArrowDisplay.h>
#include <StampDisplay/AsciiCharacterDisplay.h>
#include <StampDisplay/PlayingCardDisplay.h>
```

## Kartenanzeige

Die Kartenlogik liegt isoliert in `../lib/StampDisplay/include/StampDisplay/PlayingCardDisplay.h` und `../lib/StampDisplay/src/PlayingCardDisplay.cpp`.

Aufrufbar sind 54 Kartenindizes:

- `0` bis `51`: normales Pokerdeck mit den Farben Herz, Karo, Kreuz und Pik.
- `52`: Joker 1, angezeigt als `J1`.
- `53`: Joker 2, angezeigt als `J2`.

Die Werte werden angezeigt als `1`, `2`, `3`, `4`, `5`, `6`, `7`, `8`, `9`, `X`, `J`, `Q`, `K`. `1` steht für Ass, `X` steht für 10.

## Pfeil- und ASCII-Anzeige

Die Pfeilanzeige liegt isoliert in `../lib/StampDisplay/include/StampDisplay/ArrowDisplay.h` und `../lib/StampDisplay/src/ArrowDisplay.cpp`.

Aufrufbar sind acht Kompassrichtungen:

- `CompassDirection::N`
- `CompassDirection::NO`
- `CompassDirection::O`
- `CompassDirection::SO`
- `CompassDirection::S`
- `CompassDirection::SW`
- `CompassDirection::W`
- `CompassDirection::NW`

Die Pfeile werden mit U8g2-Primitiven selbst gezeichnet und nutzen keine Pfeil-Fonts mehr. Die Klasse unterstützt normale und invertierte Darstellung.

Die ASCII-Anzeige liegt in `../lib/StampDisplay/include/StampDisplay/AsciiCharacterDisplay.h` und `../lib/StampDisplay/src/AsciiCharacterDisplay.cpp`. Sie zeichnet ein oder zwei druckbare ASCII-Zeichen groß und zentriert. Bei einem Zeichen wird dieses einzelne Zeichen in der Mitte ausgegeben. Die Klasse unterstützt normale und invertierte Darstellung.

Auch `PlayingCardDisplay` unterstützt normale und invertierte Darstellung.

## Bedienung

- Nach dem Flashen zeigt das OLED kurz `BoardTest`, die Programmversion, `Build:` und das Builddatum.
- Danach zeigt das OLED `Test bereit` und `IO9 Start`.
- Ein Druck auf GPIO9 startet den automatischen Testmodus.
- Während der Testmodus läuft, stoppt ein weiterer Druck auf GPIO9 die Sequenz.
- Jede Anzeige bleibt `1000 ms` sichtbar.
- Die normale Sequenz ist: alle Pfeile, 16 zufällige Spielkarten inklusive möglicher Joker, Zähler `1` bis `12`, 10 zufällige ASCII-Einzelzeichen, 10 zufällige ASCII-Zeichenpaare.
- Danach wird dieselbe Sequenzart invertiert mit hellem Hintergrund wiederholt.
- Nach der invertierten Sequenz beginnt der Ablauf wieder von vorne.

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
