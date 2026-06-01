#pragma once

#include <Arduino.h>

enum class DebugLevel : uint8_t
{
    none = 0,
    info = 1,
    debug = 2,
    trace = 3
};

constexpr const char *programName = "BoardTest";
constexpr const char *programVersion = "1.3.0";
constexpr DebugLevel configuredDebugLevel = DebugLevel::info;

constexpr unsigned long serialBaudRate = 115200;
constexpr bool mirrorDebugToUsbSerial = true;
constexpr unsigned long usbSerialStartupWaitMillis = 3000;

constexpr uint8_t displayDataPin = 5;
constexpr uint8_t displayClockPin = 6;

constexpr uint8_t buttonPin = 9;
constexpr uint8_t buttonPressedLevel = LOW;
constexpr unsigned long buttonDebounceDurationMillis = 50;

constexpr unsigned long startupScreenDurationMillis = 2500;
constexpr unsigned long displayRefreshIntervalMillis = 100;
constexpr unsigned long testStepDurationMillis = 1000;
