---
Datum: 19.05.2026
Version: 5
Autor: Peter Heß, Germany (+Codex)
---
# Projektübersicht

Projektname: The Stamp Size Diy Peek Device

Dieses Repository sammelt mehrere PlatformIO-Unterprojekte für das kleine ESP32-C3-OLED-Entwicklungsboard. Die generische Hardwarebeschreibung liegt unter [docs/ESP32-C3 -OLED-Entwicklungsboard.description.md](docs/ESP32-C3%20-OLED-Entwicklungsboard.description.md).

## Entwicklungsumgebung

- PlatformIO
- ESP32 Arduino Framework
- Boardprofil: `esp32-c3-devkitm-1`
- OLED: SSD1306, 72 x 40 Pixel, I2C über GPIO5 und GPIO6
- Board-Button: GPIO9 mit `INPUT_PULLUP`, gedrückt ist `LOW`
- Serial Monitor: 115200 Baud über USB-CDC

## Unterprojekte

| Unterprojekt | Zweck |
| --- | --- |
| [BoardTest](BoardTest/PROJEKTUEBERSICHT.md) | Test-Firmware `1.2.0` für OLED, Buildanzeige, GPIO9-gesteuerten automatischen Testmodus, projektweite Display-Library, selbst gezeichnete Pfeile, ASCII-Zeichenanzeige, Spielkarten und invertierte Anzeige des ESP32-C3-OLED-Boards. |

## Projektweite Libraries

| Library | Zweck |
| --- | --- |
| [StampDisplay](lib/StampDisplay/PROJEKTUEBERSICHT.md) | Lokale PlatformIO-Library für wiederverwendbare Display-Darstellungsklassen: Pfeile, ASCII-Zeichen und Spielkarten. |

## Projektregeln

- Jedes Unterprojekt enthält eine eigene `PROJEKTUEBERSICHT.md`.
- Jedes ESP32-Programm enthält eine `config.h`.
- Debugausgaben sind in `config.h` über `none`, `info`, `debug` und `trace` schaltbar.
- Auch bei Debuglevel `none` wird beim Start ein kurzer Programmheader mit Builddatum und Konfiguration ausgegeben.
- User-facing Strings und Dokumentation sind deutsch.
- Variablen- und Methodennamen sind verständlich und ausgeschrieben.
- Datumsangaben werden im Format `DD.MM.YYYY` geschrieben.
- OTA wird nicht vorbereitet, solange es nicht explizit gefordert ist.

## Bibliotheken und Updates

Bibliotheken sollen nach Möglichkeit projektlokal versioniert werden. Projektweit wiederverwendbare eigene Klassen liegen unter `lib/` als lokale PlatformIO-Libraries. Die Library `StampDisplay` enthält die Display-Darstellungsklassen, die bisher direkt im Unterprojekt `BoardTest` lagen.

Für U8g2 wurde am 19.05.2026 bewusst entschieden, die Bibliothek wegen ihrer Größe nicht in `lib/` zu übernehmen. Das Unterprojekt `BoardTest` verwendet deshalb die in der Boardbeschreibung empfohlene PlatformIO-Abhängigkeit `olikraus/U8g2@^2.36.12`.

Updateverfahren:

1. Gewünschte Bibliotheksversion in der jeweiligen `platformio.ini` anpassen.
2. `pio run` im Unterprojekt ausführen.
3. Firmware auf dem Board prüfen.
4. Versionsänderung und relevante Hinweise in der Unterprojekt-Übersicht dokumentieren.
