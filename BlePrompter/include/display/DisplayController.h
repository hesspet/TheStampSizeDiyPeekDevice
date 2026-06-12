#pragma once

#include <Arduino.h>
#include "display/DisplayHardware.h"

enum class CompassDirection : uint8_t;
enum class EspSymbol : uint8_t;

/// Gekapselte Display-Logik für BlePrompter.
/// Enthält alle Zeichenfunktionen, die zuvor als freie Funktionen in main.cpp lagen.
/// Arbeitet board-unabhängig über die DisplayHardware-Schnittstelle.
class DisplayController
{
public:
    explicit DisplayController(DisplayHardware &hardware);

    // --- Lebenszyklus ---
    void begin();

    // --- Bildschirmseiten ---
    void drawStartupScreen(const char *programName, const char *programVersion);
    void drawIdleScreen(bool bluetoothConnected, const char *deviceIdentifier);
    void clearDisplay();

    // --- Textbefehle ---
    void drawPromptText(const char *text, bool inverted);

    // --- Symbolbefehle ---
    void drawArrow(CompassDirection compassDirection, bool inverted);
    void drawAsciiCharacters(const char *text, bool inverted);
    void drawPlayingCard(uint8_t cardIndex, bool inverted);
    void drawDiceFace(uint8_t faceValue, bool inverted);
    void drawEspSymbol(EspSymbol symbol, bool inverted);

    // --- Schlaf ---
    void drawSleepStatus(const char *firstLine, const char *secondLine);
    void drawCycleListenWindowStatus(
        const char *programName,
        const char *programVersion,
        const char *deviceIdentifier,
        uint32_t remainingSeconds);
    void drawDeepSleepCountdown(uint8_t secondsRemaining);

    void enterDisplaySleep();
    void wakeFromDisplaySleep();
    void deactivateBeforeDeepSleep();

    // --- Rotation ---
    void setUpsideDown(bool upsideDown);
    bool isUpsideDown() const;

    // --- Pufferzugriff für Countdown (OLED) ---
    DisplayHardware &getHardware();

private:
    DisplayHardware &hardware;

    bool upsideDown = false;

    // --- Hilfsfunktionen ---
    void prepareTextDisplay(bool inverted);
    void drawClearDisplayMarkers();
    void applyDisplayRotation();

    // --- OLED-Pfad: StampDisplay-Objekte ---
#if !defined(BOARD_CYD) && !defined(BOARD_M5STICKCPLUS2)
    void drawArrowOled(CompassDirection compassDirection, bool inverted);
    void drawAsciiCharactersOled(const char *text, bool inverted);
    void drawPlayingCardOled(uint8_t cardIndex, bool inverted);
    void drawDiceFaceOled(uint8_t faceValue, bool inverted);
    void drawEspSymbolOled(EspSymbol symbol, bool inverted);
#endif

    // --- CYB-Pfad: CydDisplay-Funktionen ---
#ifdef BOARD_CYD
    void drawArrowCyd(CompassDirection compassDirection, bool inverted);
    void drawAsciiCharactersCyd(const char *text, bool inverted);
    void drawPlayingCardCyd(uint8_t cardIndex, bool inverted);
    void drawDiceFaceCyd(uint8_t faceValue, bool inverted);
    void drawEspSymbolCyd(EspSymbol symbol, bool inverted);
#endif

    // --- M5StickC-Plus2-Pfad: M5GFX-Funktionen ---
#ifdef BOARD_M5STICKCPLUS2
    void drawArrowM5Stick(CompassDirection compassDirection, bool inverted);
    void drawAsciiCharactersM5Stick(const char *text, bool inverted);
    void drawPlayingCardM5Stick(uint8_t cardIndex, bool inverted);
    void drawDiceFaceM5Stick(uint8_t faceValue, bool inverted);
    void drawEspSymbolM5Stick(EspSymbol symbol, bool inverted);
#endif
};
