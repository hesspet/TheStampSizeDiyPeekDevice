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
constexpr const char *programVersion = "1.5.3";
constexpr const char *bluetoothDeviceName = "BlePrompter";
constexpr DebugLevel configuredDebugLevel = DebugLevel::info;

constexpr unsigned long serialBaudRate = 115200;
constexpr bool mirrorDebugToUsbSerial = true;
constexpr unsigned long usbSerialStartupWaitMillis = 3000;

constexpr uint8_t displayDataPin = 5;
constexpr uint8_t displayClockPin = 6;

constexpr uint8_t buttonPin = 9;
constexpr uint8_t buttonPressedLevel = LOW;
constexpr unsigned long buttonDebounceDurationMillis = 50;
constexpr unsigned long deepSleepButtonHoldDurationMillis = 5000;
constexpr unsigned long deepSleepCountdownIntervalMillis = 1000;

constexpr unsigned long startupScreenDurationMillis = 2500;
constexpr bool startCycleSleepOnPowerOn = true;
constexpr bool startCycleSleepAfterBluetoothDisconnect = true;
constexpr uint32_t defaultCycleSleepSeconds = 30;
constexpr uint32_t defaultCycleListenSeconds = 10;
constexpr uint32_t minimumCycleSleepSeconds = 5;
constexpr uint32_t maximumCycleSleepSeconds = 60;
constexpr uint32_t minimumCycleListenSeconds = 10;
constexpr uint32_t maximumCycleListenSeconds = 120;
constexpr uint32_t cycleLongListenEveryCycleCount = 5;
constexpr uint32_t cycleLongListenSeconds = 60;
constexpr uint8_t maximumCommandLength = 120;
constexpr uint8_t clearDisplayMarkerSizePixels = 2;
