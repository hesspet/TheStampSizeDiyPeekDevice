#include "display/DisplayController.h"

#include <cstdio>
#include <cstring>
#include <string>

#ifdef BOARD_CYD
#include "display/CydDisplay.h"
#include "display/Ili9341Hardware.h"
#elif defined(BOARD_M5STICKCPLUS2)
#include "display/M5StickDisplay.h"
#include "display/M5StickHardware.h"
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

#ifdef BOARD_CYD
constexpr int16_t cydCenterX = 160;
constexpr int16_t cydCenterY = 120;

uint16_t getCydForegroundColor(bool inverted)
{
    return inverted ? Ili9341Hardware::ColorInvertForeground : Ili9341Hardware::ColorForeground;
}

uint16_t getCydBackgroundColor(bool inverted)
{
    return inverted ? Ili9341Hardware::ColorInvertBackground : Ili9341Hardware::ColorBackground;
}

TFT_eSPI &getCydTft(DisplayHardware &hardware)
{
    return static_cast<Ili9341Hardware &>(hardware).getTft();
}

void prepareCydTextDisplay(DisplayHardware &hardware, bool inverted)
{
    TFT_eSPI &tft = getCydTft(hardware);
    tft.fillScreen(getCydBackgroundColor(inverted));
    tft.setTextColor(getCydForegroundColor(inverted), getCydBackgroundColor(inverted), true);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(nullptr);
}

bool isSingleShortTextLine(const char *text)
{
    size_t characterCount = 0;
    for (size_t characterIndex = 0; text[characterIndex] != '\0'; characterIndex++)
    {
        if (text[characterIndex] == '|' || text[characterIndex] == '\r' || text[characterIndex] == '\n')
        {
            return false;
        }
        characterCount++;
    }

    return characterCount >= 1 && characterCount <= 2;
}

void drawCenteredCydText(DisplayHardware &hardware, const char *text, const GFXfont *font,
    uint8_t textScale, int16_t centerY, bool inverted)
{
    TFT_eSPI &tft = getCydTft(hardware);
    tft.setTextColor(getCydForegroundColor(inverted), getCydBackgroundColor(inverted), true);
    tft.setTextSize(textScale);
    tft.setFreeFont(font);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(text, cydCenterX, centerY);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(nullptr);
}

void drawCydText(DisplayHardware &hardware, const char *text, const GFXfont *font,
    uint8_t textScale, int16_t x, int16_t y, uint8_t textDatum, bool inverted)
{
    TFT_eSPI &tft = getCydTft(hardware);
    tft.setTextColor(getCydForegroundColor(inverted), getCydBackgroundColor(inverted), true);
    tft.setTextSize(textScale);
    tft.setFreeFont(font);
    tft.setTextDatum(textDatum);
    tft.drawString(text, x, y);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(nullptr);
}

void drawCydStatusLine(DisplayHardware &hardware, const char *text, int16_t y, bool inverted)
{
    drawCydText(hardware, text, &FreeSansBold24pt7b, 1, cydCenterX, y, MC_DATUM, inverted);
}
#endif

#ifdef BOARD_M5STICKCPLUS2
constexpr int16_t m5StickCenterX = 120;
constexpr int16_t m5StickCenterY = 67;

uint16_t getM5StickForegroundColor(bool inverted)
{
    return inverted ? M5StickHardware::ColorInvertForeground : M5StickHardware::ColorForeground;
}

uint16_t getM5StickBackgroundColor(bool inverted)
{
    return inverted ? M5StickHardware::ColorInvertBackground : M5StickHardware::ColorBackground;
}

M5GFX &getM5StickDisplay(DisplayHardware &hardware)
{
    return static_cast<M5StickHardware &>(hardware).getDisplay();
}

void prepareM5StickTextDisplay(DisplayHardware &hardware, bool inverted)
{
    M5GFX &display = getM5StickDisplay(hardware);
    display.fillScreen(getM5StickBackgroundColor(inverted));
    display.setTextColor(getM5StickForegroundColor(inverted), getM5StickBackgroundColor(inverted));
    display.setTextSize(1);
    display.setTextDatum(top_left);

    static_cast<M5StickHardware &>(hardware).drawBatteryLevelLine();
}

bool isSingleShortTextLine(const char *text)
{
    size_t characterCount = 0;
    for (size_t characterIndex = 0; text[characterIndex] != '\0'; characterIndex++)
    {
        if (text[characterIndex] == '|' || text[characterIndex] == '\r' || text[characterIndex] == '\n')
        {
            return false;
        }
        characterCount++;
    }

    return characterCount >= 1 && characterCount <= 2;
}

void drawCenteredM5StickText(DisplayHardware &hardware, const char *text,
    uint8_t textScale, int16_t centerY, bool inverted)
{
    M5GFX &display = getM5StickDisplay(hardware);
    display.setTextColor(getM5StickForegroundColor(inverted), getM5StickBackgroundColor(inverted));
    display.setTextSize(textScale);
    display.setTextDatum(middle_center);
    display.drawString(text, m5StickCenterX, centerY);
    display.setTextDatum(top_left);
}
#endif

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
#elif defined(BOARD_M5STICKCPLUS2)
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
    prepareCydTextDisplay(hardware, false);
    drawCydStatusLine(hardware, programName, 86, false);
    drawCydStatusLine(hardware, programVersion, 146, false);
#elif defined(BOARD_M5STICKCPLUS2)
    prepareM5StickTextDisplay(hardware, false);
    drawCenteredM5StickText(hardware, programName, 3, 48, false);
    drawCenteredM5StickText(hardware, programVersion, 3, 88, false);
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
    prepareCydTextDisplay(hardware, false);
    drawCydStatusLine(hardware, "BlePrompter CYD", 38, false);
    drawCydStatusLine(hardware, "BLE bereit", 86, false);
    drawCydStatusLine(hardware, deviceIdentifier, 134, false);
    drawCydStatusLine(hardware, bluetoothConnected ? "Verbunden" : "Wartet...", 182, false);
    drawCydText(hardware, "Text  Symbol  Karte", &FreeSansBold24pt7b, 1, cydCenterX, 224, MC_DATUM, false);
#elif defined(BOARD_M5STICKCPLUS2)
    prepareM5StickTextDisplay(hardware, false);
    drawCenteredM5StickText(hardware, "BlePrompter M5", 2, 18, false);
    drawCenteredM5StickText(hardware, "BLE bereit", 2, 42, false);
    drawCenteredM5StickText(hardware, deviceIdentifier, 2, 66, false);
    drawCenteredM5StickText(hardware, bluetoothConnected ? "Verbunden" : "Wartet...", 2, 90, false);
    drawCenteredM5StickText(hardware, "Text Symbol Karte", 2, 116, false);
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
#elif defined(BOARD_M5STICKCPLUS2)
    const uint8_t markerSize = 4;
    const int16_t rightX = hardware.getDisplayWidth() - markerSize;
    const int16_t lowerY = hardware.getDisplayHeight() - markerSize;

    hardware.setDrawColor(M5StickHardware::ColorForeground);
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
#elif defined(BOARD_M5STICKCPLUS2)
    hardware.fillScreen(M5StickHardware::ColorBackground);
    drawClearDisplayMarkers();
    static_cast<M5StickHardware &>(hardware).drawBatteryLevelLine();
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
    prepareCydTextDisplay(hardware, inverted);

    if (isSingleShortTextLine(text))
    {
        drawCenteredCydText(hardware, text, &FreeSansBold24pt7b, 2, cydCenterY, inverted);
        return;
    }

    char lineBuffers[5][64] = {};
    uint8_t lineCount = 0;
    size_t lineStartIndex = 0;
    const size_t textLength = strlen(text);

    for (size_t characterIndex = 0; characterIndex <= textLength && lineCount < 5; characterIndex++)
    {
        if (text[characterIndex] == '|' || text[characterIndex] == '\0')
        {
            const size_t lineLength = min(characterIndex - lineStartIndex, sizeof(lineBuffers[0]) - 1);
            memcpy(lineBuffers[lineCount], text + lineStartIndex, lineLength);
            lineBuffers[lineCount][lineLength] = '\0';
            lineCount++;
            lineStartIndex = characterIndex + 1;
        }
    }

    if (lineCount == 0)
    {
        return;
    }

    constexpr int16_t lineSpacing = 54;
    const int16_t firstLineY = cydCenterY - static_cast<int16_t>((lineCount - 1) * lineSpacing / 2);

    for (uint8_t lineIndex = 0; lineIndex < lineCount; lineIndex++)
    {
        drawCenteredCydText(
            hardware,
            lineBuffers[lineIndex],
            &FreeSansBold24pt7b,
            1,
            firstLineY + lineIndex * lineSpacing,
            inverted);
    }
#elif defined(BOARD_M5STICKCPLUS2)
    prepareM5StickTextDisplay(hardware, inverted);

    if (isSingleShortTextLine(text))
    {
        drawCenteredM5StickText(hardware, text, 6, m5StickCenterY, inverted);
        return;
    }

    char lineBuffers[4][40] = {};
    uint8_t lineCount = 0;
    size_t lineStartIndex = 0;
    const size_t textLength = strlen(text);

    for (size_t characterIndex = 0; characterIndex <= textLength && lineCount < 4; characterIndex++)
    {
        if (text[characterIndex] == '|' || text[characterIndex] == '\0')
        {
            const size_t lineLength = min(characterIndex - lineStartIndex, sizeof(lineBuffers[0]) - 1);
            memcpy(lineBuffers[lineCount], text + lineStartIndex, lineLength);
            lineBuffers[lineCount][lineLength] = '\0';
            lineCount++;
            lineStartIndex = characterIndex + 1;
        }
    }

    constexpr int16_t lineSpacing = 28;
    const int16_t firstLineY = m5StickCenterY - static_cast<int16_t>((lineCount - 1) * lineSpacing / 2);

    for (uint8_t lineIndex = 0; lineIndex < lineCount; lineIndex++)
    {
        drawCenteredM5StickText(
            hardware,
            lineBuffers[lineIndex],
            3,
            firstLineY + lineIndex * lineSpacing,
            inverted);
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
#elif defined(BOARD_M5STICKCPLUS2)
    drawArrowM5Stick(compassDirection, inverted);
#else
    drawArrowOled(compassDirection, inverted);
#endif
}

void DisplayController::drawAsciiCharacters(const char *text, bool inverted)
{
#ifdef BOARD_CYD
    drawAsciiCharactersCyd(text, inverted);
#elif defined(BOARD_M5STICKCPLUS2)
    drawAsciiCharactersM5Stick(text, inverted);
#else
    drawAsciiCharactersOled(text, inverted);
#endif
}

void DisplayController::drawPlayingCard(uint8_t cardIndex, bool inverted)
{
#ifdef BOARD_CYD
    drawPlayingCardCyd(cardIndex, inverted);
#elif defined(BOARD_M5STICKCPLUS2)
    drawPlayingCardM5Stick(cardIndex, inverted);
#else
    drawPlayingCardOled(cardIndex, inverted);
#endif
}

void DisplayController::drawDiceFace(uint8_t faceValue, bool inverted)
{
#ifdef BOARD_CYD
    drawDiceFaceCyd(faceValue, inverted);
#elif defined(BOARD_M5STICKCPLUS2)
    drawDiceFaceM5Stick(faceValue, inverted);
#else
    drawDiceFaceOled(faceValue, inverted);
#endif
}

void DisplayController::drawEspSymbol(EspSymbol symbol, bool inverted)
{
#ifdef BOARD_CYD
    drawEspSymbolCyd(symbol, inverted);
#elif defined(BOARD_M5STICKCPLUS2)
    drawEspSymbolM5Stick(symbol, inverted);
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
    prepareCydTextDisplay(hardware, false);
    drawCydStatusLine(hardware, firstLine, 88, false);
    drawCydStatusLine(hardware, secondLine, 150, false);
#elif defined(BOARD_M5STICKCPLUS2)
    prepareM5StickTextDisplay(hardware, false);
    drawCenteredM5StickText(hardware, firstLine, 3, 50, false);
    drawCenteredM5StickText(hardware, secondLine, 3, 88, false);
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
    prepareCydTextDisplay(hardware, false);
    drawCydStatusLine(hardware, programName, 34, false);
    drawCydStatusLine(hardware, programVersion, 82, false);
    drawCydStatusLine(hardware, remainingText, 136, false);
    drawCydStatusLine(hardware, deviceIdentifier, 190, false);
#elif defined(BOARD_M5STICKCPLUS2)
    prepareM5StickTextDisplay(hardware, false);
    drawCenteredM5StickText(hardware, programName, 2, 24, false);
    drawCenteredM5StickText(hardware, programVersion, 2, 50, false);
    drawCenteredM5StickText(hardware, remainingText, 3, 80, false);
    drawCenteredM5StickText(hardware, deviceIdentifier, 2, 112, false);
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
    prepareCydTextDisplay(hardware, false);
    drawCydStatusLine(hardware, "Zykl. Schlaf", 54, false);
    drawCenteredCydText(hardware, countdownText, &FreeSansBold24pt7b, 2, 156, false);
#elif defined(BOARD_M5STICKCPLUS2)
    prepareM5StickTextDisplay(hardware, false);
    drawCenteredM5StickText(hardware, "Zykl. Schlaf", 2, 34, false);
    drawCenteredM5StickText(hardware, countdownText, 7, 88, false);
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
#elif defined(BOARD_M5STICKCPLUS2)
    hardware.fillScreen(M5StickHardware::ColorBackground);
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

#if !defined(BOARD_CYD) && !defined(BOARD_M5STICKCPLUS2)

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

// ========================================================================
// M5StickC-Plus2-Pfad: M5StickDisplay
// ========================================================================

#ifdef BOARD_M5STICKCPLUS2

void DisplayController::drawArrowM5Stick(CompassDirection compassDirection, bool inverted)
{
    M5StickDisplay::drawArrow(hardware, compassDirection, inverted);
}

void DisplayController::drawAsciiCharactersM5Stick(const char *text, bool inverted)
{
    M5StickDisplay::drawAsciiCharacters(hardware, text, inverted);
}

void DisplayController::drawPlayingCardM5Stick(uint8_t cardIndex, bool inverted)
{
    M5StickDisplay::drawPlayingCard(hardware, cardIndex, inverted);
}

void DisplayController::drawDiceFaceM5Stick(uint8_t faceValue, bool inverted)
{
    M5StickDisplay::drawDiceFace(hardware, faceValue, inverted);
}

void DisplayController::drawEspSymbolM5Stick(EspSymbol symbol, bool inverted)
{
    M5StickDisplay::drawEspSymbol(hardware, symbol, inverted);
}

#endif
