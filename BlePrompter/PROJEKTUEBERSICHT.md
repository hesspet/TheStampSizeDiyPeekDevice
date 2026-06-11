---
Datum: 11.06.2026
Version: 17
Autor: Peter Heß, Germany (+Codex)
---

# BlePrompter

`BlePrompter` ist eine BLE-UART-Firmware für ESP32-Boards. Sie zeigt per BLE empfangene Befehle auf einem Display an. Ziel ist eine einfache externe Steuerung ohne eigene native Smartphone-App. Geeignete Werkzeuge sind Android-Apps wie nRF Connect, Serial Bluetooth Terminal, MacroDroid mit BLE-Plugin und andere BLE-UART-fähige Werkzeuge.

## Unterstützte Boards

| Board | Display | Auflösung | Build-Umgebung |
| --- | --- | --- | --- |
| ESP32-C3 OLED | SSD1306 (I2C, monochrom) | 72 × 40 | `[env:esp32c3]` |
| CYB/CYD | ILI9341 (SPI, Farbe) | 320 × 240 | `[env:cyd]` |

## Architektur

Die Display-Logik ist in mehrere Schichten gekapselt:

```
main.cpp (BLE, Befehle, Sleep-Zyklen)
    ↓
DisplayController (Zeichenfunktionen: drawStartupScreen, drawPromptText, …)
    ↓
DisplayHardware (abstrakte Hardware-Schnittstelle)
    ├── Ssd1306Hardware (OLED, U8g2, I2C)
    └── Ili9341Hardware  (TFT, TFT_eSPI, SPI)
```

**OLED-Pfad:** `DisplayController` → `Ssd1306Hardware` → `U8g2` + `StampDisplay`-Bibliothek (Pfeile, Karten, Würfel, ESP-Symbole)

**CYB-Pfad:** `DisplayController` → `Ili9341Hardware` → `TFT_eSPI` + `CydDisplay`-Modul (gleiche Symbole, TFT-gerecht skaliert)

### Dateiübersicht

**Display-Schicht (`src/display/`, `include/display/`):**

| Datei | Zweck |
| --- | --- |
| `DisplayHardware.h` | Abstrakte Basisklasse für Display-Hardware |
| `Ssd1306Hardware.h/.cpp` | SSD1306-OLED-Treiber (ESP32-C3) |
| `Ili9341Hardware.h/.cpp` | ILI9341-TFT-Treiber (CYB/CYD) |
| `DisplayController.h/.cpp` | Gekapselte Zeichenlogik (board-unabhängig) |
| `CydDisplay.h/.cpp` | Symbol-Zeichenfunktionen für CYB (Pfeile, Karten etc.) |

**Board-Konfiguration:**

| Datei | Zweck |
| --- | --- |
| `include/config.h` | Gemeinsame + board-spezifische Konfiguration |
| `include/TFT_eSPI_Setup_CYD.h` | TFT_eSPI-Pin-Belegung für CYB |
| `platformio.ini` | Build-Umgebungen `esp32c3` und `cyd` |

## Hardwarebelegung

### ESP32-C3 OLED

| Funktion | GPIO | Hinweis |
| --- | --- | --- |
| OLED SDA | GPIO5 | I2C-Datenleitung |
| OLED SCL | GPIO6 | I2C-Taktleitung |
| Button | GPIO9 | `INPUT_PULLUP`, gedrückt ist `LOW` |

### CYB/CYD

| Funktion | GPIO | Hinweis |
| --- | --- | --- |
| TFT MOSI | GPIO13 | SPI-Daten |
| TFT MISO | GPIO12 | SPI-Daten |
| TFT SCLK | GPIO14 | SPI-Takt |
| TFT CS | GPIO15 | SPI-Chip-Select |
| TFT DC | GPIO2 | Data/Command |
| TFT RST | -1 | nicht verbunden |
| Backlight | GPIO27 | `HIGH` = ein |
| Button | GPIO0 | BOOT-Button |

## Start und Schlafzyklus

### ESP32-C3 OLED

Nach Bestromung oder Reset startet die Firmware den zyklischen Tiefschlafmodus mit einem ersten Wachfenster von `10 s`. Danach läuft der Zyklus mit `30 s` Schlafdauer und `10 s` Wachfenster weiter.

Bei Timer-Aufwachen startet BLE für das Wachfenster. Nach jedem fünften Zyklus wird das Wachfenster auf `60 s` verlängert. Eine BLE-Verbindung pausiert den Zyklus.

Bei regulären Starts zeigt das OLED kurz Programmname, Version und Builddatum, dann den BLE-Bereitschaftsstatus.

### CYB/CYD

Das CYD-Board startet ohne zyklischen Tiefschlaf. Nach dem Einschalten zeigt das TFT den Startbildschirm und danach den BLE-Bereitschaftsstatus. BLE-Sleep-Befehle (`SLEEP DISPLAY`, `SLEEP DEEP`, etc.) funktionieren wie auf dem OLED, deaktivieren aber statt des OLED das TFT-Backlight.

## BLE (beide Boards identisch)

- Bluetooth-Name: `BlePrompter-xxxx`, wobei `xxxx` aus der ESP32-Chip-ID abgeleitet wird
- Protokoll: BLE-UART kompatibel zum Nordic UART Service
- Service-UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- RX-Characteristic: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- TX-Characteristic: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
- Pairing: einfaches BLE-Bonding ohne Passkey

## Befehle (beide Boards identisch)

Die Befehlsdokumentation liegt in `./BEFEHLE.md`.

Unterstützt werden: `TEXT`, `SYMBOL`, `ESP`, `ARROW`, `CARD`, `CUBE`, `INVERT`, `U1`, `U0`, `CLEAR`, `SLEEP`, `WAKE`, `HELP` sowie Kurz-Aliasse.

## Display-Bibliotheken

**OLED-Pfad:** Nutzt `../lib/StampDisplay` für Pfeile, Symbole, ESP-Symbole, Würfel und Spielkarten. Die Spielkartenfarben werden als gerasterte Vektorformen gezeichnet, damit Herz, Karo, Kreuz und Pik auf dem kleinen OLED konsistenter und hochwertiger wirken.

**CYB-Pfad:** Nutzt das eingebaute `CydDisplay`-Modul für die gleichen Symbole, skaliert auf das größere Farbdisplay. Die Spielkartenfarben nutzen dieselben gerasterten Vektorformen wie der OLED-Pfad, aber in höherer Rasterauflösung.

## Debug und Konfiguration

Die Konfiguration liegt in `include/config.h`.

Der Debuglevel ist über `configuredDebugLevel` schaltbar: `none`, `info`, `debug`, `trace`. Auch bei `none` wird ein Programmheader ausgegeben.

## PlatformIO — Build und Upload

### ESP32-C3 OLED (Standard)

```powershell
cd C:\dev\TheStampSizeDiyPeekDevice\BlePrompter
pio run -e esp32c3
pio run -e esp32c3 --target upload
pio device monitor --port COM6 --baud 115200
```

### CYB/CYD

```powershell
cd C:\dev\TheStampSizeDiyPeekDevice\BlePrompter
pio run -e cyd
pio run -e cyd --target upload
pio device monitor --port COM11 --baud 115200
```

**Wichtig vor dem ersten CYD-Build:** `pio run -e cyd --target clean` ausführen, damit die TFT_eSPI-Library mit der neuen Konfiguration neu gebaut wird.

### Board wechseln

Um zwischen den Boards zu wechseln, die gewünschte Umgebung mit `-e <env>` angeben:

- `-e esp32c3` für das ESP32-C3-OLED-Board (Standard, `COM6`)
- `-e cyd` für das CYB/CYD-Board (`COM11`)

Bei COM-Port-Änderungen die `upload_port` und `monitor_port` in `platformio.ini` anpassen.

## Neues Board hinzufügen

1. `DisplayHardware`-Unterklasse in `include/display/` und `src/display/` erstellen
2. Board-spezifische Konfiguration in `include/config.h` unter `#ifdef BOARD_XXX` ergänzen
3. `platformio.ini`: Neue `[env:xxx]`-Sektion mit `-D BOARD_XXX` in `build_flags`
4. Bei neuen Display-Typen: Symbol-Zeichenmodul analog zu `CydDisplay` erstellen
5. `DisplayController` um `#ifdef BOARD_XXX`-Blöcke erweitern
6. Diese Dokumentation aktualisieren
