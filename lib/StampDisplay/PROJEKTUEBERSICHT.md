---
Datum: 19.05.2026
Version: 1
Autor: Peter Heß, Germany (+Codex)
---
# StampDisplay

`StampDisplay` ist eine projektweite lokale PlatformIO-Library für wiederverwendbare Display-Darstellungsklassen auf dem ESP32-C3-OLED-Board.

## Zweck

Die Library bündelt Darstellungslogik, die von mehreren Unterprojekten genutzt werden kann:

- große Pfeile für acht Kompassrichtungen.
- ein oder zwei große ASCII-Zeichen.
- Spielkarten eines vollständigen Pokerdecks inklusive zwei Joker.
- normale und invertierte Darstellung mit hellem Hintergrund.

## Dateien

| Datei | Zweck |
| --- | --- |
| `include/StampDisplay/ArrowDisplay.h` | Öffentliche API für Pfeildarstellung. |
| `src/ArrowDisplay.cpp` | Implementierung der Pfeildarstellung. |
| `include/StampDisplay/AsciiCharacterDisplay.h` | Öffentliche API für ASCII-Zeichendarstellung. |
| `src/AsciiCharacterDisplay.cpp` | Implementierung der ASCII-Zeichendarstellung. |
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
#include <StampDisplay/PlayingCardDisplay.h>
```

## Abhängigkeiten

Die Library nutzt das Arduino Framework und U8g2. U8g2 bleibt bewusst in den jeweiligen Unterprojekten über `lib_deps` konfiguriert, damit jedes Unterprojekt seine benötigten externen Abhängigkeiten explizit ausweist.
