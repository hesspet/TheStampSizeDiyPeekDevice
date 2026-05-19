#include <Arduino.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <U8g2lib.h>
#include <Wire.h>

#include "config.h"
#include "PlayingCardDisplay.h"

U8G2_SSD1306_72X40_ER_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
PlayingCardDisplay playingCardDisplay;

struct ButtonState
{
    bool lastRawButtonPressed = false;
    bool stableButtonPressed = false;
    unsigned long lastRawChangeMillis = 0;
};

ButtonState buttonState;
unsigned long lastDisplayRefreshMillis = 0;
unsigned long startupFinishedMillis = 0;

constexpr uint8_t arrowDirectionCount = 4;
constexpr uint8_t arrowGlyphs[arrowDirectionCount] = {
    'J', // Pfeil nach oben, Open Iconic: arrow-thick-top
    'G', // Pfeil nach unten, Open Iconic: arrow-thick-bottom
    'H', // Pfeil nach links, Open Iconic: arrow-thick-left
    'I'  // Pfeil nach rechts, Open Iconic: arrow-thick-right
};

const char *arrowDirectionNames[arrowDirectionCount] = {
    "Pfeil nach oben",
    "Pfeil nach unten",
    "Pfeil nach links",
    "Pfeil nach rechts"
};

constexpr uint8_t displayStateCount = arrowDirectionCount + PlayingCardDisplay::cardCount;
uint8_t currentDisplayStateIndex = 0;

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

void advanceDisplayState()
{
    currentDisplayStateIndex = (currentDisplayStateIndex + 1) % displayStateCount;
    writeTextToOutputs("Anzeige: ");
    if (currentDisplayStateIndex < arrowDirectionCount)
    {
        writeLineToOutputs(arrowDirectionNames[currentDisplayStateIndex]);
    }
    else
    {
        char cardDescription[24];
        playingCardDisplay.getCardDescription(
            currentDisplayStateIndex - arrowDirectionCount,
            cardDescription,
            sizeof(cardDescription));
        writeLineToOutputs(cardDescription);
    }
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
            advanceDisplayState();
        }
        else
        {
            writeDebugMessage(DebugLevel::debug, "Button losgelassen");
        }
    }
}

void drawStartupScreen()
{
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);
    display.setCursor(0, 10);
    display.print("BoardTest");
    display.setCursor(0, 24);
    display.print("Build:");
    display.setCursor(0, 38);
    display.print(getEuropeanBuildDate());
    display.sendBuffer();
}

void drawCurrentDisplayState()
{
    display.clearBuffer();
    display.setDrawColor(1);
    display.setFontMode(1);

    if (currentDisplayStateIndex < arrowDirectionCount)
    {
        display.setFont(u8g2_font_open_iconic_arrow_4x_t);
        display.drawGlyph(20, 36, arrowGlyphs[currentDisplayStateIndex]);
    }
    else
    {
        playingCardDisplay.drawCard(display, currentDisplayStateIndex - arrowDirectionCount);
    }

    display.sendBuffer();
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
    writeTextToOutputs("Anzeige: ");
    writeLineToOutputs(arrowDirectionNames[currentDisplayStateIndex]);
    drawStartupScreen();
    startupFinishedMillis = millis() + startupScreenDurationMillis;
}

void loop()
{
    const unsigned long currentMillis = millis();

    updateButtonState(currentMillis);

    if (currentMillis < startupFinishedMillis)
    {
        return;
    }

    if (currentMillis - lastDisplayRefreshMillis >= displayRefreshIntervalMillis)
    {
        lastDisplayRefreshMillis = currentMillis;
        drawCurrentDisplayState();
    }
}
