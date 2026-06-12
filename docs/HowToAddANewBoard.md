# Neues Board in BlePrompter einbinden

Stand: 12.06.2026

Dieses Dokument beschreibt die Stellen, die bei einer neuen Board-Variante in `BlePrompter` erweitert werden müssen. Es basiert auf der Integration des `M5StickC Plus2` und soll in späteren Chats die erneute Wissenssuche im Code reduzieren.

## Zielbild

`BlePrompter` trennt BLE-Logik, board-unabhängige Displaylogik und konkrete Display-Hardware:

```text
src/main.cpp
    BLE, Befehle, Button, Schlaflogik, Board-Auswahl

src/display/DisplayController.cpp
    gemeinsame Bildschirmseiten und Dispatcher pro Board

include/display/DisplayHardware.h
    abstrakte Display-Schnittstelle

include/display/*Hardware.h + src/display/*Hardware.cpp
    konkrete Hardwareanbindung

include/display/*Display.h + src/display/*Display.cpp
    board-spezifische Symbolzeichnungen
```

## Neue Dateien

Für ein neues Display-Board werden normalerweise vier Dateien angelegt:

```text
BlePrompter/include/display/<BoardName>Hardware.h
BlePrompter/src/display/<BoardName>Hardware.cpp
BlePrompter/include/display/<BoardName>Display.h
BlePrompter/src/display/<BoardName>Display.cpp
```

`<BoardName>Hardware` implementiert `DisplayHardware`.

`<BoardName>Display` zeichnet Pfeile, ASCII-Zeichen, Spielkarten, Würfel und ESP-Symbole für die konkrete Displaygröße.

## DisplayHardware implementieren

Pflichtdatei:

```text
BlePrompter/include/display/DisplayHardware.h
```

Die neue Hardwareklasse muss alle Methoden aus `DisplayHardware` implementieren:

```cpp
class NewBoardHardware : public DisplayHardware
{
public:
    void begin() override;
    void clearBuffer() override;
    void sendBuffer() override;
    uint8_t *getBufferPtr() override;
    size_t getBufferSize() override;

    void setFont(const void *font) override;
    void setFontMode(uint8_t mode) override;
    void enableUTF8Print() override;
    void setCursor(int16_t x, int16_t y) override;
    void print(const char *text) override;
    void print(int32_t value) override;
    int16_t getStrWidth(const char *text) override;

    int16_t getDisplayWidth() override;
    int16_t getDisplayHeight() override;

    void setDrawColor(uint16_t color) override;
    void drawBox(int16_t x, int16_t y, int16_t width, int16_t height) override;
    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2) override;
    void drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) override;
    void drawCircle(int16_t x, int16_t y, int16_t radius) override;
    void drawDisc(int16_t x, int16_t y, int16_t radius) override;
    void drawFrame(int16_t x, int16_t y, int16_t width, int16_t height) override;
    void drawPixel(int16_t x, int16_t y) override;
    void fillScreen(uint16_t color) override;

    void setDisplayRotation(uint8_t rotation) override;
    void enterHardwareSleep() override;
    void wakeFromHardwareSleep() override;
    void deactivateBeforeDeepSleep() override;
    void *getRawDisplay() override;
};
```

Bei direkt rendernden TFT-Bibliotheken wie `TFT_eSPI`, `M5GFX` oder `LovyanGFX` gilt:

- `clearBuffer()` darf direkt den Bildschirm leeren.
- `sendBuffer()` ist meistens ein No-Op.
- `getBufferPtr()` liefert `nullptr`.
- `getBufferSize()` liefert `0`.
- `getRawDisplay()` wird nur gebraucht, wenn anderer Code direkten Zugriff auf die native Displayinstanz benötigt.

## Optionale Akkuanzeige

Bei Boards mit kleinem Akku soll geprüft werden, ob die Board-Bibliothek den Ladestand oder mindestens die Akkuspannung onboard bereitstellt. Falls keine onboard Messung möglich ist und keine externe Messhardware vorgesehen ist, keine Akkuanzeige implementieren.

Beispiel M5StickC Plus2:

```cpp
int32_t getBatteryLevelPercent();
int16_t getBatteryVoltageMillivolts();
void drawBatteryLevelLine();
```

Die M5StickC-Plus2-Implementierung nutzt `M5.Power.getBatteryLevel()` und fällt bei Fehlern auf `M5.Power.getBatteryVoltage()` zurück. Die Anzeige ist ein horizontaler Strich über die lange Landscape-Seite. Die Länge entspricht dem Prozentwert:

- über `40 %`: grün
- `21 %` bis `40 %`: gelb
- `0 %` bis `20 %`: rot

Die Leiste wird im M5-Pfad in `prepareM5StickTextDisplay(...)` und `M5StickDisplay::prepareDisplay(...)` gezeichnet. Sie darf nicht in `enterHardwareSleep()` oder `deactivateBeforeDeepSleep()` erneut aktiviert werden, weil das Display im Schlafzyklus ausgeschaltet bleiben muss.

## Symbolmodul anlegen

Ein neues Symbolmodul soll dieselbe API wie `CydDisplay` und `M5StickDisplay` anbieten:

```cpp
namespace NewBoardDisplay
{
void drawArrow(DisplayHardware &hardware, CompassDirection compassDirection, bool inverted);
void drawAsciiCharacters(DisplayHardware &hardware, const char *text, bool inverted);
void drawPlayingCard(DisplayHardware &hardware, uint8_t cardIndex, bool inverted);
void drawDiceFace(DisplayHardware &hardware, uint8_t faceValue, bool inverted);
void drawEspSymbol(DisplayHardware &hardware, EspSymbol symbol, bool inverted);
}
```

Vorlage für Farb-TFTs:

```text
BlePrompter/include/display/CydDisplay.h
BlePrompter/src/display/CydDisplay.cpp
BlePrompter/include/display/M5StickDisplay.h
BlePrompter/src/display/M5StickDisplay.cpp
```

Wichtig:

- Die Anzeigegröße am Anfang der `.cpp` als Konstanten definieren.
- `CenterX` und `CenterY` daraus berechnen.
- `foregroundColor(inverted)` und `backgroundColor(inverted)` verwenden.
- Kurze Texte groß zentriert zeichnen.
- Mehrzeilige Texte mit `|` trennen, wie im Controller bereits vorgesehen.
- Spielkarten nutzen `54` Karten: `0` bis `51` normales Deck, `52` Joker 1, `53` Joker 2.
- `X` bleibt die sichtbare `10`.

## include/config.h erweitern

Datei:

```text
BlePrompter/include/config.h
```

Neues Board als eigenen `#elif defined(...)`-Block ergänzen:

```cpp
#elif defined(BOARD_NEWBOARD)

// --- NewBoard: kurze Hardwarebeschreibung ---

constexpr uint8_t buttonPin = <gpio>;
constexpr uint8_t buttonPressedLevel = LOW;
constexpr unsigned long buttonDebounceDurationMillis = 50;
constexpr unsigned long deepSleepButtonHoldDurationMillis = 5000;
constexpr unsigned long deepSleepCountdownIntervalMillis = 1000;

constexpr uint8_t clearDisplayMarkerSizePixels = 4;

// Nur als Dokumentationskonstante, wenn das Board beim Start wach bleiben soll.
constexpr bool startCycleSleepOnPowerOn_NewBoard = true;
```

Nicht neu definieren:

```cpp
startCycleSleepOnPowerOn
startCycleSleepAfterBluetoothDisconnect
defaultCycleSleepSeconds
defaultCycleListenSeconds
maximumCommandLength
```

Diese Werte sind gemeinsame Konfiguration und stehen oberhalb der Board-Blöcke.

## src/main.cpp erweitern

Datei:

```text
BlePrompter/src/main.cpp
```

Diese Stellen müssen angepasst werden:

1. Includes im Board-Block ergänzen:

```cpp
#elif defined(BOARD_NEWBOARD)
#include "display/DisplayController.h"
#include "display/NewBoardHardware.h"
#include "display/NewBoardDisplay.h"
```

2. Globale Display-Hardware instanziieren:

```cpp
#elif defined(BOARD_NEWBOARD)
NewBoardHardware displayHardware;
```

3. Startup-Header ergänzen:

```cpp
#elif defined(BOARD_NEWBOARD)
writeLineToOutputs("Board: NewBoard Display 240 x 135 Landscape");
```

4. Power-On-Schlaflogik prüfen:

```cpp
#if defined(BOARD_CYD) || defined(BOARD_NEWBOARD)
    cycleSleepState.active = false;
#else
    ...
#endif
```

5. OLED-only Power-On-Zyklus ausschließen:

```cpp
#if !defined(BOARD_CYD) && !defined(BOARD_NEWBOARD)
    ...
#endif
```

Regel:

- Kleine Akku-Boards sollen standardmäßig zyklisch schlafen.
- Dauerstrom-Boards können direkt bedienbar bleiben.

## DisplayController.h erweitern

Datei:

```text
BlePrompter/include/display/DisplayController.h
```

1. OLED-Pfad ausschließen:

```cpp
#if !defined(BOARD_CYD) && !defined(BOARD_NEWBOARD)
```

2. Private Dispatcher-Methoden ergänzen:

```cpp
#ifdef BOARD_NEWBOARD
    void drawArrowNewBoard(CompassDirection compassDirection, bool inverted);
    void drawAsciiCharactersNewBoard(const char *text, bool inverted);
    void drawPlayingCardNewBoard(uint8_t cardIndex, bool inverted);
    void drawDiceFaceNewBoard(uint8_t faceValue, bool inverted);
    void drawEspSymbolNewBoard(EspSymbol symbol, bool inverted);
#endif
```

## DisplayController.cpp erweitern

Datei:

```text
BlePrompter/src/display/DisplayController.cpp
```

Diese Bereiche müssen erweitert werden:

1. Include-Block:

```cpp
#elif defined(BOARD_NEWBOARD)
#include "display/NewBoardDisplay.h"
#include "display/NewBoardHardware.h"
```

2. Hilfsfunktionen im anonymen Namespace:

- `newBoardCenterX`
- `newBoardCenterY`
- `getNewBoardForegroundColor(...)`
- `getNewBoardBackgroundColor(...)`
- `prepareNewBoardTextDisplay(...)`
- `drawCenteredNewBoardText(...)`

3. Rotation:

```cpp
#elif defined(BOARD_NEWBOARD)
    hardware.setDisplayRotation(upsideDown ? <upsideDownRotation> : <normalRotation>);
```

4. Bildschirmseiten:

- `drawStartupScreen(...)`
- `drawIdleScreen(...)`
- `drawClearDisplayMarkers()`
- `clearDisplay()`
- `drawPromptText(...)`
- `drawSleepStatus(...)`
- `drawCycleListenWindowStatus(...)`
- `drawDeepSleepCountdown(...)`
- `enterDisplaySleep()`

5. Symbol-Dispatcher:

- `drawArrow(...)`
- `drawAsciiCharacters(...)`
- `drawPlayingCard(...)`
- `drawDiceFace(...)`
- `drawEspSymbol(...)`

6. OLED-only Implementierungsblock ausschließen:

```cpp
#if !defined(BOARD_CYD) && !defined(BOARD_NEWBOARD)
```

7. Am Dateiende neue Dispatcher-Implementierungen ergänzen:

```cpp
#ifdef BOARD_NEWBOARD

void DisplayController::drawArrowNewBoard(CompassDirection compassDirection, bool inverted)
{
    NewBoardDisplay::drawArrow(hardware, compassDirection, inverted);
}

...

#endif
```

## platformio.ini erweitern

Datei:

```text
BlePrompter/platformio.ini
```

Neue Umgebung anlegen:

```ini
[env:newboard]
platform = espressif32
board = <platformio-board-id>
framework = arduino
board_build.partitions = huge_app.csv
build_src_filter =
    +<*>
    -<display/Ssd1306Hardware.cpp>
    -<display/CydDisplay.cpp>
    -<display/Ili9341Hardware.cpp>
    -<display/M5StickDisplay.cpp>
    -<display/M5StickHardware.cpp>

upload_port = COMx
monitor_port = COMx
monitor_speed = 115200
upload_speed = <speed>

lib_deps =
    <display-library>
    h2zero/NimBLE-Arduino@^1.4.3

build_flags =
    -D BOARD_NEWBOARD
    -I include
```

Bei jeder bestehenden Umgebung müssen die neuen Board-Dateien ausgeschlossen werden. Sonst versucht PlatformIO, fremde Board-Libraries in der falschen Umgebung zu kompilieren.

Beispiel:

```ini
[env:esp32c3]
build_src_filter =
    +<*>
    -<display/CydDisplay.cpp>
    -<display/Ili9341Hardware.cpp>
    -<display/M5StickDisplay.cpp>
    -<display/M5StickHardware.cpp>
    -<display/NewBoardDisplay.cpp>
    -<display/NewBoardHardware.cpp>
```

## Buildtools erweitern

Datei:

```text
BlePrompter/tools/BuildDownloadBins.ps1
```

Neue Firmwarevariante in `$firmwareBuildConfigurations` ergänzen:

```powershell
[pscustomobject]@{
    EnvironmentName = "newboard"
    DisplayName = "NewBoard"
    ChipFamily = "ESP32"
    EspToolChip = "esp32"
    BoardName = "<platformio-board-id>"
    BootloaderOffset = "0x1000"
    FlashFrequency = "40m"
    FlashMode = "dio"
    FlashSize = "4MB"
}
```

Orientierung:

- ESP32-C3: `ChipFamily = "ESP32-C3"`, `EspToolChip = "esp32c3"`, `BootloaderOffset = "0x0000"`
- Klassischer ESP32 / ESP32-PICO: `ChipFamily = "ESP32"`, `EspToolChip = "esp32"`, `BootloaderOffset = "0x1000"`

Die Menütools `env.bat`, `compile.bat` und `flash.bat` lesen die Umgebungen aus `platformio.ini`. Bei neuen Boards muss dort normalerweise nichts ergänzt werden.

Wichtig:

- `env.bat`, `compile.bat` und `flash.bat` verwenden die Datei `tools\.active_env`.
- COM-Port-Overrides liegen in `tools\.com_ports`.

## Dokumentation aktualisieren

Pflichtdatei im Unterprojekt:

```text
BlePrompter/PROJEKTUEBERSICHT.md
```

Aktualisieren:

- Tabelle "Unterstützte Boards"
- Architekturdiagramm
- Dateiübersicht
- Hardwarebelegung
- Start- und Schlafverhalten
- Display-Bibliotheken
- PlatformIO-Build- und Uploadbefehle
- Boardwechsel-Liste

Wenn der Root-Kontext betroffen ist, zusätzlich prüfen:

```text
PROJEKTUEBERSICHT.md
AGENTS.md
```

## Validierung

Mindestens ausführen:

```powershell
cd C:\dev\TheStampSizeDiyPeekDevice\BlePrompter
& "$env:APPDATA\Python\Python313\Scripts\pio.exe" run -e newboard
& "$env:APPDATA\Python\Python313\Scripts\pio.exe" run -e esp32c3
& "$env:APPDATA\Python\Python313\Scripts\pio.exe" run -e cyd
```

Wenn ein Board angeschlossen ist:

```powershell
& "$env:APPDATA\Python\Python313\Scripts\pio.exe" run -e newboard --target upload --upload-port COMx
```

Zusätzlich:

```powershell
git -c safe.directory=C:/dev/TheStampSizeDiyPeekDevice diff --check
```

## Typische Fehler

`fatal error: <DisplayLibrary>.h: No such file or directory`

Ursache: Neue Board-Dateien werden in einer falschen PlatformIO-Umgebung mitkompiliert. `build_src_filter` in allen anderen Umgebungen ergänzen.

`redefinition of enum class CompassDirection`

Ursache: Mehrere Displayheader mit eigenen Enum-Definitionen werden gleichzeitig eingebunden. Include-Guards im Board-Block prüfen.

Display ist gedreht oder steht auf dem Kopf

Ursache: Falsche Rotation in `DisplayController::applyDisplayRotation()` oder in `Hardware::begin()`. Für Landscape-TFTs sind häufig `1` normal und `3` upside-down.

Board schläft direkt nach dem Einschalten

Ursache: Power-On-Zyklusschlaf wurde nicht im `main.cpp`-Guard ausgeschlossen. Nur Boards mit Dauerstrom oder ohne relevanten Akkubetrieb in die `cycleSleepState.active = false`-Bedingung aufnehmen.

Display bleibt im zyklischen Schlaf sichtbar oder hell

Ursache: `deactivateBeforeDeepSleep()` der neuen Hardwareklasse schaltet das Display nicht vollständig aus.

Lösung: Vor `esp_deep_sleep_start()` wird `displayController.deactivateBeforeDeepSleep()` aufgerufen. Die konkrete Hardwareklasse muss dort mindestens den Bildschirm leeren, die Helligkeit oder das Backlight abschalten und, falls verfügbar, den Display-Schlafmodus aktivieren.

Countdown-Restore stellt Bild nicht wieder her

Ursache: Direkt rendernde TFTs haben keinen Framebuffer. `getBufferPtr()` liefert `nullptr`. Das ist akzeptiert; die bestehende Prüfung verhindert Speicherfehler.

## Beispiel: M5StickC Plus2

Bei der M5StickC-Plus2-Integration wurden diese konkreten Stellen angepasst:

```text
BlePrompter/platformio.ini
BlePrompter/include/config.h
BlePrompter/include/display/DisplayController.h
BlePrompter/include/display/M5StickHardware.h
BlePrompter/include/display/M5StickDisplay.h
BlePrompter/src/main.cpp
BlePrompter/src/display/DisplayController.cpp
BlePrompter/src/display/M5StickHardware.cpp
BlePrompter/src/display/M5StickDisplay.cpp
BlePrompter/tools/BuildDownloadBins.ps1
BlePrompter/tools/compile.bat
BlePrompter/tools/flash.bat
BlePrompter/PROJEKTUEBERSICHT.md
```

Die neue Umgebung heißt:

```text
m5stickcplus2
```

Das Build-Makro heißt:

```text
BOARD_M5STICKCPLUS2
```

Der aktuell verwendete Port ist:

```text
COM4
```
