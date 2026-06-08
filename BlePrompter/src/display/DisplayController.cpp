#include "display/DisplayController.h"

#include <cstdio>
#include <cstring>
#include <string>

#ifdef BOARD_CYD
#include "display/CydDisplay.h"
#include "display/Ili9341Hardware.h"
#else
#include <U8g2lib.h>
#include <StampDisplay/ArrowDisplay.h>
#include <StampDisplay/AsciiCharacterDisplay.h>
#include <StampDisplay/DiceDisplay.h>
#include <StampDisplay/EspSymbolDisplay.h>
#include <StampDisplay/PlayingCardDisplay.h>
#endif

// ========================================================================
// Hilfsfunktionen
// ========================================================================

namespace
{

std::string getUppercaseAsciiString(const std::string &value)
{
    std::string uppercaseValue = value;
    for (size_t characterIndex = 0; characterIndex < uppercaseValue.length(); characterIndex++)
    {
        uppercaseValue[characterIndex] = static_cast<char>(
            toupper(static_cast<unsigned char>(uppercaseValue[characterIndex])));
    }
    return uppercaseValue;
}

} // namespace

// ========================================================================
// DisplayController
// ========================================================================

DisplayController::DisplayController(DisplayHardware &hardware)
    : hardware(hardware)
{
}

void DisplayController::begin()
{
    hardware.begin();
}

// ========================================================================
// Bildschirmseiten
// ========================================================================

void DisplayController::prepareTextDisplay(bool inverted)
{
    hardware.clearBuffer();
    hardware.setFontMode(1);

    if (inverted)
    {
        hardware.setDrawColor(1);
        hardware.drawBox(0, 0, hardware.getDisplayWidth(), hardware.getDisplayHeight());
        hardware.setDrawColor(0);
    }
    else
    {
        hardware.setDrawColor(1);
    }
}

void DisplayController::applyDisplayRotation()
{
#ifdef BOARD_CYD
    hardware.setDisplayRotation(upsideDown ? 3 : 1);
#else
    hardware.setDisplayRotation(upsideDown ? 2 : 0);
#endif
}

void DisplayController::drawStartupScreen(
    const char *programName,
    const char *programVersion)
{
#ifdef BOARD_CYD
    hardware.fillScreen(Ili9341Hardware::ColorBackground);
    hardware.setFont(reinterpret_cast<const void *>(static_cast<uintptr_t>(4)));
    hardware.setCursor(10, 10);
    hardware.print(programName);
    hardware.setCursor(10, 35);
    hardware.print(programVersion);
#else
    prepareTextDisplay(false);
    hardware.setFont(u8g2_font_6x10_tf);
    hardware.setCursor(0, 8);
    hardware.print(programName);
    hardware.setCursor(0, 18);
    hardware.print(programVersion);
    hardware.sendBuffer();
#endif
}

void DisplayController::drawIdleScreen(
    bool bluetoothConnected,
    const char *deviceIdentifier)
{
#ifdef BOARD_CYD
    hardware.fillScreen(Ili9341Hardware::ColorBackground);
    hardware.setFont(reinterpret_cast<const void *>(static_cast<uintptr_t>(4)));
    hardware.setCursor(10, 10);
    hardware.print("BlePrompter CYD");
    hardware.setCursor(10, 35);
    hardware.print("BLE bereit");
    hardware.setCursor(10, 60);
    hardware.print(deviceIdentifier);
    hardware.setCursor(10, 85);
    hardware.print(bluetoothConnected ? "Verbunden" : "Wartet...");
    hardware.setCursor(10, 110);
    hardware.print("Text / Symbol / Arrow");
    hardware.setCursor(10, 135);
    hardware.print("Card / Cube / Esp");
#else
    prepareTextDisplay(false);
    hardware.setFont(u8g2_font_6x10_tf);
    hardware.setCursor(0, 8);
    hardware.print("BLE bereit");
    hardware.setCursor(0, 18);
    hardware.print(deviceIdentifier);
    hardware.setCursor(0, 28);
    hardware.print(bluetoothConnected ? "Verbunden" : "Wartet...");
    hardware.setCursor(0, 38);
    hardware.print("Text/Arrow");
    hardware.sendBuffer();
#endif
}

void DisplayController::drawClearDisplayMarkers()
{
#ifdef BOARD_CYD
    const uint8_t markerSize = 4;
    const int16_t rightX = hardware.getDisplayWidth() - markerSize;
    const int16_t lowerY = hardware.getDisplayHeight() - markerSize;

    hardware.setDrawColor(Ili9341Hardware::ColorForeground);
    hardware.drawBox(0, 0, markerSize, markerSize);
    hardware.drawBox(rightX, 0, markerSize, markerSize);
    hardware.drawBox(0, lowerY, markerSize, markerSize);
    hardware.drawBox(rightX, lowerY, markerSize, markerSize);
#else
    constexpr uint8_t markerSize = 2;
    const uint8_t rightMarkerX = hardware.getDisplayWidth() - markerSize;
    const uint8_t lowerMarkerY = hardware.getDisplayHeight() - markerSize;

    hardware.setDrawColor(1);
    hardware.drawBox(0, 0, markerSize, markerSize);
    hardware.drawBox(rightMarkerX, 0, markerSize, markerSize);
    hardware.drawBox(0, lowerMarkerY, markerSize, markerSize);
    hardware.drawBox(rightMarkerX, lowerMarkerY, markerSize, markerSize);
#endif
}

void DisplayController::clearDisplay()
{
#ifdef BOARD_CYD
    hardware.fillScreen(Ili9341Hardware::ColorBackground);
    drawClearDisplayMarkers();
#else
    hardware.clearBuffer();
    drawClearDisplayMarkers();
    hardware.sendBuffer();
#endif
}

// ========================================================================
// Textbefehle
// ========================================================================

void DisplayController::drawPromptText(const char *text, bool inverted)
{
#ifdef BOARD_CYD
    const uint16_t bgColor = inverted
        ? Ili9341Hardware::ColorInvertBackground
        : Ili9341Hardware::ColorBackground;
    const uint16_t fgColor = inverted
        ? Ili9341Hardware::ColorInvertForeground
        : Ili9341Hardware::ColorForeground;

    hardware.fillScreen(bgColor);
    hardware.setFont(reinterpret_cast<const void *>(static_cast<uintptr_t>(4)));

    uint8_t lineIndex = 0;
    size_t lineStartIndex = 0;
    const size_t textLength = strlen(text);

    for (size_t characterIndex = 0; characterIndex <= textLength && lineIndex < 12; characterIndex++)
    {
        if (text[characterIndex] == '|' || text[characterIndex] == '\0')
        {
            char lineBuffer[64] = {};
            const size_t lineLength = min(characterIndex - lineStartIndex, sizeof(lineBuffer) - 1);
            memcpy(lineBuffer, text + lineStartIndex, lineLength);
            lineBuffer[lineLength] = '\0';

            hardware.setCursor(10, 5 + lineIndex * 26);
            hardware.print(lineBuffer);
            lineIndex++;
            lineStartIndex = characterIndex + 1;
        }
    }
#else
    prepareTextDisplay(inverted);
    hardware.setFont(u8g2_font_6x10_tf);

    uint8_t lineIndex = 0;
    size_t lineStartIndex = 0;
    const size_t textLength = strlen(text);

    for (size_t characterIndex = 0; characterIndex <= textLength && lineIndex < 4; characterIndex++)
    {
        if (text[characterIndex] == '|' || text[characterIndex] == '\0')
        {
            char lineBuffer[32] = {};
            const size_t lineLength = min(characterIndex - lineStartIndex, sizeof(lineBuffer) - 1);
            memcpy(lineBuffer, text + lineStartIndex, lineLength);
            lineBuffer[lineLength] = '\0';

            hardware.setCursor(0, 8 + lineIndex * 10);
            hardware.print(lineBuffer);
            lineIndex++;
            lineStartIndex = characterIndex + 1;
        }
    }

    hardware.sendBuffer();
#endif
}

// ========================================================================
// Symbolbefehle
// ========================================================================

void DisplayController::drawArrow(CompassDirection compassDirection, bool inverted)
{
#ifdef BOARD_CYD
    drawArrowCyd(compassDirection, inverted);
#else
    drawArrowOled(compassDirection, inverted);
#endif
}

void DisplayController::drawAsciiCharacters(const char *text, bool inverted)
{
#ifdef BOARD_CYD
    drawAsciiCharactersCyd(text, inverted);
#else
    drawAsciiCharactersOled(text, inverted);
#endif
}

void DisplayController::drawPlayingCard(uint8_t cardIndex, bool inverted)
{
#ifdef BOARD_CYD
    drawPlayingCardCyd(cardIndex, inverted);
#else
    drawPlayingCardOled(cardIndex, inverted);
#endif
}

void DisplayController::drawDiceFace(uint8_t faceValue, bool inverted)
{
#ifdef BOARD_CYD
    drawDiceFaceCyd(faceValue, inverted);
#else
    drawDiceFaceOled(faceValue, inverted);
#endif
}

void DisplayController::drawEspSymbol(EspSymbol symbol, bool inverted)
{
#ifdef BOARD_CYD
    drawEspSymbolCyd(symbol, inverted);
#else
    drawEspSymbolOled(symbol, inverted);
#endif
}

// ========================================================================
// Schlaf
// ========================================================================

void DisplayController::drawSleepStatus(const char *firstLine, const char *secondLine)
{
#ifdef BOARD_CYD
    hardware.fillScreen(Ili9341Hardware::ColorBackground);
    hardware.setFont(reinterpret_cast<const void *>(static_cast<uintptr_t>(6)));
    hardware.setCursor(10, 40);
    hardware.print(firstLine);
    hardware.setCursor(10, 90);
    hardware.print(secondLine);
#else
    prepareTextDisplay(false);
    hardware.setFont(u8g2_font_6x10_tf);
    hardware.setCursor(0, 8);
    hardware.print(firstLine);
    hardware.setCursor(0, 18);
    hardware.print(secondLine);
    hardware.sendBuffer();
#endif
}

void DisplayController::drawCycleListenWindowStatus(
    const char *programName,
    const char *programVersion,
    const char *deviceIdentifier,
    uint32_t remainingSeconds)
{
    char remainingText[18];
    snprintf(remainingText, sizeof(remainingText), "Noch %lus",
        static_cast<unsigned long>(remainingSeconds));

#ifdef BOARD_CYD
    hardware.fillScreen(Ili9341Hardware::ColorBackground);
    hardware.setFont(reinterpret_cast<const void *>(static_cast<uintptr_t>(6)));
    hardware.setCursor(10, 20);
    hardware.print(programName);
    hardware.setCursor(10, 60);
    hardware.print(programVersion);
    hardware.setCursor(10, 100);
    hardware.print(remainingText);
    hardware.setFont(reinterpret_cast<const void *>(static_cast<uintptr_t>(4)));
    hardware.setCursor(10, 140);
    hardware.print(deviceIdentifier);
#else
    prepareTextDisplay(false);
    hardware.setFont(u8g2_font_6x10_tf);
    hardware.setCursor(0, 8);
    hardware.print(programName);
    hardware.setCursor(0, 18);
    hardware.print(programVersion);
    hardware.setCursor(0, 28);
    hardware.print(remainingText);
    hardware.setCursor(0, 38);
    hardware.print(deviceIdentifier);
    hardware.sendBuffer();
#endif
}

void DisplayController::drawDeepSleepCountdown(uint8_t secondsRemaining)
{
    char countdownText[2] = {
        static_cast<char>('0' + secondsRemaining),
        '\0'
    };

#ifdef BOARD_CYD
    hardware.fillScreen(Ili9341Hardware::ColorBackground);
    hardware.setFont(reinterpret_cast<const void *>(static_cast<uintptr_t>(4)));
    hardware.setCursor(10, 20);
    hardware.print("Zykl. Schlaf");

    hardware.setFont(reinterpret_cast<const void *>(static_cast<uintptr_t>(7)));
    const int16_t countdownWidth = hardware.getStrWidth(countdownText);
    const int16_t countdownX = (hardware.getDisplayWidth() - countdownWidth) / 2;
    hardware.setCursor(countdownX, 130);
    hardware.print(countdownText);
#else
    prepareTextDisplay(false);
    hardware.setFont(u8g2_font_6x10_tf);
    hardware.setCursor(0, 8);
    hardware.print("Zykl. Schlaf");

    hardware.setFont(u8g2_font_logisoso20_tn);
    const int16_t countdownWidth = hardware.getStrWidth(countdownText);
    const int16_t countdownX = (hardware.getDisplayWidth() - countdownWidth) / 2;
    hardware.setCursor(countdownX, 37);
    hardware.print(countdownText);
    hardware.sendBuffer();
#endif
}

void DisplayController::enterDisplaySleep()
{
#ifdef BOARD_CYD
    hardware.fillScreen(Ili9341Hardware::ColorBackground);
    hardware.enterHardwareSleep();
#else
    hardware.clearBuffer();
    hardware.sendBuffer();
    hardware.enterHardwareSleep();
#endif
}

void DisplayController::wakeFromDisplaySleep()
{
    hardware.wakeFromHardwareSleep();
    applyDisplayRotation();
}

void DisplayController::deactivateBeforeDeepSleep()
{
    hardware.deactivateBeforeDeepSleep();
}

// ========================================================================
// Rotation
// ========================================================================

void DisplayController::setUpsideDown(bool upsideDown)
{
    this->upsideDown = upsideDown;
    applyDisplayRotation();
}

bool DisplayController::isUpsideDown() const
{
    return upsideDown;
}

DisplayHardware &DisplayController::getHardware()
{
    return hardware;
}

// ========================================================================
// OLED-Pfad: StampDisplay
// ========================================================================

#ifndef BOARD_CYD

void DisplayController::drawArrowOled(CompassDirection compassDirection, bool inverted)
{
    static ArrowDisplay arrowDisplay;
    hardware.clearBuffer();
    arrowDisplay.drawArrow(
        *static_cast<U8G2 *>(hardware.getRawDisplay()),
        compassDirection,
        inverted);
    hardware.sendBuffer();
}

void DisplayController::drawAsciiCharactersOled(const char *text, bool inverted)
{
    static AsciiCharacterDisplay asciiDisplay;
    const std::string uppercaseText = getUppercaseAsciiString(text);
    char firstChar = uppercaseText.empty() ? ' ' : uppercaseText[0];
    char secondChar = uppercaseText.length() < 2 ? '\0' : uppercaseText[1];

    hardware.clearBuffer();
    asciiDisplay.drawCharacters(
        *static_cast<U8G2 *>(hardware.getRawDisplay()),
        firstChar,
        secondChar,
        inverted);
    hardware.sendBuffer();
}

void DisplayController::drawPlayingCardOled(uint8_t cardIndex, bool inverted)
{
    static PlayingCardDisplay cardDisplay;
    hardware.clearBuffer();
    cardDisplay.drawCard(
        *static_cast<U8G2 *>(hardware.getRawDisplay()),
        cardIndex,
        inverted);
    hardware.sendBuffer();
}

void DisplayController::drawDiceFaceOled(uint8_t faceValue, bool inverted)
{
    static DiceDisplay diceDisplay;
    hardware.clearBuffer();
    diceDisplay.drawFace(
        *static_cast<U8G2 *>(hardware.getRawDisplay()),
        faceValue,
        inverted);
    hardware.sendBuffer();
}

void DisplayController::drawEspSymbolOled(EspSymbol symbol, bool inverted)
{
    static EspSymbolDisplay espDisplay;
    hardware.clearBuffer();
    espDisplay.drawSymbol(
        *static_cast<U8G2 *>(hardware.getRawDisplay()),
        symbol,
        inverted);
    hardware.sendBuffer();
}
#endif

// ========================================================================
// CYB-Pfad: CydDisplay (Stubs — werden in Phase 3 implementiert)
// ========================================================================

#ifdef BOARD_CYD

void DisplayController::drawArrowCyd(CompassDirection compassDirection, bool inverted)
{
    CydDisplay::drawArrow(hardware, compassDirection, inverted);
}

void DisplayController::drawAsciiCharactersCyd(const char *text, bool inverted)
{
    CydDisplay::drawAsciiCharacters(hardware, text, inverted);
}

void DisplayController::drawPlayingCardCyd(uint8_t cardIndex, bool inverted)
{
    CydDisplay::drawPlayingCard(hardware, cardIndex, inverted);
}

void DisplayController::drawDiceFaceCyd(uint8_t faceValue, bool inverted)
{
    CydDisplay::drawDiceFace(hardware, faceValue, inverted);
}

void DisplayController::drawEspSymbolCyd(EspSymbol symbol, bool inverted)
{
    CydDisplay::drawEspSymbol(hardware, symbol, inverted);
}

#endif
