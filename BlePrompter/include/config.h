#pragma once

#include <Arduino.h>

enum class DebugLevel : uint8_t
{
    none = 0,
    info = 1,
    debug = 2,
    trace = 3
};

constexpr const char *programName = "BlePrompter";
constexpr const char *programVersion = "1.4.0";
constexpr const char *bluetoothDeviceName = "BlePrompter";
constexpr DebugLevel configuredDebugLevel = DebugLevel::info;

constexpr unsigned long serialBaudRate = 115200;
constexpr bool mirrorDebugToUsbSerial = true;
constexpr unsigned long usbSerialStartupWaitMillis = 3000;

constexpr uint8_t displayDataPin = 5;
constexpr uint8_t displayClockPin = 6;

constexpr unsigned long startupScreenDurationMillis = 2500;
constexpr uint8_t maximumCommandLength = 120;
