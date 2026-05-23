#pragma once

#include <Arduino.h>

enum class DebugLevel : uint8_t
{
    none = 0,
    info = 1,
    debug = 2,
    trace = 3
};

constexpr const char *programName = "EnduranceTest";
constexpr const char *programVersion = "1.0.0";
constexpr DebugLevel configuredDebugLevel = DebugLevel::info;

constexpr unsigned long serialBaudRate = 115200;
constexpr bool mirrorDebugToUsbSerial = true;
constexpr unsigned long usbSerialStartupWaitMillis = 3000;

constexpr uint8_t displayDataPin = 5;
constexpr uint8_t displayClockPin = 6;

constexpr const char *wifiNetworkName = "Agathas-Netz-16";
constexpr const char *wifiPassword = "1234567890123050363";
constexpr unsigned long wifiConnectionTimeoutMillis = 20000;
constexpr uint16_t enduranceBroadcastPort = 4210;
constexpr const char *enduranceBroadcastAddress = "255.255.255.255";

constexpr unsigned long enduranceCycleDurationMillis = 60000;
constexpr unsigned long displayActionDurationMillis = 1200;
constexpr uint8_t bleScanDurationSeconds = 5;
constexpr uint8_t wifiScanPauseSeconds = 1;
constexpr size_t storedLogRecordCount = 96;
