#include <Arduino.h>
#include <NimBLEDevice.h>
#include <esp_sleep.h>

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifdef BOARD_CYD
#include "display/DisplayController.h"
#include "display/Ili9341Hardware.h"
#include "display/CydDisplay.h"
#else
#include <U8g2lib.h>
#include <Wire.h>
#include <StampDisplay/ArrowDisplay.h>
#include <StampDisplay/AsciiCharacterDisplay.h>
#include <StampDisplay/DiceDisplay.h>
#include <StampDisplay/EspSymbolDisplay.h>
#include <StampDisplay/PlayingCardDisplay.h>
#include "display/DisplayController.h"
#include "display/Ssd1306Hardware.h"
#endif

#include "config.h"

// ========================================================================
// Konstanten
// ========================================================================

constexpr const char *nordicUartServiceUuid = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
constexpr const char *nordicUartReceiveCharacteristicUuid = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
constexpr const char *nordicUartTransmitCharacteristicUuid = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

constexpr size_t savedDisplayBufferSize = 72 * 40 / 8;

// ========================================================================
// Globale Objekte
// ========================================================================

#ifdef BOARD_CYD
Ili9341Hardware displayHardware;
#else
Ssd1306Hardware displayHardware(displayDataPin, displayClockPin);
#endif

DisplayController displayController(displayHardware);

// ========================================================================
// Datenstrukturen
// ========================================================================

struct ReceivedCommand
{
    char text[maximumCommandLength + 1];
};

struct ButtonState
{
    bool lastRawButtonPressed = false;
    bool stableButtonPressed = false;
    bool countdownActive = false;
    bool displayWasSleepingBeforeCountdown = false;
    bool idleScreenWasDrawnBeforeCountdown = false;
    unsigned long lastRawChangeMillis = 0;
    unsigned long pressedStartedMillis = 0;
    uint8_t lastDisplayedCountdownValue = 255;
    uint8_t savedDisplayBuffer[savedDisplayBufferSize] = {};
};

struct CycleSleepState
{
    bool active = false;
    uint32_t sleepSeconds = defaultCycleSleepSeconds;
    uint32_t listenSeconds = defaultCycleListenSeconds;
    uint32_t cycleCount = 0;
};

QueueHandle_t receivedCommandQueue = nullptr;
NimBLECharacteristic *transmitCharacteristic = nullptr;
ButtonState buttonState;
RTC_DATA_ATTR CycleSleepState cycleSleepState;

bool displayInverted = false;
bool displaySleepActive = false;
bool bluetoothClientConnected = false;
bool shouldRestartBluetoothAdvertising = false;
bool shouldEnterCycleSleepAfterBluetoothDisconnect = false;
bool idleScreenDrawn = false;
bool cycleListenWindowActive = false;
unsigned long startupFinishedMillis = 0;
unsigned long cycleListenWindowEndsMillis = 0;
unsigned long nextCycleListenWindowDisplayUpdateMillis = 0;
uint32_t activeCycleListenSeconds = defaultCycleListenSeconds;
char deviceIdentifier[8] = "BP-0000";
char bluetoothAdvertisingName[24] = "BlePrompter";

// ========================================================================
// Debug
// ========================================================================

const char *getDebugLevelName(DebugLevel debugLevel)
{
    switch (debugLevel)
    {
        case DebugLevel::none:  return "none";
        case DebugLevel::info:  return "info";
        case DebugLevel::debug: return "debug";
        case DebugLevel::trace: return "trace";
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
    if (mirrorDebugToUsbSerial) { Serial.println(message); }
}

void writeTextToOutputs(const char *message)
{
    if (mirrorDebugToUsbSerial) { Serial.print(message); }
}

void writeDebugMessage(DebugLevel messageDebugLevel, const char *message)
{
    if (!shouldWriteDebugMessage(messageDebugLevel)) return;
    writeTextToOutputs("[");
    writeTextToOutputs(getDebugLevelName(messageDebugLevel));
    writeTextToOutputs("] ");
    writeLineToOutputs(message);
}

// ========================================================================
// Build-Datum und Identität
// ========================================================================

const char *getEuropeanBuildDate()
{
    static char formattedBuildDate[] = "00.00.0000";
    const char *compilerBuildDate = __DATE__;

    const char monthText[4] = {
        compilerBuildDate[0], compilerBuildDate[1], compilerBuildDate[2], '\0'
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

    snprintf(formattedBuildDate, sizeof(formattedBuildDate),
        "%02u.%02u.%04d", dayNumber, monthNumber, yearNumber);

    return formattedBuildDate;
}

void initializeDeviceIdentity()
{
    const uint16_t shortIdentifier = static_cast<uint16_t>(ESP.getEfuseMac() & 0xFFFFULL);
    snprintf(deviceIdentifier, sizeof(deviceIdentifier), "BP-%04X", shortIdentifier);
    snprintf(bluetoothAdvertisingName, sizeof(bluetoothAdvertisingName),
        "%s-%04X", bluetoothDeviceName, shortIdentifier);
}

// ========================================================================
// Startup-Header
// ========================================================================

void writeStartupHeader()
{
    writeLineToOutputs("");
    writeLineToOutputs("========================================");
    writeTextToOutputs("Programm: ");     writeLineToOutputs(programName);
    writeTextToOutputs("Version: ");      writeLineToOutputs(programVersion);
    writeTextToOutputs("Builddatum: ");   writeLineToOutputs(getEuropeanBuildDate());
    writeTextToOutputs("Buildzeit: ");    writeLineToOutputs(__TIME__);
    writeTextToOutputs("Debuglevel: ");   writeLineToOutputs(getDebugLevelName(configuredDebugLevel));
    writeTextToOutputs("Bluetooth-Name: "); writeLineToOutputs(bluetoothAdvertisingName);
    writeTextToOutputs("Gerätekennung: "); writeLineToOutputs(deviceIdentifier);
    writeLineToOutputs("BLE-Profil: Nordic UART Service");
#ifdef BOARD_CYD
    writeLineToOutputs("Board: CYD ESP32-WROOM-32 ILI9341 320x240");
#else
    writeLineToOutputs("Board: ESP32-C3 OLED 72 x 40");
#endif
    writeLineToOutputs("========================================");
}

void waitForUsbSerialConnection()
{
    if (!mirrorDebugToUsbSerial) return;
    const unsigned long waitStartedMillis = millis();
    while (!Serial && millis() - waitStartedMillis < usbSerialStartupWaitMillis)
    {
        delay(10);
    }
}

// ========================================================================
// BLE
// ========================================================================

void sendResponse(const char *message)
{
    writeLineToOutputs(message);
    if (bluetoothClientConnected && transmitCharacteristic != nullptr)
    {
        transmitCharacteristic->setValue(
            reinterpret_cast<const uint8_t *>(message), strlen(message));
        transmitCharacteristic->notify();
    }
}

void sendCommandStatus(const std::string &command, bool successful)
{
    std::string response = command;
    response += successful ? ":OK" : ":ERROR";
    sendResponse(response.c_str());
}

// ========================================================================
// Button
// ========================================================================

bool readButtonPressed()
{
    return digitalRead(buttonPin) == buttonPressedLevel;
}

// ========================================================================
// String-Hilfsfunktionen
// ========================================================================

std::string trimString(const std::string &value)
{
    size_t firstCharacterIndex = 0;
    while (firstCharacterIndex < value.length()
        && isspace(static_cast<unsigned char>(value[firstCharacterIndex])))
    {
        firstCharacterIndex++;
    }

    size_t lastCharacterIndex = value.length();
    while (lastCharacterIndex > firstCharacterIndex
        && isspace(static_cast<unsigned char>(value[lastCharacterIndex - 1])))
    {
        lastCharacterIndex--;
    }

    return value.substr(firstCharacterIndex, lastCharacterIndex - firstCharacterIndex);
}

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

// ========================================================================
// Command-Queue
// ========================================================================

void enqueueCommand(const std::string &commandText)
{
    if (receivedCommandQueue == nullptr) return;

    const std::string trimmedCommand = trimString(commandText);
    if (trimmedCommand.empty()) return;

    ReceivedCommand receivedCommand = {};
    strncpy(receivedCommand.text, trimmedCommand.c_str(), maximumCommandLength);
    receivedCommand.text[maximumCommandLength] = '\0';
    xQueueSend(receivedCommandQueue, &receivedCommand, 0);
}

void enqueueCommandChunk(const std::string &commandChunk)
{
    size_t lineStartIndex = 0;
    bool lineBreakFound = false;

    for (size_t characterIndex = 0; characterIndex < commandChunk.length(); characterIndex++)
    {
        if (commandChunk[characterIndex] == '\n' || commandChunk[characterIndex] == '\r')
        {
            lineBreakFound = true;
            enqueueCommand(commandChunk.substr(lineStartIndex, characterIndex - lineStartIndex));
            lineStartIndex = characterIndex + 1;
        }
    }

    if (!lineBreakFound)
    {
        enqueueCommand(commandChunk);
        return;
    }

    if (lineStartIndex < commandChunk.length())
    {
        enqueueCommand(commandChunk.substr(lineStartIndex));
    }
}

// ========================================================================
// Display-Schlaf-Helfer
// ========================================================================

void enterDisplaySleep()
{
    displayController.enterDisplaySleep();
    displaySleepActive = true;
    idleScreenDrawn = true;
}

void wakeFromDisplaySleep()
{
    if (!displaySleepActive) return;
    displayController.wakeFromDisplaySleep();
    displaySleepActive = false;
    idleScreenDrawn = false;
    writeDebugMessage(DebugLevel::info, "Display aus Schlaf geweckt");
}

// ========================================================================
// Tiefschlaf
// ========================================================================

uint32_t getActiveCycleListenSeconds()
{
    if (cycleSleepState.cycleCount > 0
        && cycleLongListenEveryCycleCount > 0
        && cycleSleepState.cycleCount % cycleLongListenEveryCycleCount == 0)
    {
        return cycleLongListenSeconds;
    }
    return cycleSleepState.listenSeconds;
}

void configureCycleSleep(uint32_t sleepSeconds, uint32_t listenSeconds)
{
    cycleSleepState.active = true;
    cycleSleepState.sleepSeconds = sleepSeconds;
    cycleSleepState.listenSeconds = listenSeconds;
    cycleSleepState.cycleCount = 0;
}

void stopCycleSleepMode()
{
    cycleSleepState.active = false;
    cycleListenWindowActive = false;
}

void enterCycleDeepSleep()
{
    char secondLine[20];
    snprintf(secondLine, sizeof(secondLine), "%lus / %lus",
        static_cast<unsigned long>(cycleSleepState.sleepSeconds),
        static_cast<unsigned long>(cycleSleepState.listenSeconds));

    displayController.drawSleepStatus("Zykl. Schlaf", secondLine);
    delay(200);
    displayController.deactivateBeforeDeepSleep();

    esp_sleep_enable_timer_wakeup(
        static_cast<uint64_t>(cycleSleepState.sleepSeconds) * 1000000ULL);
    esp_deep_sleep_start();
}

void startCycleListenWindow(unsigned long currentMillis)
{
    activeCycleListenSeconds = getActiveCycleListenSeconds();
    cycleListenWindowActive = true;
    cycleListenWindowEndsMillis = currentMillis + activeCycleListenSeconds * 1000UL;
    nextCycleListenWindowDisplayUpdateMillis = 0;
    displayController.drawCycleListenWindowStatus(
        programName, programVersion, deviceIdentifier, activeCycleListenSeconds);
}

void updateCycleListenWindow(unsigned long currentMillis)
{
    if (!cycleListenWindowActive || bluetoothClientConnected) return;

    if (currentMillis >= cycleListenWindowEndsMillis)
    {
        NimBLEDevice::getAdvertising()->stop();
        enterCycleDeepSleep();
        return;
    }

    if (currentMillis >= nextCycleListenWindowDisplayUpdateMillis)
    {
        const uint32_t remainingSeconds = static_cast<uint32_t>(
            (cycleListenWindowEndsMillis - currentMillis + 999UL) / 1000UL);
        displayController.drawCycleListenWindowStatus(
            programName, programVersion, deviceIdentifier, remainingSeconds);
        nextCycleListenWindowDisplayUpdateMillis = currentMillis + 1000UL;
    }
}

void enterTimedDeepSleep(uint64_t sleepSeconds)
{
    displayController.drawSleepStatus("Tiefschlaf", "Timer aktiv");
    delay(200);
    displayController.deactivateBeforeDeepSleep();

    esp_sleep_enable_timer_wakeup(sleepSeconds * 1000000ULL);
    esp_deep_sleep_start();
}

void enterResetOnlyDeepSleep()
{
    displayController.drawSleepStatus("Tiefschlaf", "Reset weckt");
    delay(200);
    displayController.deactivateBeforeDeepSleep();

    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_deep_sleep_start();
}

// ========================================================================
// Countdown (Button-Langdruck)
// ========================================================================

void saveCurrentDisplayStateForCountdown()
{
    buttonState.displayWasSleepingBeforeCountdown = displaySleepActive;
    buttonState.idleScreenWasDrawnBeforeCountdown = idleScreenDrawn;

    if (displaySleepActive) return;

    DisplayHardware &hw = displayController.getHardware();
    const size_t displayBufferSize = hw.getBufferSize();
    const size_t bytesToCopy = displayBufferSize < savedDisplayBufferSize
        ? displayBufferSize : savedDisplayBufferSize;

    if (bytesToCopy > 0 && hw.getBufferPtr() != nullptr)
    {
        memcpy(buttonState.savedDisplayBuffer, hw.getBufferPtr(), bytesToCopy);
    }
}

void restoreDisplayStateAfterCountdown()
{
    buttonState.countdownActive = false;
    buttonState.lastDisplayedCountdownValue = 255;

    if (buttonState.displayWasSleepingBeforeCountdown)
    {
        enterDisplaySleep();
        return;
    }

    DisplayHardware &hw = displayController.getHardware();
    const size_t displayBufferSize = hw.getBufferSize();
    const size_t bytesToCopy = displayBufferSize < savedDisplayBufferSize
        ? displayBufferSize : savedDisplayBufferSize;

    if (bytesToCopy > 0 && hw.getBufferPtr() != nullptr)
    {
        memcpy(hw.getBufferPtr(), buttonState.savedDisplayBuffer, bytesToCopy);
        hw.sendBuffer();
    }

    idleScreenDrawn = buttonState.idleScreenWasDrawnBeforeCountdown;
}

void startDeepSleepCountdown(unsigned long currentMillis)
{
    saveCurrentDisplayStateForCountdown();
    wakeFromDisplaySleep();

    buttonState.countdownActive = true;
    buttonState.pressedStartedMillis = currentMillis;
    buttonState.lastDisplayedCountdownValue = 5;
    displayController.drawDeepSleepCountdown(buttonState.lastDisplayedCountdownValue);
    writeDebugMessage(DebugLevel::info, "Tiefschlaf-Countdown gestartet");
}

void updateDeepSleepCountdown(unsigned long currentMillis)
{
    if (!buttonState.countdownActive) return;

    const unsigned long elapsedMillis = currentMillis - buttonState.pressedStartedMillis;
    const uint8_t secondsRemaining = elapsedMillis >= deepSleepButtonHoldDurationMillis
        ? 0
        : static_cast<uint8_t>(
            (deepSleepButtonHoldDurationMillis - elapsedMillis + deepSleepCountdownIntervalMillis - 1)
            / deepSleepCountdownIntervalMillis);

    if (secondsRemaining != buttonState.lastDisplayedCountdownValue)
    {
        buttonState.lastDisplayedCountdownValue = secondsRemaining;
        displayController.drawDeepSleepCountdown(secondsRemaining);
    }

    if (elapsedMillis >= deepSleepButtonHoldDurationMillis)
    {
        writeDebugMessage(DebugLevel::info, "Zyklischer Schlaf durch langen Buttondruck aktiviert");
        configureCycleSleep(defaultCycleSleepSeconds, defaultCycleListenSeconds);
        delay(500);
        enterCycleDeepSleep();
    }
}

void cancelDeepSleepCountdown()
{
    if (!buttonState.countdownActive) return;
    writeDebugMessage(DebugLevel::debug, "Tiefschlaf-Countdown abgebrochen");
    restoreDisplayStateAfterCountdown();
}

void updateButtonState(unsigned long currentMillis)
{
    const bool rawButtonPressed = readButtonPressed();

    if (rawButtonPressed != buttonState.lastRawButtonPressed)
    {
        buttonState.lastRawButtonPressed = rawButtonPressed;
        buttonState.lastRawChangeMillis = currentMillis;
        writeDebugMessage(DebugLevel::trace,
            rawButtonPressed ? "Roher Button-Zustand: gedrückt" : "Roher Button-Zustand: frei");
    }

    if (currentMillis - buttonState.lastRawChangeMillis < buttonDebounceDurationMillis)
    {
        if (rawButtonPressed)
        {
            updateDeepSleepCountdown(currentMillis);
        }
        return;
    }

    if (rawButtonPressed != buttonState.stableButtonPressed)
    {
        buttonState.stableButtonPressed = rawButtonPressed;

        if (buttonState.stableButtonPressed)
        {
            writeDebugMessage(DebugLevel::info, "Button gedrückt");
            startDeepSleepCountdown(currentMillis);
        }
        else
        {
            writeDebugMessage(DebugLevel::debug, "Button losgelassen");
            cancelDeepSleepCountdown();
        }
    }

    updateDeepSleepCountdown(currentMillis);
}

// ========================================================================
// Command-Parsing (unverändert)
// ========================================================================

bool tryParseDiceValue(const std::string &text, uint8_t &faceValue)
{
    const std::string trimmedText = trimString(text);
    if (trimmedText.empty()) return false;

    for (size_t characterIndex = 0; characterIndex < trimmedText.length(); characterIndex++)
    {
        if (!isdigit(static_cast<unsigned char>(trimmedText[characterIndex])))
            return false;
    }

    errno = 0;
    char *endPointer = nullptr;
    const long parsedValue = strtol(trimmedText.c_str(), &endPointer, 10);
    if (errno == ERANGE || endPointer == trimmedText.c_str() || *endPointer != '\0')
        return false;

    if (parsedValue < 1 || parsedValue > 6) return false;

    faceValue = static_cast<uint8_t>(parsedValue);
    return true;
}

bool tryParseCompassDirection(const std::string &argument, CompassDirection &compassDirection)
{
    const std::string directionText = getUppercaseAsciiString(trimString(argument));

    if (directionText == "N")  { compassDirection = CompassDirection::N;  return true; }
    if (directionText == "NE") { compassDirection = CompassDirection::NO; return true; }
    if (directionText == "E")  { compassDirection = CompassDirection::O;  return true; }
    if (directionText == "SE") { compassDirection = CompassDirection::SO; return true; }
    if (directionText == "S")  { compassDirection = CompassDirection::S;  return true; }
    if (directionText == "SW") { compassDirection = CompassDirection::SW; return true; }
    if (directionText == "W")  { compassDirection = CompassDirection::W;  return true; }
    if (directionText == "NW") { compassDirection = CompassDirection::NW; return true; }

    return false;
}

bool tryParseCompactArrowCommand(const std::string &commandName, CompassDirection &compassDirection)
{
    if (commandName.length() < 2 || commandName.length() > 4 || commandName[0] != 'A')
        return false;
    return tryParseCompassDirection(commandName.substr(1), compassDirection);
}

bool tryParseCardSuit(const std::string &text, uint8_t &suitIndex)
{
    const std::string suitText = getUppercaseAsciiString(trimString(text));

    if (suitText == "HEART" || suitText == "HEARTS" || suitText == "H")
        { suitIndex = 0; return true; }
    if (suitText == "DIAMOND" || suitText == "DIAMONDS" || suitText == "D")
        { suitIndex = 1; return true; }
    if (suitText == "CLUB" || suitText == "CLUBS" || suitText == "C")
        { suitIndex = 2; return true; }
    if (suitText == "SPADE" || suitText == "SPADES" || suitText == "S")
        { suitIndex = 3; return true; }

    return false;
}

bool tryParseCardRank(const std::string &text, uint8_t &rankIndex)
{
    const std::string rankText = getUppercaseAsciiString(trimString(text));

    if (rankText == "1" || rankText == "A" || rankText == "ACE")
        { rankIndex = 0; return true; }
    if (rankText.length() == 1 && rankText[0] >= '2' && rankText[0] <= '9')
        { rankIndex = static_cast<uint8_t>(rankText[0] - '1'); return true; }
    if (rankText == "10" || rankText == "X")
        { rankIndex = 9; return true; }
    if (rankText == "J" || rankText == "JACK")
        { rankIndex = 10; return true; }
    if (rankText == "Q" || rankText == "QUEEN")
        { rankIndex = 11; return true; }
    if (rankText == "K" || rankText == "KING")
        { rankIndex = 12; return true; }

    return false;
}

bool tryParsePlayingCard(const std::string &argument, uint8_t &cardIndex)
{
    const std::string trimmedArgument = trimString(argument);
    const std::string uppercaseArgument = getUppercaseAsciiString(trimmedArgument);

    if (uppercaseArgument == "J1" || uppercaseArgument == "JOKER1" || uppercaseArgument == "JOKER 1")
        { cardIndex = 52; return true; }
    if (uppercaseArgument == "J2" || uppercaseArgument == "JOKER2" || uppercaseArgument == "JOKER 2")
        { cardIndex = 53; return true; }

    char *parseEnd = nullptr;
    const long parsedCardIndex = strtol(trimmedArgument.c_str(), &parseEnd, 10);
    if (parseEnd != trimmedArgument.c_str() && *parseEnd == '\0'
        && parsedCardIndex >= 0 && parsedCardIndex < 54)
    {
        cardIndex = static_cast<uint8_t>(parsedCardIndex);
        return true;
    }

    const size_t separatorIndex = trimmedArgument.find(' ');
    if (separatorIndex == std::string::npos) return false;

    uint8_t suitIndex = 0;
    uint8_t rankIndex = 0;
    if (!tryParseCardSuit(trimmedArgument.substr(0, separatorIndex), suitIndex)
        || !tryParseCardRank(trimmedArgument.substr(separatorIndex + 1), rankIndex))
    {
        return false;
    }

    cardIndex = static_cast<uint8_t>(suitIndex * 13 + rankIndex);
    return true;
}

bool tryParseCompactCardSuit(char suitCharacter, uint8_t &suitIndex)
{
    switch (suitCharacter)
    {
        case 'H': suitIndex = 0; return true;
        case 'D': suitIndex = 1; return true;
        case 'C': suitIndex = 2; return true;
        case 'S': suitIndex = 3; return true;
    }
    return false;
}

bool tryParseCompactPlayingCardCommand(const std::string &commandName, uint8_t &cardIndex)
{
    if (commandName.length() < 3 || commandName.length() > 4 || commandName[0] != 'C')
        return false;

    if (commandName == "CJ1") { cardIndex = 52; return true; }
    if (commandName == "CJ2") { cardIndex = 53; return true; }

    uint8_t suitIndex = 0;
    uint8_t rankIndex = 0;
    if (!tryParseCompactCardSuit(commandName[1], suitIndex)
        || !tryParseCardRank(commandName.substr(2), rankIndex))
    {
        return false;
    }

    cardIndex = static_cast<uint8_t>(suitIndex * 13 + rankIndex);
    return true;
}

bool tryParseEspSymbol(const std::string &text, EspSymbol &espSymbol)
{
    const std::string symbolText = getUppercaseAsciiString(trimString(text));

    if (symbolText == "C" || symbolText == "CIRCLE" || symbolText == "KREIS")
        { espSymbol = EspSymbol::circle; return true; }
    if (symbolText == "G" || symbolText == "CROSS" || symbolText == "KREUZ")
        { espSymbol = EspSymbol::cross; return true; }
    if (symbolText == "W" || symbolText == "WAVE" || symbolText == "WAVES" || symbolText == "WELLEN")
        { espSymbol = EspSymbol::waves; return true; }
    if (symbolText == "Q" || symbolText == "SQUARE" || symbolText == "QUADRAT")
        { espSymbol = EspSymbol::square; return true; }
    if (symbolText == "S" || symbolText == "STAR" || symbolText == "STERN")
        { espSymbol = EspSymbol::star; return true; }

    return false;
}

bool tryParseCompactEspSymbolCommand(const std::string &commandName, EspSymbol &espSymbol)
{
    if (commandName.length() != 2 || commandName[0] != 'E') return false;
    return tryParseEspSymbol(commandName.substr(1), espSymbol);
}

bool isEnabledText(const std::string &text)
{
    const std::string uppercaseText = getUppercaseAsciiString(trimString(text));
    return uppercaseText == "ON" || uppercaseText == "1" || uppercaseText == "TRUE";
}

bool isDisabledText(const std::string &text)
{
    const std::string uppercaseText = getUppercaseAsciiString(trimString(text));
    return uppercaseText == "OFF" || uppercaseText == "0" || uppercaseText == "FALSE";
}

bool tryParsePositiveSeconds(const std::string &text, uint64_t &seconds)
{
    const std::string trimmedText = trimString(text);
    if (trimmedText.empty()) return false;

    for (size_t characterIndex = 0; characterIndex < trimmedText.length(); characterIndex++)
    {
        if (!isdigit(static_cast<unsigned char>(trimmedText[characterIndex])))
            return false;
    }

    errno = 0;
    char *endPointer = nullptr;
    const unsigned long long parsedSeconds = strtoull(trimmedText.c_str(), &endPointer, 10);
    if (errno == ERANGE || endPointer == trimmedText.c_str() || *endPointer != '\0'
        || parsedSeconds == 0)
    {
        return false;
    }

    seconds = parsedSeconds;
    return true;
}

bool tryParseCycleSleepArguments(const std::string &argument,
    uint32_t &sleepSeconds, uint32_t &listenSeconds)
{
    if (argument.empty())
    {
        sleepSeconds = defaultCycleSleepSeconds;
        listenSeconds = defaultCycleListenSeconds;
        return true;
    }

    const size_t separatorIndex = argument.find(' ');
    if (separatorIndex == std::string::npos) return false;

    uint64_t parsedSleepSeconds = 0;
    uint64_t parsedListenSeconds = 0;
    if (!tryParsePositiveSeconds(argument.substr(0, separatorIndex), parsedSleepSeconds)
        || !tryParsePositiveSeconds(argument.substr(separatorIndex + 1), parsedListenSeconds))
    {
        return false;
    }

    if (parsedSleepSeconds < minimumCycleSleepSeconds
        || parsedSleepSeconds > maximumCycleSleepSeconds
        || parsedListenSeconds < minimumCycleListenSeconds
        || parsedListenSeconds > maximumCycleListenSeconds)
    {
        return false;
    }

    sleepSeconds = static_cast<uint32_t>(parsedSleepSeconds);
    listenSeconds = static_cast<uint32_t>(parsedListenSeconds);
    return true;
}

// ========================================================================
// Help
// ========================================================================

void sendHelp()
{
    const char *helpText =
        "Befehle: TEXT|SYMBOL|ESP|ARROW|CARD|CUBE|INVERT|U1|U0|"
        "CLEAR|SLEEP|WAKE|HELP";
    sendResponse(helpText);
}

// ========================================================================
// Command-Verarbeitung (angepasst für DisplayController)
// ========================================================================

void processCommand(const char *receivedCommandText)
{
    const std::string commandText = trimString(std::string(receivedCommandText));
    if (commandText.empty()) return;

    const size_t separatorIndex = commandText.find(' ');
    const std::string commandName = getUppercaseAsciiString(
        separatorIndex == std::string::npos ? commandText : commandText.substr(0, separatorIndex));
    const std::string argument = separatorIndex == std::string::npos
        ? "" : trimString(commandText.substr(separatorIndex + 1));

    // Alias-Prüfung
    const std::string effectiveCommandName = (commandName == "TXT") ? std::string("TEXT")
        : (commandName == "SYM") ? std::string("SYMBOL")
        : (commandName == "INV") ? std::string("INVERT")
        : (commandName == "CLS") ? std::string("CLEAR")
        : commandName;

    // Compact commands
    if (commandName == "SA")
    {
        displayController.drawAsciiCharacters("A", displayInverted);
        sendCommandStatus(commandName, true);
        idleScreenDrawn = true;
        return;
    }
    if (commandName == "SOK")
    {
        displayController.drawAsciiCharacters("OK", displayInverted);
        sendCommandStatus(commandName, true);
        idleScreenDrawn = true;
        return;
    }
    if (commandName == "I1")
    {
        displayInverted = true;
        sendCommandStatus(commandName, true);
        return;
    }
    if (commandName == "I0")
    {
        displayInverted = false;
        sendCommandStatus(commandName, true);
        return;
    }
    if (commandName == "U1")
    {
        displayController.setUpsideDown(true);
        sendCommandStatus(commandName, true);
        return;
    }
    if (commandName == "U0")
    {
        displayController.setUpsideDown(false);
        sendCommandStatus(commandName, true);
        return;
    }
    if (commandName == "CL")
    {
        displayController.clearDisplay();
        sendCommandStatus(commandName, true);
        idleScreenDrawn = true;
        return;
    }
    if (commandName == "H" || commandName == "?")
    {
        sendHelp();
        return;
    }
    if (commandName == "WAKE")
    {
        stopCycleSleepMode();
        sendCommandStatus(commandName, true);
        return;
    }

    // Compact-Symbol-Befehle
    {
        CompassDirection compactArrowDir = CompassDirection::N;
        if (tryParseCompactArrowCommand(commandName, compactArrowDir))
        {
            displayController.drawArrow(compactArrowDir, displayInverted);
            sendCommandStatus(commandName, true);
            idleScreenDrawn = true;
            return;
        }
    }

    {
        uint8_t compactCardIndex = 0;
        if (tryParseCompactPlayingCardCommand(commandName, compactCardIndex))
        {
            displayController.drawPlayingCard(compactCardIndex, displayInverted);
            sendCommandStatus(commandName, true);
            idleScreenDrawn = true;
            return;
        }
    }

    {
        EspSymbol compactEspSymbol = EspSymbol::circle;
        if (tryParseCompactEspSymbolCommand(commandName, compactEspSymbol))
        {
            displayController.drawEspSymbol(compactEspSymbol, displayInverted);
            sendCommandStatus(commandName, true);
            idleScreenDrawn = true;
            return;
        }
    }

    // Lang-Befehle
    if (effectiveCommandName == "TEXT")
    {
        if (argument.empty()) { sendCommandStatus(commandText, false); return; }
        displayController.drawPromptText(argument.c_str(), displayInverted);
        sendCommandStatus(commandText, true);
        idleScreenDrawn = true;
        return;
    }

    if (effectiveCommandName == "SYMBOL")
    {
        if (argument.empty()) { sendCommandStatus(commandText, false); return; }
        displayController.drawAsciiCharacters(argument.c_str(), displayInverted);
        sendCommandStatus(commandText, true);
        idleScreenDrawn = true;
        return;
    }

    if (effectiveCommandName == "ESP")
    {
        EspSymbol espSymbol = EspSymbol::circle;
        if (!tryParseEspSymbol(argument, espSymbol))
            { sendCommandStatus(commandText, false); return; }
        displayController.drawEspSymbol(espSymbol, displayInverted);
        sendCommandStatus(commandText, true);
        idleScreenDrawn = true;
        return;
    }

    if (effectiveCommandName == "CUBE" || effectiveCommandName == "CUBES")
    {
        uint8_t faceValue = 0;
        if (!tryParseDiceValue(argument, faceValue))
            { sendCommandStatus(commandText, false); return; }
        displayController.drawDiceFace(faceValue, displayInverted);
        sendCommandStatus(commandText, true);
        idleScreenDrawn = true;
        return;
    }

    if (effectiveCommandName == "ARROW")
    {
        CompassDirection compassDirection = CompassDirection::N;
        if (!tryParseCompassDirection(argument, compassDirection))
            { sendCommandStatus(commandText, false); return; }
        displayController.drawArrow(compassDirection, displayInverted);
        sendCommandStatus(commandText, true);
        idleScreenDrawn = true;
        return;
    }

    if (effectiveCommandName == "CARD")
    {
        uint8_t cardIndex = 0;
        if (!tryParsePlayingCard(argument, cardIndex))
            { sendCommandStatus(commandText, false); return; }
        displayController.drawPlayingCard(cardIndex, displayInverted);
        sendCommandStatus(commandText, true);
        idleScreenDrawn = true;
        return;
    }

    if (effectiveCommandName == "INVERT")
    {
        if (isEnabledText(argument))  { displayInverted = true;  sendCommandStatus(commandText, true); return; }
        if (isDisabledText(argument)) { displayInverted = false; sendCommandStatus(commandText, true); return; }
        sendCommandStatus(commandText, false);
        return;
    }

    if (effectiveCommandName == "SLEEP")
    {
        const size_t sleepSeparatorIndex = argument.find(' ');
        const std::string sleepMode = getUppercaseAsciiString(
            sleepSeparatorIndex == std::string::npos ? argument : argument.substr(0, sleepSeparatorIndex));
        const std::string sleepArgument = sleepSeparatorIndex == std::string::npos
            ? "" : trimString(argument.substr(sleepSeparatorIndex + 1));

        if (sleepMode == "DISPLAY")
        {
            sendCommandStatus(commandText, true);
            enterDisplaySleep();
            return;
        }

        if (sleepMode == "DEEP")
        {
            uint64_t sleepSeconds = 0;
            if (!tryParsePositiveSeconds(sleepArgument, sleepSeconds))
                { sendCommandStatus(commandText, false); return; }
            sendCommandStatus(commandText, true);
            enterTimedDeepSleep(sleepSeconds);
            return;
        }

        if (sleepMode == "CYCLE")
        {
            uint32_t sleepSeconds = defaultCycleSleepSeconds;
            uint32_t listenSeconds = defaultCycleListenSeconds;
            if (!tryParseCycleSleepArguments(sleepArgument, sleepSeconds, listenSeconds))
                { sendCommandStatus(commandText, false); return; }
            sendCommandStatus(commandText, true);
            configureCycleSleep(sleepSeconds, listenSeconds);
            enterCycleDeepSleep();
            return;
        }

        if (sleepMode == "RESET")
        {
            sendCommandStatus(commandText, true);
            enterResetOnlyDeepSleep();
            return;
        }

        sendCommandStatus(commandText, false);
        return;
    }

    if (effectiveCommandName == "CLEAR")
    {
        displayController.clearDisplay();
        sendCommandStatus(commandText, true);
        idleScreenDrawn = true;
        return;
    }

    if (effectiveCommandName == "HELP")
    {
        sendHelp();
        return;
    }

    sendCommandStatus(commandText, false);
}

// ========================================================================
// BLE-Callbacks
// ========================================================================

class BluetoothServerCallbacks : public NimBLEServerCallbacks
{
public:
    void onConnect(NimBLEServer *server) override
    {
        bluetoothClientConnected = true;
        stopCycleSleepMode();
        idleScreenDrawn = displaySleepActive;
        writeDebugMessage(DebugLevel::info, "BLE connected");
    }

    void onDisconnect(NimBLEServer *server) override
    {
        bluetoothClientConnected = false;
        if (startCycleSleepAfterBluetoothDisconnect)
        {
            shouldEnterCycleSleepAfterBluetoothDisconnect = true;
            shouldRestartBluetoothAdvertising = false;
        }
        else
        {
            shouldRestartBluetoothAdvertising = true;
        }
        idleScreenDrawn = displaySleepActive;
        writeDebugMessage(DebugLevel::info, "BLE disconnected");
    }
};

class BluetoothReceiveCallbacks : public NimBLECharacteristicCallbacks
{
public:
    void onWrite(NimBLECharacteristic *characteristic) override
    {
        enqueueCommandChunk(characteristic->getValue());
    }
};

void startBluetooth()
{
    NimBLEDevice::init(bluetoothAdvertisingName);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEDevice::setSecurityAuth(true, false, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

    NimBLEServer *server = NimBLEDevice::createServer();
    server->setCallbacks(new BluetoothServerCallbacks());

    NimBLEService *service = server->createService(nordicUartServiceUuid);
    NimBLECharacteristic *receiveCharacteristic = service->createCharacteristic(
        nordicUartReceiveCharacteristicUuid,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    receiveCharacteristic->setCallbacks(new BluetoothReceiveCallbacks());

    transmitCharacteristic = service->createCharacteristic(
        nordicUartTransmitCharacteristicUuid,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    transmitCharacteristic->setValue("Ready");

    service->start();

    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(nordicUartServiceUuid);
    advertising->setName(bluetoothAdvertisingName);
    advertising->setScanResponse(true);
    advertising->setMinInterval(32);
    advertising->setMaxInterval(64);
    advertising->setMinPreferred(0x06);
    advertising->setMaxPreferred(0x12);
    advertising->start();

    writeDebugMessage(DebugLevel::info, "BLE advertising started");
}

// ========================================================================
// Setup und Loop
// ========================================================================

void setup()
{
    Serial.begin(serialBaudRate);
    delay(100);
    initializeDeviceIdentity();

    const esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
    const bool isCycleTimerWakeup =
        cycleSleepState.active && wakeupCause == ESP_SLEEP_WAKEUP_TIMER;
    const bool shouldStartCycleSleepAfterPowerOn =
        startCycleSleepOnPowerOn && wakeupCause == ESP_SLEEP_WAKEUP_UNDEFINED;

    // CYD: kein zyklischer Tiefschlaf beim Power-On.
    // ESP32-C3 OLED: zyklischer Sleep spart OLED-Strom.
#ifdef BOARD_CYD
    cycleSleepState.active = false;
#else
    if (shouldStartCycleSleepAfterPowerOn)
    {
        cycleSleepState.active = true;
    }
#endif

    if (!isCycleTimerWakeup && !shouldStartCycleSleepAfterPowerOn)
    {
        cycleSleepState.active = false;
        waitForUsbSerialConnection();
    }

    receivedCommandQueue = xQueueCreate(6, sizeof(ReceivedCommand));
    pinMode(buttonPin, INPUT_PULLUP);

    displayController.begin();
    displayController.setUpsideDown(false);

    writeStartupHeader();

    // Nur ESP32-C3 OLED: zyklischer Tiefschlaf beim Power-On.
#ifndef BOARD_CYD
    if (shouldStartCycleSleepAfterPowerOn)
    {
        configureCycleSleep(defaultCycleSleepSeconds, defaultCycleListenSeconds);
        startBluetooth();
        startupFinishedMillis = ULONG_MAX;
        startCycleListenWindow(millis());
        return;
    }
#endif

    startBluetooth();

    if (isCycleTimerWakeup)
    {
        cycleSleepState.cycleCount++;
        startupFinishedMillis = ULONG_MAX;
        startCycleListenWindow(millis());
        return;
    }

    displayController.drawStartupScreen(programName, programVersion);
    startupFinishedMillis = millis() + startupScreenDurationMillis;
}

void loop()
{
    const unsigned long currentMillis = millis();

    if (shouldEnterCycleSleepAfterBluetoothDisconnect && !bluetoothClientConnected)
    {
        shouldEnterCycleSleepAfterBluetoothDisconnect = false;
        configureCycleSleep(cycleSleepState.sleepSeconds, cycleSleepState.listenSeconds);
        enterCycleDeepSleep();
        return;
    }

    if (shouldRestartBluetoothAdvertising)
    {
        NimBLEDevice::getAdvertising()->start();
        shouldRestartBluetoothAdvertising = false;
        writeDebugMessage(DebugLevel::info, "BLE advertising restarted");
    }

    updateButtonState(currentMillis);
    if (buttonState.countdownActive)
    {
        delay(10);
        return;
    }

    ReceivedCommand receivedCommand = {};
    while (receivedCommandQueue != nullptr
        && xQueueReceive(receivedCommandQueue, &receivedCommand, 0) == pdTRUE)
    {
        wakeFromDisplaySleep();
        processCommand(receivedCommand.text);
    }

    updateCycleListenWindow(currentMillis);

    if (!displaySleepActive && !idleScreenDrawn && currentMillis >= startupFinishedMillis)
    {
        displayController.drawIdleScreen(bluetoothClientConnected, deviceIdentifier);
        idleScreenDrawn = true;
    }

    delay(10);
}
