#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WiFiUdp.h>
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
WiFiUDP broadcastUdp;

struct LogRecord
{
    uint32_t cycleNumber = 0;
    unsigned long uptimeMillis = 0;
    unsigned long cycleMillis = 0;
    char action[20] = "";
    char result[16] = "";
    char detail[96] = "";
    int32_t value = 0;
};

LogRecord logRecords[storedLogRecordCount];
size_t logRecordWriteIndex = 0;
size_t logRecordUsedCount = 0;
uint32_t currentCycleNumber = 0;

constexpr CompassDirection compassDirections[ArrowDisplay::directionCount] = {
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

void drawTextScreen(const char *firstLine, const char *secondLine, const char *thirdLine = nullptr, bool inverted = false)
{
    display.clearBuffer();
    display.setFontMode(1);
    display.setDrawColor(1);

    if (inverted)
    {
        display.drawBox(0, 0, 72, 40);
        display.setDrawColor(0);
    }

    display.setFont(u8g2_font_6x10_tf);
    display.setCursor(0, 10);
    display.print(firstLine);
    display.setCursor(0, 24);
    display.print(secondLine);

    if (thirdLine != nullptr)
    {
        display.setCursor(0, 38);
        display.print(thirdLine);
    }

    display.sendBuffer();
}

void drawStartupScreen()
{
    drawTextScreen(programName, "V 1.0.0", getEuropeanBuildDate());
}

void drawFatalWifiError()
{
    drawTextScreen("WLAN Fehler", wifiNetworkName, "Test gestoppt", true);
}

void appendJsonEscaped(char *buffer, size_t bufferSize, const char *text)
{
    size_t bufferLength = strlen(buffer);
    for (size_t textIndex = 0; text[textIndex] != '\0' && bufferLength + 2 < bufferSize; textIndex++)
    {
        const char character = text[textIndex];
        if (character == '"' || character == '\\')
        {
            if (bufferLength + 3 >= bufferSize)
            {
                break;
            }

            buffer[bufferLength++] = '\\';
            buffer[bufferLength++] = character;
        }
        else if (static_cast<uint8_t>(character) < 32)
        {
            buffer[bufferLength++] = ' ';
        }
        else
        {
            buffer[bufferLength++] = character;
        }

        buffer[bufferLength] = '\0';
    }
}

void formatLogRecordAsJson(const LogRecord &logRecord, char *buffer, size_t bufferSize)
{
    snprintf(
        buffer,
        bufferSize,
        "{\"type\":\"log\",\"program\":\"%s\",\"version\":\"%s\",\"cycle\":%lu,\"uptime_ms\":%lu,\"cycle_ms\":%lu,\"action\":\"",
        programName,
        programVersion,
        static_cast<unsigned long>(logRecord.cycleNumber),
        logRecord.uptimeMillis,
        logRecord.cycleMillis);
    appendJsonEscaped(buffer, bufferSize, logRecord.action);
    strncat(buffer, "\",\"result\":\"", bufferSize - strlen(buffer) - 1);
    appendJsonEscaped(buffer, bufferSize, logRecord.result);
    strncat(buffer, "\",\"detail\":\"", bufferSize - strlen(buffer) - 1);
    appendJsonEscaped(buffer, bufferSize, logRecord.detail);

    char valueBuffer[40];
    snprintf(valueBuffer, sizeof(valueBuffer), "\",\"value\":%ld}", static_cast<long>(logRecord.value));
    strncat(buffer, valueBuffer, bufferSize - strlen(buffer) - 1);
}

void writeLogRecordToSerial(const LogRecord &logRecord)
{
    char lineBuffer[320];
    formatLogRecordAsJson(logRecord, lineBuffer, sizeof(lineBuffer));
    writeLineToOutputs(lineBuffer);
}

void rememberLogRecord(
    unsigned long cycleStartedMillis,
    const char *action,
    const char *result,
    const char *detail,
    int32_t value)
{
    LogRecord &logRecord = logRecords[logRecordWriteIndex];
    logRecord.cycleNumber = currentCycleNumber;
    logRecord.uptimeMillis = millis();
    logRecord.cycleMillis = logRecord.uptimeMillis - cycleStartedMillis;
    snprintf(logRecord.action, sizeof(logRecord.action), "%s", action);
    snprintf(logRecord.result, sizeof(logRecord.result), "%s", result);
    snprintf(logRecord.detail, sizeof(logRecord.detail), "%s", detail);
    logRecord.value = value;

    logRecordWriteIndex = (logRecordWriteIndex + 1) % storedLogRecordCount;
    if (logRecordUsedCount < storedLogRecordCount)
    {
        logRecordUsedCount++;
    }

    writeLogRecordToSerial(logRecord);
}

void clearStoredLogRecords()
{
    logRecordWriteIndex = 0;
    logRecordUsedCount = 0;
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
    writeTextToOutputs("Zyklusdauer ms: ");
    Serial.println(enduranceCycleDurationMillis);
    writeLineToOutputs("Board: ESP32-C3 OLED 72 x 40");
    writeLineToOutputs("========================================");
}

char getRandomPrintableAsciiCharacter()
{
    return static_cast<char>(random(33, 127));
}

void showDisplayAction(unsigned long cycleStartedMillis, uint8_t actionIndex)
{
    const bool inverted = actionIndex % 2 == 1;
    const uint8_t displayKind = actionIndex % 4;
    char detail[80];

    display.clearBuffer();
    display.setDrawColor(1);
    display.setFontMode(1);

    if (displayKind == 0)
    {
        const CompassDirection compassDirection = compassDirections[actionIndex % ArrowDisplay::directionCount];
        arrowDisplay.drawArrow(display, compassDirection, inverted);
        snprintf(detail, sizeof(detail), "Pfeil %s", arrowDisplay.getDirectionDescription(compassDirection));
    }
    else if (displayKind == 1)
    {
        const uint8_t cardIndex = static_cast<uint8_t>(random(0, PlayingCardDisplay::cardCount));
        playingCardDisplay.drawCard(display, cardIndex, inverted);
        char cardDescription[32];
        playingCardDisplay.getCardDescription(cardIndex, cardDescription, sizeof(cardDescription));
        snprintf(detail, sizeof(detail), "Karte %s", cardDescription);
    }
    else if (displayKind == 2)
    {
        const EspSymbol espSymbol = espSymbols[actionIndex % EspSymbolDisplay::symbolCount];
        espSymbolDisplay.drawSymbol(display, espSymbol, inverted);
        snprintf(detail, sizeof(detail), "Symbol %s", espSymbolDisplay.getSymbolDescription(espSymbol));
    }
    else
    {
        const char firstCharacter = getRandomPrintableAsciiCharacter();
        const char secondCharacter = getRandomPrintableAsciiCharacter();
        asciiCharacterDisplay.drawCharacters(display, firstCharacter, secondCharacter, inverted);
        snprintf(detail, sizeof(detail), "ASCII %c%c", firstCharacter, secondCharacter);
    }

    display.sendBuffer();
    rememberLogRecord(cycleStartedMillis, "display", "ok", detail, actionIndex);
    delay(displayActionDurationMillis);
}

void runBleScan(unsigned long cycleStartedMillis)
{
    drawTextScreen("BLE Scan", "läuft", "bitte warten");
    writeDebugMessage(DebugLevel::info, "BLE-Scan gestartet");

    BLEDevice::init("");
    BLEScan *bleScan = BLEDevice::getScan();
    bleScan->setActiveScan(true);
    bleScan->setInterval(100);
    bleScan->setWindow(99);

    BLEScanResults bleScanResults = bleScan->start(bleScanDurationSeconds, false);
    const int foundDeviceCount = bleScanResults.getCount();

    char detail[80];
    snprintf(detail, sizeof(detail), "Gefundene BLE-Geräte: %d", foundDeviceCount);
    rememberLogRecord(cycleStartedMillis, "ble_scan", "ok", detail, foundDeviceCount);
    bleScan->clearResults();
    BLEDevice::deinit(false);

    drawTextScreen("BLE Scan", "fertig", detail);
    delay(1000);
}

void runWifiScan(unsigned long cycleStartedMillis)
{
    drawTextScreen("WLAN Scan", "läuft", "bitte warten");
    writeDebugMessage(DebugLevel::info, "WLAN-Scan gestartet");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);
    delay(static_cast<unsigned long>(wifiScanPauseSeconds) * 1000);

    const int foundNetworkCount = WiFi.scanNetworks(false, true);
    int32_t strongestSignal = -999;
    char strongestNetwork[34] = "";

    for (int networkIndex = 0; networkIndex < foundNetworkCount; networkIndex++)
    {
        if (WiFi.RSSI(networkIndex) > strongestSignal)
        {
            strongestSignal = WiFi.RSSI(networkIndex);
            snprintf(strongestNetwork, sizeof(strongestNetwork), "%s", WiFi.SSID(networkIndex).c_str());
        }
    }

    char detail[96];
    snprintf(
        detail,
        sizeof(detail),
        "Netze: %d, stärkstes: %s, RSSI: %ld",
        foundNetworkCount,
        strongestNetwork[0] == '\0' ? "unbekannt" : strongestNetwork,
        static_cast<long>(strongestSignal));
    rememberLogRecord(cycleStartedMillis, "wifi_scan", "ok", detail, foundNetworkCount);

    WiFi.scanDelete();
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);

    drawTextScreen("WLAN Scan", "fertig", detail);
    delay(1000);
}

void simulateNormalUsage(unsigned long cycleStartedMillis)
{
    drawTextScreen("Nutzung", "simuliert", "Sensorpause");
    rememberLogRecord(cycleStartedMillis, "usage", "ok", "Kurze Ruhephase und Anzeigewechsel", 1);
    delay(2000);

    for (uint8_t usageIndex = 0; usageIndex < 5; usageIndex++)
    {
        char progressLine[16];
        snprintf(progressLine, sizeof(progressLine), "%u von 5", usageIndex + 1);
        drawTextScreen("Nutzung", "aktiv", progressLine, usageIndex % 2 == 1);
        rememberLogRecord(cycleStartedMillis, "usage", "ok", progressLine, usageIndex + 1);
        delay(1000);
    }
}

void fillRemainingCycleTime(unsigned long cycleStartedMillis)
{
    uint8_t progressIndex = 0;
    while (millis() - cycleStartedMillis < enduranceCycleDurationMillis)
    {
        char cycleLine[20];
        const unsigned long remainingMillis = enduranceCycleDurationMillis - (millis() - cycleStartedMillis);
        snprintf(cycleLine, sizeof(cycleLine), "Rest %lus", remainingMillis / 1000);
        drawTextScreen("Zyklus", "läuft", cycleLine, progressIndex % 2 == 1);

        if (progressIndex % 3 == 0)
        {
            rememberLogRecord(cycleStartedMillis, "progress", "ok", cycleLine, static_cast<int32_t>(remainingMillis));
        }

        progressIndex++;
        delay(1500);
    }
}

bool connectToWifi()
{
    drawTextScreen("WLAN", "verbinden", wifiNetworkName);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(wifiNetworkName, wifiPassword);

    const unsigned long connectionStartedMillis = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - connectionStartedMillis < wifiConnectionTimeoutMillis)
    {
        delay(500);
        writeTextToOutputs(".");
    }

    writeLineToOutputs("");
    return WiFi.status() == WL_CONNECTED;
}

void sendUdpLine(const char *line)
{
    broadcastUdp.beginPacket(enduranceBroadcastAddress, enduranceBroadcastPort);
    broadcastUdp.print(line);
    broadcastUdp.endPacket();
    delay(25);
}

void broadcastStoredLogRecords()
{
    drawTextScreen("Broadcast", "sendet", "UDP 4210");

    char lineBuffer[320];
    snprintf(
        lineBuffer,
        sizeof(lineBuffer),
        "{\"type\":\"cycle_start\",\"program\":\"%s\",\"version\":\"%s\",\"cycle\":%lu,\"uptime_ms\":%lu,\"record_count\":%u}",
        programName,
        programVersion,
        static_cast<unsigned long>(currentCycleNumber),
        millis(),
        static_cast<unsigned int>(logRecordUsedCount));
    sendUdpLine(lineBuffer);

    const size_t firstRecordIndex =
        logRecordUsedCount == storedLogRecordCount ? logRecordWriteIndex : 0;

    for (size_t recordOffset = 0; recordOffset < logRecordUsedCount; recordOffset++)
    {
        const size_t recordIndex = (firstRecordIndex + recordOffset) % storedLogRecordCount;
        formatLogRecordAsJson(logRecords[recordIndex], lineBuffer, sizeof(lineBuffer));
        sendUdpLine(lineBuffer);
    }

    snprintf(
        lineBuffer,
        sizeof(lineBuffer),
        "{\"type\":\"cycle_end\",\"program\":\"%s\",\"version\":\"%s\",\"cycle\":%lu,\"uptime_ms\":%lu}",
        programName,
        programVersion,
        static_cast<unsigned long>(currentCycleNumber),
        millis());
    sendUdpLine(lineBuffer);
}

void stopWifi()
{
    broadcastUdp.stop();
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
}

void runEnduranceCycle()
{
    currentCycleNumber++;
    clearStoredLogRecords();

    const unsigned long cycleStartedMillis = millis();
    char detail[48];
    snprintf(detail, sizeof(detail), "Zyklus %lu gestartet", static_cast<unsigned long>(currentCycleNumber));
    drawTextScreen("Dauertest", "Zyklus Start", detail);
    rememberLogRecord(cycleStartedMillis, "cycle", "start", detail, 0);
    delay(1000);

    runBleScan(cycleStartedMillis);

    for (uint8_t actionIndex = 0; actionIndex < 10; actionIndex++)
    {
        showDisplayAction(cycleStartedMillis, actionIndex);
    }

    runWifiScan(cycleStartedMillis);
    simulateNormalUsage(cycleStartedMillis);
    fillRemainingCycleTime(cycleStartedMillis);

    rememberLogRecord(cycleStartedMillis, "cycle", "done", "Testzyklus abgeschlossen", millis() - cycleStartedMillis);

    if (!connectToWifi())
    {
        rememberLogRecord(cycleStartedMillis, "wifi_connect", "error", "Zielnetz nicht erreichbar", WiFi.status());
        drawFatalWifiError();
        writeDebugMessage(DebugLevel::info, "Zielnetz nicht erreichbar, Test gestoppt");
        while (true)
        {
            delay(1000);
        }
    }

    rememberLogRecord(cycleStartedMillis, "wifi_connect", "ok", WiFi.localIP().toString().c_str(), WiFi.RSSI());
    broadcastUdp.begin(enduranceBroadcastPort);
    broadcastStoredLogRecords();
    rememberLogRecord(cycleStartedMillis, "broadcast", "ok", "UDP-Broadcast abgeschlossen", logRecordUsedCount);
    stopWifi();
    clearStoredLogRecords();
}

void setup()
{
    Serial.begin(serialBaudRate);
    delay(200);
    waitForUsbSerialConnection();

    Wire.begin(displayDataPin, displayClockPin);
    display.begin();
    display.enableUTF8Print();

    randomSeed(micros());
    writeStartupHeader();
    drawStartupScreen();
    delay(2500);
}

void loop()
{
    runEnduranceCycle();
}
