#include <Arduino.h>
#include <NimBLEDevice.h>
#include <U8g2lib.h>
#include <Wire.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <StampDisplay/ArrowDisplay.h>
#include <StampDisplay/AsciiCharacterDisplay.h>
#include <StampDisplay/EspSymbolDisplay.h>
#include <StampDisplay/PlayingCardDisplay.h>

#include "config.h"

constexpr const char *nordicUartServiceUuid = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
constexpr const char *nordicUartReceiveCharacteristicUuid = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
constexpr const char *nordicUartTransmitCharacteristicUuid = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

struct ReceivedCommand
{
    char text[maximumCommandLength + 1];
};

U8G2_SSD1306_72X40_ER_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
ArrowDisplay arrowDisplay;
AsciiCharacterDisplay asciiCharacterDisplay;
EspSymbolDisplay espSymbolDisplay;
PlayingCardDisplay playingCardDisplay;

QueueHandle_t receivedCommandQueue = nullptr;
NimBLECharacteristic *transmitCharacteristic = nullptr;

bool displayInverted = false;
bool displayUpsideDown = false;
bool bluetoothClientConnected = false;
bool shouldRestartBluetoothAdvertising = false;
bool idleScreenDrawn = false;
unsigned long startupFinishedMillis = 0;

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

void sendResponse(const char *message)
{
    writeLineToOutputs(message);

    if (bluetoothClientConnected && transmitCharacteristic != nullptr)
    {
        transmitCharacteristic->setValue(message);
        transmitCharacteristic->notify();
    }
}

void applyDisplayRotation()
{
    display.setDisplayRotation(displayUpsideDown ? U8G2_R2 : U8G2_R0);
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
    writeTextToOutputs("Bluetooth-Name: ");
    writeLineToOutputs(bluetoothDeviceName);
    writeLineToOutputs("BLE-Profil: Nordic UART Service");
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

std::string trimString(const std::string &value)
{
    size_t firstCharacterIndex = 0;
    while (firstCharacterIndex < value.length() && isspace(static_cast<unsigned char>(value[firstCharacterIndex])))
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

void enqueueCommand(const std::string &commandText)
{
    if (receivedCommandQueue == nullptr)
    {
        return;
    }

    const std::string trimmedCommand = trimString(commandText);
    if (trimmedCommand.empty())
    {
        return;
    }

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

void prepareTextDisplay(bool inverted)
{
    display.clearBuffer();
    display.setFontMode(1);

    if (inverted)
    {
        display.setDrawColor(1);
        display.drawBox(0, 0, display.getDisplayWidth(), display.getDisplayHeight());
        display.setDrawColor(0);
    }
    else
    {
        display.setDrawColor(1);
    }
}

void drawStartupScreen()
{
    prepareTextDisplay(false);
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
    prepareTextDisplay(false);
    display.setFont(u8g2_font_6x10_tf);
    display.setCursor(0, 8);
    display.print("BLE ready");
    display.setCursor(0, 18);
    display.print(bluetoothDeviceName);
    display.setCursor(0, 28);
    display.print(bluetoothClientConnected ? "Connected" : "Waiting...");
    display.setCursor(0, 38);
    display.print("Text/Arrow");
    display.sendBuffer();
    idleScreenDrawn = true;
}

void clearDisplay()
{
    display.clearBuffer();
    display.sendBuffer();
    idleScreenDrawn = true;
}

void drawPromptText(const char *text)
{
    prepareTextDisplay(displayInverted);
    display.setFont(u8g2_font_6x10_tf);

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

            display.setCursor(0, 8 + lineIndex * 10);
            display.print(lineBuffer);
            lineIndex++;
            lineStartIndex = characterIndex + 1;
        }
    }

    display.sendBuffer();
    idleScreenDrawn = true;
}

void drawArrow(CompassDirection compassDirection)
{
    display.clearBuffer();
    arrowDisplay.drawArrow(display, compassDirection, displayInverted);
    display.sendBuffer();
    idleScreenDrawn = true;
}

void drawAsciiCharacters(const char *text)
{
    const std::string uppercaseText = getUppercaseAsciiString(text);
    char firstCharacter = uppercaseText.empty() ? ' ' : uppercaseText[0];
    char secondCharacter = uppercaseText.length() < 2 ? '\0' : uppercaseText[1];

    display.clearBuffer();
    asciiCharacterDisplay.drawCharacters(display, firstCharacter, secondCharacter, displayInverted);
    display.sendBuffer();
    idleScreenDrawn = true;
}

void drawPlayingCard(uint8_t cardIndex)
{
    display.clearBuffer();
    playingCardDisplay.drawCard(display, cardIndex, displayInverted);
    display.sendBuffer();
    idleScreenDrawn = true;
}

void drawEspSymbol(EspSymbol espSymbol)
{
    display.clearBuffer();
    espSymbolDisplay.drawSymbol(display, espSymbol, displayInverted);
    display.sendBuffer();
    idleScreenDrawn = true;
}

bool tryParseCompassDirection(const std::string &argument, CompassDirection &compassDirection)
{
    const std::string directionText = getUppercaseAsciiString(trimString(argument));

    if (directionText == "N")
    {
        compassDirection = CompassDirection::N;
        return true;
    }
    if (directionText == "NE")
    {
        compassDirection = CompassDirection::NO;
        return true;
    }
    if (directionText == "E")
    {
        compassDirection = CompassDirection::O;
        return true;
    }
    if (directionText == "SE")
    {
        compassDirection = CompassDirection::SO;
        return true;
    }
    if (directionText == "S")
    {
        compassDirection = CompassDirection::S;
        return true;
    }
    if (directionText == "SW")
    {
        compassDirection = CompassDirection::SW;
        return true;
    }
    if (directionText == "W")
    {
        compassDirection = CompassDirection::W;
        return true;
    }
    if (directionText == "NW")
    {
        compassDirection = CompassDirection::NW;
        return true;
    }

    return false;
}

bool tryParseCompactArrowCommand(const std::string &commandName, CompassDirection &compassDirection)
{
    if (commandName.length() < 2 || commandName.length() > 4 || commandName[0] != 'A')
    {
        return false;
    }

    return tryParseCompassDirection(commandName.substr(1), compassDirection);
}

bool tryParseCardSuit(const std::string &text, uint8_t &suitIndex)
{
    const std::string suitText = getUppercaseAsciiString(trimString(text));

    if (suitText == "HEART" || suitText == "HEARTS" || suitText == "H")
    {
        suitIndex = 0;
        return true;
    }
    if (suitText == "DIAMOND" || suitText == "DIAMONDS" || suitText == "D")
    {
        suitIndex = 1;
        return true;
    }
    if (suitText == "CLUB" || suitText == "CLUBS" || suitText == "C")
    {
        suitIndex = 2;
        return true;
    }
    if (suitText == "SPADE" || suitText == "SPADES" || suitText == "S")
    {
        suitIndex = 3;
        return true;
    }

    return false;
}

bool tryParseCardRank(const std::string &text, uint8_t &rankIndex)
{
    const std::string rankText = getUppercaseAsciiString(trimString(text));

    if (rankText == "1" || rankText == "A" || rankText == "ACE")
    {
        rankIndex = 0;
        return true;
    }
    if (rankText.length() == 1 && rankText[0] >= '2' && rankText[0] <= '9')
    {
        rankIndex = static_cast<uint8_t>(rankText[0] - '1');
        return true;
    }
    if (rankText == "10" || rankText == "X")
    {
        rankIndex = 9;
        return true;
    }
    if (rankText == "J" || rankText == "JACK")
    {
        rankIndex = 10;
        return true;
    }
    if (rankText == "Q" || rankText == "QUEEN")
    {
        rankIndex = 11;
        return true;
    }
    if (rankText == "K" || rankText == "KING")
    {
        rankIndex = 12;
        return true;
    }

    return false;
}

bool tryParsePlayingCard(const std::string &argument, uint8_t &cardIndex)
{
    const std::string trimmedArgument = trimString(argument);
    const std::string uppercaseArgument = getUppercaseAsciiString(trimmedArgument);

    if (uppercaseArgument == "J1" || uppercaseArgument == "JOKER1" || uppercaseArgument == "JOKER 1")
    {
        cardIndex = 52;
        return true;
    }
    if (uppercaseArgument == "J2" || uppercaseArgument == "JOKER2" || uppercaseArgument == "JOKER 2")
    {
        cardIndex = 53;
        return true;
    }

    char *parseEnd = nullptr;
    const long parsedCardIndex = strtol(trimmedArgument.c_str(), &parseEnd, 10);
    if (parseEnd != trimmedArgument.c_str() && *parseEnd == '\0'
        && parsedCardIndex >= 0 && parsedCardIndex < PlayingCardDisplay::cardCount)
    {
        cardIndex = static_cast<uint8_t>(parsedCardIndex);
        return true;
    }

    const size_t separatorIndex = trimmedArgument.find(' ');
    if (separatorIndex == std::string::npos)
    {
        return false;
    }

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
        case 'H':
            suitIndex = 0;
            return true;
        case 'D':
            suitIndex = 1;
            return true;
        case 'C':
            suitIndex = 2;
            return true;
        case 'S':
            suitIndex = 3;
            return true;
    }

    return false;
}

bool tryParseCompactPlayingCardCommand(const std::string &commandName, uint8_t &cardIndex)
{
    if (commandName.length() < 3 || commandName.length() > 4 || commandName[0] != 'C')
    {
        return false;
    }

    if (commandName == "CJ1")
    {
        cardIndex = 52;
        return true;
    }
    if (commandName == "CJ2")
    {
        cardIndex = 53;
        return true;
    }

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
    {
        espSymbol = EspSymbol::circle;
        return true;
    }
    if (symbolText == "G" || symbolText == "CROSS" || symbolText == "KREUZ")
    {
        espSymbol = EspSymbol::cross;
        return true;
    }
    if (symbolText == "W" || symbolText == "WAVE" || symbolText == "WAVES" || symbolText == "WELLEN")
    {
        espSymbol = EspSymbol::waves;
        return true;
    }
    if (symbolText == "Q" || symbolText == "SQUARE" || symbolText == "QUADRAT")
    {
        espSymbol = EspSymbol::square;
        return true;
    }
    if (symbolText == "S" || symbolText == "STAR" || symbolText == "STERN")
    {
        espSymbol = EspSymbol::star;
        return true;
    }

    return false;
}

bool tryParseCompactEspSymbolCommand(const std::string &commandName, EspSymbol &espSymbol)
{
    if (commandName.length() != 2 || commandName[0] != 'E')
    {
        return false;
    }

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

void sendHelp()
{
    sendResponse("Commands: TEXT, S*, E*, A*, CHX, CJ1, I1, I0, U1, U0, CL, H");
}

void processCommand(const char *rawCommand)
{
    const std::string command = trimString(rawCommand);
    if (command.empty())
    {
        return;
    }

    writeTextToOutputs("Command: ");
    writeLineToOutputs(command.c_str());

    const size_t separatorIndex = command.find(' ');
    const std::string commandName = getUppercaseAsciiString(
        separatorIndex == std::string::npos ? command : command.substr(0, separatorIndex));
    const std::string argument = separatorIndex == std::string::npos
        ? ""
        : trimString(command.substr(separatorIndex + 1));

    if (separatorIndex == std::string::npos)
    {
        if (commandName == "H")
        {
            sendHelp();
            return;
        }

        if (commandName == "CL")
        {
            clearDisplay();
            sendResponse("OK: Display cleared");
            return;
        }

        if (commandName == "I1")
        {
            displayInverted = true;
            sendResponse("OK: Invert on");
            return;
        }

        if (commandName == "I0")
        {
            displayInverted = false;
            sendResponse("OK: Invert off");
            return;
        }

        if (commandName == "U1")
        {
            displayUpsideDown = true;
            applyDisplayRotation();
            sendResponse("OK: Upside down on");
            return;
        }

        if (commandName == "U0")
        {
            displayUpsideDown = false;
            applyDisplayRotation();
            sendResponse("OK: Upside down off");
            return;
        }

        if (commandName.length() >= 2 && commandName.length() <= 3 && commandName[0] == 'S')
        {
            drawAsciiCharacters(commandName.substr(1).c_str());
            sendResponse("OK: Symbol shown");
            return;
        }

        EspSymbol compactEspSymbol = EspSymbol::circle;
        if (tryParseCompactEspSymbolCommand(commandName, compactEspSymbol))
        {
            drawEspSymbol(compactEspSymbol);
            sendResponse("OK: ESP symbol shown");
            return;
        }

        CompassDirection compactCompassDirection = CompassDirection::N;
        if (tryParseCompactArrowCommand(commandName, compactCompassDirection))
        {
            drawArrow(compactCompassDirection);
            sendResponse("OK: Arrow shown");
            return;
        }

        uint8_t compactCardIndex = 0;
        if (tryParseCompactPlayingCardCommand(commandName, compactCardIndex))
        {
            drawPlayingCard(compactCardIndex);
            sendResponse("OK: Card shown");
            return;
        }
    }

    if (commandName == "TEXT" || commandName == "TXT")
    {
        if (argument.empty())
        {
            sendResponse("ERROR: Text missing");
            return;
        }

        drawPromptText(argument.c_str());
        sendResponse("OK: Text shown");
        return;
    }

    if (commandName == "SYMBOL" || commandName == "SYM")
    {
        if (argument.empty())
        {
            sendResponse("ERROR: Symbol missing");
            return;
        }

        drawAsciiCharacters(argument.c_str());
        sendResponse("OK: Symbol shown");
        return;
    }

    if (commandName == "ESP")
    {
        EspSymbol espSymbol = EspSymbol::circle;
        if (!tryParseEspSymbol(argument, espSymbol))
        {
            sendResponse("ERROR: ESP symbol unknown");
            return;
        }

        drawEspSymbol(espSymbol);
        sendResponse("OK: ESP symbol shown");
        return;
    }

    if (commandName == "ARROW")
    {
        CompassDirection compassDirection = CompassDirection::N;
        if (!tryParseCompassDirection(argument, compassDirection))
        {
            sendResponse("ERROR: Arrow unknown");
            return;
        }

        drawArrow(compassDirection);
        sendResponse("OK: Arrow shown");
        return;
    }

    if (commandName == "CARD")
    {
        uint8_t cardIndex = 0;
        if (!tryParsePlayingCard(argument, cardIndex))
        {
            sendResponse("ERROR: Card unknown");
            return;
        }

        drawPlayingCard(cardIndex);
        sendResponse("OK: Card shown");
        return;
    }

    if (commandName == "INVERT" || commandName == "INV")
    {
        if (isEnabledText(argument))
        {
            displayInverted = true;
            sendResponse("OK: Invert on");
            return;
        }
        if (isDisabledText(argument))
        {
            displayInverted = false;
            sendResponse("OK: Invert off");
            return;
        }

        sendResponse("ERROR: Use ON or OFF");
        return;
    }

    if (commandName == "CLEAR" || commandName == "CLS")
    {
        clearDisplay();
        sendResponse("OK: Display cleared");
        return;
    }

    if (commandName == "HELP" || commandName == "?")
    {
        sendHelp();
        return;
    }

    sendResponse("ERROR: Command unknown");
}

class BluetoothServerCallbacks : public NimBLEServerCallbacks
{
public:
    void onConnect(NimBLEServer *server) override
    {
        bluetoothClientConnected = true;
        idleScreenDrawn = false;
        writeDebugMessage(DebugLevel::info, "BLE connected");
    }

    void onDisconnect(NimBLEServer *server) override
    {
        bluetoothClientConnected = false;
        shouldRestartBluetoothAdvertising = true;
        idleScreenDrawn = false;
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
    NimBLEDevice::init(bluetoothDeviceName);
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
    advertising->setScanResponse(true);
    advertising->start();

    writeDebugMessage(DebugLevel::info, "BLE advertising started");
}

void setup()
{
    Serial.begin(serialBaudRate);
    delay(200);
    waitForUsbSerialConnection();

    receivedCommandQueue = xQueueCreate(6, sizeof(ReceivedCommand));

    Wire.begin(displayDataPin, displayClockPin);
    display.begin();
    display.enableUTF8Print();

    writeStartupHeader();
    drawStartupScreen();
    startupFinishedMillis = millis() + startupScreenDurationMillis;

    startBluetooth();
}

void loop()
{
    if (shouldRestartBluetoothAdvertising)
    {
        NimBLEDevice::getAdvertising()->start();
        shouldRestartBluetoothAdvertising = false;
        writeDebugMessage(DebugLevel::info, "BLE advertising restarted");
    }

    ReceivedCommand receivedCommand = {};
    while (receivedCommandQueue != nullptr && xQueueReceive(receivedCommandQueue, &receivedCommand, 0) == pdTRUE)
    {
        processCommand(receivedCommand.text);
    }

    if (!idleScreenDrawn && millis() >= startupFinishedMillis)
    {
        drawIdleScreen();
    }

    delay(10);
}
