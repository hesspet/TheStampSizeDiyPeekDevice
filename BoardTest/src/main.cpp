#include <Arduino.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <U8g2lib.h>
#include <Wire.h>

#include <StampDisplay/ArrowDisplay.h>
#include <StampDisplay/AsciiCharacterDisplay.h>
#include <StampDisplay/EspSymbolDisplay.h>
#include <StampDisplay/PlayingCardDisplay.h>

#include "config.h"

U8G2_SSD1306_72X40_ER_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
ArrowDisplay arrowDisplay;
AsciiCharacterDisplay asciiCharacterDisplay;
EspSymbolDisplay espSymbolDisplay;
PlayingCardDisplay playingCardDisplay;

struct ButtonState
{
    bool lastRawButtonPressed = false;
    bool stableButtonPressed = false;
    unsigned long lastRawChangeMillis = 0;
};

enum class TestDisplayKind : uint8_t
{
    arrow,
    card,
    counter,
    asciiSingle,
    asciiPair,
    espSymbol
};

enum class TestDisplaySeries : uint8_t
{
    arrows,
    cards,
    counters,
    asciiSingles,
    asciiPairs,
    espSymbols,
    invertedArrows,
    invertedCards,
    invertedCounters,
    invertedAsciiSingles,
    invertedAsciiPairs,
    invertedEspSymbols
};

ButtonState buttonState;
unsigned long startupFinishedMillis = 0;
unsigned long currentTestStepStartedMillis = 0;
bool testModeRunning = false;
bool idleScreenDrawn = false;
bool currentTestStepInverted = false;
uint8_t currentTestStepIndex = 0;
uint8_t currentCardIndex = 0;
uint8_t currentCounterValue = 1;
char currentAsciiFirstCharacter = 'A';
char currentAsciiSecondCharacter = '\0';
CompassDirection currentArrowDirection = CompassDirection::N;
EspSymbol currentEspSymbol = EspSymbol::circle;
TestDisplayKind currentDisplayKind = TestDisplayKind::arrow;
TestDisplaySeries currentDisplaySeries = TestDisplaySeries::arrows;

constexpr CompassDirection arrowDirections[ArrowDisplay::directionCount] = {
    CompassDirection::N,
    CompassDirection::NO,
    CompassDirection::O,
    CompassDirection::SO,
    CompassDirection::S,
    CompassDirection::SW,
    CompassDirection::W,
    CompassDirection::NW
};

constexpr EspSymbol espSymbols[EspSymbolDisplay::symbolCount] = {
    EspSymbol::circle,
    EspSymbol::cross,
    EspSymbol::waves,
    EspSymbol::square,
    EspSymbol::star
};

constexpr uint8_t randomCardDisplayCount = 16;
constexpr uint8_t counterDisplayCount = 12;
constexpr uint8_t randomSingleAsciiDisplayCount = 10;
constexpr uint8_t randomPairAsciiDisplayCount = 10;
constexpr uint8_t testDisplaySeriesCount = 12;

const char *getDebugLevelName(DebugLevel debugLevel)
{
    switch (debugLevel)
    {
        case DebugLevel::none:
            return "none";
        case DebugLevel::info:
            return "info";
        case DebugLevel::debug:
            return "debug";
        case DebugLevel::trace:
            return "trace";
    }

    return "unbekannt";
}

bool shouldWriteDebugMessage(DebugLevel messageDebugLevel)
{
    return static_cast<uint8_t>(messageDebugLevel) <= static_cast<uint8_t>(configuredDebugLevel)
        && configuredDebugLevel != DebugLevel::none;
}

void writeLineToOutputs(const char *message)
{
    if (mirrorDebugToUsbSerial)
    {
        Serial.println(message);
    }
}

void writeTextToOutputs(const char *message)
{
    if (mirrorDebugToUsbSerial)
    {
        Serial.print(message);
    }
}

void writeDebugMessage(DebugLevel messageDebugLevel, const char *message)
{
    if (!shouldWriteDebugMessage(messageDebugLevel))
    {
        return;
    }

    writeTextToOutputs("[");
    writeTextToOutputs(getDebugLevelName(messageDebugLevel));
    writeTextToOutputs("] ");
    writeLineToOutputs(message);
}

const char *getEuropeanBuildDate()
{
    static char formattedBuildDate[] = "00.00.0000";
    const char *compilerBuildDate = __DATE__;

    const char monthText[4] = {
        compilerBuildDate[0],
        compilerBuildDate[1],
        compilerBuildDate[2],
        '\0'
    };

    const char *monthNames[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    uint8_t monthNumber = 0;
    for (uint8_t monthIndex = 0; monthIndex < 12; monthIndex++)
    {
        if (strcmp(monthText, monthNames[monthIndex]) == 0)
        {
            monthNumber = monthIndex + 1;
            break;
        }
    }

    const uint8_t dayNumber = static_cast<uint8_t>(atoi(&compilerBuildDate[4]));
    const int yearNumber = atoi(&compilerBuildDate[7]);

    snprintf(
        formattedBuildDate,
        sizeof(formattedBuildDate),
        "%02u.%02u.%04d",
        dayNumber,
        monthNumber,
        yearNumber);

    return formattedBuildDate;
}

void writeStartupHeader()
{
    writeLineToOutputs("");
    writeLineToOutputs("========================================");
    writeTextToOutputs("Programm: ");
    writeLineToOutputs(programName);
    writeTextToOutputs("Version: ");
    writeLineToOutputs(programVersion);
    writeTextToOutputs("Builddatum: ");
    writeLineToOutputs(getEuropeanBuildDate());
    writeTextToOutputs("Buildzeit: ");
    writeLineToOutputs(__TIME__);
    writeTextToOutputs("Debuglevel: ");
    writeLineToOutputs(getDebugLevelName(configuredDebugLevel));
    writeLineToOutputs("Board: ESP32-C3 OLED 72 x 40");
    writeLineToOutputs("========================================");
}

void waitForUsbSerialConnection()
{
    if (!mirrorDebugToUsbSerial)
    {
        return;
    }

    const unsigned long waitStartedMillis = millis();
    while (!Serial && millis() - waitStartedMillis < usbSerialStartupWaitMillis)
    {
        delay(10);
    }
}

bool readButtonPressed()
{
    return digitalRead(buttonPin) == buttonPressedLevel;
}

char getRandomPrintableAsciiCharacter()
{
    return static_cast<char>(random(33, 127));
}

void drawStartupScreen()
{
    display.clearBuffer();
    display.setDrawColor(1);
    display.setFont(u8g2_font_6x10_tf);
    display.setCursor(0, 8);
    display.print(programName);
    display.setCursor(0, 18);
    display.print("V ");
    display.print(programVersion);
    display.setCursor(0, 28);
    display.print("Build:");
    display.setCursor(0, 38);
    display.print(getEuropeanBuildDate());
    display.sendBuffer();
}

void drawIdleScreen()
{
    display.clearBuffer();
    display.setDrawColor(1);
    display.setFont(u8g2_font_6x10_tf);
    display.setCursor(0, 14);
    display.print("Test bereit");
    display.setCursor(0, 30);
    display.print("IO9 Start");
    display.sendBuffer();
    idleScreenDrawn = true;
}

void writeCurrentTestStepDescription()
{
    writeTextToOutputs("Anzeige: ");

    if (currentTestStepInverted)
    {
        writeTextToOutputs("invertiert, ");
    }

    switch (currentDisplayKind)
    {
        case TestDisplayKind::arrow:
            writeLineToOutputs(arrowDisplay.getDirectionDescription(currentArrowDirection));
            break;
        case TestDisplayKind::card:
        {
            char cardDescription[24];
            playingCardDisplay.getCardDescription(currentCardIndex, cardDescription, sizeof(cardDescription));
            writeLineToOutputs(cardDescription);
            break;
        }
        case TestDisplayKind::counter:
            writeTextToOutputs("Zähler ");
            Serial.println(currentCounterValue);
            break;
        case TestDisplayKind::asciiSingle:
            writeTextToOutputs("ASCII Einzelzeichen ");
            Serial.println(currentAsciiFirstCharacter);
            break;
        case TestDisplayKind::asciiPair:
            writeTextToOutputs("ASCII Zeichenpaar ");
            Serial.print(currentAsciiFirstCharacter);
            Serial.println(currentAsciiSecondCharacter);
            break;
        case TestDisplayKind::espSymbol:
            writeLineToOutputs(espSymbolDisplay.getSymbolDescription(currentEspSymbol));
            break;
    }
}

const char *getCurrentTestDisplaySeriesDescription()
{
    switch (currentDisplaySeries)
    {
        case TestDisplaySeries::arrows:
            return "Pfeile";
        case TestDisplaySeries::cards:
            return "Spielkarten";
        case TestDisplaySeries::counters:
            return "Zähler";
        case TestDisplaySeries::asciiSingles:
            return "ASCII Einzelzeichen";
        case TestDisplaySeries::asciiPairs:
            return "ASCII Zeichenpaare";
        case TestDisplaySeries::espSymbols:
            return "ESP-Symbole";
        case TestDisplaySeries::invertedArrows:
            return "Pfeile invertiert";
        case TestDisplaySeries::invertedCards:
            return "Spielkarten invertiert";
        case TestDisplaySeries::invertedCounters:
            return "Zähler invertiert";
        case TestDisplaySeries::invertedAsciiSingles:
            return "ASCII Einzelzeichen invertiert";
        case TestDisplaySeries::invertedAsciiPairs:
            return "ASCII Zeichenpaare invertiert";
        case TestDisplaySeries::invertedEspSymbols:
            return "ESP-Symbole invertiert";
    }

    return "Symbolreihe unbekannt";
}

uint8_t getCurrentTestDisplaySeriesStepCount()
{
    switch (currentDisplaySeries)
    {
        case TestDisplaySeries::arrows:
        case TestDisplaySeries::invertedArrows:
            return ArrowDisplay::directionCount;
        case TestDisplaySeries::cards:
        case TestDisplaySeries::invertedCards:
            return randomCardDisplayCount;
        case TestDisplaySeries::counters:
        case TestDisplaySeries::invertedCounters:
            return counterDisplayCount;
        case TestDisplaySeries::asciiSingles:
        case TestDisplaySeries::invertedAsciiSingles:
            return randomSingleAsciiDisplayCount;
        case TestDisplaySeries::asciiPairs:
        case TestDisplaySeries::invertedAsciiPairs:
            return randomPairAsciiDisplayCount;
        case TestDisplaySeries::espSymbols:
        case TestDisplaySeries::invertedEspSymbols:
            return EspSymbolDisplay::symbolCount;
    }

    return 1;
}

void writeCurrentTestDisplaySeriesDescription()
{
    writeTextToOutputs("Symbolreihe: ");
    writeLineToOutputs(getCurrentTestDisplaySeriesDescription());
}

void prepareTestStep(uint8_t testStepIndex)
{
    currentTestStepInverted =
        currentDisplaySeries == TestDisplaySeries::invertedArrows
        || currentDisplaySeries == TestDisplaySeries::invertedCards
        || currentDisplaySeries == TestDisplaySeries::invertedCounters
        || currentDisplaySeries == TestDisplaySeries::invertedAsciiSingles
        || currentDisplaySeries == TestDisplaySeries::invertedAsciiPairs
        || currentDisplaySeries == TestDisplaySeries::invertedEspSymbols;

    switch (currentDisplaySeries)
    {
        case TestDisplaySeries::arrows:
        case TestDisplaySeries::invertedArrows:
            currentDisplayKind = TestDisplayKind::arrow;
            currentArrowDirection = arrowDirections[testStepIndex % ArrowDisplay::directionCount];
            break;
        case TestDisplaySeries::cards:
        case TestDisplaySeries::invertedCards:
            currentDisplayKind = TestDisplayKind::card;
            currentCardIndex = static_cast<uint8_t>(random(0, PlayingCardDisplay::cardCount));
            break;
        case TestDisplaySeries::counters:
        case TestDisplaySeries::invertedCounters:
            currentDisplayKind = TestDisplayKind::counter;
            currentCounterValue = (testStepIndex % counterDisplayCount) + 1;
            break;
        case TestDisplaySeries::asciiSingles:
        case TestDisplaySeries::invertedAsciiSingles:
            currentDisplayKind = TestDisplayKind::asciiSingle;
            currentAsciiFirstCharacter = getRandomPrintableAsciiCharacter();
            currentAsciiSecondCharacter = '\0';
            break;
        case TestDisplaySeries::asciiPairs:
        case TestDisplaySeries::invertedAsciiPairs:
            currentDisplayKind = TestDisplayKind::asciiPair;
            currentAsciiFirstCharacter = getRandomPrintableAsciiCharacter();
            currentAsciiSecondCharacter = getRandomPrintableAsciiCharacter();
            break;
        case TestDisplaySeries::espSymbols:
        case TestDisplaySeries::invertedEspSymbols:
            currentDisplayKind = TestDisplayKind::espSymbol;
            currentEspSymbol = espSymbols[testStepIndex % EspSymbolDisplay::symbolCount];
            break;
    }
}

void drawCurrentTestStep()
{
    display.clearBuffer();
    display.setDrawColor(1);
    display.setFontMode(1);

    switch (currentDisplayKind)
    {
        case TestDisplayKind::arrow:
            arrowDisplay.drawArrow(display, currentArrowDirection, currentTestStepInverted);
            break;
        case TestDisplayKind::card:
            playingCardDisplay.drawCard(display, currentCardIndex, currentTestStepInverted);
            break;
        case TestDisplayKind::counter:
        {
            char counterText[3];
            snprintf(counterText, sizeof(counterText), "%u", currentCounterValue);
            asciiCharacterDisplay.drawCharacters(display, counterText, currentTestStepInverted);
            break;
        }
        case TestDisplayKind::asciiSingle:
            asciiCharacterDisplay.drawCharacters(display, currentAsciiFirstCharacter, '\0', currentTestStepInverted);
            break;
        case TestDisplayKind::asciiPair:
            asciiCharacterDisplay.drawCharacters(
                display,
                currentAsciiFirstCharacter,
                currentAsciiSecondCharacter,
                currentTestStepInverted);
            break;
        case TestDisplayKind::espSymbol:
            espSymbolDisplay.drawSymbol(display, currentEspSymbol, currentTestStepInverted);
            break;
    }

    display.sendBuffer();
}

void showCurrentTestStep(unsigned long currentMillis)
{
    prepareTestStep(currentTestStepIndex);
    writeCurrentTestStepDescription();
    drawCurrentTestStep();
    currentTestStepStartedMillis = currentMillis;
}

void advanceTestStep(unsigned long currentMillis)
{
    currentTestStepIndex = (currentTestStepIndex + 1) % getCurrentTestDisplaySeriesStepCount();
    showCurrentTestStep(currentMillis);
}

void selectNextTestDisplaySeries(unsigned long currentMillis)
{
    currentDisplaySeries = static_cast<TestDisplaySeries>(
        (static_cast<uint8_t>(currentDisplaySeries) + 1) % testDisplaySeriesCount);
    currentTestStepIndex = 0;
    writeCurrentTestDisplaySeriesDescription();
    showCurrentTestStep(currentMillis);
}

void startTestMode(unsigned long currentMillis)
{
    randomSeed(micros());
    startupFinishedMillis = 0;
    testModeRunning = true;
    idleScreenDrawn = false;
    currentTestStepIndex = 0;
    currentDisplaySeries = TestDisplaySeries::arrows;
    writeDebugMessage(DebugLevel::info, "Testmodus gestartet");
    writeCurrentTestDisplaySeriesDescription();
    showCurrentTestStep(currentMillis);
}

void handleButtonPressed(unsigned long currentMillis)
{
    if (testModeRunning)
    {
        selectNextTestDisplaySeries(currentMillis);
        return;
    }

    startTestMode(currentMillis);
}

void updateButtonState(unsigned long currentMillis)
{
    const bool rawButtonPressed = readButtonPressed();

    if (rawButtonPressed != buttonState.lastRawButtonPressed)
    {
        buttonState.lastRawButtonPressed = rawButtonPressed;
        buttonState.lastRawChangeMillis = currentMillis;
        writeDebugMessage(DebugLevel::trace, rawButtonPressed ? "Roher Button-Zustand: gedrückt" : "Roher Button-Zustand: frei");
    }

    if (currentMillis - buttonState.lastRawChangeMillis < buttonDebounceDurationMillis)
    {
        return;
    }

    if (rawButtonPressed != buttonState.stableButtonPressed)
    {
        buttonState.stableButtonPressed = rawButtonPressed;

        if (buttonState.stableButtonPressed)
        {
            writeDebugMessage(DebugLevel::info, "Button gedrückt");
            handleButtonPressed(currentMillis);
        }
        else
        {
            writeDebugMessage(DebugLevel::debug, "Button losgelassen");
        }
    }
}

void setup()
{
    Serial.begin(serialBaudRate);
    delay(200);
    waitForUsbSerialConnection();

    pinMode(buttonPin, INPUT_PULLUP);

    Wire.begin(displayDataPin, displayClockPin);
    display.begin();
    display.enableUTF8Print();

    writeStartupHeader();
    drawStartupScreen();
    startupFinishedMillis = millis() + startupScreenDurationMillis;
}

void loop()
{
    const unsigned long currentMillis = millis();

    updateButtonState(currentMillis);

    if (!testModeRunning)
    {
        if (!idleScreenDrawn && currentMillis >= startupFinishedMillis)
        {
            drawIdleScreen();
        }

        return;
    }

    if (currentMillis - currentTestStepStartedMillis >= testStepDurationMillis)
    {
        advanceTestStep(currentMillis);
    }
}
