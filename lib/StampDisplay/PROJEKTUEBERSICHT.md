---
Datum: 11.06.2026
Version: 5
Autor: Peter Heß, Germany (+Codex)
---
# StampDisplay

`StampDisplay` ist eine projektweite lokale PlatformIO-Library für wiederverwendbare Display-Darstellungsklassen auf dem ESP32-C3-OLED-Board.

## Zweck

Die Library bündelt Darstellungslogik, die von mehreren Unterprojekten genutzt werden kann:

- große Pfeile für acht Kompassrichtungen.
- ein oder zwei große Symbole auf ASCII-Basis.
- ESP-Symbole in Form von Kreis, Kreuz, Wellen, Quadrat und Stern.
- Spielkarten eines vollständigen Pokerdecks inklusive zwei Joker mit englischen Pokerbezeichnungen.
- normale und invertierte Darstellung mit hellem Hintergrund.

Die Spielkartendarstellung wird von `BoardTest` und `BlePrompter` gemeinsam genutzt. Für `BlePrompter` ist wichtig, dass die Anzeige zur englischen BLE-Befehlssyntax passt: `Heart`, `Diamond`, `Clubs`, `Spade`, `Ace`, `Jack`, `Queen`, `King`; `X` bleibt die sichtbare 10.

Die Spielkartenfarben Herz, Karo, Kreuz und Pik werden als gerasterte Vektorformen gezeichnet. Dadurch wirken die Kartenfarben auf dem kleinen OLED klarer und konsistenter mit der CYB/CYD-Darstellung.

## Dateien

| Datei | Zweck |
| --- | --- |
| `include/StampDisplay/ArrowDisplay.h` | Öffentliche API für Pfeildarstellung. |
| `src/ArrowDisplay.cpp` | Implementierung der Pfeildarstellung. |
| `include/StampDisplay/AsciiCharacterDisplay.h` | Öffentliche API für Symboldarstellung auf ASCII-Basis. |
| `src/AsciiCharacterDisplay.cpp` | Implementierung der Symboldarstellung auf ASCII-Basis. |
| `include/StampDisplay/EspSymbolDisplay.h` | Öffentliche API für ESP-Symboldarstellung. |
| `src/EspSymbolDisplay.cpp` | Implementierung der ESP-Symboldarstellung mit U8g2-Primitiven. |
| `include/StampDisplay/PlayingCardDisplay.h` | Öffentliche API für Spielkartendarstellung. |
| `src/PlayingCardDisplay.cpp` | Implementierung der Spielkartendarstellung. |
| `library.json` | PlatformIO-Metadaten der lokalen Library. |

## Nutzung

Unterprojekte binden die projektweite Library über ihre `platformio.ini` ein:

```ini
lib_extra_dirs =
    ../lib
```

Danach können die Header eindeutig über den Library-Präfix eingebunden werden:

```cpp
#include <StampDisplay/ArrowDisplay.h>
#include <StampDisplay/AsciiCharacterDisplay.h>
#include <StampDisplay/EspSymbolDisplay.h>
#include <StampDisplay/PlayingCardDisplay.h>
```

## Abhängigkeiten

Die Library nutzt das Arduino Framework und U8g2. U8g2 bleibt bewusst in den jeweiligen Unterprojekten über `lib_deps` konfiguriert, damit jedes Unterprojekt seine benötigten externen Abhängigkeiten explizit ausweist.
